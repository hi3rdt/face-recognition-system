import boto3
import os
import urllib.parse


rekognition = boto3.client('rekognition', region_name='ap-southeast-1')
dynamodb = boto3.client('dynamodb', region_name='ap-southeast-1')
s3 = boto3.client('s3', region_name='ap-southeast-1')


COLLECTION_ID = os.environ.get('COLLECTION_ID', 'YOUR_FACE_COLLECTION') 
USER_TABLE = 'YourDynamoDBTableName'

def handle_s3_registration(event):
    """
    Hàm này xử lý sự kiện khi có ảnh upload lên S3
    """
    try:
        
        record = event['Records'][0]
        bucket = record['s3']['bucket']['name']
        key = urllib.parse.unquote_plus(record['s3']['object']['key'], encoding='utf-8')

        
        if not key.startswith('index/'):
            print(f"Bỏ qua file: {key} (Không nằm trong thư mục index/)")
            return {'statusCode': 200, 'body': 'Ignored (Wrong folder)'}

        
        file_name = os.path.basename(key)
        person_name = os.path.splitext(file_name)[0].replace("_", " ")

        print(f"--> [Auto Register] Đang học khuôn mặt: {person_name}")

       
        response = rekognition.index_faces(
            CollectionId=COLLECTION_ID,
            Image={'S3Object': {'Bucket': bucket, 'Name': key}},
            ExternalImageId=person_name.replace(" ", "_"), 
            MaxFaces=1,
            QualityFilter="AUTO",
            DetectionAttributes=['ALL']
        )

       
        if response['FaceRecords']:
            face_id = response['FaceRecords'][0]['Face']['FaceId']
            print(f"--> Rekognition cấp ID: {face_id}")

            
            dynamodb.put_item(
                TableName=USER_TABLE,
                Item={
                    'RekognitionId': {'S': face_id},
                    'FullName': {'S': person_name},
                    'OriginalImage': {'S': key}
                }
            )
            print(f"--> Đăng ký thành công: {person_name}")
            return {'statusCode': 200, 'body': f"Registered: {person_name}"}
        else:
            print("--> Lỗi: Không tìm thấy khuôn mặt nào.")
            return {'statusCode': 400, 'body': "No face found in image"}

    except Exception as e:
        print(f"Lỗi Registration: {str(e)}")
        return {'statusCode': 500, 'body': str(e)}