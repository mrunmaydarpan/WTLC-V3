// Basic.ino
// Simple autoConnect usage example

#define WM_ASYNC // Turn on Async mode
#include "AsyncWiFiManager.h" // https://github.com/lbussy/AsyncWiFiManager

void setup() {
    // Put your setup code here, to run once:

    Serial.begin(BAUD);

    WiFi.mode(WIFI_STA); // Explicitly set mode, ESP defaults to STA+AP
    
    // AsyncWiFiManager, Local intialization. Once its business is done,
    // there is no need to keep it around
    AsyncWiFiManager wm;

    // Reset settings - wipe credentials for testing
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name
    // ( "AutoConnectAP"), if empty will auto generate SSID, if password is
    // blank it will be an anonymous AP (wm.autoConnect()) then goes into a
    // blocking loop awaiting configuration and will return success result.

    bool res;
    // res = wm.autoConnect(); // Auto generated AP name from chipid, anonymous
    res = wm.autoConnect("AutoConnectAP"); // Anonymous AP
    // res = wm.autoConnect("AutoConnectAP","password"); // Password protected named AP

    if(!res) {
        Serial.println("Failed to connect.");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("Connected.");
    }
}

void loop() {
    // Put your main code here, to run repeatedly:
}
