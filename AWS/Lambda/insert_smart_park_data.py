import json
import boto3

def lambda_handler(event, context):
    try:
        thing = event.get('thing')

        thing_name = thing['thing_name']
        serial_number = thing['serial_number']
        thing_type = thing['thing_type']
        
        thing_purpose = thing['thing_purpose']
        event_type = "Entrance" if thing_purpose == 'entr' else "Exit"

        event_time = thing['event_time']

        max_capacity = int(event['max_capacity'])
        current_car_count = int(event['current_car_count'])
        car_change_value = event['value']

        dynamodb = boto3.resource('dynamodb')
        smart_parking_data_table = dynamodb.Table('smart_parking_data')

        new_car_count = current_car_count + car_change_value

        if new_car_count >= 0 and new_car_count <= max_capacity:
            smart_parking_data_table.put_item(
                Item={
                    'thing_name': thing_name,
                    'serial_number': serial_number,
                    'thing_type': thing_type,
                    'event_type': event_type,
                    'event_time': event_time,
                    'current_car_count': new_car_count
                }
            )

        return {
            'statusCode': 200,
            'body': json.dumps('Success!')
        }
        
    except Exception as e:
        print(f"Exception: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps(f"Error: {str(e)}")
        }

