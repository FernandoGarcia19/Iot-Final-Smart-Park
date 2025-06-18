import json
import boto3

iotClient = boto3.client('iot-data')
lambdaClient = boto3.client('lambda')    
    
def lambda_handler(event, context):
    try:
        thing_name = event.get('thing_name')
        thing_type = event.get('thing_type')
        thing_purpose = event.get('thing_purpose')
        serial_number = event.get('serial_number')
        event_time = event.get('event_time')


        topic = f"$aws/things/{thing_name}/shadow/update"

        payload = {
            "state": {
                "desired": {
                    "gate_actuator": "CLOSED"
                }
            }
        }

        event_type = "Entrance" if thing_purpose == "entr" else "Exit"

        dynamodb = boto3.resource('dynamodb')
        parking_data_table = dynamodb.Table('parking_lot')
        response = parking_data_table.get_item(Key={'serial_number': serial_number})
        item = response['Item']
        current_car_count = item['current_car_count']
        isEmpty = current_car_count <= 0
        if thing_purpose == "entr":
            iotClient.publish(topic=topic, qos=1, payload=json.dumps(payload))
            new_car_count = current_car_count + 1
            parking_data_table.update_item(
                Key={
                    'serial_number': serial_number
                },
                UpdateExpression='SET current_car_count = :car_count',
                ExpressionAttributeValues={
                    ':car_count': new_car_count
                }
            )
            smart_parking_data_table = boto3.resource('dynamodb').Table('smart_parking_data')
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
        else:
            iotClient.publish(topic=topic, qos=1, payload=json.dumps(payload))
            if not isEmpty:
                new_car_count = current_car_count - 1
                parking_data_table.update_item(
                    Key={
                        'serial_number': serial_number
                    },
                    UpdateExpression='SET current_car_count = :car_count',
                    ExpressionAttributeValues={
                        ':car_count': new_car_count
                    }
                )
                smart_parking_data_table = boto3.resource('dynamodb').Table('smart_parking_data')
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
        }
        
    except Exception as e:
        print(f"Exception: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps(f"Error: {str(e)}")
        }