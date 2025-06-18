import json
import boto3
dynamodb = boto3.resource('dynamodb')

def lambda_handler(event, context):
    # TODO implement
    try:
        thing = event['thing']
        serial_number = thing['serial_number']

        parking_data_table = dynamodb.Table('parking_lot')
        response = parking_data_table.get_item(Key={'serial_number': serial_number})
        parking_lot = response['Item']

        current_car_count = int(parking_lot['current_car_count'])
        max_capacity = int(parking_lot['max_capacity'])
        car_change_value = event['value']

        new_car_count = current_car_count + car_change_value
        
        if new_car_count >= 0 and new_car_count <= max_capacity:
            parking_data_table.update_item(
                Key={
                    'serial_number': serial_number
                },
                UpdateExpression='SET current_car_count = :new_car_count',
                ExpressionAttributeValues={
                    ':new_car_count': new_car_count
                }
            )

        return {
            'statusCode': 200,
            'body': json.dumps('Hello from Lambda!')
        }
        
    except Exception as e:
        print(f"Exception: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps(f"Error: {str(e)}")
        }
