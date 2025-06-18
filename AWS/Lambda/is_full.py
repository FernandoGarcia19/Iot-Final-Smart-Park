import json
import boto3

iotClient = boto3.client('iot-data')
dynamodb = boto3.resource('dynamodb')
    
def lambda_handler(event, context):
    try:
        
        serial_number = event.get('serial_number')
        parking_data_table = dynamodb.Table('parking_lot')
        
        response = parking_data_table.get_item(Key={'serial_number': serial_number})
        item = response['Item']

        current_car_count = item['current_car_count']
        max_capacity = item['max_capacity']

        is_full = current_car_count >= max_capacity

        return {"is_full": is_full}
        
    except Exception as e:
        print(f"Exception: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps(f"Error: {str(e)}")
        }