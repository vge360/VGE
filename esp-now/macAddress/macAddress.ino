#include <WiFiMulti.h>

WiFiMulti wifiMulti;

void setup() {
  Serial.begin(115200);

  // Khởi động WiFiMulti và thêm các mạng WiFi
  wifiMulti.addAP("Nang Luong Xanh", "nangluongxanh360");
  wifiMulti.addAP("SSID2", "password2");

  Serial.println("Connecting to WiFi...");
  // Chờ đợi cho đến khi kết nối thành công
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // In địa chỉ MAC của thiết bị
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Có thể thêm mã xử lý ở đây nếu cần
}
