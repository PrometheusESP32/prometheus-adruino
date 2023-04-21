/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <ESPmDNS.h>
#include <Stepper.h>
#include <ArduinoJson.h>
#include <string>
#include <thread>
#include <protothreads.h>

pt ptMotor1;
pt ptMotor2;

const char *ssid = "Naras";
const char *password = "-Naras-CPE290821-";

AsyncWebServer server(80);

int motor1_in1 = 18;
int motor1_in2 = 19;
int motor1_in3 = 12;
int motor1_in4 = 13;

int motor2_in1 = 22;
int motor2_in2 = 23;
int motor2_in3 = 25;
int motor2_in4 = 26;

int motor3_in1 = 16;
int motor3_in2 = 17;
int motor3_in3 = 32;
int motor3_in4 = 33;

bool newRequest1 = false;
bool newRequest2 = false;
bool newRequest3 = false;

String direction1 = "";
String direction2 = "";
String direction3 = "";

int speed1 = 0;
int speed2 = 0;
int speed3 = 0;

int steps1 = 0;
int steps2 = 0;
int steps3 = 0;

const int led = 2;

const int number_of_steps = 2048;

Stepper myStepper1(number_of_steps, motor1_in1, motor1_in2, motor1_in3, motor1_in4);
Stepper myStepper2(number_of_steps, motor2_in1, motor2_in2, motor2_in3, motor2_in4);
Stepper myStepper3(number_of_steps, motor3_in1, motor3_in2, motor3_in3, motor3_in4);

void handleRoot(AsyncWebServerRequest *request)
{
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60);
  request->send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound(AsyncWebServerRequest *request)
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void)
{
  PT_INIT(&ptMotor1);
  PT_INIT(&ptMotor2);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/test.svg", HTTP_GET, drawGraph);
  server.on("/status", HTTP_POST, handleStatus);
  server.on(
      "/handle/motor1", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleMotor1);
  server.on(
      "/handle/motor2", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleMotor2);
  server.on(
      "/handle/motor3", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleMotor3);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
  // std::thread first(motor1, &newRequest1, direction1, myStepper1, steps1, speed1);
  // std::thread second(motor2, &newRequest2, direction2, myStepper2, steps2, speed2);

  // first.join();
  // second.join();

  // if (newRequest3) {
  //   if (direction3.equals("CW")) {
  //     myStepper3.setSpeed(speed3);
  //     // Spin the stepper clockwise direction
  //     myStepper3.step(steps3);
  //   } else {
  //     myStepper3.setSpeed(speed3);
  //     // Spin the stepper counterclockwise direction
  //     myStepper3.step(-steps3);
  //   }
  //   newRequest3 = false;
  // }

  PT_SCHEDULE(ptMotor1Thread(&ptMotor1));
  PT_SCHEDULE(ptMotor2Thread(&ptMotor2));
}

int ptMotor1Thread(struct pt *pt)
{
  PT_BEGIN(pt);

  while (true)
  {
    motor1(&newRequest1, direction1, myStepper1, steps1, speed1);
  }

  PT_END(pt);
}

int ptMotor2Thread(struct pt *pt)
{
  PT_BEGIN(pt);

  while (true)
  {
    motor2(&newRequest2, direction2, myStepper2, steps2, speed2);
  }

  PT_END(pt);
}

void motor1(bool *newRequest, String direction, Stepper myStepper, int steps, int speed)
{
  while (true)
  {
    if (*newRequest)
    {
      if (direction.equals("CW"))
      {
        myStepper.setSpeed(speed);
        // Spin the stepper clockwise direction
        myStepper.step(steps);
      }
      else
      {
        myStepper.setSpeed(speed);
        // Spin the stepper counterclockwise direction
        myStepper.step(-steps);
      }
      *newRequest = false;
    }
  }
}

void motor2(bool *newRequest, String direction, Stepper myStepper, int steps, int speed)
{
  while (true)
  {
    if (*newRequest)
    {
      if (direction.equals("CW"))
      {
        myStepper.setSpeed(speed);
        // Spin the stepper clockwise direction
        myStepper.step(steps);
      }
      else
      {
        myStepper.setSpeed(speed);
        // Spin the stepper counterclockwise direction
        myStepper.step(-steps);
      }
      *newRequest = false;
    }
  }
}

void drawGraph(AsyncWebServerRequest *request)
{
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10)
  {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  request->send(200, "image/svg+xml", out);
}

void handleStatus(AsyncWebServerRequest *request)
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    delay(500);
  }
  request->send(200, "text/plain", "OK");
}

JsonObject mapJsonObject(uint8_t *data)
{
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, String((char *)data));
  return doc.as<JsonObject>();
}

void handleMotor1(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  JsonObject root = mapJsonObject(data);
  const int speed = root["speed"];
  const int steps = root["spr"];
  const String direction = root["direction"];
  speed1 = speed;
  steps1 = steps;
  direction1 = direction;
  request->send(200, "application/json", "ok");
  Serial.printf("Motor 1 : speed : %d\n", speed);
  Serial.printf("Motor 1 : stepsPerRevolution : %d\n", steps);
  Serial.printf("Motor 1 : direction : %s\n", direction);
  newRequest1 = true;
}

void handleMotor2(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  JsonObject root = mapJsonObject(data);
  const int speed = root["speed"];
  const int steps = root["spr"];
  const String direction = root["direction"];
  speed2 = speed;
  steps2 = steps;
  direction2 = direction;
  request->send(200, "application/json", "ok");
  Serial.printf("Motor 2 : speed : %d\n", speed);
  Serial.printf("Motor 2 : stepsPerRevolution : %d\n", steps);
  Serial.printf("Motor 2 : direction : %s\n", direction);
  newRequest2 = true;
}

void handleMotor3(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  JsonObject root = mapJsonObject(data);
  const int speed = root["speed"];
  const int steps = root["spr"];
  const String direction = root["direction"];
  speed3 = speed;
  steps3 = steps;
  direction3 = direction;
  request->send(200, "application/json", "ok");
  Serial.printf("Motor 3 : speed : %d\n", speed);
  Serial.printf("Motor 3 : stepsPerRevolution : %d\n", steps);
  Serial.printf("Motor 3 : direction : %s\n", direction);
  newRequest3 = true;
}
