#include <WiFi.h>

#define PORT  23
// Gioi han so luong clients ket noi
#define MAX_CLIENTS 3
const char* ssid = "haidinh"; // Đặt SSID của mạng Wi-Fi
const char* password = "123456789";  // Đặt mật khẩu của mạng Wi-Fi


IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Khoi tao port de clients ket noi.
WiFiServer server(PORT);
WiFiClient clients[MAX_CLIENTS];

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Setting soft-AP configuration ... ");

  //Cau hinh acces point, cai dat soft AP de client ket noi vao.
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);

  //In ra local_IP cua AP.
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Telnet server started");
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Lắng nghe cho các máy khách mới

  if (client) { // Nếu một máy khách mới kết nối
    Serial.println("New Client.");
    String currentLine = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          // Khi nhận một dòng mới từ máy khách
          if (currentLine.length() == 0) {
            // Gửi một phản hồi đến máy khách
            client.println("Hello from ESP32 Server!");
            break; // Thoát khỏi vòng lặp khi gửi phản hồi
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop(); // Ngắt kết nối với máy khách
    Serial.println("Client Disconnected.");
  }
}
