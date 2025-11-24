import json
import base64
import boto3
import os
import io 
from datetime import datetime
from botocore.exceptions import ClientError
from PIL import Image, ImageStat
from registration import handle_s3_registration


rekognition = boto3.client('rekognition', region_name='ap-southeast-1')
dynamodb = boto3.client('dynamodb', region_name='ap-southeast-1')
ses = boto3.client('ses', region_name='ap-southeast-1')
s3 = boto3.client('s3', region_name='ap-southeast-1')
iot_data = boto3.client('iot-data', region_name='ap-southeast-1')

COLLECTION_ID = 'YOUR_FACE_COLLECTION'  
USER_TABLE = 'YOUR_DYNAMODB_TABLE_NAME'  
SENDER_EMAIL = os.environ.get('SENDER_EMAIL')

# LIVENESS CHECK
def check_glare(image_bytes):
    try:
        img = Image.open(io.BytesIO(image_bytes)).convert('RGB')
        pixels = img.load()
        width, height = img.size
        
        
        bright_pixels = 0      
        low_saturation_pixels = 0 
        high_saturation_pixels = 0 
        max_brightness = 0

        center_x, center_y = width // 2, height // 2
        scan_radius = min(width, height) // 3
        
        for x in range(center_x - scan_radius, center_x + scan_radius, 2):
            for y in range(center_y - scan_radius, center_y + scan_radius, 2):
                r, g, b = pixels[x, y]
                
              
                brightness = max(r, g, b)
                if brightness > max_brightness:
                    max_brightness = brightness

              
                if brightness > 150:
                    bright_pixels += 1
                    
                    
                    color_diff = max(r, g, b) - min(r, g, b)
                    
                    if color_diff < 20: 
                       
                        low_saturation_pixels += 1
                    else:
                       
                        high_saturation_pixels += 1

        print(f"--> Stats: Max={max_brightness}, LowSat={low_saturation_pixels}, HighSat={high_saturation_pixels}")

    
        
        if bright_pixels < 20:
            return False, "Low Light"

        
        if low_saturation_pixels > high_saturation_pixels:
             return True, f"Phat hien man hinh (Mau nhat - Low Saturation: {low_saturation_pixels} > {high_saturation_pixels})"
             
        
        if max_brightness > 240 and low_saturation_pixels > 50:
             return True, "Chay sang trang tinh (Glare)"

        return False, "Real Face"
        
    except Exception as e:
        print(f"Lỗi check glare: {e}")
        return False, "Error"


        
def lambda_handler(event, context):

    if 'Record' in event and 's3' in event['Record'][0]:
        print("S3 Upload")
        return handle_s3_registration(event)


    try:
        image_data_b64 = event.get('image', '')
        device_id = event.get('device_id', 'unknown_cam')

        if not image_data_b64:
            print("Lỗi: Không có ảnh")
            return

        image_bytes = base64.b64decode(image_data_b64)

        
        is_spoof, reason = check_glare(image_bytes)
        
        if is_spoof:
            print(f"--> CẢNH BÁO AN NINH: Tấn công giả mạo! Lý do: {reason}")
            
            # Gửi báo động về ESP32 ngay lập tức
            result_payload = {
                "status": "Fail",
                "name": "FAKE FACE!", 
                "similarity": 0
            }
            iot_data.publish(
                topic='esp32/result',
                qos=0,
                payload=json.dumps(result_payload)
            )
            
            # Gửi email cảnh báo đặc biệt
            if SENDER_EMAIL:
                iso_time = datetime.now().strftime("%H:%M:%S %d-%m-%Y")
                subject = "🚨 CẢNH BÁO: TẤN CÔNG GIẢ MẠO"
                body = f"Camera {device_id} phát hiện nỗ lực giả mạo bằng màn hình/ảnh in vào lúc {iso_time}."
                try:
                    ses.send_email(
                        Source=SENDER_EMAIL,
                        Destination={'ToAddresses': [SENDER_EMAIL]},
                        Message={'Subject': {'Data': subject}, 'Body': {'Text': {'Data': body}}}
                    )
                    print("Đã gửi email cảnh báo giả mạo")
                except Exception as e:
                    print(f"Lỗi gửi mail: {e}")
            
            
            return {'statusCode': 200, 'body': 'Spoof detected'}

        
        found_name = "Unknown"
        similarity = 0.0
        status = "Fail"
        
        try:
            response = rekognition.search_faces_by_image(
                CollectionId=COLLECTION_ID,
                Image={'Bytes': image_bytes},
                MaxFaces=1,
                FaceMatchThreshold=90
            )
            
            
            if 'FaceMatches' in response and len(response['FaceMatches']) > 0:
                print("--> Có khuôn mặt khớp!")
                match = response['FaceMatches'][0]
                similarity = match['Similarity']
                face_id = match['Face']['FaceId']

                
                ddb_res = dynamodb.get_item(
                    TableName=USER_TABLE,
                    Key={'RekognitionId': {'S': face_id}}
                )
                
                if 'Item' in ddb_res:
                    found_name = ddb_res['Item'].get('FullName', {'S': 'Unknown'})['S']
                    status = "Success"
                else:
                    found_name = "Unknown (Unregistered)"
            else:
                print("--> Không khớp ai trong Collection")

        except ClientError as e:
             
            error_code = e.response['Error']['Code']
            if error_code == 'InvalidParameterException':
                print("--> Rekognition: Không tìm thấy khuôn mặt nào trong ảnh (Ảnh mờ/tối/không người).")
                found_name = "No Face Found"
                status = "Fail"
            else:
                raise e

        # Gửi kết quả về lại ESP32
        result_payload = {
            "status": status,
            "name": found_name,
            "similarity": round(similarity, 1)
        }
        
        iot_data.publish(
            topic='esp32/result',
            qos=0,
            payload=json.dumps(result_payload)
        )
        print(f"Đã gửi kết quả về MQTT: {found_name}")

        
        if status == "Fail" and found_name != "No Face Found" and SENDER_EMAIL:
            iso_time = datetime.now().strftime("%H:%M:%S %d-%m-%Y")
            subject = "⚠️ CẢNH BÁO: Phát hiện người lạ"
            body = f"Camera {device_id} phát hiện người lạ ({found_name}) vào lúc {iso_time}."
            
            try:
                ses.send_email(
                    Source=SENDER_EMAIL,
                    Destination={'ToAddresses': [SENDER_EMAIL]},
                    Message={
                        'Subject': {'Data': subject},
                        'Body': {'Text': {'Data': body}}
                    }
                )
                print("Đã gửi email cảnh báo")
            except Exception as e:
                print(f"Lỗi gửi mail: {e}")

        return {'statusCode': 200, 'body': 'Done'}

    except Exception as e:
        print(f"Lỗi Lambda (Fatal): {str(e)}")
        return {'statusCode': 500, 'body': str(e)}