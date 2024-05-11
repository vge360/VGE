#include "emonesp.h"
#include "wifi.h"
#include "app_config.h"
// #include "lcd.h"

#include <WiFi.h>         // Connect to Wifi
#include <ESPmDNS.h>      // Resolve URL for update server etc.
#include <DNSServer.h>    // Required for captive portal


DNSServer dnsServer;                  // Create class DNS server, captive portal re-direct
const byte DNS_PORT = 53;

// Access Point SSID, password & IP address. SSID will be softAP_ssid + chipID to make SSID unique
const char *softAP_ssid = "OpenEVSE";
const char *softAP_password = "openevse";
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
int apClients = 0;

// Wifi Network Strings
String connected_network = "";
String ipaddress = "";

int client_disconnects = 0;
bool client_retry = false;
unsigned long client_retry_time = 0;

#ifdef WIFI_LED
#ifndef WIFI_LED_ON_STATE
#define WIFI_LED_ON_STATE LOW
#endif

#ifndef WIFI_LED_AP_TIME
#define WIFI_LED_AP_TIME 1000
#endif

#ifndef WIFI_LED_AP_CONNECTED_TIME
#define WIFI_LED_AP_CONNECTED_TIME 100
#endif

#ifndef WIFI_LED_STA_CONNECTING_TIME
#define WIFI_LED_STA_CONNECTING_TIME 500
#endif

int wifiLedState = !WIFI_LED_ON_STATE;
unsigned long wifiLedTimeOut = millis();
#endif

#ifndef WIFI_BUTTON
#define WIFI_BUTTON 0
#endif

#ifndef WIFI_BUTTON_AP_TIMEOUT
#define WIFI_BUTTON_AP_TIMEOUT              (5 * 1000)
#endif

#ifndef WIFI_BUTTON_FACTORY_RESET_TIMEOUT
#define WIFI_BUTTON_FACTORY_RESET_TIMEOUT   (10 * 1000)
#endif

#ifndef WIFI_CLIENT_RETRY_TIMEOUT
#define WIFI_CLIENT_RETRY_TIMEOUT (5 * 60 * 1000)
#endif




int wifiButtonState = HIGH;
unsigned long wifiButtonTimeOut = millis();
bool apMessage = false;

