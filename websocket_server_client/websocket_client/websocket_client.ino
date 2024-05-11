#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <PubSubClient.h>

const char* ssid = "Nang Luong Xanh";
const char* password = "nangluongxanh360";
const char* webSocketServerHost = "hot24h.net"; // Thay "server_ip_address" bằng IP của WebSocket Server
const int webSocketServerPort = 8080; // Đảm bảo cổng này trùng với cổng WebSocket Server
// const char* webSocketServerHost = "192.168.1.18"; // Thay "server_ip_address" bằng IP của WebSocket Server
// const int webSocketServerPort = 81; // Đảm bảo cổng này trùng với cổng WebSocket Server

// MQTT Broker
const char* mqtt_server = "hot24h.net";
const int mqtt_port = 8083;

WebSocketsClient webSocket;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Cấu trúc dữ liệu để nhận thông tin
// Cấu trúc này phải khớp với cấu trúc bên gửi
typedef struct struct_message {
  int id;
  int analogValue1;
  int analogValue2;
  int analogValue3;
} struct_message;

// Tạo một biến struct_message gọi là myData
struct_message myData;




void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Xử lý và gửi dữ liệu qua WebSocket
    String msg = "Data from MQTT: " + String((char*)payload);
    webSocket.sendTXT(msg);
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      webSocket.sendTXT("/day");
      webSocket.sendTXT("Topic");
      // webSocket.sendTXT(String(myData.id));
      webSocket.sendBIN((uint8_t *)&myData.id, sizeof(myData.id));
      break;
    case WStype_BIN:
      if (length == sizeof(struct_message)) {
        struct_message* msg = (struct_message*)payload; // Ép kiểu con trỏ để truy cập vào cấu trúc dữ liệu
        Serial.print("ID: ");
        Serial.println(msg->id);
        Serial.print("Analog Value 1: ");
        Serial.println(msg->analogValue1);
        Serial.print("Analog Value 2: ");
        Serial.println(msg->analogValue2);
        Serial.print("Analog Value 3: ");
        Serial.println(msg->analogValue3);
      } else {
        Serial.println("Received data size does not match struct_message size.");
      }
      break;
    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
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

  webSocket.begin(webSocketServerHost, webSocketServerPort, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000); // Tự động kết nối lại nếu mất kết nối

  
}

void loop() {

  
  webSocket.loop();
  mqttClient.loop();
  // Ví dụ gán giá trị cho myData
  myData.id = 1;
  myData.analogValue1 = random(0, 1024); // Sinh số ngẫu nhiên từ 0 đến 1023
  myData.analogValue2 = random(0, 1024);
  myData.analogValue3 = random(0, 1024);

  // Serial.println(myData.analogValue1);


  

}