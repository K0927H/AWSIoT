import array
from ast import Interactive
import asyncio
from email.utils import parsedate
from shutil import ExecError
import sys
from meross_iot.manager import MerossManager
from meross_iot.http_api import MerossHttpClient
import json
import base64
import re

INT_RE = re.compile(r'^[-+]?\d+$')

MEROSS_EMAIL = "merossEmal"
MEROSS_PASSWORD = "merossPassword"
E26_UUID = "UUID of your E26 device"

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

async def control_e26(BrightNess=10):

    http_api_client = await MerossHttpClient.async_from_user_password(api_base_url='https://iotx-eu.meross.com',
                                                                        email=MEROSS_EMAIL, 
                                                                        password=MEROSS_PASSWORD)

    manager = MerossManager(http_client=http_api_client)

    await manager.async_init()
    await manager.async_device_discovery()

    try:
        devices = manager.find_devices()
        for dev in devices:
            if dev.uuid==E26_UUID:
                await dev.async_update()
                await dev.async_set_light_color(channel=0,luminance=BrightNess)
    except:
        manager.close()
        return{
            "statusCode":500,
            "body":json.dumps({"success":False, "message":"Device Not Found"})
        }
    finally:
        manager.close()


async def control_Color_e26(Color=(100,100,100)):

    http_api_client = await MerossHttpClient.async_from_user_password(api_base_url='https://iotx-eu.meross.com',
                                                                        email=MEROSS_EMAIL, 
                                                                        password=MEROSS_PASSWORD)

    manager = MerossManager(http_client=http_api_client)

    await manager.async_init()
    await manager.async_device_discovery()

    try:
        devices = manager.find_devices()
        for dev in devices:
            if dev.uuid==E26_UUID:
                await dev.async_update()
                await dev.async_set_light_color(channel=0,rgb=Color)
    except Exception as e:
            manager.close()
            return{
            "statusCode":500,
            "body":json.dumps({"success":False, "message":str(e)})
            } 

    finally:
        manager.close()

    print("ColorSetDone")


async def control_ColorTemp_e26(temp=50):

    http_api_client = await MerossHttpClient.async_from_user_password(api_base_url='https://iotx-eu.meross.com',
                                                                        email=MEROSS_EMAIL, 
                                                                        password=MEROSS_PASSWORD)

    manager = MerossManager(http_client=http_api_client)

    await manager.async_init()
    await manager.async_device_discovery()

    try:
        devices = manager.find_devices()
        for dev in devices:
            if dev.uuid==E26_UUID:
                await dev.async_update()
                await dev.async_set_light_color(channel=0,temperature=temp)
    except Exception as e:
            manager.close()
            return{
            "statusCode":500,
            "body":json.dumps({"success":False, "message":str(e)})
            } 

    finally:
        manager.close()

    print("ColorSetDone")

def lambda_handler(event, context):

    try:

        if 'mode' in event:
            mode=event['mode']
            mode=int(mode)
            if mode == 0:
                value=event['value']
                asyncio.run(control_e26(value))
                return{
                "statusCode":200,
                "body":json.dumps({"success":True,"brightness":value})
                }
            elif mode== 1:
                value=event['value']

                if isinstance(value,list):
                    value=tuple(value)

                asyncio.run(control_Color_e26(value))
                return{
                "statusCode":200,
                "body":json.dumps({"success":True,"ColorValue":value})
                }
            elif mode ==2:
                
                value = event['value']
                
                asyncio.run(control_ColorTemp_e26(value))
                return{
                "statusCode":200,
                "body":json.dumps({"success":True, "temp":value})
                }

        else:
            body_text=_get_body_text(event)
            if body_text:
                try:
                    parsed=json.loads(body_text)
                except Exception as e:
                    return{
                    "statusCode":500,
                    "body":json.dumps({"success":False, "message":str(e)})
                    }

                if isinstance(parsed,dict) and 'mode' in parsed:
                    modetemp=parsed['mode']
                    if isinstance(modetemp,int):
                        mode=modetemp
                    if isinstance(modetemp,str) and INT_RE.match(mode):
                        mode=modetemp
                    
                    if mode==0:
                        valtemp=parsed['value']
                        if isinstance(valtemp,int):
                            value=valtemp
                        if isinstance(valtemp,str) and INT_RE.match(valtemp):
                            value=int(valtemp)
                        asyncio.run(control_e26(value))
                        return{
                        "statusCode":200,
                        "body":json.dumps({"success":True,"brightness":value})
                        }
                    elif mode == 1:
                        valtemp=parsed['value']
                        if isinstance(valtemp,tuple) or isinstance(valtemp,list):
                            value=tuple(valtemp)      
                        asyncio.run(control_Color_e26(value))
                        return{
                        "statusCode":200,
                        "body":json.dumps({"success":True,"ColorValue":value})
                        }
                
                    elif mode == 2:
                        valtemp=parsed['value']
                        if isinstance(valtemp, int):
                            value=valtemp
                        if isinstance(valtemp, str) and INT_RE.match(valtemp):
                            value=int(valtemp)
                        asyncio.run(control_ColorTemp_e26(value))
                        return{
                        "statusCode":200,
                        "body":json.dumps({"success":True, "temp":value})
                        }
        
    except Exception as e:
        return{
            "statusCode":500,
            "body":json.dumps({"success":False, "message":str(e)})
        }
    

if __name__=='__main__':
    loop = asyncio.get_event_loop()
    loop.run_until_complete(control_e26(0))
    loop.stop()