#include <Arduino.h>
#ifdef ARDUINO_M5Stack_ATOM
#include <M5Atom.h>
#else
#include <M5Stack.h>
#include <M5StackUpdater.h>
#endif
#include <WiFi.h>
#include <Preferences.h>

typedef enum CmdStat_T {
  CMD_STAT_INITIAL = 0,
  CMD_STAT_WAIT_SSID,
  CMD_STAT_WAIT_PASSWD,
} CmdStat;


static void showWiFiSetting() {
  Preferences preferences;

  {
    preferences.begin("wifi-config");
    String ssid = preferences.getString("WIFI_SSID");
    String passwd = preferences.getString("WIFI_PASSWD");
    preferences.end();

    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PASS: ");
    Serial.println(passwd);
  }

  {
    char wifi_ssid[37] = {0};
    char wifi_key[66] = {0};
    preferences.begin("nvs.net80211", true);
    preferences.getBytes("sta.ssid", wifi_ssid, sizeof(wifi_ssid));
    preferences.getBytes("sta.pswd", wifi_key, sizeof(wifi_key));
    preferences.end();

    if (wifi_ssid[0] != 0 && wifi_ssid[4] != 0) {
      Serial.println("Latest connected WiFi parameters:");
      Serial.printf("SSID: %s\n", &wifi_ssid[4]);
      Serial.printf("PASS: %s\n", wifi_key);
    }
  }
}

static void showInitial() {
  Serial.println("------------------");
  Serial.println("1 -- Scan WiFi");
  Serial.println("2 -- Set WiFi");
  Serial.println("3 -- WiFi Connection Test");
  Serial.println("4 -- Clear WiFi configurations");
  Serial.println("0 -- Deep sleep");
  Serial.println("Type command number [1-4,0]:");
}

static void scanWiFi() {
  Serial.print("Scan WiFi ... ");
#if ARDUINO_M5Stack_ATOM
  M5.dis.fillpix(0x707070);
  M5.update();
#endif

  const int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No network found");
#if ARDUINO_M5Stack_ATOM
    M5.dis.fillpix(0x700000);
#endif
  } else {
    Serial.printf("%d networks found!\n", n);

    for (int i = 0; i < n; i++) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d: ", i + 1);
      Serial.print(WiFi.SSID(i));
      Serial.printf(" (%d) ", WiFi.RSSI(i));
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
    }
#if ARDUINO_M5Stack_ATOM
    M5.dis.fillpix(0x700000);
#endif
  }
}

static void testWiFiConnection() {
  Serial.println("");

  Preferences preferences;
  preferences.begin("wifi-config");
  String ssid = preferences.getString("WIFI_SSID");
  String passwd = preferences.getString("WIFI_PASSWD");
  preferences.end();

  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("PASS: ");
  Serial.println(passwd);

  if (ssid == "") {
    Serial.println("SSID is not set. Cancelled.");
    return;
  }

  Serial.println("WiFi connection test");
#if ARDUINO_M5Stack_ATOM
  M5.dis.fillpix(0x707070);
  M5.update();
#endif

  for (uint8_t i = 0; i < 30 * 2 * 3; i++) {
    Serial.print(".");
    if (i % (30 * 2) == 0) {
      WiFi.disconnect(true);
      delay(1);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), passwd.c_str());
    }
    delay(500);
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }}

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected.");
    Serial.print("  IP Address :  ");
    Serial.println(WiFi.localIP());
    Serial.print("  Subnet Mask : ");
    Serial.println(WiFi.subnetMask());
    Serial.print("  Gateway :     ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("  DNS :         ");
    Serial.println(WiFi.dnsIP());
  } else {
    Serial.println(" Failed to connect WiFi.");
  }
  WiFi.disconnect(true);
  delay(100);
  Serial.println("WiFi disabled.");

#if ARDUINO_M5Stack_ATOM
  M5.dis.fillpix(0x700000);
#endif
}

void setup() {
  //Serial.begin(115200); // start serial for output
  Serial.print("initializing...");

#if ARDUINO_M5Stack_ATOM
  M5.begin(true, false, true);
  //Wire.begin(26, 32);
  delay(1);
  M5.dis.clear();
  M5.dis.fillpix(0x707070);
  M5.update();
#else
  //---osmar
  M5.begin();
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  //---osmar

  //Wire.begin();

  // mute speaker
  M5.Speaker.begin();
  M5.Speaker.mute();

  M5.Lcd.println("Connect USB Serial with bow rate 115200bps");
#endif

  showWiFiSetting();
  scanWiFi();
  showInitial();
}

void loop() {
  static CmdStat cmdStatus = CMD_STAT_INITIAL;
  bool transit = false;

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    switch (cmdStatus) {
      case CMD_STAT_INITIAL:
        if (command == "1") {
          scanWiFi();
          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        } else if (command == "2") {
          cmdStatus = CMD_STAT_WAIT_SSID;
          transit = true;
        } else if (command == "3") {
          testWiFiConnection();
          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        } else if (command == "4") {
          Preferences preferences;

          preferences.begin("wifi-config");
          if ( preferences.clear() ) {
            Serial.println("wifi-config clear ... done");
          } else {
            Serial.println("wifi-config clear ... failed");
          }
          preferences.end();

          preferences.begin("nvs.net80211");
          if ( preferences.clear() ) {
            Serial.println("Latest connected WiFi config (nvs.net80211) clear ... done");
          } else {
            Serial.println("Latest connected WiFi config (nvs.net80211) clear ... failed");
          }
          preferences.end();

          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        } else if (command == "0") {
#if ARDUINO_M5Stack_ATOM
          M5.dis.clear();
          M5.update();
          esp_sleep_enable_ext0_wakeup((gpio_num_t)39, LOW);
#else
          M5.Lcd.setBrightness(0);
          M5.Lcd.sleep();
          esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_B_PIN, LOW);
#endif
          Serial.println("Deep sleep");
          delay(10);
          esp_deep_sleep_start();
          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        }
        break;
      case CMD_STAT_WAIT_SSID:
        if (command != "") {
          Serial.println(command);
          Preferences preferences;
          preferences.begin("wifi-config");
          preferences.putString("WIFI_SSID", command);
          preferences.end();
          cmdStatus = CMD_STAT_WAIT_PASSWD;
          transit = true;
        } else {
          Serial.println("Cancelled");
          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        }
        break;
      case CMD_STAT_WAIT_PASSWD:
        if (command != "") {
          Serial.println(command);
          Preferences preferences;
          preferences.begin("wifi-config");
          preferences.putString("WIFI_PASSWD", command);
          preferences.end();
          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        } else {
          Serial.println("Cancelled");
          cmdStatus = CMD_STAT_INITIAL;
          transit = true;
        }
        break;
    }
  }

  if (transit) {
    switch (cmdStatus) {
      case CMD_STAT_INITIAL:
        showInitial();
        break;
      case CMD_STAT_WAIT_SSID:
        Serial.print("SSID? ");
        break;
      case CMD_STAT_WAIT_PASSWD:
        Serial.print("PASS? ");
        break;
    }
  }

  M5.update();
}
