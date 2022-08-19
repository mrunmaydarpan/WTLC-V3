/**
 * Advanced.ino
 * 
 * AsyncWiFiManager advanced demo, contains advanced configurartion options.
 * Implements TRIGGER_PIN button press, press for ondemand configportal, hold
 * for 3 seconds to reset settings.
 */

#define WM_ASYNC
#include <AsyncWiFiManager.h> // https://github.com/lbussy/AsyncWiFiManager

#define TRIGGER_PIN 0

void checkButton();
String getParam(String);
void saveParamCallback();

AsyncWiFiManager wm;                    // global wm instance
AsyncWiFiManagerParameter custom_field; // global param ( for non blocking w/params )

void setup() {
    // Put your setup code here, to run once:

    Serial.begin(115200);

    WiFi.mode(WIFI_STA); // Explicitly set mode, ESP defaults to STA+AP

    Serial.setDebugOutput(true);
    delay(3000);
    Serial.println("\n Starting");

    pinMode(TRIGGER_PIN, INPUT);

    // wm.resetSettings(); // wipe settings

    // Add a custom input field
    int customFieldLength = 40;

    // new (&custom_field) AsyncWiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");

    // Test custom html input type(checkbox)
    // new (&custom_field) AsyncWiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type

    // test custom html(radio)
    const char *custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
    new (&custom_field) AsyncWiFiManagerParameter(custom_radio_str); // custom html input

    wm.addParameter(&custom_field);
    wm.setSaveParamsCallback(saveParamCallback);

    // Custom menu via array or vector
    //
    // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
    // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
    // wm.setMenu(menu,6);
    std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
    wm.setMenu(menu);

    // set dark theme
    wm.setClass("invert");

    // Set static IP
    // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // Set static ip, gw, sn
    // wm.setShowStaticFields(true); // Force show static ip fields
    // wm.setShowDnsFields(true);    // Force show dns field always

    // wm.setConnectTimeout(20); // How long to try to connect before continuing
    wm.setConfigPortalTimeout(30); // Auto close configportal after n seconds
    // wm.setCaptivePortalEnable(false); // Disable captive portal redirection
    // wm.setAPClientCheck(true); // Avoid timeout if client connected to softap

    // WiFi scan settings
    // wm.setRemoveDuplicateAPs(false); // Do not remove duplicate AP names (true)
    // wm.setMinimumSignalQuality(20);  // Set min RSSI (percentage) to show in scans, null = 8%
    // wm.setShowInfoErase(false);      // Do not show erase button on info page
    // wm.setScanDispPerc(true);        // Show RSSI as percentage not graph icons

    // wm.setBreakAfterConfig(true);   // Always exit configportal even if WiFi save fails

    bool res;
    // res = wm.autoConnect(); // Auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // Anonymous AP
    res = wm.autoConnect("AutoConnectAP", "password"); // Password protected AP

    if (!res) {
        Serial.println("Failed to connect or hit timeout.");
        // ESP.restart();
    } else {
        // If you get here you have connected to the WiFi
        Serial.println("Connected.");
    }
}

void checkButton() {
    // Check for button press
    if (digitalRead(TRIGGER_PIN) == LOW) {
        // Poor-man's debounce/press-hold, code not ideal for production
        delay(50);
        if (digitalRead(TRIGGER_PIN) == LOW) {
            Serial.println("Button Pressed");
            // Still holding button for 3000 ms, reset settings, code not ideaa for production
            delay(3000); // Reset delay hold
            if (digitalRead(TRIGGER_PIN) == LOW) {
                Serial.println("Button held.  Erasing config and restarting.");
                wm.resetSettings();
                ESP.restart();
            }

            // Start portal w/delay
            Serial.println("Starting config portal.");
            wm.setConfigPortalTimeout(120);

            if (!wm.startConfigPortal("OnDemandAP", "password")) {
                Serial.println("Failed to connect or hit timeout.");
                delay(3000);
                // ESP.restart();
            } else {
                // If you get here you have connected to the WiFi
                Serial.println("Connected.");
            }
        }
    }
}

String getParam(String name) {
    // Read parameter from server, for custom HTML input
    String value;
    if (wm.server->hasArg(name)) {
        value = wm.server->arg(name);
    }
    return value;
}

void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired.");
    Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

void loop() {
    checkButton();
    // Put your main code here, to run repeatedly:
}
