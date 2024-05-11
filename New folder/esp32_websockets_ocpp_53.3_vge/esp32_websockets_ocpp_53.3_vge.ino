// ================================ Thư viện ===================================
#include <dummy.h>

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <WiFiMulti.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_now.h>

// =============================== khai báo biến và giá trị======================
#define WIFI_SSID "Nang Luong Xanh"
#define WIFI_PASSWORD "nangluongxanh360"
#define WS_HOST "hot24h.net" //Đây là địa chỉ của máy chủ WebSocket.
#define WS_PORT 8080 //Đây là cổng kết nối của máy chủ WebSocket.
#define WS_URL "/" //Đây là URL của máy chủ WebSocket.
#define JSON_DOC_SIZE 2048
#define MSG_SIZE 256

#define LEDC_CHANNEL_0     0
#define LEDC_TIMER_13_BIT  13
#define LEDC_BASE_FREQ     1000 // Frequency 1KHz
#define LED_PIN            21  // Change to GPIO21 for PWM signal

// Define the GPIO pins
#define HEARTBEAT_PIN           22
#define AUTHORIZE_PIN   18
#define METER_VALUES_PIN    19
#define RESET_PIN               23

const char* STATION_ID = "vge_01_001";

bool heartbeatFlag = false;  // Flag to indicate if heartbeat should be sent
unsigned long heartbeatInterval = 5000;  // Interval for sending heartbeat in milliseconds
unsigned long lastHeartbeatTime = 0;  // Time of the last heartbeat sent

const char* ssid = "Nang Luong Xanh";
const char* password = "nangluongxanh360";
const char* webSocketServerHost = "hot24h.net"; // Thay "server_ip_address" bằng IP của WebSocket Server
const int webSocketServerPort = 8080; // Đảm bảo cổng này trùng với cổng WebSocket Server

WiFiMulti wifiMulti;
WebSocketsClient wsClient;
WebSocketsClient webSocket;

// ==================Cấu trúc dữ liệu để nhận thông tin======================
// Cấu trúc này phải khớp với cấu trúc bên gửi
typedef struct struct_message {
  int id;
  int analogValue1;
  int analogValue2;
  int analogValue3;
} struct_message;

// Tạo một biến struct_message gọi là myData
struct_message myData;

// Tạo một mảng chứa tất cả các cấu trúc
struct_message boardsStruct[3];
// ======================= FUNCTION CHO ESP-NOW ==============================================
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Gói tin nhận từ: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Cập nhật cấu trúc với dữ liệu mới nhận được
  boardsStruct[myData.id-1].analogValue1 = myData.analogValue1;
  boardsStruct[myData.id-1].analogValue2 = myData.analogValue2;
  boardsStruct[myData.id-1].analogValue3 = myData.analogValue3;
  Serial.printf("Giá trị analogValue1: %d \n", boardsStruct[myData.id-1].analogValue1);
  Serial.printf("Giá trị analogValue2: %d \n", boardsStruct[myData.id-1].analogValue2);
  Serial.printf("Giá trị analogValue3: %d \n", boardsStruct[myData.id-1].analogValue3);
  Serial.println();
}
// ======================= FUNCTION CHO GIAO THỨC WEBSOCKET ==================================

// ---------------- gửi tin nhẵn đang lỗi gì đó cho server-------------------------
void sendErrorMessage(const char *error) {
  char msg[MSG_SIZE];
  sprintf(msg, "{\"action\":\"msg\",\"type\":\"error\",\"body\":\"%s\"}", error); //đây là đoạn Json theo cấu trúc dữ liệu theo server có thể thay đổi theo cấu trúc của server. 
  wsClient.sendTXT(msg); 
}

// ---------------- gửi tin nhắn "ok" đến server.------------------------------------
void sendOkMessage() {
  wsClient.sendTXT("{\"action\":\"msg\",\"type\":\"status\",\"body\":\"ok\"}");
}

