#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiManager.h>
#include <ESP_DoubleResetDetector.h>
#include <ArduinoJson.h>

#define JSON_CONFIG_FILE "/wifi1_config.json"
#define DRD_TIMEOUT 10
#define DRD_ADDRESS 0
// ======================================================================================
// reset đôi
DoubleResetDetector *drd;
//flag for saving data
bool shouldSaveConfig = false;

// lưu trữ thông tin config 
char testString[50] = "deafult value";
int testNumber = 1500;
bool testBool = true;
int day = 3; // 0 == Monday etc
char *daysOfWeek[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

//===================== hàm config =====================================

void saveConfigFile()
{
  Serial.println(F("Saving config"));
  StaticJsonDocument<512> json;
  json["testString"] = testString;
  json["testNumber"] = testNumber;
  json["testBool"] = testBool;
  json["day"] = day;

  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
  }

  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  configFile.close();
}

// ================= hàm gọi lại file ===============================

bool loadConfigFile()
{

  Serial.println("mounting FS...");

  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("opened config file");
        StaticJsonDocument<512> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("\nparsed json");

          strcpy(testString, json["testString"]);
          testNumber = json["testNumber"].as<int>();
          testBool = json["testBool"].as<bool>();
          day = json["day"].as<int>();

          return true;
        }
        else
        {
          Serial.println("failed to load json config");
        }
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  //end read
  return false;
}

// ================= Thông báo lưu lại cấu hình ===============================
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered Conf Mode");

  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());
}

//======================= điều chỉnh HTML ===================================
String getCustomParamValue(WiFiManager *myWiFiManager, String name)
{
  String value;

  int numArgs = myWiFiManager->server->args();
  for (int i = 0; i < numArgs; i++) {
    Serial.println(myWiFiManager->server->arg(i));
  }
  if (myWiFiManager->server->hasArg(name))
  {
    value = myWiFiManager->server->arg(name);
  }
  return value;
}


void net_manager_setup()
{
    bool forceConfig = false;

    drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
    if (drd->detectDoubleReset())
    {
        Serial.println(F("thiết lập cấu hình "));
        forceConfig = true;
    }

    bool spiffsSetup = loadConfigFile();
    if (!spiffsSetup)
    {
        Serial.println(F("Forcing config mode as there is no saved config"));
        forceConfig = true;
    }

    WiFiManager wm;

    wm.setSaveConfigCallback(saveConfigCallback);
    // gọi lại khi kết nối wifi trước đó không thành công
    wm.setAPCallback(configModeCallback);

    WiFiManagerParameter custom_text_box("key_text", "Enter your string here", testString, 50);

    // Text box (Number)
    char convertedValue[6];
    sprintf(convertedValue, "%d", testNumber); // Need to convert to string to display a default value.

    WiFiManagerParameter custom_text_box_num("key_num", "Enter your number here", convertedValue, 7); // 7 == max length

    //Check Box
    char *customHtml;
    if (testBool)
    {
        customHtml = "type=\"checkbox\" checked";
    }
    else
    {
        customHtml = "type=\"checkbox\"";
    }
    WiFiManagerParameter custom_checkbox("key_bool", "Checkbox", "T", 2, customHtml);

    if (forceConfig)
    {
        if (!wm.startConfigPortal("WifiVGE1", "clock123"))
        {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
        }
    }
    else
    {
        if (!wm.autoConnect("WifiVGE1", "clock123"))
        {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        // if we still have not connected restart and try all over again
        ESP.restart();
        delay(5000);
        }
    }

    // thêm các thông số của bạn ở đây
    //add all your parameters here
    wm.addParameter(&custom_text_box);
    wm.addParameter(&custom_text_box_num);
    wm.addParameter(&custom_checkbox);
    // wm.addParameter(&custom_hidden); // Needs to be added before the javascript that hides it
    // wm.addParameter(&custom_field);

      // lưu vào fs
    if (shouldSaveConfig)
    {
        // Lets deal with the user config values

        // Copy the string value
        strncpy(testString, custom_text_box.getValue(), sizeof(testString));
        Serial.print("testString: ");
        Serial.println(testString);

        //Convert the number value
        testNumber = atoi(custom_text_box_num.getValue());
        Serial.print("testNumber: ");
        Serial.println(testNumber);

        //Handle the bool value
        testBool = (strncmp(custom_checkbox.getValue(), "T", 1) == 0);
        Serial.print("testBool: ");
        if (testBool)
        {
        Serial.println("true");
        }
        else
        {
        Serial.println("false");
        }

        saveConfigFile();
    }
}


#endif