void StartUp()
{
#if !OLED
    const char *compile_date = __DATE__;
#endif
    Serial.println(F("Ready......."));
    Serial.println("SW: " + String(_VERSION));
#if !OLED
    Serial.println("DT: " + String(compile_date));
#endif
    Serial.println("MaxDistance: " + String(MaxDistance));
    Serial.println("MinDistance: " + String(MinDistance));
    Serial.println("start at: " + String(MotorStartThreshold));
    Serial.println("Stator: " + String(STATOR_TYPE));
    Serial.println("ManualOff: " + String(EEPROM.read(manualOff_mem)));
    Serial.println("MotorState: " + String(EEPROM.read(motorState_mem)));
    Serial.println("Mode: " + String(EEPROM.read(AutoMode_mem)));
    Serial.println(F("Starting."));
    pinMode(buzz, OUTPUT);
#ifdef WM_SET
    if (digitalRead(PB) == LOW)
    {
        wm.resetSettings();
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setFont(NULL);
        display.setCursor(5, 17);
        display.println("RESET WIFI");
        display.setTextSize(1);
        display.setCursor(5, 44);
        display.print("WAIT FOR RESTART");
        display.display();
        delay(2000);
        ESP.restart();
    }
#endif
    display.clearDisplay();
    delay(500);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(4, 4);
    display.println("SW:          HW:");
    display.setCursor(22, 4);
    display.print(_VERSION);
    display.setCursor(101, 4);
    display.print(_HARDWARE);
    display.setCursor(80, 20);
    display.print("SENSOR:");
    display.setCursor(122, 20);
    display.print(SENSOR_DISP);
    display.setCursor(92, 32);
    display.print("MODE:");
    display.setCursor(122, 32);
    display.print(STATOR_TYPE);
    // display.drawRect(0, 0, 128, 16, 1);
    display.drawBitmap(4, 16, mdtronix_icon, 63, 48, 1);
    display.setCursor(79, 55);
    display.print("MDtronix");
    display.display();
    delay(3000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(4, 16);
    display.println("MIN:");
    display.setCursor(29, 16);
    display.print(MinDistance);
    display.setCursor(4, 30);
    display.print("MAX:");
    display.setCursor(29, 30);
    display.print(MaxDistance);
    display.setCursor(4, 44);
    display.print("START AT: ");
    display.setCursor(59, 44);
    display.print(MotorStartThreshold);
    display.display();
    delay(1000);
    display.clearDisplay();
    // debugln();

    digitalWrite(led, LOW);
}