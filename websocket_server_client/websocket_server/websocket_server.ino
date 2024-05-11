#include <WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "Nang Luong Xanh";
const char* password = "nangluongxanh360";

WebSocketsServer webSocket = WebSocketsServer(81); // Dùng cổng 81 cho WebSocket Server

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("Client #%u connected\n", num);
      webSocket.sendTXT(num, "Hello Client");
      break;
    case WStype_TEXT:
      Serial.printf("Received text: %s\n", payload);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}