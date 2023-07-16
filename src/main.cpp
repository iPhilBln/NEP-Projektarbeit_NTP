#include <Arduino.h>
#include "wifi.hpp"

#define button 18

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true); 
  delay(1000);

  pinMode(button, INPUT);

  if (digitalRead(button) == LOW) {
    delay(500);
    wifiCleanup();
  }

  wifiBegin();
}

unsigned long timestamp = 0;

void loop() {
  wifiProcess();
/*
  if (millis() - timestamp > 5000) {
    timestamp = millis();
    int paramCount = wifiGetParamCounter();
    for (uint8_t i = 0; i < paramCount; i++) {
      const char* param = wifiGetParamValue(i);
      if (param != nullptr) 
        Serial.println(param);
    }
    Serial.println();
  }
*/
  if (digitalRead(button) == LOW) {
    delay(500);
    ESP.restart();
  }
}
