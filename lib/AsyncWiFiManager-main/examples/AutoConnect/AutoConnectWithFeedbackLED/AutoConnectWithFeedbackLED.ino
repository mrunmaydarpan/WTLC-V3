// AutoConnectWithFeedbackLED.ino
//
// LED will blink when in config mode

#define WM_ASYNC
#include "AsyncWiFiManager.h" // https://github.com/lbussy/AsyncWiFiManager

// for LED status
#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;

void tick()
{
    // Toggle state
    digitalWrite(LED, !digitalRead(LED)); // set pin to the opposite state
}

// Gets called when AsyncWiFiManager enters configuration mode
void configModeCallback(AsyncWiFiManager *myAsyncWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    // If you used auto generated SSID, print it
    Serial.println(myAsyncWiFiManager->getConfigPortalSSID());
    // Entered config mode, make led toggle faster
    ticker.attach(0.2, tick);
}

void setup()
{
    // Put your setup code here, to run once:
    WiFi.mode(WIFI_STA); // Explicitly set mode, esp defaults to STA+AP
    Serial.begin(115200);

    // Set led pin as output
    pinMode(LED, OUTPUT);
    // start ticker with 0.5 because we start in AP mode and try to connect
    ticker.attach(0.6, tick);

    // AsyncWiFiManager: Local intialization. Once its business is done,
    // there is no need to keep it around
    AsyncWiFiManager wm;
    // Reset settings - for testing
    // wm.resetSettings();

    // Set callback that gets called when connecting to previous WiFi fails,
    // and enters Access Point mode
    wm.setAPCallback(configModeCallback);

    // Fetches ssid and pass and tries to connect. If it does not connect it
    // starts an access point with the specified name, here "AutoConnectAP,"
    // and goes into a blocking loop awaiting configuration.
    if (!wm.autoConnect())
    {
        Serial.println("Failed to connect and hit timeout.");
        // Reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(1000);
    }

    // If you get here you have connected to the WiFi
    Serial.println("Connected.");
    ticker.detach();
    // Keep LED on
    digitalWrite(LED, LOW);
}

void loop()
{
    // Put your main code here, to run repeatedly:
}
