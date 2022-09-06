const char *min_ = "min";
const char *max_ = "max";
const char *thres_ = "threshold";
const char *stator_ = "stator";

// URI Set
#define HOME_URI "/"
#define SETTING_URI "/setting"
#define RESET_URI "/rst"
#define RESET_WIFI_URI "/rstwifi"

#ifdef WM_SET
void configModeCallback(AsyncWiFiManager *myAsyncWiFiManager)
{
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(5, 22);
    display.println("SETUP WIFI");
    display.display();
    delay(2000);
    debugln("Entered config mode");
}

void save_callback()
{
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(5, 23);
    display.println("WIFI SAVED");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(5, 23);
    display.println("REBOOTING");
    display.display();
    delay(1000);
    ESP.restart();
}

#if HA_INIT
void saveParamsCallback()
{
    StaticJsonDocument<200> doc;
    doc["mqtt_server"] = custom_mqtt_server.getValue();
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("Failed to open config file for writing");
    }
    serializeJson(doc, configFile);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(5, 17);
    display.println("SERVER:");
    display.setCursor(5, 37);
    display.print(custom_mqtt_server.getValue());
    display.display();
    delay(1000);
}
#endif
#endif

void WIFI_CONNECT()
{
    // wm.resetSettings();
#ifdef WM_SET
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(0, 16);
    display.println("CONNECTING");
    display.setCursor(0, 35);
    display.setTextSize(2);
    display.print("WiFi");
    display.display();
    delay(1000);
#endif
#ifdef WM_SET
    WiFi.mode(WIFI_AP_STA);
    wm.setConfigPortalBlocking(false);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(device_name);
#if HA_INIT
    wm.setSaveConfigCallback(save_callback);
    wm.setSaveParamsCallback(saveParamsCallback);
    wm.addParameter(&custom_mqtt_server);
#endif
    // wm.setAPCallback(configModeCallback);
    if (wm.autoConnect(device_name)) // if Connected successfully
    {
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setFont(NULL);
        display.setCursor(5, 17);
        display.println("CONNECTED TO");
        display.setCursor(5, 30);
        display.print(WiFi.SSID());
        display.setCursor(5, 43);
        display.print(WiFi.localIP());
        display.drawBitmap(90, 20, done_icon, 24, 24, 1);
        display.display();
        delay(2000);
#if HA_INIT
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setFont(NULL);
        display.setCursor(5, 17);
        display.println("SERVER:");
        display.setCursor(5, 37);
        display.print(BROKER_ADDR);
        display.display();
#endif
    }
    else
    { // if not connected
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setFont(NULL);
        display.setCursor(0, 17);
        display.println("WIFI NOT  CONNECTED");
        display.display();
        delay(1000);
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setFont(NULL);
        display.setCursor(5, 22);
        display.println("SETUP WIFI");
        display.display();
    }
    delay(1000);

#else
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(device_name);
#endif
}

String home_processor(const String &var)
{
    if (var == "level_req")
        return String(value);
    if (var == "rst_wifi_set")
#ifdef WM_SET
    {
        String s = "";
        s = "<form action=\"/rstwifi\">";
        s += "<button class=\"rst\">Reset WiFi</button>";
        s += "</form>";
        s += "<br>";
        return s;
    }
#else
    {
        return "";
    }
#endif
    return String();
}

String setting_proccesor(const String &var)
{
    if (var == "max_height")
    {
        return String(MaxDistance);
    }
    if (var == "min_height")
    {
        return String(MinDistance);
    }
    if (var == "start_at")
    {
        return String(MotorStartThreshold);
    }
    switch (STATOR_TYPE)
    {
    case 1:
        if (var == "selector_1")
            return "selected";
        break;
    case 2:
        if (var == "selector_2")
            return "selected";
        break;
    case 3:
        if (var == "selector_3")
            return "selected";
        break;
    default:
        break;
    }
    return String();
}

String css_proccesor(const String &var)
{
    return String();
}

static const char rebootSend[] PROGMEM =
    "<META http-equiv=\"refresh\" content=\"10;URL=/\">Rebooting...";

