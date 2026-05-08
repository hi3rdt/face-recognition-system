idf_component_register(SRCS "aws_iot_core.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver mqtt json shared_res tft_display) 