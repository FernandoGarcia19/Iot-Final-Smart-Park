import json
import boto3

iotClient = boto3.client('iot-data')
lambdaClient = boto3.client('lambda')    
    
def lambda_handler(event, context):
    try:
        
        thing_purpose = event.get('thing_purpose')
        thing_name = event.get('thing_name')
        serial_number = event.get('serial_number')
        topic = f"$aws/things/{thing_name}/shadow/update"

        payload = {
            "state": {
                "desired": {
                    "gate_actuator": "OPEN"
                }
            }
        }

        print(f"thing_purpose: {thing_purpose}")
        if thing_purpose == "entr":
            dynamodb = boto3.resource('dynamodb')
            parking_data_table = dynamodb.Table('parking_lot')
            response = parking_data_table.get_item(Key={'serial_number': serial_number})
            item = response['Item']
            current_car_count = item['current_car_count']
            max_capacity = item['max_capacity']
            isFull = current_car_count >= max_capacity
            if not isFull:
                iotClient.publish(topic=topic, qos=1, payload=json.dumps(payload))     
        else:
            iotClient.publish(topic=topic, qos=1, payload=json.dumps(payload))
        
        return {
            'statusCode': 200,
        }
        
    except Exception as e:
        print(f"Exception: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps(f"Error: {str(e)}")
        }