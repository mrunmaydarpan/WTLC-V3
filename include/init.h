#if HA_INIT
bool loadConfig()
{
    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1024)
    {
        Serial.println("Config file size is too large");
        return false;
    }

    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    StaticJsonDocument<200> doc;
    auto error = deserializeJson(doc, buf.get());
    if (error)
    {
        Serial.println("Failed to parse config file");
        return false;
    }
    // BROKER_ADDR = doc["mqtt_server"];
    strcpy(BROKER_ADDR, doc["mqtt_server"]);

    Serial.print("Loaded serverName: ");
    Serial.println(BROKER_ADDR);
    return true;
}
#endif