void setting_code()
{
    server.onNotFound([]()
                      { TP.processAndSend("home.html", home_processor); });
    server.on(HOME_URI, HTTP_GET, []()
              { TP.processAndSend("home.html", home_processor); });
    server.on("/style.css", HTTP_GET, []()
              { TP.processAndSend("style.css", css_proccesor); });
    server.on(SETTING_URI, HTTP_GET, []()
              {
                  debugln("setting pages");
                  TP.processAndSend("setting_html.html",setting_proccesor); });
    server.on("/get_setting", HTTP_GET, []()
              {
                  StaticJsonDocument<200> doc;
                  doc["min"] = MinDistance;
                  doc["max"] = MaxDistance;
                  doc["threshold"] = MotorStartThreshold;
                  doc["starter"] = STATOR_TYPE;
                  String reply;
                  serializeJsonPretty(doc,reply);
                  server.send(200,"text/json",reply); });

    // server.on("/save", HTTP_GET, []()
    server.on("/set", HTTP_GET, []()
              {
                  int min;
                  int max;
                  int threshold;
                  int stator;
                  String reply;
                if (server.hasArg(min_) && server.hasArg(max_) && server.hasArg(thres_) && server.hasArg(stator_))
                  {
                      min = server.arg(min_).toInt();
                      max = server.arg(max_).toInt();
                      threshold = server.arg(thres_).toInt();
                      stator = server.arg(stator_).toInt();

                      if (min == 0 || max == 0 || threshold == 0)
                      {
                          reply = "error";
                          server.send(201, "text/plain", reply);
                      }
                      else if (min < max && threshold <= 70 && threshold >= 20 && stator < 4 && stator != 0)
                      {
                        EEPROM.write(minDistance_mem, min);
                        EEPROM.write(maxDistance_mem, max);
                        EEPROM.write(MotorStartThreshold_mem, threshold);
                        EEPROM.write(StatorType_mem, stator);
                        EEPROM.commit();
                        Serial.print("saved data");
                        server.send(200, "text/html", rebootSend);
                        delay(2000);
                        ESP.restart();
                      }
                  }
                  else if(server.hasArg("pump")) //control pump from app
                  {
                      MotorState = server.arg("pump").toInt();
                      server.send(203,"text/plain","ok");
                  }
                  else if(server.hasArg("mode")) //change mode from app
                  {
                      AutoMode = server.arg("mode").toInt();
                      server.send(204,"text/plain","ok");
                  }
                  else
                  {
                      reply = "No message sent";
                      server.send(202, "text/plain", reply);
                  } });
    server.on(RESET_URI, HTTP_GET, []()
              { 
              server.send(200, "text/html", rebootSend);
              delay(2000);
              ESP.restart(); });
#if WM_set
    server.on(RESET_WIFI_URI, HTTP_GET, []()
              {
                server.send(200, "text/html", rebootSend);
                wm.resetSettings();
                delay(2000);
                ESP.restart(); });
#endif
    // server.on("get_level", HTTP_GET, []()
    //           { server.send(200, "text/plain", String(value)); });
    if (!MDNS.begin("mdtronix-wtlc"))
    {
        debugln("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    debugln("mDNS responder started");
    httpUpdater.setup(&server);
    server.begin();
    MDNS.addService("http", "tcp", 80);
    dnsServer.start(DNS_PORT, "*", apIP);
}
#if HA_INIT
void set_device()
{
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac)); // Unique ID must be set!
    device.setName(DEVICE_NAME);
    device.setSoftwareVersion(DEVICE_VERSION);
    device.setManufacturer(DEVICE_MANUFACTURER);
    device.setModel(DEVICE_MODEL);
    device.enableSharedAvailability();
    device.enableLastWill();
    pump_HA.onStateChanged(pump_action); // handle switch state
    pump_HA.setName(pump_name);
    pump_HA.setIcon("mdi:water");
    value_HA.setName(level_name);
    value_HA.setIcon("mdi:waves");
    value_HA.setUnitOfMeasurement("%");
    distance_HA.setName(distance_name);
    distance_HA.setIcon("mdi:ruler");
    distance_HA.setUnitOfMeasurement("cm");
    mode_HA.onStateChanged(mode_action);
    mode_HA.setName(mode_name);
    mode_HA.setIcon("mdi:nintendo-switch");
    sensor_error_HA.setName(SensorError_name);

    IPAddress HA_IP;
    HA_IP.fromString(BROKER_ADDR);
    mqtt_HA.begin(HA_IP, BROKER_USER, BROKER_PASS);
}
#endif