//  --------------- xử lý các tin nhắn được nhận từ server --------------------------------
void handleMessage(uint8_t *payload) {
  StaticJsonDocument<JSON_DOC_SIZE> doc; //được sử dụng để phân tích cú pháp và lưu trữ dữ liệu từ tin nhắn JSON nhận được từ máy chủ.
  DeserializationError error = deserializeJson(doc, payload);// đọc từ payload nhận đưuọc từ máy chủ và lưu trữ kết quả vào đối tượng "doc" còn nếu có lỗi thì đc lưu vào biến error.
  // nếu có lỗi thì in ra serial
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    sendErrorMessage(error.c_str());
    return;
  }
  // xác minh hoạt động kết nối
  if (doc["type"].as<String>() == "Heartbeat") {
    digitalWrite(HEARTBEAT_PIN, HIGH);
    return;
  }
  // đoạn này kiểm tra loại tin nhắn và thực hiện các hành động theo từng loại tin nhắn  
  if (doc["type"].as<String>() == "Authorize") {
    digitalWrite(AUTHORIZE_PIN, HIGH);
    delay(100);
    digitalWrite(AUTHORIZE_PIN, LOW);
    return;
  }

  if (doc["type"].as<String>() == "MeterValues") {
    digitalWrite(METER_VALUES_PIN, HIGH);
    delay(100);
    digitalWrite(METER_VALUES_PIN, LOW);
    // send the "MeterValues" message
    wsClient.sendTXT("{\"type\":\"MeterValues\"}");
    return;
  }

  if (doc["type"].as<String>() == "Reset") {
    digitalWrite(RESET_PIN, HIGH);
    delay(100);
    digitalWrite(RESET_PIN, LOW);
    return;
  }

  sendErrorMessage("unsupported OCPP command type");
  return;
}
//  ----------------hàm này xử lý kết nối với websocket------------------------

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      break;
    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
      break;
  }
}


void onWSEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("WS Connected");
      break;
    case WStype_DISCONNECTED:
      Serial.println("WS Disconnected");
      break;
    case WStype_TEXT:
      Serial.printf("WS Message: %s\n", payload);

      handleMessage(payload);

      break;
  }
}



// ======================================== SETUP ===============================
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Setup pins
  pinMode(HEARTBEAT_PIN, OUTPUT);
  pinMode(AUTHORIZE_PIN, OUTPUT);
  pinMode(METER_VALUES_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");

  webSocket.begin(webSocketServerHost, webSocketServerPort, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000); // Tự động kết nối lại nếu mất kết nối


  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  // Chờ kết nối WiFi
  Serial.println("Wifi connection...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println("Connected");


  //muốn bắt đầu kết nối
  wsClient.beginSSL(WS_HOST, WS_PORT, WS_URL,  "", "wss"); //"wss": Đây là giao thức được sử dụng cho kết nối WebSocket bảo mật. , WS_URL
  wsClient.onEvent(onWSEvent); // hàm này sẽ đc gọi khi có sự kiện kết nối
  // wsClient.begin(webSocketServerHost, webSocketServerPort, "/");
  // wsClient.onEvent(onWSEvent);
  // wsClient.setReconnectInterval(5000); // Tự động kết nối lại nếu mất kết nối




  // Khởi tạo ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    return;
  }
  // Đăng ký nhận callback khi có gói tin nhận được
  esp_now_register_recv_cb(OnDataRecv);


  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
}


// ======================================== LOOP ===============================
void loop() {
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED); // bật led khi có kết nối wifi
  wsClient.loop();

  // Dòng này tính toán duty cycle (tỷ lệ thời gian cao so với tổng chu kỳ) cho kênh PWM được cấu hình trước đó, sau đó viết giá trị duty cycle đó vào kênh đó.
  uint32_t dutyCycle = (uint32_t)(pow(2, LEDC_TIMER_13_BIT) * 0.533); // 53.3% duty cycle
  ledcWrite(LEDC_CHANNEL_0, dutyCycle);

  delay(1000); // Just to avoid watchdog timer resets

  // Check if heartbeat should be sent
  if (digitalRead(HEARTBEAT_PIN) == HIGH && millis() - lastHeartbeatTime >= heartbeatInterval) {
    wsClient.sendTXT("{\"type\":\"Heartbeat\"}");
    lastHeartbeatTime = millis();
  }

  // Ensure all pins are low when not actively used
  digitalWrite(HEARTBEAT_PIN, LOW);
  digitalWrite(AUTHORIZE_PIN, LOW);
  digitalWrite(METER_VALUES_PIN, LOW);
  digitalWrite(RESET_PIN, LOW);
}
