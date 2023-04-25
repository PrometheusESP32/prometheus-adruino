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

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t Task4;

void motor(void *pvParameters) {
  char *data = ((char *)pvParameters);
  Serial.println(data);

  // int num[2];
  // int index = 0;
  // while (json != NULL) {
  //   num[index] = stoi(json);
  //   json = strtok(NULL, ",");
  //   index += 1;
  // }
  // const int speedMotor = num[0];
  // const int moveTo = num[1];
  // Serial.printf("moveTo(%d) speedMotor(%d)\n", moveTo, speedMotor);
  // for (int i = 0; i <= moveTo; i++) {
  //   Serial.printf("Print from thread(%d) index(%d) in value %d....\n", xPortGetCoreID(), i, moveTo);
  // }
  vTaskDelete(NULL);
}

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
  char *name;

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

  void setName(char *s) {
    name = s;
  }

  char *getName() {
    return name;
  }

  void run(int num) {
    if (moveTo != currentPosition) {
      // Serial.printf("Detail : currentPosition(%d) moveTo(%d) IN1(%d) IN2(%d) IN3(%d) IN4(%d)\n", currentPosition, moveTo, in1, in2, in3, in4);
      char value[100];
      snprintf(value, 100, "%d,%d", moveTo, speedMotor);
      try {
        switch (num) {
          case 1:
            xTaskCreatePinnedToCore(motor, getName(), configMINIMAL_STACK_SIZE, (void *)value, 1, &Task1, ARDUINO_RUNNING_CORE);
            break;
          case 2:
            xTaskCreatePinnedToCore(motor, getName(), configMINIMAL_STACK_SIZE, (void *)value, 1, &Task2, ARDUINO_RUNNING_CORE);
            break;
          case 3:
            xTaskCreatePinnedToCore(motor, getName(), configMINIMAL_STACK_SIZE, (void *)value, 1, &Task3, ARDUINO_RUNNING_CORE);
            break;
          case 4:
            xTaskCreatePinnedToCore(motor, getName(), configMINIMAL_STACK_SIZE, (void *)value, 1, &Task4, ARDUINO_RUNNING_CORE);
            break;
        }
      } catch (...) {
        Serial.printf("Create task error.");
      }
    }
    // for (int index = 0; index <= indexMoveto; index++) {
    //   if (moveTo > currentPosition) {
    //     if (mode == 1) {
    //       for (int i = 0; i < 4; i++) {
    //         if (currentPosition != moveTo) {
    //           digitalWrite(in1, fullStep[i][0]);
    //           digitalWrite(in2, fullStep[i][1]);
    //           digitalWrite(in3, fullStep[i][2]);
    //           digitalWrite(in4, fullStep[i][3]);
    //           currentPosition += 1;
    //           Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 4, fullStep[i][0], fullStep[i][1], fullStep[i][2], fullStep[i][3]);
    //           vTaskDelay(speedMotor);
    //         }
    //       }
    //     } else if (mode == 2) {
    //       for (int i = 0; i < 8; i++) {
    //         if (currentPosition != moveTo) {
    //           digitalWrite(in1, halfStep[i][0]);
    //           digitalWrite(in2, halfStep[i][1]);
    //           digitalWrite(in3, halfStep[i][2]);
    //           digitalWrite(in4, halfStep[i][3]);
    //           currentPosition += 1;
    //           Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 8, halfStep[i][0], halfStep[i][1], halfStep[i][2], halfStep[i][3]);
    //           vTaskDelay(speedMotor);
    //         }
    //       }
    //     }
    //   } else if (moveTo < currentPosition) {
    //     if (mode == 1) {
    //       for (int i = 4 - 1; i >= 0; i--) {
    //         if (currentPosition != moveTo) {
    //           digitalWrite(in1, fullStep[i][0]);
    //           digitalWrite(in2, fullStep[i][1]);
    //           digitalWrite(in3, fullStep[i][2]);
    //           digitalWrite(in4, fullStep[i][3]);
    //           currentPosition -= 1;
    //           Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 4, fullStep[i][0], fullStep[i][1], fullStep[i][2], fullStep[i][3]);
    //           vTaskDelay(speedMotor);
    //         }
    //       }
    //     } else if (mode == 2) {
    //       for (int i = 8 - 1; i >= 0; i--) {
    //         if (currentPosition != moveTo) {
    //           digitalWrite(in1, halfStep[i][0]);
    //           digitalWrite(in2, halfStep[i][1]);
    //           digitalWrite(in3, halfStep[i][2]);
    //           digitalWrite(in4, halfStep[i][3]);
    //           currentPosition -= 1;
    //           Serial.printf("Detail : CORE(%d) currentPosition(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n", xPortGetCoreID(), currentPosition, 8, halfStep[i][0], halfStep[i][1], halfStep[i][2], halfStep[i][3]);
    //           vTaskDelay(speedMotor);
    //         }
    //       }
    //     }
    //   }
    // }
  }
};