// -------------------------------------------------------------------
// Start Access Point
// Access point is used for wifi network selection
// -------------------------------------------------------------------
void
startAP() {
  DBUGLN("Starting AP");

  if (wifi_mode_is_sta()) {
    WiFi.disconnect(true);
  }

  WiFi.enableAP(true);

  WiFi.softAPConfig(apIP, apIP, netMsk);

  
  // Khởi tạo SSID cho AP với ID chip để đảm bảo tính duy nhất
  String softAP_ssid_ID = String(softAP_ssid) + "_" + String((uint32_t)ESP.getEfuseMac(), HEX);

  // Chọn một kênh ngẫu nhiên từ các kênh 1, 6, 11
  int channel = (random(3) * 5) + 1;
  WiFi.softAP(softAP_ssid_ID.c_str(), softAP_password, channel);

  // Thiết lập máy chủ DNS, chuyển hướng tất cả các yêu cầu đến IP của AP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  // Lấy và hiển thị địa chỉ IP của AP
  IPAddress myIP = WiFi.softAPIP();
  char tmpStr[40];
  sprintf(tmpStr, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
  ipaddress = tmpStr;
  Serial.print("AP IP Address: ");
  Serial.println(tmpStr);
  // Các lệnh để hiển thị trên LCD có thể được thêm vào đây nếu bạn sử dụng một module LCD
  // lcd_display(F("SSID: OpenEVSE"), 0, 0, 0, LCD_CLEAR_LINE);
  // lcd_display(F("Pass: openevse"), 0, 1, 15 * 1000, LCD_CLEAR_LINE);

  apClients = 0;

}

// -------------------------------------------------------------------
// Start Client, attempt to connect to Wifi network
// -------------------------------------------------------------------
void
startClient()
{
  DEBUG.print("Connecting to SSID: ");
  DEBUG.println(esid.c_str());
  // DEBUG.print(" epass:");
  // DEBUG.println(epass.c_str());

  client_disconnects = 0;

  WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.setHostname(esp_hostname.c_str()); 

  // Bật chế độ Station (STA)
  WiFi.mode(WIFI_STA);
  WiFi.enableSTA(true);
}

static void wifi_start()
{
  // 1) If no network configured start up access point
  if (esid == 0 || esid == "")
  {
    startAP();
  }
  // 2) else try and connect to the configured network
  else
  {
    startClient();
  }
}
//////////////////////////////////////////////////////////////////////////////////
//========================== Hàm kết nối wifi =================================
/////////////////////////////////////////////////////////////////////////////////

void wifi_onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  #ifdef WIFI_LED
    wifiLedState = WIFI_LED_ON_STATE;
    digitalWrite(WIFI_LED, wifiLedState);
  #endif

  IPAddress myAddress = WiFi.localIP();
  char tmpStr[40];
  sprintf(tmpStr, "%d.%d.%d.%d", myAddress[0], myAddress[1], myAddress[2], myAddress[3]);
  ipaddress = tmpStr;
  DEBUG.print("Connected, IP: ");
  DEBUG.println(tmpStr);
  // lcd_display(F("IP Address"), 0, 0, 0, LCD_CLEAR_LINE);
  // lcd_display(tmpStr, 0, 1, 5000, LCD_CLEAR_LINE);

  // Copy the connected network and ipaddress to global strings for use in status request
  connected_network = esid;

  // Clear any error state
  client_disconnects = 0;
  client_retry = false;
}
//////////////////////////////////////////////////////////////////////////////////
//========================== Hàm Lỗi kết nối =================================
/////////////////////////////////////////////////////////////////////////////////
void wifi_onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  DBUGF("WiFi dissconnected: %s",
  WIFI_DISCONNECT_REASON_UNSPECIFIED == event.reason ? "WIFI_DISCONNECT_REASON_UNSPECIFIED" :
  WIFI_DISCONNECT_REASON_AUTH_EXPIRE == event.reason ? "WIFI_DISCONNECT_REASON_AUTH_EXPIRE" :
  WIFI_DISCONNECT_REASON_AUTH_LEAVE == event.reason ? "WIFI_DISCONNECT_REASON_AUTH_LEAVE" :
  WIFI_DISCONNECT_REASON_ASSOC_EXPIRE == event.reason ? "WIFI_DISCONNECT_REASON_ASSOC_EXPIRE" :
  WIFI_DISCONNECT_REASON_ASSOC_TOOMANY == event.reason ? "WIFI_DISCONNECT_REASON_ASSOC_TOOMANY" :
  WIFI_DISCONNECT_REASON_NOT_AUTHED == event.reason ? "WIFI_DISCONNECT_REASON_NOT_AUTHED" :
  WIFI_DISCONNECT_REASON_NOT_ASSOCED == event.reason ? "WIFI_DISCONNECT_REASON_NOT_ASSOCED" :
  WIFI_DISCONNECT_REASON_ASSOC_LEAVE == event.reason ? "WIFI_DISCONNECT_REASON_ASSOC_LEAVE" :
  WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED == event.reason ? "WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED" :
  WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD == event.reason ? "WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD" :
  WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD == event.reason ? "WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD" :
  WIFI_DISCONNECT_REASON_IE_INVALID == event.reason ? "WIFI_DISCONNECT_REASON_IE_INVALID" :
  WIFI_DISCONNECT_REASON_MIC_FAILURE == event.reason ? "WIFI_DISCONNECT_REASON_MIC_FAILURE" :
  WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT == event.reason ? "WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT" :
  WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT == event.reason ? "WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT" :
  WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS == event.reason ? "WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS" :
  WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID == event.reason ? "WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID" :
  WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID == event.reason ? "WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID" :
  WIFI_DISCONNECT_REASON_AKMP_INVALID == event.reason ? "WIFI_DISCONNECT_REASON_AKMP_INVALID" :
  WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION == event.reason ? "WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION" :
  WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP == event.reason ? "WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP" :
  WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED == event.reason ? "WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED" :
  WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED == event.reason ? "WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED" :
  WIFI_DISCONNECT_REASON_BEACON_TIMEOUT == event.reason ? "WIFI_DISCONNECT_REASON_BEACON_TIMEOUT" :
  WIFI_DISCONNECT_REASON_NO_AP_FOUND == event.reason ? "WIFI_DISCONNECT_REASON_NO_AP_FOUND" :
  WIFI_DISCONNECT_REASON_AUTH_FAIL == event.reason ? "WIFI_DISCONNECT_REASON_AUTH_FAIL" :
  WIFI_DISCONNECT_REASON_ASSOC_FAIL == event.reason ? "WIFI_DISCONNECT_REASON_ASSOC_FAIL" :
  WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT == event.reason ? "WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT" :
  "UNKNOWN");

  client_disconnects++;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//========================== Hàm cấu hình mạng và khỏi động thiết bị =================================
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
// wifi_setup() 
setup(){
#ifdef WIFI_LED
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, wifiLedState);
#endif

  randomSeed(analogRead(0));

  // If we have an SSID configured at this point we have likely
  // been running another firmware, clear the results
  if(wifi_is_client_configured()) {
    WiFi.persistent(true);
    WiFi.disconnect(true);
    // ESP.eraseConfig();
  }

  // Stop the WiFi module
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
            // When connected to an AP, you typically cannot access SSID directly from the info struct,
            // so you retrieve it from the WiFi class.
            Serial.printf("Connected to SSID: %s\n", WiFi.SSID().c_str());
        }
  });
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
            wifi_onStationModeGotIP(event, info);  // Call your handler function
        }
  });
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            wifi_onStationModeDisconnected(event, info);  // Call your handler function
        }
  });
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
            apClients++; // Increment the number of connected clients
            Serial.printf("Device connected to SoftAP, total connected: %d\n", apClients);
            // Optionally display IP or other info. Here just showing count.
        }
  });
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
            apClients--; // Decrement the number of connected clients
            Serial.printf("Device disconnected from SoftAP, total connected: %d\n", apClients);
            // You might want to handle reconnections or clean-ups here.
        }
  });

  wifi_start();

  if (MDNS.begin(esp_hostname.c_str())) {
    MDNS.addService("http", "tcp", 80);
  }
}
//////////////////////////////////////////////////////////////////////////////////
//========================== Hàm Loop =================================
/////////////////////////////////////////////////////////////////////////////////
void
//wifi_loop() 
loop(){
  Profile_Start(wifi_loop);

//  bool isClient = wifi_mode_is_sta();
  bool isClientOnly = wifi_mode_is_sta_only();
//  bool isAp = wifi_mode_is_ap();
  bool isApOnly = wifi_mode_is_ap_only();

#ifdef WIFI_LED
  if ((isApOnly || !WiFi.isConnected()) &&
      millis() > wifiLedTimeOut)
  {
    wifiLedState = !wifiLedState;
    digitalWrite(WIFI_LED, wifiLedState);

    int ledTime = isApOnly ? (0 == apClients ? WIFI_LED_AP_TIME : WIFI_LED_AP_CONNECTED_TIME) : WIFI_LED_STA_CONNECTING_TIME;
    wifiLedTimeOut = millis() + ledTime;
  }
#endif

#if defined(WIFI_LED) && WIFI_BUTTON == WIFI_LED
  digitalWrite(WIFI_BUTTON, HIGH);
  pinMode(WIFI_BUTTON, INPUT_PULLUP);
#endif

  // Pressing the boot button for 5 seconds will turn on AP mode, 10 seconds will factory reset
  int button = digitalRead(WIFI_BUTTON);

#if defined(WIFI_LED) && WIFI_BUTTON == WIFI_LED
  pinMode(WIFI_BUTTON, OUTPUT);
  digitalWrite(WIFI_LED, wifiLedState);
#endif

  //DBUGF("%lu %d %d", millis() - wifiButtonTimeOut, button, wifiButtonState);
  if(wifiButtonState != button)
  {
    wifiButtonState = button;
    if(LOW == button) {
      DBUGF("Button pressed");
      wifiButtonTimeOut = millis();
      apMessage = false;
    } else {
      DBUGF("Button released");
      if(millis() > wifiButtonTimeOut + WIFI_BUTTON_AP_TIMEOUT) {
        startAP();
      }
    }
  }

  if(LOW == wifiButtonState && millis() > wifiButtonTimeOut + WIFI_BUTTON_FACTORY_RESET_TIMEOUT)
  {
    // lcd_display(F("Factory Reset"), 0, 0, 0, LCD_CLEAR_LINE);
    // lcd_display(F(""), 0, 1, 10 * 1000, LCD_CLEAR_LINE);
    // lcd_loop();

    delay(1000);

    // config_reset();
    // ESP.eraseConfig();
    WiFi.disconnect(true);

    delay(50);
    // ESP.reset();
    ESP.restart();
  }
  else if(false == apMessage && LOW == wifiButtonState && millis() > wifiButtonTimeOut + WIFI_BUTTON_AP_TIMEOUT) 
  {
    // lcd_display(F("Access Point"), 0, 0, 0, LCD_CLEAR_LINE);
    // lcd_display(F(""), 0, 1, 10 * 1000, LCD_CLEAR_LINE);
    // lcd_loop();
    apMessage = true;
  }

  // Manage state while connecting
  if(isClientOnly && !WiFi.isConnected())
  {
    // If we have failed to connect turn on the AP
    if(client_disconnects > 2) {
      startAP();
      client_retry = true;
      client_retry_time = millis() + WIFI_CLIENT_RETRY_TIMEOUT;
    }
  }

  // Remain in AP mode for 5 Minutes before resetting
  if(isApOnly && 0 == apClients && client_retry && millis() > client_retry_time) {
    DEBUG.println("client re-try, resetting");
    delay(50);
    // ESP.reset();
    ESP.restart();
  }

  dnsServer.processNextRequest(); // Captive portal DNS re-dierct

  Profile_End(wifi_loop, 5);
}

void
wifi_restart() {
  wifi_disconnect();
  wifi_start();
}

void
wifi_disconnect() {
  wifi_turn_off_ap();
  if (wifi_mode_is_sta()) {
    WiFi.disconnect(true);
  }
}

void wifi_turn_off_ap()
{
  if(wifi_mode_is_ap())
  {
    WiFi.softAPdisconnect(true);
    dnsServer.stop();
  }
}

void wifi_turn_on_ap()
{
  DBUGF("wifi_turn_on_ap %d", WiFi.getMode());
  if(!wifi_mode_is_ap()) {
    startAP();
  }
}

bool wifi_client_connected()
{
  return WiFi.isConnected() && (WIFI_STA == (WiFi.getMode() & WIFI_STA));
}
