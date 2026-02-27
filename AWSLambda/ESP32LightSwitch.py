import json
import os
import boto3

import re
import base64

INT_RE = re.compile(r'^[-+]?\d+$')

commands=["on","off"]

TOPIC = os.environ.get("AWS_IOT_TOPIC", "esp32/roomLight/commands")

client = boto3.client('iot-data')

def _get_body_text(event):
    body = event.get("body")
    if body is None:
        return None
    if event.get("isBase64Encoded"):
        try:
            body = base64.b64decode(body).decode("utf-8")
        except Exception as e:
            return{
            "statusCode":500,
            "body":json.dumps({"success":False, "message":str(e)})
            }   
    return body.strip()

def lambda_handler(event, context):

    print("EVENT:", json.dumps(event, ensure_ascii=False))

    try:
        if 'command' in event:
            sentCommand = event['command']

            if isinstance(sentCommand, str):
                if sentCommand in commands:
                    commandStr = sentCommand
                elif isinstance(sentCommand, str) and INT_RE.match(sentCommand):
                    commandIndex = int(sentCommand)
                    commandStr = commands[commandIndex % len(commands)]
                else:
                    commandStr = "invalid"
            elif isinstance(sentCommand, int):
                commandStr = commands[sentCommand % len(commands)]
                
        else:
            body_text = _get_body_text(event)
            if body_text:
                try:
                    parsed = json.loads(body_text)
                except Exception as e:
                    return{
                    "statusCode":500,
                    "body":json.dumps({"success":False, "message":str(e)})
                    }

                if isinstance(parsed, dict) and 'command' in parsed:
                    commandTemp = parsed['command']
                    sentCommand = str(commandTemp)
                
                if isinstance(sentCommand, str):
                    if sentCommand in commands:
                        commandStr = sentCommand
                    elif isinstance(sentCommand, str) and INT_RE.match(sentCommand):
                        commandIndex = int(sentCommand)
                        commandStr = commands[commandIndex % len(commands)]
                    else:
                        commandStr = "invalid"
                elif isinstance(sentCommand, int):
                    commandStr = commands[sentCommand % len(commands)]

        payload={
            "command": commandStr
        }

    except Exception as e:
        return{
            "statusCode":500,
            "body":json.dumps({"success":False, "message":str(e)})
        }

    
    resp = client.publish(
        topic=TOPIC,
        qos=1,
        payload=json.dumps(payload)
    )
    return {
        "statusCode": 200,
        "body": json.dumps({"published_to": TOPIC, "payload": payload})
    }

if __name__ == "__main__":
    print(lambda_handler({"command":"test"}, None))