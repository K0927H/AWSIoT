#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <IRsend.h>
#include <Servo.h>

#pragma region head


#pragma region WIFI Configuration

const char* WIFI_SSID = "WiFiSSID";
const char* WIFI_PASS = "WiFiPassword";

#pragma endregion


#pragma region AWS IoT Configuration

const char* AWS_IOT_ENDPOINT = "--------.amazonaws.com";

const char* MQTT_TOPIC = "esp32/Switch/commands";

const char* AWS_CERT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
COMMENT: Amazon Root CA 1
-----END CERTIFICATE-----
)EOF";

const char* AWS_CERT_CRT = R"EOF(
-----BEGIN CERTIFICATE-----
COMMENT: Your device certificate here
-----END CERTIFICATE-----
)EOF";

const char* AWS_CERT_PRIVATE = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
COMMENT: Your private key here
-----END RSA PRIVATE KEY-----
)EOF";

#pragma endregion


#pragma region Function Declarations

void servoF1();
void servoF2();

void Bot_Forward();
void Bot_Backward();
void Bot_Left();
void Bot_Right();
void Bot_Stop();

void connectAWS();

bool contains(const String& target,const String arr[], size_t arrSize);

#pragma endregion


#pragma region Global Variables

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

IRsend irsend(19);

Servo servo1;
Servo servo2;

String commands[]={ "servo1","servo2","forward","backward","left","right","stop"};

#pragma endregion


#pragma region IR Remote Signals




uint16_t botforward[17] = {7893,2595,5260,2600,5265,2605,2635,2605,5265,2605,2635,2605,2635,2605,2635,2605,5245};
uint16_t botbackward[17] = {7893, 2596,  2634, 2604,  2628, 2606,  5260, 2606,  5264, 2606,  2636, 2604,  2634, 2604,  2634, 2604,  2638};
uint16_t botright[17] = {7856, 2628,  5236, 2626,  2604, 2630,  5236, 2630,  5236, 2632,  2606, 2632,  2606, 2632,  2608, 2632,  5240};
uint16_t botleft[17] = {7852, 2628,  2602, 2630,  5230, 2632,  5232, 2632,  5188, 2678,  2606, 2632,  2604, 2632,  2606, 2634,  5238};
uint16_t botstop[17] = {7902, 2598,  5240, 2628,  5266, 2604,  5268, 2602,  5244, 2630,  2638, 2602,  2638, 2604,  2636, 2604,  2638};

#pragma endregion


#pragma endregion


#pragma region mainfunctions

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i=0; i < length; i++) {
    msg += (char)payload[i];
  } 

  Serial.print("Received on [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  StaticJsonDocument<200> doc;
  DeserializationError err = deserializeJson(doc, msg);

  if(!err){
    String command = doc["command"];

    if(command!="invalid"){
      if(contains(command,commands,sizeof(commands)/sizeof(commands[0]))){
        if(command=="servo1"){
          servoF1();
        }
        else if(command=="servo2"){
          servoF2();
        }
        else if(command=="forward"){
          Bot_Forward();
        }
        else if(command=="backward"){
          Bot_Backward();
        }
        else if(command=="left"){
          Bot_Left();
        }
        else if(command=="right"){
          Bot_Right();
        }
        else if(command=="stop"){
          Bot_Stop();
        }
      }
    }
  }
}


bool contains(const String& target,const String arr[], size_t arrSize) {
  for (size_t i = 0; i < arrSize; i++) {
    if (arr[i] == target) {
      return true;
    }
  }
  return false;
}


void connectAWS() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("esp32-client")) { 
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC);
      Serial.print("Subscribed to ");
      Serial.println(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2s");
      delay(2000);
    }
  }
}


void setup()
{


  Serial.begin(115200);

  servo1.attach(16);
  servo2.attach(4);

  irsend.begin();

  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(callback);

  connectAWS();
}


void loop()
{
  if(!client.connected()) {
    connectAWS();
  }
  client.loop();
}


#pragma endregion


#pragma region Hardware functions


void Bot_Forward()
{
  irsend.sendRaw(botforward, 17, 38);
  Serial.println("Bot Forward");
}

void Bot_Backward()
{
  irsend.sendRaw(botbackward, 17, 38);
  Serial.println("Bot Backward");
}

void Bot_Left()
{
  irsend.sendRaw(botleft, 17, 38);
  Serial.println("Bot Left");
}

void Bot_Right()
{
  irsend.sendRaw(botright, 17, 38);
  Serial.println("Bot Right");
}

void Bot_Stop()
{
  irsend.sendRaw(botstop, 17, 38);
  Serial.println("Bot Stop");
}



void servoF1()
{
  Serial.println("Servo 1 Activated");
  servo1.write(0);
  delay(500);
  servo1.write(90);
}


void servoF2()
{
  Serial.println("Servo 2 Activated");
  servo2.write(0);
  delay(500);
  servo2.write(90);
} 


#pragma endregion


