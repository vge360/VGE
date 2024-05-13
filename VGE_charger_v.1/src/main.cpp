#include <Arduino.h>
#include <ArduinoJson.h>

#include "net_manager.h"

const int PIN_LED = 2;


void setup() {
  net_manager_setup();
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);


  // wifi connect
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  delay(10);
  Serial.println("hello");
  digitalWrite(PIN_LED, LOW);
  // If we get here, we are connected to the WiFi
  digitalWrite(PIN_LED, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


}

void loop() {

  drd->loop();

}

