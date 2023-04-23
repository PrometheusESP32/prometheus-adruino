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
#include <ArduinoJson.h>
#include <string>

class MyStepper {
private:
  long step;
  long currentPosition;
  long moveTo;
  int mode;
  int speedMotor;
  int in1;
  int in2;
  int in3;
  int in4;

public:
  MyStepper(int m, int a, int b, int c, int d) {
    Serial.printf("Detail : MODE(%d) IN1(%d) IN2(%d) IN3(%d) IN4(%d)\n", mode, in1, in2, in3, in4);
    mode = m;
    in1 = a;
    in2 = b;
    in3 = c;
    in4 = d;
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
  }
  void setStep(long s) {
    step = s;
  }

  long getStep() {
    return step;
  }

  void setCurrentPosition(long s) {
    currentPosition = s;
  }

  long getCurrentPosition() {
    return currentPosition;
  }

  void setMoveTo(long s) {
    moveTo = s;
  }

  long getMoveTo() {
    return moveTo;
  }

  void setSpeedMotor(int s) {
    speedMotor = s;
  }

  int getSpeedMotor() {
    return speedMotor;
  }

  int getDistantPosition() {
    return currentPosition - moveTo;
  }

  void run() {
    const int fullStep[4][4] = {
      { 1, 0, 0, 1 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 1 }
    };
    const int halfStep[8][4] = {
      { 1, 0, 0, 1 }, { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 0, 1 }
    };
    // Serial.printf("Detail : currentPosition(%d) moveTo(%d) IN1(%d) IN2(%d) IN3(%d) IN4(%d)\n", currentPosition, moveTo, in1, in2, in3, in4);
    if (moveTo > currentPosition) {
      if (mode == 1) {
        for (int i = 0; i < 4; i++) {
          if (currentPosition != moveTo) {
            digitalWrite(in1, fullStep[i][0]);
            digitalWrite(in2, fullStep[i][1]);
            digitalWrite(in3, fullStep[i][2]);
            digitalWrite(in4, fullStep[i][3]);
            currentPosition += 1;
            Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 4, fullStep[i][0], fullStep[i][1], fullStep[i][2], fullStep[i][3]);
            vTaskDelay(speedMotor);
          }
        }
      } else if (mode == 2) {
        for (int i = 0; i < 8; i++) {
          if (currentPosition != moveTo) {
            digitalWrite(in1, halfStep[i][0]);
            digitalWrite(in2, halfStep[i][1]);
            digitalWrite(in3, halfStep[i][2]);
            digitalWrite(in4, halfStep[i][3]);
            currentPosition += 1;
            Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 8, halfStep[i][0], halfStep[i][1], halfStep[i][2], halfStep[i][3]);
            vTaskDelay(speedMotor);
          }
        }
      }
    } else if (moveTo < currentPosition) {
      if (mode == 1) {
        for (int i = 4 - 1; i >= 0; i--) {
          if (currentPosition != moveTo) {
            digitalWrite(in1, fullStep[i][0]);
            digitalWrite(in2, fullStep[i][1]);
            digitalWrite(in3, fullStep[i][2]);
            digitalWrite(in4, fullStep[i][3]);
            currentPosition -= 1;
            Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 4, fullStep[i][0], fullStep[i][1], fullStep[i][2], fullStep[i][3]);
            vTaskDelay(speedMotor);
          }
        }
      } else if (mode == 2) {
        for (int i = 8 - 1; i >= 0; i--) {
          if (currentPosition != moveTo) {
            digitalWrite(in1, halfStep[i][0]);
            digitalWrite(in2, halfStep[i][1]);
            digitalWrite(in3, halfStep[i][2]);
            digitalWrite(in4, halfStep[i][3]);
            currentPosition -= 1;
            Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 8, halfStep[i][0], halfStep[i][1], halfStep[i][2], halfStep[i][3]);
            vTaskDelay(speedMotor);
          }
        }
      }
    }
  }
};

#define MOTOR1_IN1 23
#define MOTOR1_IN2 22
#define MOTOR1_IN3 21
#define MOTOR1_IN4 19

#define MOTOR2_IN1 32
#define MOTOR2_IN2 33
#define MOTOR2_IN3 25
#define MOTOR2_IN4 26

// #define MOTOR3_IN1 16
// #define MOTOR3_IN1 17
// #define MOTOR3_IN1 32
// #define MOTOR3_IN1 33

MyStepper stepper1(1, MOTOR1_IN1, MOTOR1_IN2, MOTOR1_IN3, MOTOR1_IN4);
MyStepper stepper2(1, MOTOR2_IN1, MOTOR2_IN2, MOTOR2_IN3, MOTOR2_IN4);

const char *ssid = "Naras";
const char *password = "-Naras-CPE290821-";

AsyncWebServer server(80);

const int LED = 2;

void setup(void) {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/test.svg", HTTP_GET, drawGraph);
  server.on("/status", HTTP_POST, handleStatus);
  server.on(
    "/handle/motor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleMotor);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  stepper1.setStep(500);
  stepper1.setCurrentPosition(0);
  stepper1.setSpeedMotor(10);
  stepper2.setStep(500);
  stepper2.setCurrentPosition(0);
  stepper2.setSpeedMotor(10);
  xTaskCreatePinnedToCore(motor1, "motor1", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(motor2, "motor2", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
}

void motor1(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    stepper1.run();
  }
}

void motor2(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    stepper2.run();
  }
}

void loop(void) {
}

void handleMotor(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  if (!root["positions"].isNull()) {
    JsonArray positionsRaw = root["positions"];
    for (JsonObject repo : positionsRaw) {
      if (repo["index"].as<int>() == 0) {
        stepper1.setMoveTo(repo["value"].as<long>());
      } else if (repo["index"].as<int>() == 1) {
        stepper2.setMoveTo(repo["value"].as<long>());
      }
      Serial.printf("Index : %d Value : %d\n", repo["index"].as<int>(), repo["value"].as<long>());
    }
  }
  request->send(200, "application/json", "ok");
}

void handleRoot(AsyncWebServerRequest *request) {
  digitalWrite(LED, 1);
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
  digitalWrite(LED, 0);
}

void handleNotFound(AsyncWebServerRequest *request) {
  digitalWrite(LED, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, "text/plain", message);
  digitalWrite(LED, 0);
}

void drawGraph(AsyncWebServerRequest *request) {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  request->send(200, "image/svg+xml", out);
}

void handleStatus(AsyncWebServerRequest *request) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED, 1);
    delay(500);
    digitalWrite(LED, 0);
    delay(500);
  }
  request->send(200, "text/plain", "OK");
}

DynamicJsonDocument mapJsonObject(uint8_t *data) {
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, String((char *)data));
  return doc;
}
