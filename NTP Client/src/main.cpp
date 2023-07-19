#include <Arduino.h>
#include "wifi.hpp"
#include "ntp_client.hpp"

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

  if (millis() - timestamp > 1 * 1000) {
    timestamp = millis();
    for(NTPClient* slave : NTPClient::slaves) {
      slave->setTimestampDifSlave();
    }
    Serial.println(NTPClient::toJSON());
    Serial.println();
  }

  if (digitalRead(button) == LOW) {
    delay(500);
    ESP.restart();
  }
}
