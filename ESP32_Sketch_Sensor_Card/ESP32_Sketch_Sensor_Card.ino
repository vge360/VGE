// =======================Khai báo thư viện===========================
#include <WiFiMulti.h>
#include <esp_now.h>

WiFiMulti wifiMulti;

// REPLACE WITH YOUR RECEIVER MAC Address địa chỉ của người nhận
uint8_t broadcastAddress[] = {0xFC, 0xB4, 0x67, 0x92, 0x5C, 0x1C};

// ========================khai báo pinout==========================
///////////////////////////////////////////////////////////
const int analogInputPin1 = 36;
const int analogInputPin2 = 39;
const int analogInputPin3 = 34;

const int outputPin1 = 23;
const int outputPin2 = 22;
const int outputPin3 = 21;


// ================dữ liệu gửi đi====================
typedef struct struct_message {
  int id;
  int analogValue1;
  int analogValue2;
  int analogValue3;
} struct_message;

// Khai báo cấu trúc để lưu giữ dữ liệu gửi đi
struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t"); // trạng thái gửi gói cuối cùng
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"); // 
}
// ==========================setup=============================
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

  // Thêm thiết bị nhận vào danh sách peers
  memcpy(peerInfo.peer_addr, broadcastAddress, 6); 
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(outputPin1, OUTPUT);
  pinMode(outputPin2, OUTPUT);
  pinMode(outputPin3, OUTPUT);

}
//===========================vòng loop=============================
void loop() {
  //--------id---------
  myData.id = 2;

  // ----------khai bao bien--------------------
  int analogValue1 = analogRead(analogInputPin1);
  int analogValue2 = analogRead(analogInputPin2);
  int analogValue3 = analogRead(analogInputPin3);

  // Condition 1
  if (analogValue1 >= 650) {
    digitalWrite(outputPin1, HIGH);
  } else {
    digitalWrite(outputPin1, LOW);
  }

  // Condition 2
  if (analogValue2 <= 340) {
    digitalWrite(outputPin2, HIGH);
  } else {
    digitalWrite(outputPin2, LOW);
  }

  // Condition 3
  if (analogValue3 >= 2100) {
    digitalWrite(outputPin3, HIGH);
  } else {
    digitalWrite(outputPin3, LOW);
  }


  Serial.print("Analog Input 1: ");
  Serial.println();
  Serial.print("Analog Input 2: ");
  Serial.println(myData.analogValue2);
  Serial.print("Analog Input 3: ");
  Serial.println(myData.analogValue3);

  // gửi thông điệp qua ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }

  delay(1000);  
}
