import json
import boto3

iotClient = boto3.client('iot-data')  
    
def lambda_handler(event, context):
    try:
        thing_name = event.get('thing_name')
        topic = f"$aws/things/{thing_name}/shadow/update"
        payload = {
            "state": {
                "desired": {
                    "gate_actuator": "CLOSED"
                }
            }
        }
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