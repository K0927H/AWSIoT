#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <Stepper.h>

#pragma region head


#pragma region WIFI Configuration

const char* WIFI_SSID = "WiFiSSID";
const char* WIFI_PASS = "WiFiPassword";

#pragma endregion


#pragma region AWS IoT Configuration

const char* AWS_IOT_ENDPOINT = "--------.amazonaws.com";

const char* MQTT_TOPIC = "esp32/Curtain/commands";

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


void curtain_open();
void curtain_close();

void connectAWS();

bool contains(const String& target,const String arr[], size_t arrSize);

#pragma endregion


#pragma region Global Variables

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

const int stepsPerRevolution = 9216;

const int motorPin1 = 32;
const int motorPin2 = 33;
const int motorPin3 = 25;
const int motorPin4 = 26;

Stepper myStepper(stepsPerRevolution,motorPin1,motorPin3,motorPin2,motorPin4);

String commands[] = {"open","close"};

#pragma endregion




#pragma endregion


#pragma region mainfunctions

void callback(char* topic, byte* payload, unsigned int length) {

  myStepper.setSpeed(3);

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
        if(command == "open"){
          curtain_open();
        } else if(command == "close"){
          curtain_close();
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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, attempting reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
      delay(500);
      Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi reconnect failed — will retry when loop runs");
      return;
    }
  }

  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String clientId = "esp32-client-" + mac;

  unsigned long backoff = 1000;
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect(clientId.c_str())) {
      Serial.println(" connected");
      client.subscribe(MQTT_TOPIC);
      Serial.print("Subscribed to ");
      Serial.println(MQTT_TOPIC);
      break;
    } else {
      int st = client.state();
      Serial.print(" failed, rc=");
      Serial.print(st);
      Serial.print(" — retry in ");
      Serial.print(backoff / 1000);
      Serial.println("s");
      delay(backoff);
      if (backoff < 16000) backoff *= 2;
    }
  }
}


void setup()
{


  Serial.begin(115200);

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
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
      delay(500);
      Serial.print('.');
    }
    Serial.println();
  }

  if (!client.connected()) {
    connectAWS();
  }

  client.loop();
}


#pragma endregion


#pragma region Hardware functions

void curtain_open() {
  myStepper.step(stepsPerRevolution);
  delay(1000);
  Serial.println("Curtain opened");
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
}

void curtain_close() {
  myStepper.step(-stepsPerRevolution);
  delay(1000);
  Serial.println("Curtain closed");
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
}

#pragma endregion


