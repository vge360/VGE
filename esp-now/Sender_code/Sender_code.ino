#include <WiFiMulti.h>
#include <esp_now.h>

WiFiMulti wifiMulti;

// REPLACE WITH YOUR RECEIVER MAC Address địa chỉ của người nhận
uint8_t broadcastAddress[] = {0xFC, 0xB4, 0x67, 0x92, 0x5C, 0x1C};

typedef struct struct_message {
    int id; // must be unique for each sender board
    int x;
    int y;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t"); // trạng thái gửi gói cuối cùng
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"); // 
}

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo WiFiMulti và thêm các mạng
  wifiMulti.addAP("Nang Luong Xanh", "nangluongxanh360");
  wifiMulti.addAP("SSID2", "password2");

  // Chờ kết nối WiFi thành công
  Serial.println("Connecting to WiFi...");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected to WiFi");

  // Chế độ WiFi là STA để sử dụng ESP-NOW
  WiFi.mode(WIFI_STA);
  
  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Đăng ký callback
  esp_now_register_send_cb(OnDataSent);

  // Thiết lập và thêm peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  myData.id = 1;
  myData.x = random(0,50);
  myData.y = random(0,50);
  
  Serial.print(myData.x);
  Serial.print(myData.y);

  // Gửi thông qua ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
  delay(2000);
}
