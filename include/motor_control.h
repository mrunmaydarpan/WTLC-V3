void PumpON_command()
{
    if (value <= 100)
    {
        MotorState = true;
    }
}

void PumpOFF_command()
{
    MotorState = false;
}

void motor_on()
{
#if Buzzer
    tone(buzz, 4500, 300);
#endif
    digitalWrite(led, HIGH);
    switch (STATOR_TYPE)
    {
    case 1: // Normal Mode
        digitalWrite(Relay_ON, HIGH);
        break;
    case 2: // stator type
        digitalWrite(Relay_ON, HIGH);
        digitalWrite(Relay_ON_2, HIGH);
        delay(on_delay);
        digitalWrite(Relay_ON, LOW);
        digitalWrite(Relay_ON_2, LOW);
        break;
        // case 3: // CapacitorRun Mode
        //     digitalWrite(Relay_ON, HIGH);
        //     delay(1500);
        //     digitalWrite(Relay_ON, LOW);
        //     break;
    };
}

void motor_off()
{
#if Buzzer
    tone(buzz, 4500, 500);
#endif
    digitalWrite(led, LOW);
    switch (STATOR_TYPE)
    {
    case 1: // Normal Mode
        digitalWrite(Relay_ON, LOW);
        break;
    case 2: // stator type
        digitalWrite(Relay_OFF, HIGH);
        delay(off_delay);
        digitalWrite(Relay_OFF, LOW);
        break;
        // case 3: // CapacitorRun Mode
        //     digitalWrite(Relay_OFF, HIGH);
        //     delay(1500);
        //     digitalWrite(Relay_OFF, LOW);
        //     break;
    }
}

void DRY_RUN_CHECK()
{
    if (DryRun)
    {
        if (dryRun_LastDistance <= Distance)
        {
            debugln("Dry run");
            DryRunState = true;
            errorCountState = true;
        }
        else if (dryRun_LastDistance > (Distance + 2))
        {
            debugln("all ok");
            dryRun_LastDistance = Distance;
        }
    }
}