static int fullStep[4][4] = {
  { 1, 0, 0, 1 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 1 }
};
static int halfStep[8][4] = {
  { 1, 0, 0, 1 }, { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 0, 1 }
};

#define MOTOR1_IN1 23
#define MOTOR1_IN2 22
#define MOTOR1_IN3 21
#define MOTOR1_IN4 19

#define MOTOR2_IN1 32
#define MOTOR2_IN2 33
#define MOTOR2_IN3 25
#define MOTOR2_IN4 26

#define MOTOR3_IN1 27
#define MOTOR3_IN2 14
#define MOTOR3_IN3 13
#define MOTOR3_IN4 18

#define MOTOR4_IN1 15
#define MOTOR4_IN2 4
#define MOTOR4_IN3 17
#define MOTOR4_IN4 5

const char *ssid = "Naras";
const char *password = "-Naras-CPE290821-";

AsyncWebServer server(80);

const int LED = 2;

static MyStepper stepper1(1, MOTOR1_IN1, MOTOR1_IN2, MOTOR1_IN3, MOTOR1_IN4);
static MyStepper stepper2(1, MOTOR2_IN1, MOTOR2_IN2, MOTOR2_IN3, MOTOR2_IN4);
static MyStepper stepper3(1, MOTOR3_IN1, MOTOR3_IN2, MOTOR3_IN3, MOTOR3_IN4);
static MyStepper stepper4(1, MOTOR4_IN1, MOTOR4_IN2, MOTOR4_IN3, MOTOR4_IN4);

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
  server.on("/status", HTTP_POST, handleStatus);
  server.on(
    "/handle/motor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleMotor);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  stepper1.setCurrentPosition(0);
  stepper1.setSpeedMotor(0);
  // xTaskCreatePinnedToCore(motor1, "motor1", 2048, NULL, 20, &Task1, ARDUINO_RUNNING_CORE);
  stepper2.setCurrentPosition(0);
  stepper2.setSpeedMotor(0);
  // xTaskCreatePinnedToCore(motor2, "motor2", 2048, NULL, 20, &Task2, ARDUINO_RUNNING_CORE);
  stepper3.setCurrentPosition(0);
  stepper3.setSpeedMotor(0);
  // xTaskCreatePinnedToCore(motor3, "motor3", 2048, NULL, 20, &Task3, ARDUINO_RUNNING_CORE);
  stepper4.setCurrentPosition(0);
  stepper4.setSpeedMotor(0);
  // xTaskCreatePinnedToCore(motor4, "motor4", 2048, NULL, 20, &Task4, ARDUINO_RUNNING_CORE);
}

void loop(void) {
  stepper1.run(1);
  stepper2.run(2);
  stepper3.run(3);
  stepper4.run(4);
}

void handleMotor(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  request->send(200, "application/json", "ok");
  if (!root["positions"].isNull()) {
    JsonArray positionsRaw = root["positions"];
    for (JsonObject repo : positionsRaw) {
      switch (repo["index"].as<int>()) {
        case 0:
          stepper1.setMoveTo(repo["value"].as<long>());
          // xTaskCreatePinnedToCore(motor1, "motor1", 2048, NULL, 20, &Task1, ARDUINO_RUNNING_CORE);
          break;
        case 1:
          stepper2.setMoveTo(repo["value"].as<long>());
          // xTaskCreatePinnedToCore(motor2, "motor2", 2048, NULL, 20, &Task2, ARDUINO_RUNNING_CORE);
          break;
        case 2:
          stepper3.setMoveTo(repo["value"].as<long>());
          // xTaskCreatePinnedToCore(motor3, "motor3", 2048, NULL, 20, &Task3, ARDUINO_RUNNING_CORE);
          break;
        case 3:
          stepper4.setMoveTo(repo["value"].as<long>());
          // xTaskCreatePinnedToCore(motor4, "motor4", 2048, NULL, 20, &Task4, ARDUINO_RUNNING_CORE);
          break;
      }
      Serial.printf("Index : %d Value : %d\n", repo["index"].as<int>(), repo["value"].as<long>());
    }
  }
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
