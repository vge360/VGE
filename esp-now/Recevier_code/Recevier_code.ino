#include <esp_now.h>
#include <WiFiMulti.h>  // Sử dụng thư viện WiFiMulti thay vì WiFi

WiFiMulti wifiMulti;

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

// Tạo một mảng chứa tất cả các cấu trúc
struct_message boardsStruct[3];

// Hàm callback sẽ được thực thi khi dữ liệu được nhận
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

void setup() {
  Serial.begin(115200);

  // Thêm các mạng WiFi
  wifiMulti.addAP("Nang Luong Xanh", "nangluongxanh360");
  wifiMulti.addAP("SSID2", "password2");

  // Chờ kết nối WiFi
  Serial.println("Đang kết nối WiFi...");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nĐã kết nối WiFi");

  // Khởi tạo ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    return;
  }

  // Đăng ký nhận callback khi có gói tin nhận được gửi
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Nơi để truy cập biến của từng bảng nếu cần
  // int board1X = boardsStruct[0].x;
  // int board1Y = boardsStruct[0].y;
  // int board2X = boardsStruct[1].x;
  // int board2Y = boardsStruct[1].y;
  // int board3X = boardsStruct[2].x;
  // int board3Y = boardsStruct[2].y;

  delay(1000);  // Thời gian trì hoãn để giảm tần suất thực thi vòng lặp
}
