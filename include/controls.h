void lcdDefault()
{
#if OLED

#else
    lcd.setCursor(0, 0);
    lcd.print("PUMP:");
    lcd.setCursor(5, 0);
    lcd.print(MotorState ? "ON " : "OFF");
    lcd.setCursor(10, 0);
    lcd.print(AutoMode ? "  AUTO" : "MANUAL");
    lcd.setCursor(0, 1);
    lcd.print("Lv:");
#endif
}

void Debug()
{
    debug("Lv:" + String(value));
    debug("|");
    debug(AutoMode ? "A" : "M");
    debug("|");
    debug("R:" + String(MotorState));
    debug("|");
    debug("MOff:" + String(ManualOff));
    debug("|");
    debug("Distance:" + String(Distance));
    debug("|");
    debug("MotoMode:" + String(STATOR_TYPE));
    debug("|");
    debug("DistX:" + String(DistanceX));
    debug("|");
    debugln();
}

void buttonEvent()
{
    /*-----------------------CONTROL BUTTON OPERATION---------------------*/
    if (button.wasPressed())
    {
        if (MotorState == true)
        {
            if (value >= MotorStartThreshold)
            {
                ManualOff = false;
            }
            else
            {
                ManualOff = true;
                AutoMode = false;
            }
            PumpOFF_command();
        }
    }
    else if (button.pressedFor(LONG_PRESS))
    {
        ManualOff = false;
        DryRunState = false;
        errorCountState = false;
        PumpON_command();
        if (AutoMode == false && modeButton.isPressed())
        {
            AutoMode = true;
            EEPROM.commit();
        }
    }

    /*-----------------------MODE BUTTON OPERATION------------------------*/
    if (modeButton.wasPressed())
    {
        if (AutoMode)
        {
            AutoMode = false;
        }
        else
        {
            AutoMode = true;
        }

#if HA_INIT
        mode_HA.setState(AutoMode);
#endif
    }
}

void OneTimeRun()
{
    if (MotorState != LastMotorState)
    {
        if (MotorState == true)
        {
            motor_on();
            if (DryRun)
            {
                dryRun_LastDistance = Distance;
                EEPROM.write(dryRun_LastDistance_mem, dryRun_LastDistance);
                dryRun_timer = t.every(DryRunTime, DRY_RUN_CHECK);
            }
        }
        else
        {
            motor_off();
            if (DryRun)
            {
                t.stop(dryRun_timer);
            }
        }
#if HA_INIT
        pump_HA.setState(MotorState);
#endif
        debugln("MotorState Changed");
    }
    LastMotorState = MotorState;

    if (value != LastValue)
    {
    }
    LastValue = value;

    if (errorCountState != LasterrorCountState)
    {
#if OLED
#else
        lcd.clear();
#endif
        if (errorCountState == true)
        {
            PumpOFF_command();
            debugln("ERROR");
            ManualOff = true;
            AutoMode = false;

#if OLED
#else
            lcd.setCursor(3, 1);
            if (DryRunState)
            {
                lcd.print("DRY-RUN");
            }
            else
            {
                lcd.print("ERROR");
            }
#endif
            EEPROM.write(motorState_mem, 0);
            ledBlink = t.oscillate(led, 500, HIGH);
        }
        else
        {
            t.stop(ledBlink);
            debugln("Error State Stop");
        }
    }
    LasterrorCountState = errorCountState;
}

void update_lcd()
{
#if OLED
    display.clearDisplay();
    if (errorCountState)
    {
        display.drawBitmap(4, 16, alert_icon, 48, 48, 1);
        display.setCursor(55, 20);

        if (DryRunState)
        {
            display.print("DRY RUN");
        }
        else
        {
            display.print("SENSOR ERROR");
        }
    }
    else
    {
        display.setTextSize(5);
        display.setCursor(0, 23);
        display.setTextWrap(0);
        display.setCursor(4, 23);
        display.print(value);
        display.print("%");
    }
    display.setTextSize(1);
    display.setCursor(4, 4);
    display.print(AutoMode ? "AUTO" : "MANUAL");
#if HA_INIT
    if (mqtt_HA.isConnected())
        display.drawBitmap(112, 2, network_icon, 12, 12, 1);
#endif
    display.display();
#else
    if (errorCountState == false)
    {
        lcd.setCursor(3, 1);
        lcd.print(value);
        lcd.print("%");
        lcd.print("   ");
    }
    lcd.setCursor(15, 1);
    if (blink_state == true)
    {
        blink_state = false;
        lcd.write(0);
    }
    else
    {
        lcd.write(1);
        blink_state = true;
    }
#endif
#if HA_INIT
    mode_HA.setState(AutoMode);
    value_HA.setValue(value);
    distance_HA.setValue(DistanceX);
    sensor_error_HA.setValue(errorCountState ? "Error" : "Ok");
#endif
}

#if HA_INIT
void pump_action(bool state, HASwitch *s)
{
    if (state == true)
    {
        ManualOff = false;
        DryRunState = false;
        errorCountState = false;
        PumpON_command();
        if (AutoMode == false && modeButton.isPressed())
        {
            AutoMode = true;
        }
    }
    else
    {
        if (value >= MotorStartThreshold)
        {
            ManualOff = false;
        }
        else
        {
            ManualOff = true;
            AutoMode = false;
        }
        PumpOFF_command();
    }
}

void mode_action(bool state, HASwitch *s)
{
    if (state == true)
    {
        AutoMode = true;
    }
    else
    {
        AutoMode = false;
    }
}
#endif