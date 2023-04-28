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

AsyncWebServer server(80);

const int LED = 2;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t Task4;

static int fullStep[4][4] = {
  { 1, 0, 0, 1 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 1 }
};
static int halfStep[8][4] = {
  { 1, 0, 0, 1 }, { 1, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 0, 0, 1 }
};

typedef struct Data_t {
  long moveTo;
  int speedMotor;
  int port1;
  int port2;
  int port3;
  int port4;
  int motorNumber;
  long *currentPosition;
  int mode;
} GenericData_t;

void motor(void *pvParameters) {
  GenericData_t *data = (GenericData_t *)pvParameters;
  const long moveTo = data->moveTo;
  const int speedMotor = data->speedMotor;
  const int motorNumber = data->motorNumber;
  const int in1 = data->port1;
  const int in2 = data->port2;
  const int in3 = data->port3;
  const int in4 = data->port4;
  long *currentPosition = data->currentPosition;
  const int mode = data->mode;
  Serial.printf("Print from thread(%d) in value %d and position(%d)....\n", xPortGetCoreID(), motorNumber, *currentPosition);
  for (;;) {
    if (moveTo > *currentPosition) {
      if (mode == 1) {
        for (int i = 0; i < 4; i++) {
          if (*currentPosition != moveTo) {
            digitalWrite(in1, fullStep[i][0]);
            digitalWrite(in2, fullStep[i][1]);
            digitalWrite(in3, fullStep[i][2]);
            digitalWrite(in4, fullStep[i][3]);
            *currentPosition += 1;
            Serial.printf("Detail : CORE(%d) currentPosition(%d) MOTOR(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n",
                          xPortGetCoreID(),
                          *currentPosition,
                          motorNumber,
                          mode == 1 ? 4 : 8,
                          mode == 1 ? fullStep[i][0] : halfStep[i][0],
                          mode == 1 ? fullStep[i][1] : halfStep[i][1],
                          mode == 1 ? fullStep[i][2] : halfStep[i][2],
                          mode == 1 ? fullStep[i][3] : halfStep[i][3]);
            vTaskDelay(speedMotor / portTICK_PERIOD_MS);
          } else {
            break;
          }
        }
      } else if (mode == 2) {
        for (int i = 0; i < 8; i++) {
          if (*currentPosition != moveTo) {
            digitalWrite(in1, halfStep[i][0]);
            digitalWrite(in2, halfStep[i][1]);
            digitalWrite(in3, halfStep[i][2]);
            digitalWrite(in4, halfStep[i][3]);
            *currentPosition += 1;
            Serial.printf("Detail : CORE(%d) currentPosition(%d) MOTOR(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n",
                          xPortGetCoreID(),
                          *currentPosition,
                          motorNumber,
                          mode == 1 ? 4 : 8,
                          mode == 1 ? fullStep[i][0] : halfStep[i][0],
                          mode == 1 ? fullStep[i][1] : halfStep[i][1],
                          mode == 1 ? fullStep[i][2] : halfStep[i][2],
                          mode == 1 ? fullStep[i][3] : halfStep[i][3]);
            vTaskDelay(speedMotor / portTICK_PERIOD_MS);
          } else {
            break;
          }
        }
      }
    } else if (moveTo < *currentPosition) {
      for (int i = *currentPosition; i > moveTo; i--) {
        if (mode == 1) {
          for (int i = 4 - 1; i >= 0; i--) {
            if (*currentPosition != moveTo) {
              digitalWrite(in1, fullStep[i][0]);
              digitalWrite(in2, fullStep[i][1]);
              digitalWrite(in3, fullStep[i][2]);
              digitalWrite(in4, fullStep[i][3]);
              *currentPosition -= 1;
              Serial.printf("Detail : CORE(%d) currentPosition(%d) MOTOR(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n",
                            xPortGetCoreID(),
                            *currentPosition,
                            motorNumber,
                            mode == 1 ? 4 : 8,
                            mode == 1 ? fullStep[i][0] : halfStep[i][0],
                            mode == 1 ? fullStep[i][1] : halfStep[i][1],
                            mode == 1 ? fullStep[i][2] : halfStep[i][2],
                            mode == 1 ? fullStep[i][3] : halfStep[i][3]);
              vTaskDelay(speedMotor / portTICK_PERIOD_MS);
            } else {
              break;
            }
          }
        } else if (mode == 2) {
          for (int i = 8 - 1; i >= 0; i--) {
            if (*currentPosition != moveTo) {
              digitalWrite(in1, halfStep[i][0]);
              digitalWrite(in2, halfStep[i][1]);
              digitalWrite(in3, halfStep[i][2]);
              digitalWrite(in4, halfStep[i][3]);
              *currentPosition -= 1;
              Serial.printf("Detail : CORE(%d) currentPosition(%d) MOTOR(%d) SIZE(%d) B1(%d) B2(%d) B3(%d) B4(%d)\n",
                            xPortGetCoreID(),
                            *currentPosition,
                            motorNumber,
                            mode == 1 ? 4 : 8,
                            mode == 1 ? fullStep[i][0] : halfStep[i][0],
                            mode == 1 ? fullStep[i][1] : halfStep[i][1],
                            mode == 1 ? fullStep[i][2] : halfStep[i][2],
                            mode == 1 ? fullStep[i][3] : halfStep[i][3]);
              vTaskDelay(speedMotor / portTICK_PERIOD_MS);
            } else {
              break;
            }
          }
        }
      }
    } else {
      break;
    }
  }
  digitalWrite(in1, 0);
  digitalWrite(in2, 0);
  digitalWrite(in3, 0);
  digitalWrite(in4, 0);
  switch (motorNumber) {
    case 1:
      vTaskDelete(Task1);
      break;
    case 2:
      vTaskDelete(Task2);
      break;
    case 3:
      vTaskDelete(Task3);
      break;
    case 4:
      vTaskDelete(Task4);
      break;
  }
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
  MyStepper() {}

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

  void setupPort() {
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
  }

  void clearPort() {
    digitalWrite(in1, 0);
    digitalWrite(in2, 0);
    digitalWrite(in3, 0);
    digitalWrite(in4, 0);
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

  long *getCurrentPosition() {
    return &currentPosition;
  }

  long getCurrentPositionRaw() {
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

  void setPort1(int s) {
    in1 = s;
  }

  int getPort1() {
    return in1;
  }

  void setPort2(int s) {
    in2 = s;
  }

  int getPort2() {
    return in2;
  }

  void setPort3(int s) {
    in3 = s;
  }

  int getPort3() {
    return in3;
  }

  void setPort4(int s) {
    in4 = s;
  }

  int getPort4() {
    return in4;
  }

  void setMode(int s) {
    mode = s;
  }

  int getMode() {
    return mode;
  }
};

static MyStepper stepper1(1, MOTOR1_IN1, MOTOR1_IN2, MOTOR1_IN3, MOTOR1_IN4);
static MyStepper stepper2(1, MOTOR2_IN1, MOTOR2_IN2, MOTOR2_IN3, MOTOR2_IN4);
static MyStepper stepper3(1, MOTOR3_IN1, MOTOR3_IN2, MOTOR3_IN3, MOTOR3_IN4);
static MyStepper stepper4(1, MOTOR4_IN1, MOTOR4_IN2, MOTOR4_IN3, MOTOR4_IN4);

class WifiSetup {
private:
  char *ssid;
  char *password;
public:
  WifiSetup() {}

  void setSsid(char *s) {
    this->ssid = s;
  }

  char *getSsid() {
    return this->ssid;
  }

  void setPassword(char *s) {
    this->password = s;
  }

  char *getPassword() {
    return this->password;
  }
};

static WifiSetup wifiSetup;

void setup(void) {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  wifiSetup.setSsid("NS");
  wifiSetup.setPassword("-Naras-CPE290821-");
  WiFi.begin(wifiSetup.getSsid(), wifiSetup.getPassword());
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifiSetup.getSsid());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_POST, handleStatus);
  server.on(
    "/handle/motor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleMotor);
  server.on(
    "/handle/cancle", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleCancleThreadRunning);
  server.on(
    "/handle/suspend", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSuspends);
  server.on(
    "/handle/resume", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleResume);
  server.on(
    "/handle/change-mode", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleChangeMode);
  server.on(
    "/handle/change-port", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleChangePort);
  server.on("/handle/inquiry", HTTP_GET, getMotorValue);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  stepper1.setCurrentPosition(0);
  stepper1.setSpeedMotor(0);
  stepper1.setName("MotorI");
  stepper2.setCurrentPosition(0);
  stepper2.setSpeedMotor(0);
  stepper2.setName("MotorII");
  stepper3.setCurrentPosition(0);
  stepper3.setSpeedMotor(0);
  stepper3.setName("MotorIII");
  stepper4.setCurrentPosition(0);
  stepper4.setSpeedMotor(0);
  stepper4.setName("MotorIIII");
}

void loop(void) {
  int num = Serial.read();
  if (num == 0) {
    Task1 = xTaskGetHandle("MotorI");
    if (Task1 != NULL) {
      vTaskDelete(Task1);
    }
    Task2 = xTaskGetHandle("MotorII");
    if (Task2 != NULL) {
      vTaskDelete(Task2);
    }
    Task3 = xTaskGetHandle("MotorIII");
    if (Task3 != NULL) {
      vTaskDelete(Task3);
    }
    Task4 = xTaskGetHandle("MotorIIII");
    if (Task4 != NULL) {
      vTaskDelete(Task4);
    }
  } else if (num == 1) {
    Task1 = xTaskGetHandle("MotorI");
    if (Task1 != NULL) {
      vTaskDelete(Task1);
    }
  } else if (num == 2) {
    Task2 = xTaskGetHandle("MotorII");
    if (Task2 != NULL) {
      vTaskDelete(Task2);
    }
  } else if (num == 3) {
    Task3 = xTaskGetHandle("MotorIII");
    if (Task3 != NULL) {
      vTaskDelete(Task3);
    }
  } else if (num == 4) {
    Task4 = xTaskGetHandle("MotorIIII");
    if (Task4 != NULL) {
      vTaskDelete(Task4);
    }
  }
  delay(1000);
}

void handleMotor(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  Serial.printf("Running on thread : %d\n", xPortGetCoreID());
  try {
    if (!root["positions"].isNull()) {
      JsonObject jo = root["positions"].as<JsonObject>();
      if (jo["index"].as<int>() == 1) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskDelete(Task1);
        }
        stepper1.setMoveTo(jo["value"].as<long>());
        if (!jo["speed"].isNull()) {
          stepper1.setSpeedMotor(jo["speed"].as<int>());
        }
        GenericData_t dataT = { stepper1.getMoveTo(), stepper1.getSpeedMotor(), stepper1.getPort1(), stepper1.getPort2(), stepper1.getPort3(), stepper1.getPort4(), 1, stepper1.getCurrentPosition(), stepper1.getMode() };
        xTaskCreatePinnedToCore(motor, stepper1.getName(), 4096, (void *)&dataT, 1, &Task1, 0);
      } else if (jo["index"].as<int>() == 2) {
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskDelete(Task2);
        }
        stepper2.setMoveTo(jo["value"].as<long>());
        if (!jo["speed"].isNull()) {
          stepper2.setSpeedMotor(jo["speed"].as<int>());
        }
        GenericData_t dataT = { stepper2.getMoveTo(), stepper2.getSpeedMotor(), stepper2.getPort1(), stepper2.getPort2(), stepper2.getPort3(), stepper2.getPort4(), 2, stepper2.getCurrentPosition(), stepper2.getMode() };
        xTaskCreatePinnedToCore(motor, stepper2.getName(), 4096, (void *)&dataT, 1, &Task2, 1);
      } else if (jo["index"].as<int>() == 3) {
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskDelete(Task3);
        }
        stepper3.setMoveTo(jo["value"].as<long>());
        if (!jo["speed"].isNull()) {
          stepper3.setSpeedMotor(jo["speed"].as<int>());
        }
        GenericData_t dataT = { stepper3.getMoveTo(), stepper3.getSpeedMotor(), stepper3.getPort1(), stepper3.getPort2(), stepper3.getPort3(), stepper3.getPort4(), 3, stepper3.getCurrentPosition(), stepper3.getMode() };
        xTaskCreatePinnedToCore(motor, stepper3.getName(), 4096, (void *)&dataT, 1, &Task3, 0);
      } else if (jo["index"].as<int>() == 4) {
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskDelete(Task4);
        }
        stepper4.setMoveTo(jo["value"].as<long>());
        if (!jo["speed"].isNull()) {
          stepper4.setSpeedMotor(jo["speed"].as<int>());
        }
        GenericData_t dataT = { stepper4.getMoveTo(), stepper4.getSpeedMotor(), stepper4.getPort1(), stepper4.getPort2(), stepper4.getPort3(), stepper4.getPort4(), 4, stepper4.getCurrentPosition(), stepper4.getMode() };
        xTaskCreatePinnedToCore(motor, stepper4.getName(), 4096, (void *)&dataT, 1, &Task4, 1);
      }
      Serial.printf("Index : %d Value : %d\n", jo["index"].as<int>(), jo["value"].as<long>());
    }
    request->send(200, "application/json", "{\"status\":\"success\"}");
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleCancleThreadRunning(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  try {
    if (!root["motor"].isNull()) {
      Serial.printf("Motor: %d\n", root["motor"].as<int>());
      if (root["motor"].as<int>() == 0) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskDelete(Task1);
          stepper1.clearPort();
        }
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskDelete(Task2);
          stepper2.clearPort();
        }
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskDelete(Task3);
          stepper3.clearPort();
        }
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskDelete(Task4);
          stepper4.clearPort();
        }
      } else if (root["motor"].as<int>() == 1) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskDelete(Task1);
          stepper1.clearPort();
        }
      } else if (root["motor"].as<int>() == 2) {
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskDelete(Task2);
          stepper2.clearPort();
        }
      } else if (root["motor"].as<int>() == 3) {
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskDelete(Task3);
          stepper3.clearPort();
        }
      } else if (root["motor"].as<int>() == 4) {
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskDelete(Task4);
          stepper4.clearPort();
        }
      } else {
        Serial.println("Motor number is not found.");
      }
    }
    request->send(200, "application/json", "{\"status\":\"success\"}");
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void getMotorValue(AsyncWebServerRequest *request) {
  int paramsNr = request->params();
  const int capacity = JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  JsonObject data = doc.createNestedObject("data");
  MyStepper stepper;
  try {
    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter *p = request->getParam(i);
      Serial.print("Param name: ");
      Serial.println(p->name());
      Serial.print("Param value: ");
      Serial.println(p->value());
      Serial.println("------");
      if (p->name().equals("motor")) {
        const long motorNumber = p->value().toInt();
        if (motorNumber == 1) {
          stepper = stepper1;
        } else if (motorNumber == 2) {
          stepper = stepper2;
        } else if (motorNumber == 3) {
          stepper = stepper3;
        } else if (motorNumber == 4) {
          stepper = stepper4;
        }
      }
      data["currentPosition"] = stepper.getCurrentPositionRaw();
      data["port1"] = stepper.getPort1();
      data["port2"] = stepper.getPort2();
      data["port3"] = stepper.getPort3();
      data["port4"] = stepper.getPort4();
      data["name"] = stepper.getName();
      data["moveToPosition"] = stepper.getMoveTo();
      data["mode"] = stepper.getMode();
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(doc, *response);
      request->send(response);
    }
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleSuspends(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  try {
    if (!root["motor"].isNull()) {
      Serial.printf("Motor: %d\n", root["motor"].as<int>());
      if (root["motor"].as<int>() == 0) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskSuspend(Task1);
        }
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskSuspend(Task2);
        }
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskSuspend(Task3);
        }
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskSuspend(Task4);
        }
      } else if (root["motor"].as<int>() == 1) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskSuspend(Task1);
        }
      } else if (root["motor"].as<int>() == 2) {
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskSuspend(Task2);
        }
      } else if (root["motor"].as<int>() == 3) {
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskSuspend(Task3);
        }
      } else if (root["motor"].as<int>() == 4) {
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskSuspend(Task4);
        }
      } else {
        Serial.println("Motor number is not found.");
      }
    }
    request->send(200, "application/json", "{\"status\":\"success\"}");
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleResume(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  try {
    if (!root["motor"].isNull()) {
      Serial.printf("Motor: %d\n", root["motor"].as<int>());
      if (root["motor"].as<int>() == 0) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskResume(Task1);
        }
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskResume(Task2);
        }
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskResume(Task3);
        }
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskResume(Task4);
        }
      } else if (root["motor"].as<int>() == 1) {
        Task1 = xTaskGetHandle("MotorI");
        if (Task1 != NULL) {
          vTaskResume(Task1);
        }
      } else if (root["motor"].as<int>() == 2) {
        Task2 = xTaskGetHandle("MotorII");
        if (Task2 != NULL) {
          vTaskResume(Task2);
        }
      } else if (root["motor"].as<int>() == 3) {
        Task3 = xTaskGetHandle("MotorIII");
        if (Task3 != NULL) {
          vTaskResume(Task3);
        }
      } else if (root["motor"].as<int>() == 4) {
        Task4 = xTaskGetHandle("MotorIIII");
        if (Task4 != NULL) {
          vTaskResume(Task4);
        }
      } else {
        Serial.println("Motor number is not found.");
      }
    }
    request->send(200, "application/json", "{\"status\":\"success\"}");
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleChangeMode(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  try {
    if (!root["motor"].isNull()) {
      Serial.printf("Motor: %d\n", root["motor"].as<int>());
      if (root["motor"].as<int>() == 1) {
        if (!root["mode"].isNull()) {
          stepper1.setMode(root["mode"].as<int>());
        }
      } else if (root["motor"].as<int>() == 2) {
        if (!root["mode"].isNull()) {
          stepper2.setMode(root["mode"].as<int>());
        }
      } else if (root["motor"].as<int>() == 3) {
        if (!root["mode"].isNull()) {
          stepper3.setMode(root["mode"].as<int>());
        }
      } else if (root["motor"].as<int>() == 4) {
        if (!root["mode"].isNull()) {
          stepper4.setMode(root["mode"].as<int>());
        }
      } else {
        Serial.println("Motor number is not found.");
      }
    }
    request->send(200, "application/json", "{\"status\":\"success\"}");
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleChangePort(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DynamicJsonDocument root = mapJsonObject(data);
  try {
    if (!root["motor"].isNull()) {
      Serial.printf("Motor: %d\n", root["motor"].as<int>());
      if (root["motor"].as<int>() == 1) {
        if (!root["port1"].isNull()) {
          stepper1.setPort1(root["port1"].as<int>());
        }
        if (!root["port2"].isNull()) {
          stepper1.setPort2(root["port2"].as<int>());
        }
        if (!root["port3"].isNull()) {
          stepper1.setPort3(root["port3"].as<int>());
        }
        if (!root["port4"].isNull()) {
          stepper1.setPort4(root["port4"].as<int>());
        }
        stepper1.setupPort();
      } else if (root["motor"].as<int>() == 2) {
        if (!root["port1"].isNull()) {
          stepper2.setPort1(root["port1"].as<int>());
        }
        if (!root["port2"].isNull()) {
          stepper2.setPort2(root["port2"].as<int>());
        }
        if (!root["port3"].isNull()) {
          stepper2.setPort3(root["port3"].as<int>());
        }
        if (!root["port4"].isNull()) {
          stepper2.setPort4(root["port4"].as<int>());
        }
        stepper2.setupPort();
      } else if (root["motor"].as<int>() == 3) {
        if (!root["port1"].isNull()) {
          stepper3.setPort1(root["port1"].as<int>());
        }
        if (!root["port2"].isNull()) {
          stepper3.setPort2(root["port2"].as<int>());
        }
        if (!root["port3"].isNull()) {
          stepper3.setPort3(root["port3"].as<int>());
        }
        if (!root["port4"].isNull()) {
          stepper3.setPort4(root["port4"].as<int>());
        }
        stepper3.setupPort();
      } else if (root["motor"].as<int>() == 4) {
        if (!root["port1"].isNull()) {
          stepper4.setPort1(root["port1"].as<int>());
        }
        if (!root["port2"].isNull()) {
          stepper4.setPort2(root["port2"].as<int>());
        }
        if (!root["port3"].isNull()) {
          stepper4.setPort3(root["port3"].as<int>());
        }
        if (!root["port4"].isNull()) {
          stepper4.setPort4(root["port4"].as<int>());
        }
        stepper4.setupPort();
      } else {
        Serial.println("Motor number is not found.");
      }
    }
    request->send(200, "application/json", "{\"status\":\"success\"}");
  } catch (...) {
    request->send(500, "application/json", "{\"status\":\"error\"}");
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
