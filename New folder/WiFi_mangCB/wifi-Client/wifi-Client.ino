#include <WiFi.h>

const char* ssid = "haidinh"; // Đặt SSID của mạng Wi-Fi
const char* password = "123456789";  // Đặt mật khẩu của mạng Wi-Fi

IPAddress server_ip(192, 168, 4, 1);
#define PORT  23
// port 23 la port cua esp8226 lam AP da khoi tao.
WiFiClient client;

void setup() {
  uint8_t i = 0;
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  // Kiem tra tình trang ket noi, neu chua ket noi duoc
  // se in chuoi "connecting..." tren man hinh serial terminal.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(500);
  }
}
unsigned long previousMillis = 0;
void loop() {
  WiFiClient client; // Tạo một đối tượng WiFiClient

  // if (!client.connect(host, 80)) { // Kết nối với máy chủ
  //   Serial.println("Connection to host failed");
  //   delay(1000);
  //   return;
  // }
  if (WiFi.status() == WL_CONNECTED) {
    // Kiem tra neu client(STA) chua duoc ket noi.
    // Kiem tra tiep tuc neu khong duoc ket noi den IP va PORT cua server(AP
    // thi in ra serial terminal chuoi "connection failed".
    while (!client.connected()) {
      if (!client.connect(server_ip, PORT)) {
        Serial.println("connection failed");
        delay(1000);
        return;
      }
    }

  client.println("hhhhhhhh!"); // Gửi dữ liệu đến máy chủ
  delay(10);

  // Đọc dữ liệu phản hồi từ máy chủ
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  client.stop(); // Ngắt kết nối
  delay(2000); // Chờ trước khi thực hiện lại vòng lặp
}
}
