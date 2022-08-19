void ReadMem()
{
    if (EEPROM.read(maxDistance_mem) > 250 || EEPROM.read(maxDistance_mem) == 0)
    {
        EEPROM.write(maxDistance_mem, 120);
    }
    if (EEPROM.read(minDistance_mem) > EEPROM.read(maxDistance_mem) || EEPROM.read(minDistance_mem) == 0)
    {
        EEPROM.write(minDistance_mem, 20);
    }
    if (EEPROM.read(manualOff_mem) > 1)
    {
        EEPROM.write(manualOff_mem, 0);
    }
    if (EEPROM.read(MotorStartThreshold_mem) > 70 ||
        EEPROM.read(MotorStartThreshold_mem) < 20)
    {
        EEPROM.write(MotorStartThreshold_mem, 20);
    }
    if (EEPROM.read(LastMotorState_mem) > 1)
    {
        EEPROM.write(LastMotorState_mem, 0);
    }
    if (EEPROM.read(StatorType_mem) == 0 || EEPROM.read(StatorType_mem) > 10)
    {
        EEPROM.write(StatorType_mem, 1);
    }
    if (EEPROM.read(AutoMode_mem) > 1)
    {
        EEPROM.write(AutoMode_mem, 1);
    }
    if (EEPROM.read(DryRun_mem) > 1)
    {
        EEPROM.write(DryRun_mem, 0);
    }
    MotorStartThreshold = EEPROM.read(MotorStartThreshold_mem);
    ManualOff = EEPROM.read(manualOff_mem);
    MaxDistance = EEPROM.read(maxDistance_mem);
    MotorState = EEPROM.read(motorState_mem);
    MinDistance = EEPROM.read(minDistance_mem);
    LastMotorState = EEPROM.read(LastMotorState_mem);
    dryRun_LastDistance = EEPROM.read(dryRun_LastDistance_mem);
    STATOR_TYPE = EEPROM.read(StatorType_mem);
    AutoMode = EEPROM.read(AutoMode_mem);
    DryRun = EEPROM.read(DryRun_mem);
    EEPROM.commit();
}