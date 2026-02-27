#include <WiFi.h>
#include <WebServer.h>

#include <IRsend.h>

#include <Arduino.h>
#include <Servo.h>


uint16_t openData[99] = {3488, 1720,  454, 412,  454, 1278,  454, 412,  454, 412,  452, 412,  454, 412,  454, 412,  454, 412,  456, 412,  452, 412,  454, 412,  454, 412,  454, 412,  454, 1278,  454, 414,  452, 412,  454, 412,  454, 412,  454, 412,  454, 412,  454, 412,  454, 410,  454, 412,  454, 1278,  456, 410,  456, 412,  454, 412,  454, 412,  454, 412,  454, 412,  454, 412,  454, 412,  454, 412,  454, 1276,  454, 410,  456, 1278,  456, 412,  454, 412,  452, 1278,  454, 412,  454, 412,  452, 1280,  454, 412,  454, 1278,  454, 412,  454, 412,  454, 1278,  454, 1278,  454};  // PANASONIC 400401005253
uint16_t closeData[99] = {3508, 1700,  476, 390,  474, 1258,  474, 392,  474, 392,  472, 394,  474, 392,  472, 392,  474, 392,  474, 392,  474, 392,  476, 390,  476, 390,  474, 390,  474, 1258,  474, 390,  474, 392,  474, 390,  474, 392,  474, 392,  474, 392,  474, 392,  474, 392,  472, 394,  474, 1258,  476, 390,  474, 392,  472, 392,  474, 392,  474, 392,  476, 392,  474, 390,  474, 392,  474, 1258,  474, 1258,  476, 390,  474, 1258,  476, 390,  474, 392,  472, 1258,  472, 394,  472, 1258,  474, 1258,  472, 394,  472, 1260,  472, 394,  474, 392,  474, 1258,  474, 1258,  474};  // PANASONIC 40040100D2D3

uint16_t botforward[17] = {7893,2595,5260,2600,5265,2605,2635,2605,5265,2605,2635,2605,2635,2605,2635,2605,5245};
uint16_t botbackward[17] = {7893, 2596,  2634, 2604,  2628, 2606,  5260, 2606,  5264, 2606,  2636, 2604,  2634, 2604,  2634, 2604,  2638};
uint16_t botleft[17] = {7856, 2628,  5236, 2626,  2604, 2630,  5236, 2630,  5236, 2632,  2606, 2632,  2606, 2632,  2608, 2632,  5240};
uint16_t botright[17] = {7852, 2628,  2602, 2630,  5230, 2632,  5232, 2632,  5188, 2678,  2606, 2632,  2604, 2632,  2606, 2634,  5238};
uint16_t botstop[17] = {7902, 2598,  5240, 2628,  5266, 2604,  5268, 2602,  5244, 2630,  2638, 2602,  2638, 2604,  2636, 2604,  2638};


void curtain_open();
void curtain_close();

void Bot_Forward();
void Bot_Backward();
void Bot_Left();
void Bot_Right();
void Bot_Stop();

void light_sw1();
void light_sw2();
void dummy_status();

const char* ssid = "WiFiSSID";
const char* pass = "WiFiPassword";

const int ledPin = 2;
IRsend irsend(19);

Servo servo1;
Servo servo2;

WebServer server(80);



void setup() {

  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  servo1.attach(16);
  servo2.attach(4);

  irsend.begin();

  IPAddress local_IP(192, 168, 11, 50);
  IPAddress gateway(192, 168, 11, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.disconnect();
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  printf("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/curtain_open", curtain_open);
  server.on("/curtain_close", curtain_close);
  server.on("/light_sw1", light_sw1);
  server.on("/light_sw2", light_sw2);
  server.on("/bot_forward", Bot_Forward);
  server.on("/bot_backward", Bot_Backward);
  server.on("/bot_left", Bot_Left);
  server.on("/bot_right", Bot_Right);
  server.on("/bot_stop", Bot_Stop);

  server.begin();
}

void loop() { server.handleClient(); }

void curtain_open() {
  digitalWrite(ledPin, LOW);
  for (int i = 0; i <= 5; i++) {
    irsend.sendRaw(openData, 99, 38);
    delay(50);
  };
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "CURTAIN OPENED");
}

void curtain_close() {
  digitalWrite(ledPin, LOW);
  for (int i = 0; i <= 5; i++) {
    irsend.sendRaw(closeData, 99, 38);
    delay(50);
  }
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "CURTAIN CLOSED");
}



void Bot_Forward()
{
  digitalWrite(ledPin, LOW);
  irsend.sendRaw(botforward, 17, 38);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "movingForward");
}

void Bot_Backward()
{
  digitalWrite(ledPin, LOW);
  irsend.sendRaw(botbackward, 17, 38);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "movingBackward");
}

void Bot_Left()
{
  digitalWrite(ledPin, LOW);
  irsend.sendRaw(botleft, 17, 38);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "turningLeft");
}

void Bot_Right()
{
  digitalWrite(ledPin, LOW);
  irsend.sendRaw(botright, 17, 38);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "turningRight");
}

void Bot_Stop()
{
  digitalWrite(ledPin, LOW);
  irsend.sendRaw(botstop, 17, 38);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "stopped");
}


void light_sw1() {
  digitalWrite(ledPin, LOW);
  servo1.write(0);
  delay(500);
  servo1.write(90);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "LIGHT SW1 TOGGLED");
}

void light_sw2() {
  digitalWrite(ledPin, LOW);
  servo2.write(0);
  delay(500);
  servo2.write(90);
  digitalWrite(ledPin, HIGH);

  server.send(200, "text/plain", "LIGHT SW2 TOGGLED");
}