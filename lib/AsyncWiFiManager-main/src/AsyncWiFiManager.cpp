/*
    * AsyncWiFiManager.h
    *
    * AsyncWiFiManager, a library for the ESP8266/Arduino platform
    * for configuration of WiFi credentials using a Captive Portal
    * and async libraries
    *
    * @author Creator tzapu
    * @author tablatronix
    * @author LBussy
    * @version 0.0.0
    * @license MIT
*/

#include "AsyncWiFiManager.h"

#if defined(ESP8266) || defined(ESP32)

#ifdef ESP32
uint8_t AsyncWiFiManager::_lastconxresulttmp = WL_IDLE_STATUS;
#endif

/**
 * --------------------------------------------------------------------------------
 *  AsyncWiFiManagerParameter
 * --------------------------------------------------------------------------------
**/

AsyncWiFiManagerParameter::AsyncWiFiManagerParameter()
{
    AsyncWiFiManagerParameter("");
}

AsyncWiFiManagerParameter::AsyncWiFiManagerParameter(const char *custom)
{
    _id = NULL;
    _label = NULL;
    _length = 1;
    _value = NULL;
    _labelPlacement = WFM_LABEL_BEFORE;
    _customHTML = custom;
}

AsyncWiFiManagerParameter::AsyncWiFiManagerParameter(const char *id, const char *label)
{
    init(id, label, "", 0, "", WFM_LABEL_BEFORE);
}

AsyncWiFiManagerParameter::AsyncWiFiManagerParameter(const char *id, const char *label, const char *defaultValue, int length)
{
    init(id, label, defaultValue, length, "", WFM_LABEL_BEFORE);
}

AsyncWiFiManagerParameter::AsyncWiFiManagerParameter(const char *id, const char *label, const char *defaultValue, int length, const char *custom)
{
    init(id, label, defaultValue, length, custom, WFM_LABEL_BEFORE);
}

AsyncWiFiManagerParameter::AsyncWiFiManagerParameter(const char *id, const char *label, const char *defaultValue, int length, const char *custom, int labelPlacement)
{
    init(id, label, defaultValue, length, custom, labelPlacement);
}

void AsyncWiFiManagerParameter::init(const char *id, const char *label, const char *defaultValue, int length, const char *custom, int labelPlacement)
{
    _id = id;
    _label = label;
    _labelPlacement = labelPlacement;
    _customHTML = custom;
    setValue(defaultValue, length);
}

AsyncWiFiManagerParameter::~AsyncWiFiManagerParameter()
{
    if (_value != NULL)
    {
        delete[] _value;
    }
    _length = 0; // setting length 0, ideally the entire parameter should be removed, or added to AsyncWiFiManager scope so it follows
}

// @note debug is not available in wmparameter class
void AsyncWiFiManagerParameter::setValue(const char *defaultValue, int length)
{
    if (!_id)
    {
        // Serial.println("cannot set value of this parameter");
        return;
    }

    // if(strlen(defaultValue) > length){
    //   // Serial.println("defaultValue length mismatch");
    //   // return false; //@todo bail
    // }

    _length = length;
    _value = new char[_length + 1];
    memset(_value, 0, _length + 1); // explicit null

    if (defaultValue != NULL)
    {
        strncpy(_value, defaultValue, _length);
    }
}
const char *AsyncWiFiManagerParameter::getValue()
{
    return _value;
}
const char *AsyncWiFiManagerParameter::getID()
{
    return _id;
}
const char *AsyncWiFiManagerParameter::getPlaceholder()
{
    return _label;
}
const char *AsyncWiFiManagerParameter::getLabel()
{
    return _label;
}
int AsyncWiFiManagerParameter::getValueLength()
{
    return _length;
}
int AsyncWiFiManagerParameter::getLabelPlacement()
{
    return _labelPlacement;
}
const char *AsyncWiFiManagerParameter::getCustomHTML()
{
    return _customHTML;
}

/**
 * [addParameter description]
 * @access public
 * @param {[type]} AsyncWiFiManagerParameter *p [description]
 */
bool AsyncWiFiManager::addParameter(AsyncWiFiManagerParameter *p)
{

    // check param id is valid, unless null
    if (p->getID())
    {
        for (size_t i = 0; i < strlen(p->getID()); i++)
        {
            if (!(isAlphaNumeric(p->getID()[i])) && !(p->getID()[i] == '_'))
            {
                DEBUG_WM(DEBUG_ERROR, F("[ERROR] parameter IDs can only contain alpha numeric chars"));
                return false;
            }
        }
    }

    // init params if never malloc
    if (_params == NULL)
    {
        DEBUG_WM(DEBUG_DEV, F("allocating params bytes:"), _max_params * sizeof(AsyncWiFiManagerParameter *));
        _params = (AsyncWiFiManagerParameter **)malloc(_max_params * sizeof(AsyncWiFiManagerParameter *));
    }

    // resize the params array by increment of WIFI_MANAGER_MAX_PARAMS
    if (_paramsCount == _max_params)
    {
        _max_params += WIFI_MANAGER_MAX_PARAMS;
        DEBUG_WM(DEBUG_DEV, F("Updated _max_params:"), _max_params);
        DEBUG_WM(DEBUG_DEV, F("re-allocating params bytes:"), _max_params * sizeof(AsyncWiFiManagerParameter *));
        AsyncWiFiManagerParameter **new_params = (AsyncWiFiManagerParameter **)realloc(_params, _max_params * sizeof(AsyncWiFiManagerParameter *));
        // DEBUG_WM(WIFI_MANAGER_MAX_PARAMS);
        // DEBUG_WM(_paramsCount);
        // DEBUG_WM(_max_params);
        if (new_params != NULL)
        {
            _params = new_params;
        }
        else
        {
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] failed to realloc params, size not increased."));
            return false;
        }
    }

    _params[_paramsCount] = p;
    _paramsCount++;

    DEBUG_WM(DEBUG_VERBOSE, F("Added Parameter:"), p->getID());
    return true;
}

/**
 * [getParameters description]
 * @access public
 */
AsyncWiFiManagerParameter **AsyncWiFiManager::getParameters()
{
    return _params;
}

/**
 * [getParametersCount description]
 * @access public
 */
int AsyncWiFiManager::getParametersCount()
{
    return _paramsCount;
}

/**
 * --------------------------------------------------------------------------------
 *  AsyncWiFiManager 
 * --------------------------------------------------------------------------------
**/

// constructors
AsyncWiFiManager::AsyncWiFiManager(Stream &consolePort) : _debugPort(consolePort)
{
    AsyncWiFiManagerInit();
}

AsyncWiFiManager::AsyncWiFiManager()
{
    AsyncWiFiManagerInit();
}

void AsyncWiFiManager::AsyncWiFiManagerInit()
{
    setMenu(_menuIdsDefault);
    if (_debug && _debugLevel > DEBUG_DEV)
        debugPlatformInfo();
    _max_params = WIFI_MANAGER_MAX_PARAMS;
}

// destructor
AsyncWiFiManager::~AsyncWiFiManager()
{
    _end();
    // parameters
    // @todo below belongs to AsyncWiFiManagerparameter
    if (_params != NULL)
    {
        DEBUG_WM(DEBUG_DEV, F("Freeing allocated parameters."));
        free(_params);
        _params = NULL;
    }

// @todo remove event
// WiFi.onEvent(std::bind(&WiFiManager::WiFiEvent,this,_1,_2));
#ifdef ESP32
    WiFi.removeEvent(wm_event_id);
#endif

    DEBUG_WM(DEBUG_DEV, F("Unloading AsyncWiFiManager."));
}

void AsyncWiFiManager::_begin()
{
    if (_hasBegun)
        return;
    _hasBegun = true;
    // _usermode = WiFi.getMode();

#ifndef ESP32
    WiFi.persistent(false); // disable persistent so scannetworks and mode switching do not cause overwrites
#endif
}

void AsyncWiFiManager::_end()
{
    _hasBegun = false;
    if (_userpersistent)
        WiFi.persistent(true); // reenable persistent, there is no getter we rely on _userpersistent
                               // if(_usermode != WIFI_OFF) WiFi.mode(_usermode);
}

// AUTOCONNECT

boolean AsyncWiFiManager::autoConnect()
{
    String ssid = getDefaultAPName();
    return autoConnect(ssid.c_str(), NULL);
}

/**
 * [autoConnect description]
 * @access public
 * @param  {[type]} char const         *apName     [description]
 * @param  {[type]} char const         *apPassword [description]
 * @return {[type]}      [description]
 */
boolean AsyncWiFiManager::autoConnect(char const *apName, char const *apPassword)
{
    DEBUG_WM(F("AutoConnect."));
    if (getWiFiIsSaved())
    {

        _begin();

        // attempt to connect using saved settings, on fail fallback to AP config portal
        if (!WiFi.enableSTA(true))
        {
            // handle failure mode Brownout detector etc.
            DEBUG_WM(DEBUG_ERROR, F("[FATAL] Unable to enable WiFi."));
            return false;
        }

        WiFiSetCountry();

#ifdef ESP32
        if (esp32persistent)
            WiFi.persistent(false); // disable persistent for esp32 after esp_wifi_start or else saves wont work
#endif

        _usermode = WIFI_STA; // When using autoconnect , assume the user wants sta mode on permanently.

        // no getter for autoreconnectpolicy before this
        // https://github.com/esp8266/Arduino/pull/4359
        // so we must force it on else, if not connectimeout then waitforconnectionresult gets stuck endless loop
        WiFi_autoReconnect();

        // set hostname before stating
        if ((String)_hostname != "")
        {
            setupHostname(true);
            //             DEBUG_WM(DEBUG_VERBOSE, "Setting hostname:", _hostname);
            //             bool res = true;
            // #ifdef ESP8266
            //             res = WiFi.hostname(_hostname);
            // #ifdef ESP8266MDNS_H
            //             DEBUG_WM(DEBUG_VERBOSE, F("Setting MDNS hostname."));
            //             if (MDNS.begin(_hostname))
            //             {
            //                 MDNS.addService("http", "tcp", 80);
            //             }
            // #endif
            // #elif defined(ESP32)
            //             // @note hostname must be set after STA_START
            //             delay(200); // do not remove, give time for STA_START
            //             res = WiFi.setHostname(_hostname);
            // #ifdef ESP32MDNS_H
            //             DEBUG_WM(DEBUG_VERBOSE, F("Setting MDNS hostname."));
            //             if (MDNS.begin(_hostname))
            //             {
            //                 MDNS.addService("http", "tcp", 80);
            //             }
            // #endif
            // #endif

            //             if (!res)
            //                 DEBUG_WM(DEBUG_ERROR, F("[ERROR] hostname: set failed."));

            //             if (WiFi.status() == WL_CONNECTED)
            //             {
            //                 DEBUG_WM(DEBUG_VERBOSE, F("Reconnecting to set new hostname."));
            //                 // WiFi.reconnect(); // This does not reset dhcp
            //                 WiFi_Disconnect();
            //                 delay(200); // do not remove, need a delay for disconnect to change status()
            //             }
        }

        // if already connected, or try stored connect
        // @note @todo ESP32 has no autoconnect, so connectwifi will always be called unless user called begin etc before
        // @todo check if correct ssid == saved ssid when already connected
        bool connected = false;
        if (WiFi.status() == WL_CONNECTED)
        {
            connected = true;
            DEBUG_WM(F("AutoConnect: ESP already connected."));
            setSTAConfig();
        }

        if (connected || connectWifi("", "") == WL_CONNECTED)
        {
            //connected
            DEBUG_WM(F("AutoConnect: SUCCESS"));
            DEBUG_WM(F("STA IP address:"), WiFi.localIP());
            _lastconxresult = WL_CONNECTED;

            if ((String)_hostname != "")
            {
#ifdef ESP8266
                DEBUG_WM(DEBUG_DEV, F("hostname: STA "), WiFi.hostname());
#elif defined(ESP32)
                DEBUG_WM(DEBUG_DEV, F("hostname: STA "), WiFi.getHostname());
#endif
            }
            return true;
        }

        // possibly skip the config portal
        if (!_enableConfigPortal)
        {
            return false;
        }

        DEBUG_WM(F("AutoConnect: FAILED"));
    }
    else
        DEBUG_WM(F("No credentials are saved, skipping connect."));

    // not connected start configportal
    // return startConfigPortal(apName, apPassword);
    bool res = startConfigPortal(apName, apPassword);
    return res;
}

bool AsyncWiFiManager::setupHostname(bool restart)
{
    if ((String)_hostname == "")
    {
        DEBUG_WM(DEBUG_DEV, "No Hostname to set");
        return false;
    }
    else
        DEBUG_WM(DEBUG_DEV, "setupHostname: ", _hostname);
    bool res = true;
#ifdef ESP8266
    DEBUG_WM(DEBUG_VERBOSE, "Setting WiFi hostname");
    res = WiFi.hostname(_hostname);
// #ifdef ESP8266MDNS_H
#ifdef WM_MDNS
    DEBUG_WM(DEBUG_VERBOSE, "Setting MDNS hostname, tcp 80");
    if (MDNS.begin(_hostname))
    {
        MDNS.addService("http", "tcp", 80);
    }
#endif
#elif defined(ESP32)
    // @note hostname must be set after STA_START
    delay(200); // do not remove, give time for STA_START
    res = WiFi.setHostname(_hostname);
    // #ifdef ESP32MDNS_H
#ifdef WM_MDNS
    DEBUG_WM(DEBUG_VERBOSE, "Setting MDNS hostname, tcp 80");
    if (MDNS.begin(_hostname))
    {
        MDNS.addService("http", "tcp", 80);
    }
#endif
#endif

    if (!res)
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] hostname: set failed!"));

    if (restart && (WiFi.status() == WL_CONNECTED))
    {
        DEBUG_WM(DEBUG_VERBOSE, F("reconnecting to set new hostname"));
        // WiFi.reconnect(); // This does not reset dhcp
        WiFi_Disconnect();
        delay(200); // do not remove, need a delay for disconnect to change status()
    }

    return res;
}

// CONFIG PORTAL
bool AsyncWiFiManager::startAP()
{
    bool ret = true;
    DEBUG_WM(F("StartAP with SSID:"), _apName);

#ifdef ESP8266
    // @bug workaround for bug #4372 https://github.com/esp8266/Arduino/issues/4372
    if (!WiFi.enableAP(true))
    {
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] enableAP failed."));
        return false;
    }
    delay(500); // workaround delay
#endif

    // setup optional soft AP static ip config
    if (_ap_static_ip)
    {
        DEBUG_WM(F("Custom AP IP/GW/Subnet:"));
        if (!WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn))
        {
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] softAPConfig failed."));
        }
    }

    //@todo add callback here if needed to modify ap but cannot use setAPStaticIPConfig
    //@todo rework wifi channelsync as it will work unpredictably when not connected in sta

    int32_t channel = 0;
    if (_channelSync)
        channel = WiFi.channel();
    else
        channel = _apChannel;

    if (channel > 0)
    {
        DEBUG_WM(DEBUG_VERBOSE, F("Starting AP on channel:"), channel);
    }

    // start soft AP with password or anonymous
    // default channel is 1 here and in esplib, @todo just change to default remove conditionals
    if (_apPassword != "")
    {
        if (channel > 0)
        {
            ret = WiFi.softAP(_apName.c_str(), _apPassword.c_str(), channel, _apHidden);
        }
        else
        {
            ret = WiFi.softAP(_apName.c_str(), _apPassword.c_str(), 1, _apHidden); //password option
        }
    }
    else
    {
        DEBUG_WM(DEBUG_VERBOSE, F("AP has anonymous access."));
        if (channel > 0)
        {
            ret = WiFi.softAP(_apName.c_str(), "", channel, _apHidden);
        }
        else
        {
            ret = WiFi.softAP(_apName.c_str(), "", 1, _apHidden);
        }
    }

    if (_debugLevel >= DEBUG_DEV)
        debugSoftAPConfig();

    if (!ret)
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] There was a problem starting the AP."));
    // @todo add softAP retry here

    delay(500); // slight delay to make sure we get an AP IP
    DEBUG_WM(F("AP IP address:"), WiFi.softAPIP());

// set ap hostname
#ifdef ESP32
    if (ret && (String)_hostname != "")
    {
        DEBUG_WM(DEBUG_VERBOSE, F("Setting softAP Hostname:"), _hostname);
        bool res = WiFi.softAPsetHostname(_hostname);
        if (!res)
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] hostname: AP set failed."));
        DEBUG_WM(DEBUG_DEV, F("Hostname: AP"), WiFi.softAPgetHostname());
    }
#endif

    return ret;
}

/**
 * [startWebPortal description]
 * @access public
 * @return {[type]} [description]
 */
void AsyncWiFiManager::startWebPortal()
{
    if (configPortalActive || webPortalActive)
        return;
    setupConfigPortal();
    webPortalActive = true;
}

/**
 * [stopWebPortal description]
 * @access public
 * @return {[type]} [description]
 */
void AsyncWiFiManager::stopWebPortal()
{
    if (!configPortalActive && !webPortalActive)
        return;
    DEBUG_WM(DEBUG_VERBOSE, F("Stopping web portal."));
    webPortalActive = false;
    shutdownConfigPortal();
}

boolean AsyncWiFiManager::configPortalHasTimeout()
{

    if (_configPortalTimeout == 0 || (_apClientCheck && (WiFi_softap_num_stations() > 0)))
    {
        if (millis() - timer > 30000)
        {
            timer = millis();
            DEBUG_WM(DEBUG_VERBOSE, "Num clients: " + (String)WiFi_softap_num_stations());
        }
        _configPortalStart = millis(); // kludge, bump configportal start time to skew timeouts
        return false;
    }
    // handle timeout
    if (_webClientCheck && (_webPortalAccessed > _configPortalStart) > 0)
        _configPortalStart = _webPortalAccessed;

    if (millis() > _configPortalStart + _configPortalTimeout)
    {
        DEBUG_WM(F("Config portal has timed out."));
        return true;
    }
    else if (_debugLevel > 0)
    {
        // log timeout
        if (_debug)
        {
            uint16_t logintvl = 30000; // how often to emit timeing out counter logging
            if ((millis() - timer) > logintvl)
            {
                timer = millis();
                DEBUG_WM(DEBUG_VERBOSE, F("Portal timeout in "), (String)((_configPortalStart + _configPortalTimeout - millis()) / 1000) + (String)F(" seconds"));
            }
        }
    }

    return false;
}

void AsyncWiFiManager::setupConfigPortal()
{

    DEBUG_WM(F("Starting web portal."));

    // setup dns and web servers
    dnsServer.reset(new DNSServer());
    server.reset(new WM_WebServer(_httpPort));

    if (_httpPort != 80)
        DEBUG_WM(DEBUG_VERBOSE, "http server started with custom port: ", _httpPort); // @todo not showing ip

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    // DEBUG_WM("dns server started port: ",DNS_PORT);
    DEBUG_WM(DEBUG_DEV, F("DNS server started with IP:"), WiFi.softAPIP());
    dnsServer->start(DNS_PORT, F("*"), WiFi.softAPIP());

    // @todo new callback, webserver started, callback cannot override handlers, but can grab them first

    if (_webservercallback != NULL)
    {
        _webservercallback();
    }

    /* Setup httpd callbacks, web pages: root, WiFi config pages, SO captive portal detectors and not found. */
    server->on(String(FPSTR(R_root)).c_str(), std::bind(&AsyncWiFiManager::handleRoot, this));
    server->on(String(FPSTR(R_wifi)).c_str(), std::bind(&AsyncWiFiManager::handleWifi, this, true));
    server->on(String(FPSTR(R_wifinoscan)).c_str(), std::bind(&AsyncWiFiManager::handleWifi, this, false));
    server->on(String(FPSTR(R_wifisave)).c_str(), std::bind(&AsyncWiFiManager::handleWifiSave, this));
    server->on(String(FPSTR(R_info)).c_str(), std::bind(&AsyncWiFiManager::handleInfo, this));
    server->on(String(FPSTR(R_param)).c_str(), std::bind(&AsyncWiFiManager::handleParam, this));
    server->on(String(FPSTR(R_paramsave)).c_str(), std::bind(&AsyncWiFiManager::handleParamSave, this));
    server->on(String(FPSTR(R_restart)).c_str(), std::bind(&AsyncWiFiManager::handleReset, this));
    server->on(String(FPSTR(R_exit)).c_str(), std::bind(&AsyncWiFiManager::handleExit, this));
    server->on(String(FPSTR(R_close)).c_str(), std::bind(&AsyncWiFiManager::handleClose, this));
    server->on(String(FPSTR(R_erase)).c_str(), std::bind(&AsyncWiFiManager::handleErase, this, false));
    server->on(String(FPSTR(R_status)).c_str(), std::bind(&AsyncWiFiManager::handleWiFiStatus, this));
    server->onNotFound(std::bind(&AsyncWiFiManager::handleNotFound, this));

    server->begin(); // Web server start
    DEBUG_WM(DEBUG_VERBOSE, F("HTTP server started."));

    if (_preloadwifiscan)
        WiFi_scanNetworks(true, true); // preload wifiscan , async
}

boolean AsyncWiFiManager::startConfigPortal()
{
    String ssid = getDefaultAPName();
    return startConfigPortal(ssid.c_str(), NULL);
}

/**
 * [startConfigPortal description]
 * @access public
 * @param  {[type]} char const         *apName     [description]
 * @param  {[type]} char const         *apPassword [description]
 * @return {[type]}      [description]
 */
boolean AsyncWiFiManager::startConfigPortal(char const *apName, char const *apPassword)
{
    _begin();

    //setup AP
    _apName = apName; // @todo check valid apname ?
    _apPassword = apPassword;

    DEBUG_WM(DEBUG_VERBOSE, F("Starting config portal."));

    if (_apName == "")
        _apName = getDefaultAPName();

    if (!validApPassword())
        return false;

    // HANDLE issues with STA connections, shutdown sta if not connected, or else this will hang channel scanning and softap will not respond
    // @todo sometimes still cannot connect to AP for no known reason, no events in log either
    if (_disableSTA || (!WiFi.isConnected() && _disableSTAConn))
    {
        // this fixes most ap problems, however, simply doing mode(WIFI_AP) does not work if sta connection is hanging, must `wifi_station_disconnect`
        WiFi_Disconnect();
        WiFi_enableSTA(false);
        DEBUG_WM(DEBUG_VERBOSE, F("Disabling STA."));
    }
    else
    {
        // @todo even if sta is connected, it is possible that softap connections will fail, IOS says "invalid password", windows says "cannot connect to this network" researching
        WiFi_enableSTA(true);
    }

    // init configportal globals to known states
    configPortalActive = true;
    bool result = connect = abort = false; // loop flags, connect true success, abort true break
    uint8_t state;

    _configPortalStart = millis();

    // start access point
    DEBUG_WM(DEBUG_VERBOSE, F("Enabling AP."));
    startAP();
    WiFiSetCountry();

    // do AP callback if set
    if (_apcallback != NULL)
    {
        _apcallback(this);
    }

    // init configportal
    DEBUG_WM(DEBUG_DEV, F("setupConfigPortal()"));
    setupConfigPortal();

    if (!_configPortalIsBlocking)
    {
        DEBUG_WM(DEBUG_VERBOSE, F("Config portal running, non blocking processing."));
        return result;
    }

    DEBUG_WM(DEBUG_VERBOSE, F("Config portal running in blocking mode, waiting for clients."));
    // blocking loop waiting for config
    while (1)
    {

        // if timed out or abort, break
        if (configPortalHasTimeout() || abort)
        {
            DEBUG_WM(DEBUG_DEV, F("Config portal abort."));
            shutdownConfigPortal();
            result = abort ? portalAbortResult : portalTimeoutResult; // false, false
            break;
        }

        state = processConfigPortal();

        // status change, break
        if (state != WL_IDLE_STATUS)
        {
            result = (state == WL_CONNECTED); // true if connected
            break;
        }

        yield(); // watchdog
    }

    DEBUG_WM(DEBUG_NOTIFY, F("Config portal exiting."));
    return result;
}

/**
 * [process description]
 * @access public
 * @return {[type]} [description]
 */
boolean AsyncWiFiManager::process()
{
// process mdns, esp32 not required
#if defined(WM_MDNS) && defined(ESP8266)
    MDNS.update();
#endif

    if (webPortalActive || (configPortalActive && !_configPortalIsBlocking))
    {
        uint8_t state = processConfigPortal();
        return state == WL_CONNECTED;
    }
    return false;
}

//using esp enums returns for now, should be fine
uint8_t AsyncWiFiManager::processConfigPortal()
{
    //DNS handler
    dnsServer->processNextRequest();
    //HTTP handler
    server->handleClient();

    // Waiting for save
    if (connect)
    {
        connect = false;
        DEBUG_WM(DEBUG_VERBOSE, F("Processing save."));
        if (_enableCaptivePortal)
            delay(_cpclosedelay); // keeps the captiveportal from closing too fast.

        // skip wifi if no ssid
        if (_ssid == "")
        {
            DEBUG_WM(DEBUG_VERBOSE, F("No SSID, skipping WiFi save."));
        }
        else
        {
            // attempt sta connection to submitted _ssid, _pass
            if (connectWifi(_ssid, _pass) == WL_CONNECTED)
            {

                DEBUG_WM(F("[SUCCESS] Connect to new AP."));
                DEBUG_WM(F("Got IP address:"));
                DEBUG_WM(WiFi.localIP());

                if (_savewificallback != NULL)
                {
                    _savewificallback();
                }
                shutdownConfigPortal();
                return WL_CONNECTED; // CONNECT SUCCESS
            }
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] Connect to new AP failed."));
        }

        if (_shouldBreakAfterConfig)
        {

            // do save callback
            // @todo this is more of an exiting callback than a save, clarify when this should actually occur
            // confirm or verify data was saved to make this more accurate callback
            if (_savewificallback != NULL)
            {
                DEBUG_WM(DEBUG_VERBOSE, F("WiFi/Param save callback."));
                _savewificallback();
            }
            shutdownConfigPortal();
            return WL_CONNECT_FAILED; // CONNECT FAIL
        }
        else
        {
            // clear save strings
            _ssid = "";
            _pass = "";
            // if connect fails, turn sta off to stabilize AP
            WiFi_Disconnect();
            WiFi_enableSTA(false);
            DEBUG_WM(DEBUG_VERBOSE, F("Disabling STA."));
        }
    }

    return WL_IDLE_STATUS;
}

/**
 * [shutdownConfigPortal description]
 * @access public
 * @return bool success (softapdisconnect)
 */
bool AsyncWiFiManager::shutdownConfigPortal()
{
    if (webPortalActive)
        return false;

    //DNS handler
    dnsServer->processNextRequest();
    //HTTP handler
    server->handleClient();

    // @todo what is the proper way to shutdown and free the server up
    server->stop();
    server.reset();
    dnsServer->stop(); //  free heap ?
    dnsServer.reset();

    WiFi.scanDelete(); // free wifi scan results

    if (!configPortalActive)
        return false;

    // turn off AP
    // @todo bug workaround
    // https://github.com/esp8266/Arduino/issues/3793
    // [APdisconnect] set_config failed! *WM: disconnect configportal - softAPdisconnect failed
    // still no way to reproduce reliably
    DEBUG_WM(DEBUG_VERBOSE, F("Disconnect config portal."));
    bool ret = false;
    ret = WiFi.softAPdisconnect(false);
    if (!ret)
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] Eisconnect config portal - softAPdisconnect failed."));
    delay(1000);
    DEBUG_WM(DEBUG_VERBOSE, F("Restoring usermode:"), getModeString(_usermode));
    WiFi_Mode(_usermode); // restore users wifi mode, BUG https://github.com/esp8266/Arduino/issues/4372
    if (WiFi.status() == WL_IDLE_STATUS)
    {
        WiFi.reconnect(); // restart wifi since we disconnected it in startconfigportal
        DEBUG_WM(DEBUG_VERBOSE, F("WiFi reconnect, was idle."));
    }
    DEBUG_WM(DEBUG_VERBOSE, F("WiFi status:"), getWLStatusString(WiFi.status()));
    DEBUG_WM(DEBUG_VERBOSE, F("WiFi mode:"), getModeString(WiFi.getMode()));
    configPortalActive = false;
    _end();
    return ret;
}

// @todo refactor this up into separate functions
// one for connecting to flash , one for new client
// clean up, flow is convoluted, and causes bugs
uint8_t AsyncWiFiManager::connectWifi(String ssid, String pass)
{
    DEBUG_WM(DEBUG_VERBOSE, F("Connecting as WiFi client."));

    uint8_t retry = 1;
    uint8_t connRes = (uint8_t)WL_NO_SSID_AVAIL;

    setSTAConfig();
    //@todo catch failures in set_config

    // make sure sta is on before `begin` so it does not call enablesta->mode while persistent is ON ( which would save WM AP state to EEPROM !)

    if (_cleanConnect)
        WiFi_Disconnect(); // disconnect before begin, in case anything is hung, this causes a 2 seconds delay for connect
    // @todo find out what status is when this is needed, can we detect it and handle it, say in between states or idle_status

    while (retry <= _connectRetries)
    {
        if (_connectRetries > 1)
        {
            DEBUG_WM(F("Connect Wifi, ATTEMPT #"), (String)retry + " of " + (String)_connectRetries);
        }

        // if ssid argument provided connect to that
        if (ssid != "")
        {
            wifiConnectNew(ssid, pass);
            if (_saveTimeout > 0)
            {
                connRes = waitForConnectResult(_saveTimeout); // use default save timeout for saves to prevent bugs in esp->waitforconnectresult loop
            }
            else
            {
                connRes = waitForConnectResult(0);
            }
        }
        else
        {
            // connect using saved ssid if there is one
            if (WiFi_hasAutoConnect())
            {
                wifiConnectDefault();
                connRes = waitForConnectResult();
            }
            else
            {
                DEBUG_WM(F("No WiFi save, skipping."));
            }
        }

        DEBUG_WM(DEBUG_VERBOSE, F("Connection result:"), getWLStatusString(connRes));
        retry++;
    }

// WPS enabled? https://github.com/esp8266/Arduino/pull/4889
#ifdef NO_EXTRA_4K_HEAP
    // do WPS, if WPS options enabled and not connected and no password was supplied
    // @todo this seems like wrong place for this, is it a fallback or option?
    if (_tryWPS && connRes != WL_CONNECTED && pass == "")
    {
        startWPS();
        // should be connected at the end of WPS
        connRes = waitForConnectResult();
    }
#endif

    if (connRes != WL_SCAN_COMPLETED)
    {
        updateConxResult(connRes);
    }

    return connRes;
}

/**
 * connect to a new wifi ap
 * @since $dev
 * @param  String ssid 
 * @param  String pass 
 * @return bool success
 */
bool AsyncWiFiManager::wifiConnectNew(String ssid, String pass)
{
    bool ret = false;
    DEBUG_WM(F("Connected:"), WiFi.status() == WL_CONNECTED);
    DEBUG_WM(F("Connecting to new AP:"), ssid);
    DEBUG_WM(DEBUG_DEV, F("Using password:"), pass);
    WiFi_enableSTA(true, storeSTAmode); // storeSTAmode will also toggle STA on in default opmode (persistent) if true (default)
    WiFi.persistent(true);
    // #ifdef ESP8266
    //     ret = WiFi.begin(ssid.c_str(), pass.c_str(), WiFi.channel(), WiFi.BSSID());
    // #else
    //     ret = WiFi.begin(ssid.c_str(), pass.c_str());
    // #endif
    ret = WiFi.begin(ssid.c_str(), pass.c_str());
    WiFi.persistent(false);
    if (!ret)
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] WiFi begin failed."));
    return ret;
}

/**
 * connect to stored wifi
 * @since dev
 * @return bool success
 */
bool AsyncWiFiManager::wifiConnectDefault()
{
    bool ret = false;
    DEBUG_WM(F("Connecting to saved AP:"), WiFi_SSID(true));
    DEBUG_WM(DEBUG_DEV, F("Using password:"), WiFi_psk(true));
    ret = WiFi_enableSTA(true, storeSTAmode);
    delay(500); // If this is not here, credentials are not detected when saved
    DEBUG_WM(DEBUG_DEV, "Mode after delay: " + getModeString(WiFi.getMode()));
    if (!ret)
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] WiFi enableSta failed."));
    ret = WiFi.begin();
    if (!ret)
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] WiFi begin failed."));
    return ret;
}

/**
 * set sta config if set
 * @since $dev
 * @return bool success
 */
bool AsyncWiFiManager::setSTAConfig()
{
    DEBUG_WM(DEBUG_DEV, F("STA static IP:"), _sta_static_ip);
    bool ret = true;
    if (_sta_static_ip)
    {
        DEBUG_WM(DEBUG_VERBOSE, F("Custom static IP/GW/Subnet/DNS."));
        if (_sta_static_dns)
        {
            DEBUG_WM(DEBUG_VERBOSE, F("Custom static DNS."));
            ret = WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn, _sta_static_dns);
        }
        else
        {
            DEBUG_WM(DEBUG_VERBOSE, F("Custom STA IP/GW/Subnet."));
            ret = WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
        }

        if (!ret)
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] WiFi config failed."));
        else
            DEBUG_WM(F("STA IP set:"), WiFi.localIP());
    }
    else
    {
        DEBUG_WM(DEBUG_VERBOSE, F("setSTAConfig static ip not set, skipping."));
    }
    return ret;
}

// @todo change to getLastFailureReason and do not touch conxresult
void AsyncWiFiManager::updateConxResult(uint8_t status)
{
    // hack in wrong password detection
    _lastconxresult = status;
#ifdef ESP8266
    if (_lastconxresult == WL_CONNECT_FAILED)
    {
        if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
        {
            _lastconxresult = WL_STATION_WRONG_PASSWORD;
        }
    }
#elif defined(ESP32)
    // if(_lastconxresult == WL_CONNECT_FAILED){
    if (_lastconxresult == WL_CONNECT_FAILED || _lastconxresult == WL_DISCONNECTED)
    {
        DEBUG_WM(DEBUG_DEV, F("lastconxresulttmp:"), getWLStatusString(_lastconxresulttmp));
        if (_lastconxresulttmp != WL_IDLE_STATUS)
        {
            _lastconxresult = _lastconxresulttmp;
            // _lastconxresulttmp = WL_IDLE_STATUS;
        }
    }
#endif
    DEBUG_WM(DEBUG_DEV, F("lastconxresult:"), getWLStatusString(_lastconxresult));
}

uint8_t AsyncWiFiManager::waitForConnectResult()
{
    if (_connectTimeout > 0)
        DEBUG_WM(DEBUG_VERBOSE, _connectTimeout, F("ms connectTimeout set."));
    return waitForConnectResult(_connectTimeout);
}

/**
 * waitForConnectResult
 * @param  uint16_t timeout  in seconds
 * @return uint8_t  WL Status
 */
uint8_t AsyncWiFiManager::waitForConnectResult(uint16_t timeout)
{
    if (timeout == 0)
    {
        DEBUG_WM(F("connectTimeout not set, ESP waitForConnectResult."));
        return WiFi.waitForConnectResult();
    }

    unsigned long timeoutmillis = millis() + timeout;
    DEBUG_WM(DEBUG_VERBOSE, timeout, F("ms timeout, waiting for connection."), false);
    uint8_t status = WiFi.status();

    while (millis() < timeoutmillis)
    {
        status = WiFi.status();
        // @todo detect additional states, connect happens, then dhcp then get ip, there is some delay here, make sure not to timeout if waiting on IP
        if (status == WL_CONNECTED || status == WL_CONNECT_FAILED)
        {
            if (_debug)
                Serial.println();
            return status;
        }
        if (_debug)
            Serial.print(".");
        delay(100);
    }
    if (_debug)
        Serial.println();
    return status;
}

// WPS enabled? https://github.com/esp8266/Arduino/pull/4889
#ifdef NO_EXTRA_4K_HEAP
void AsyncWiFiManager::startWPS()
{
    DEBUG_WM(F("Start WPS."));
#ifdef ESP8266
    WiFi.beginWPSConfig();
#else
    // @todo
#endif
    DEBUG_WM(F("End WPS."));
}
#endif

String AsyncWiFiManager::getHTTPHead(String title)
{
    String page;
    page += FPSTR(HTTP_HEAD_START);
    page.replace(FPSTR(T_v), title);
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += _customHeadElement;

    if (_bodyClass != "")
    {
        String p = FPSTR(HTTP_HEAD_END);
        p.replace(FPSTR(T_c), _bodyClass); // add class str
        page += p;
    }
    else
    {
        page += FPSTR(HTTP_HEAD_END);
    }

    return page;
}

/** 
 * HTTPD handler for page requests
 */
void AsyncWiFiManager::handleRequest()
{
    _webPortalAccessed = millis();
}

/** 
 * HTTPD CALLBACK root or redirect to captive portal
 */
void AsyncWiFiManager::handleRoot()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Root"));
    if (captivePortal())
        return; // If captive portal redirect instead of displaying the page
    handleRequest();
    String page = getHTTPHead(FPSTR(S_options)); // @token options @todo replace options with title
    String str = FPSTR(HTTP_ROOT_MAIN);
    str.replace(FPSTR(T_v), configPortalActive ? _apName : WiFi.localIP().toString()); // use ip if ap is not active for heading
    page += str;
    page += FPSTR(HTTP_PORTAL_OPTIONS);
    page += getMenuOut();
    reportStatus(page);
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);
    // server->close(); // testing reliability fix for content length mismatches during mutiple flood hits  WiFi_scanNetworks(); // preload wifiscan
    if (_preloadwifiscan)
        WiFi_scanNetworks(_scancachetime, true); // preload wifiscan throttled, async
                                                 // @todo buggy, captive portals make a query on every page load, causing this to run every time in addition to the real page load
                                                 // I dont understand why, when you are already in the captive portal, I guess they want to know that its still up and not done or gone
                                                 // if we can detect these and ignore them that would be great, since they come from the captive portal redirect maybe there is a refferer
}

/**
 * HTTPD CALLBACK Wifi config page handler
 */
void AsyncWiFiManager::handleWifi(boolean scan)
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Wifi"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titlewifi)); // @token titlewifi
    if (scan)
    {
        // DEBUG_WM(DEBUG_DEV,"refresh flag:",server->hasArg(F("refresh")));
        WiFi_scanNetworks(server->hasArg(F("refresh")), false); //wifiscan, force if arg refresh
        page += getScanItemOut();
    }
    String pitem = "";

    pitem = FPSTR(HTTP_FORM_START);
    pitem.replace(FPSTR(T_v), F("wifisave")); // set form action
    page += pitem;

    pitem = FPSTR(HTTP_FORM_WIFI);
    pitem.replace(FPSTR(T_v), WiFi_SSID());

    if (_showPassword)
    {
        pitem.replace(FPSTR(T_p), WiFi_psk());
    }
    else
    {
        pitem.replace(FPSTR(T_p), FPSTR(S_passph));
    }
    page += pitem;

    page += getStaticOut();
    page += FPSTR(HTTP_FORM_WIFI_END);
    if (_paramsInWifi && _paramsCount > 0)
    {
        page += FPSTR(HTTP_FORM_PARAM_HEAD);
        page += getParamOut();
    }
    page += FPSTR(HTTP_FORM_END);
    page += FPSTR(HTTP_SCAN_LINK);
    if (_showBack)
        page += FPSTR(HTTP_BACKBTN);
    reportStatus(page);
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);
    // server->close(); // testing reliability fix for content length mismatches during mutiple flood hits

    DEBUG_WM(DEBUG_DEV, F("Sent config page."));
}

/**
 * HTTPD CALLBACK Wifi param page handler
 */
void AsyncWiFiManager::handleParam()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Param"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titleparam)); // @token titlewifi

    String pitem = "";

    pitem = FPSTR(HTTP_FORM_START);
    pitem.replace(FPSTR(T_v), F("paramsave"));
    page += pitem;

    page += getParamOut();
    page += FPSTR(HTTP_FORM_END);
    if (_showBack)
        page += FPSTR(HTTP_BACKBTN);
    reportStatus(page);
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);

    DEBUG_WM(DEBUG_DEV, F("Sent param page."));
}

String AsyncWiFiManager::getMenuOut()
{
    String page;

    for (auto menuId : _menuIds)
    {
        if (((String)menuId == "param") && (_paramsCount == 0))
            continue; // no params set, omit params from menu, @todo this may be undesired by someone
        page += HTTP_PORTAL_MENU[menuId];
    }

    return page;
}

// // is it possible in softap mode to detect aps without scanning
// bool AsyncWiFiManager::WiFi_scanNetworksForAP(bool force){
//   WiFi_scanNetworks(force);
// }

void AsyncWiFiManager::WiFi_scanComplete(int networksFound)
{
    _lastscan = millis();
    _numNetworks = networksFound;
    DEBUG_WM(DEBUG_VERBOSE, F("Async WiFi scan completed in "), "in " + (String)(_lastscan - _startscan) + " ms.");
    DEBUG_WM(DEBUG_VERBOSE, F("Async WiFi scan found:"), _numNetworks);
}

bool AsyncWiFiManager::WiFi_scanNetworks()
{
    return WiFi_scanNetworks(false, false);
}

bool AsyncWiFiManager::WiFi_scanNetworks(unsigned int cachetime, bool async)
{
    return WiFi_scanNetworks(millis() - _lastscan > cachetime, async);
}
bool AsyncWiFiManager::WiFi_scanNetworks(unsigned int cachetime)
{
    return WiFi_scanNetworks(millis() - _lastscan > cachetime, false);
}
bool AsyncWiFiManager::WiFi_scanNetworks(bool force, bool async)
{
    // DEBUG_WM(DEBUG_DEV,"scanNetworks async:",async == true);
    // DEBUG_WM(DEBUG_DEV,_numNetworks,(millis()-_lastscan ));
    // DEBUG_WM(DEBUG_DEV,"scanNetworks force:",force == true);
    if (force || _numNetworks == 0 || (millis() - _lastscan > 60000))
    {
        int8_t res;
        _startscan = millis();
        if (async && _asyncScan)
        {
#ifdef ESP8266
#ifndef WM_NOASYNC // no async available < 2.4.0
            DEBUG_WM(DEBUG_VERBOSE, F("WiFi Scan ASYNC started"));
            using namespace std::placeholders; // for `_1`
            WiFi.scanNetworksAsync(std::bind(&AsyncWiFiManager::WiFi_scanComplete, this, _1));
#else
            res = WiFi.scanNetworks();
#endif
#else
            DEBUG_WM(DEBUG_VERBOSE, F("Async WiFi scan started."));
            res = WiFi.scanNetworks(true);
#endif
            return false;
        }
        else
        {
            res = WiFi.scanNetworks();
        }
        if (res == WIFI_SCAN_FAILED)
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] scan failed."));
        else if (res == WIFI_SCAN_RUNNING)
        {
            DEBUG_WM(DEBUG_ERROR, F("[ERROR] scan waiting."), false);
            while (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
            {
                if (_debug)
                    Serial.print(".");
                delay(100);
            }
            if (_debug)
                Serial.println();
            _numNetworks = WiFi.scanComplete();
        }
        else if (res >= 0)
            _numNetworks = res;
        _lastscan = millis();
        DEBUG_WM(DEBUG_VERBOSE, F("WiFi scan completed"), "in " + (String)(_lastscan - _startscan) + F(" ms."));
        return true;
    }
    else
        DEBUG_WM(DEBUG_VERBOSE, F("Scan was cached"), (String)(millis() - _lastscan) + F(" ms ago."));
    return false;
}

String AsyncWiFiManager::AsyncWiFiManager::getScanItemOut()
{
    String page;

    if (!_numNetworks)
        WiFi_scanNetworks(); // scan in case this gets called before any scans

    int n = _numNetworks;
    if (n == 0)
    {
        DEBUG_WM(F("No networks found."));
        page += FPSTR(S_nonetworks); // @token nonetworks
    }
    else
    {
        DEBUG_WM(n, F("networks found."));
        //sort networks
        int indices[n];
        for (int i = 0; i < n; i++)
        {
            indices[i] = i;
        }

        // RSSI SORT
        for (int i = 0; i < n; i++)
        {
            for (int j = i + 1; j < n; j++)
            {
                if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
                {
                    std::swap(indices[i], indices[j]);
                }
            }
        }

        /* test std:sort
        std::sort(indices, indices + n, [](const int & a, const int & b) -> bool
        {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
        });
       */

        // remove duplicates ( must be RSSI sorted )
        if (_removeDuplicateAPs)
        {
            String cssid;
            for (int i = 0; i < n; i++)
            {
                if (indices[i] == -1)
                    continue;
                cssid = WiFi.SSID(indices[i]);
                for (int j = i + 1; j < n; j++)
                {
                    if (cssid == WiFi.SSID(indices[j]))
                    {
                        DEBUG_WM(DEBUG_VERBOSE, F("Duplicate AP:"), WiFi.SSID(indices[j]));
                        indices[j] = -1; // set dup aps to index -1
                    }
                }
            }
        }

        // token precheck, to speed up replacements on large ap lists
        String HTTP_ITEM_STR = FPSTR(HTTP_ITEM);

        // toggle icons with percentage
        HTTP_ITEM_STR.replace("{qp}", FPSTR(HTTP_ITEM_QP));
        HTTP_ITEM_STR.replace("{h}", _scanDispOptions ? "" : "h");
        HTTP_ITEM_STR.replace("{qi}", FPSTR(HTTP_ITEM_QI));
        HTTP_ITEM_STR.replace("{h}", _scanDispOptions ? "h" : "");

        // set token precheck flags
        bool tok_r = HTTP_ITEM_STR.indexOf(FPSTR(T_r)) > 0;
        bool tok_R = HTTP_ITEM_STR.indexOf(FPSTR(T_R)) > 0;
        bool tok_e = HTTP_ITEM_STR.indexOf(FPSTR(T_e)) > 0;
        bool tok_q = HTTP_ITEM_STR.indexOf(FPSTR(T_q)) > 0;
        bool tok_i = HTTP_ITEM_STR.indexOf(FPSTR(T_i)) > 0;

        //display networks in page
        for (int i = 0; i < n; i++)
        {
            if (indices[i] == -1)
                continue; // skip dups

            DEBUG_WM(DEBUG_VERBOSE, F("AP:"), (String)WiFi.RSSI(indices[i]) + " " + (String)WiFi.SSID(indices[i]));

            int rssiperc = getRSSIasQuality(WiFi.RSSI(indices[i]));
            uint8_t enc_type = WiFi.encryptionType(indices[i]);

            if (_minimumQuality == -1 || _minimumQuality < rssiperc)
            {
                String item = HTTP_ITEM_STR;
                item.replace(FPSTR(T_v), htmlEntities(WiFi.SSID(indices[i]))); // ssid no encoding
                if (tok_e)
                    item.replace(FPSTR(T_e), encryptionTypeStr(enc_type));
                if (tok_r)
                    item.replace(FPSTR(T_r), (String)rssiperc); // rssi percentage 0-100
                if (tok_R)
                    item.replace(FPSTR(T_R), (String)WiFi.RSSI(indices[i])); // rssi db
                if (tok_q)
                    item.replace(FPSTR(T_q), (String) int(round(map(rssiperc, 0, 100, 1, 4)))); //quality icon 1-4
                if (tok_i)
                {
                    if (enc_type != WM_WIFIOPEN)
                    {
                        item.replace(FPSTR(T_i), F("l"));
                    }
                    else
                    {
                        item.replace(FPSTR(T_i), "");
                    }
                }
                //DEBUG_WM(item);
                page += item;
                delay(0);
            }
            else
            {
                DEBUG_WM(DEBUG_VERBOSE, F("Skipping , does not meet _minimumQuality."));
            }
        }
        page += FPSTR(HTTP_BR);
    }

    return page;
}

String AsyncWiFiManager::getIpForm(String id, String title, String value)
{
    String item = FPSTR(HTTP_FORM_LABEL);
    item += FPSTR(HTTP_FORM_PARAM);
    item.replace(FPSTR(T_i), id);
    item.replace(FPSTR(T_n), id);
    item.replace(FPSTR(T_p), FPSTR(T_t));
    // item.replace(FPSTR(T_p), default);
    item.replace(FPSTR(T_t), title);
    item.replace(FPSTR(T_l), F("15"));
    item.replace(FPSTR(T_v), value);
    item.replace(FPSTR(T_c), "");
    return item;
}

String AsyncWiFiManager::getStaticOut()
{
    String page;
    if ((_staShowStaticFields || _sta_static_ip) && _staShowStaticFields >= 0)
    {
        DEBUG_WM(DEBUG_DEV, F("_staShowStaticFields"));
        page += FPSTR(HTTP_FORM_STATIC_HEAD);
        // @todo how can we get these accurate settings from memory , wifi_get_ip_info does not seem to reveal if struct ip_info is static or not
        page += getIpForm(FPSTR(S_ip), FPSTR(S_staticip), (_sta_static_ip ? _sta_static_ip.toString() : "")); // @token staticip
        // WiFi.localIP().toString();
        page += getIpForm(FPSTR(S_gw), FPSTR(S_staticgw), (_sta_static_gw ? _sta_static_gw.toString() : "")); // @token staticgw
        // WiFi.gatewayIP().toString();
        page += getIpForm(FPSTR(S_sn), FPSTR(S_subnet), (_sta_static_sn ? _sta_static_sn.toString() : "")); // @token subnet
                                                                                                            // WiFi.subnetMask().toString();
    }

    if ((_staShowDns || _sta_static_dns) && _staShowDns >= 0)
    {
        page += getIpForm(FPSTR(S_dns), FPSTR(S_staticdns), (_sta_static_dns ? _sta_static_dns.toString() : "")); // @token dns
    }

    if (page != "")
        page += FPSTR(HTTP_BR); // @todo remove these, use css

    return page;
}

String AsyncWiFiManager::getParamOut()
{
    String page;

    if (_paramsCount > 0)
    {

        String HTTP_PARAM_temp = FPSTR(HTTP_FORM_LABEL);
        HTTP_PARAM_temp += FPSTR(HTTP_FORM_PARAM);
        bool tok_I = HTTP_PARAM_temp.indexOf(FPSTR(T_I)) > 0;
        bool tok_i = HTTP_PARAM_temp.indexOf(FPSTR(T_i)) > 0;
        bool tok_n = HTTP_PARAM_temp.indexOf(FPSTR(T_n)) > 0;
        bool tok_p = HTTP_PARAM_temp.indexOf(FPSTR(T_p)) > 0;
        bool tok_t = HTTP_PARAM_temp.indexOf(FPSTR(T_t)) > 0;
        bool tok_l = HTTP_PARAM_temp.indexOf(FPSTR(T_l)) > 0;
        bool tok_v = HTTP_PARAM_temp.indexOf(FPSTR(T_v)) > 0;
        bool tok_c = HTTP_PARAM_temp.indexOf(FPSTR(T_c)) > 0;

        char valLength[5];
        // add the extra parameters to the form
        for (int i = 0; i < _paramsCount; i++)
        {
            if (_params[i] == NULL || _params[i]->_length == 0)
            {
                DEBUG_WM(DEBUG_ERROR, F("[ERROR] AsyncWiFiManagerParameter is out of scope."));
                break;
            }

            // label before or after, @todo this could be done via floats or CSS and eliminated
            String pitem;
            switch (_params[i]->getLabelPlacement())
            {
            case WFM_LABEL_BEFORE:
                pitem = FPSTR(HTTP_FORM_LABEL);
                pitem += FPSTR(HTTP_FORM_PARAM);
                break;
            case WFM_LABEL_AFTER:
                pitem = FPSTR(HTTP_FORM_PARAM);
                pitem += FPSTR(HTTP_FORM_LABEL);
                break;
            default:
                // WFM_NO_LABEL
                pitem = FPSTR(HTTP_FORM_PARAM);
                break;
            }

            // Input templating
            // "<br/><input id='{i}' name='{n}' maxlength='{l}' value='{v}' {c}>";
            // if no ID use customhtml for item, else generate from param string
            if (_params[i]->getID() != NULL)
            {
                if (tok_I)
                    pitem.replace(FPSTR(T_I), (String)FPSTR(S_parampre) + (String)i); // T_I id number
                if (tok_i)
                    pitem.replace(FPSTR(T_i), _params[i]->getID()); // T_i id name
                if (tok_n)
                    pitem.replace(FPSTR(T_n), _params[i]->getID()); // T_n id name alias
                if (tok_p)
                    pitem.replace(FPSTR(T_p), FPSTR(T_t)); // T_p replace legacy placeholder token
                if (tok_t)
                    pitem.replace(FPSTR(T_t), _params[i]->getLabel()); // T_t title/label
                snprintf(valLength, 5, "%d", _params[i]->getValueLength());
                if (tok_l)
                    pitem.replace(FPSTR(T_l), valLength); // T_l value length
                if (tok_v)
                    pitem.replace(FPSTR(T_v), _params[i]->getValue()); // T_v value
                if (tok_c)
                    pitem.replace(FPSTR(T_c), _params[i]->getCustomHTML()); // T_c meant for additional attributes, not html, but can stuff
            }
            else
            {
                pitem = _params[i]->getCustomHTML();
            }

            page += pitem;
        }
    }

    return page;
}

void AsyncWiFiManager::handleWiFiStatus()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP WiFi status "));
    handleRequest();
    String page;
// String page = "{\"result\":true,\"count\":1}";
#ifdef WM_JSTEST
    page = FPSTR(HTTP_JS);
#endif
    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);
}

/** 
 * HTTPD CALLBACK save form and redirect to WLAN config page again
 */
void AsyncWiFiManager::handleWifiSave()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP WiFi save "));
    DEBUG_WM(DEBUG_DEV, F("Method:"), server->method() == HTTP_GET ? (String)FPSTR(S_GET) : (String)FPSTR(S_POST));
    handleRequest();

    // @todo use new callback for before paramsaves
    if (_presavecallback != NULL)
    {
        _presavecallback();
    }

    //SAVE/connect here
    _ssid = server->arg(F("s")).c_str();
    _pass = server->arg(F("p")).c_str();

    if (_paramsInWifi)
        doParamSave();

    if (server->arg(FPSTR(S_ip)) != "")
    {
        //_sta_static_ip.fromString(server->arg(FPSTR(S_ip));
        String ip = server->arg(FPSTR(S_ip));
        optionalIPFromString(&_sta_static_ip, ip.c_str());
        DEBUG_WM(DEBUG_DEV, F("Static IP:"), ip);
    }
    if (server->arg(FPSTR(S_gw)) != "")
    {
        String gw = server->arg(FPSTR(S_gw));
        optionalIPFromString(&_sta_static_gw, gw.c_str());
        DEBUG_WM(DEBUG_DEV, F("Static gateway:"), gw);
    }
    if (server->arg(FPSTR(S_sn)) != "")
    {
        String sn = server->arg(FPSTR(S_sn));
        optionalIPFromString(&_sta_static_sn, sn.c_str());
        DEBUG_WM(DEBUG_DEV, F("Static netmask:"), sn);
    }
    if (server->arg(FPSTR(S_dns)) != "")
    {
        String dns = server->arg(FPSTR(S_dns));
        optionalIPFromString(&_sta_static_dns, dns.c_str());
        DEBUG_WM(DEBUG_DEV, F("Static DNS:"), dns);
    }

    String page;

    if (_ssid == "")
    {
        page = getHTTPHead(FPSTR(S_titlewifisettings)); // @token titleparamsaved
        page += FPSTR(HTTP_PARAMSAVED);
    }
    else
    {
        page = getHTTPHead(FPSTR(S_titlewifisaved)); // @token titlewifisaved
        page += FPSTR(HTTP_SAVED);
    }
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->sendHeader(FPSTR(HTTP_HEAD_CORS), FPSTR(HTTP_HEAD_CORS_ALLOW_ALL));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);

    DEBUG_WM(DEBUG_DEV, F("Sent WiFi save page."));

    connect = true; //signal ready to connect/reset process in processConfigPortal
}

void AsyncWiFiManager::handleParamSave()
{

    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP WiFi save "));
    DEBUG_WM(DEBUG_DEV, F("Method:"), server->method() == HTTP_GET ? (String)FPSTR(S_GET) : (String)FPSTR(S_POST));
    handleRequest();

    doParamSave();

    String page = getHTTPHead(FPSTR(S_titleparamsaved)); // @token titleparamsaved
    page += FPSTR(HTTP_PARAMSAVED);
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);

    DEBUG_WM(DEBUG_DEV, F("Sent param save page."));
}

void AsyncWiFiManager::doParamSave()
{
    // @todo use new callback for before paramsaves, is this really needed?
    if (_presavecallback != NULL)
    {
        _presavecallback();
    }

    //parameters
    if (_paramsCount > 0)
    {
        DEBUG_WM(DEBUG_VERBOSE, F("Parameters"));
        DEBUG_WM(DEBUG_VERBOSE, FPSTR(D_HR));

        for (int i = 0; i < _paramsCount; i++)
        {
            if (_params[i] == NULL || _params[i]->_length == 0)
            {
                DEBUG_WM(DEBUG_ERROR, F("[ERROR] AsyncWiFiManagerParameter is out of scope."));
                break; // @todo might not be needed anymore
            }
            //read parameter from server
            String name = (String)FPSTR(S_parampre) + (String)i;
            String value;
            if (server->hasArg(name))
            {
                value = server->arg(name);
            }
            else
            {
                value = server->arg(_params[i]->getID());
            }

            //store it in params array
            value.toCharArray(_params[i]->_value, _params[i]->_length + 1); // length+1 null terminated
            DEBUG_WM(DEBUG_VERBOSE, (String)_params[i]->getID() + F(":"), value);
        }
        DEBUG_WM(DEBUG_VERBOSE, FPSTR(D_HR));
    }

    if (_saveparamscallback != NULL)
    {
        _saveparamscallback();
    }
}

/** 
 * HTTPD CALLBACK info page
 */
void AsyncWiFiManager::handleInfo()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Info"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titleinfo)); // @token titleinfo
    reportStatus(page);

    uint16_t infos = 0;

//@todo convert to enum or refactor to strings
//@todo wrap in build flag to remove all info code for memory saving
#ifdef ESP8266
    infos = 27;
    String infoids[] = {
        F("esphead"),
        F("uptime"),
        F("chipid"),
        F("fchipid"),
        F("idesize"),
        F("flashsize"),
        F("sdkver"),
        F("corever"),
        F("bootver"),
        F("cpufreq"),
        F("freeheap"),
        F("memsketch"),
        F("memsmeter"),
        F("lastreset"),
        F("wifihead"),
        F("apip"),
        F("apmac"),
        F("apssid"),
        F("apbssid"),
        F("staip"),
        F("stagw"),
        F("stasub"),
        F("dnss"),
        F("host"),
        F("stamac"),
        F("conx"),
        F("autoconx")};

#elif defined(ESP32)
    infos = 22;
    String infoids[] = {
        F("esphead"),
        F("uptime"),
        F("chipid"),
        F("chiprev"),
        F("idesize"),
        F("sdkver"),
        F("cpufreq"),
        F("freeheap"),
        F("lastreset"),
        // F("temp"),
        F("wifihead"),
        F("apip"),
        F("apmac"),
        F("aphost"),
        F("apssid"),
        F("apbssid"),
        F("staip"),
        F("stagw"),
        F("stasub"),
        F("dnss"),
        F("host"),
        F("stamac"),
        F("conx")};
#endif

    for (size_t i = 0; i < infos; i++)
    {
        if (infoids[i] != NULL)
            page += getInfoData(infoids[i]);
    }
    page += F("</dl>");
    if (_showInfoErase)
        page += FPSTR(HTTP_ERASEBTN);
    if (_showBack)
        page += FPSTR(HTTP_BACKBTN);
    page += FPSTR(HTTP_HELP);
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);

    DEBUG_WM(DEBUG_DEV, F("Sent info page."));
}

String AsyncWiFiManager::getInfoData(String id)
{

    String p;
    // @todo add WM versioning
    if (id == F("esphead"))
        p = FPSTR(HTTP_INFO_esphead);
    else if (id == F("wifihead"))
        p = FPSTR(HTTP_INFO_wifihead);
    else if (id == F("uptime"))
    {
        // subject to rollover!
        p = FPSTR(HTTP_INFO_uptime);
        p.replace(FPSTR(T_1), (String)(millis() / 1000 / 60));
        p.replace(FPSTR(T_2), (String)((millis() / 1000) % 60));
    }
    else if (id == F("chipid"))
    {
        p = FPSTR(HTTP_INFO_chipid);
        p.replace(FPSTR(T_1), String(WIFI_getChipId(), HEX));
    }
#ifdef ESP32
    else if (id == F("chiprev"))
    {
        p = FPSTR(HTTP_INFO_chiprev);
        String rev = (String)ESP.getChipRevision();
#ifdef _SOC_EFUSE_REG_H_
        String revb = (String)(REG_READ(EFUSE_BLK0_RDATA3_REG) >> (EFUSE_RD_CHIP_VER_RESERVE_S) && EFUSE_RD_CHIP_VER_RESERVE_V);
        p.replace(FPSTR(T_1), rev + "<br/>" + revb);
#else
        p.replace(FPSTR(T_1), rev);
#endif
    }
#endif
#ifdef ESP8266
    else if (id == F("fchipid"))
    {
        p = FPSTR(HTTP_INFO_fchipid);
        p.replace(FPSTR(T_1), (String)ESP.getFlashChipId());
    }
#endif
    else if (id == F("idesize"))
    {
        p = FPSTR(HTTP_INFO_idesize);
        p.replace(FPSTR(T_1), (String)ESP.getFlashChipSize());
    }
    else if (id == F("flashsize"))
    {
#ifdef ESP8266
        p = FPSTR(HTTP_INFO_flashsize);
        p.replace(FPSTR(T_1), (String)ESP.getFlashChipRealSize());
#endif
    }
    else if (id == F("sdkver"))
    {
        p = FPSTR(HTTP_INFO_sdkver);
#ifdef ESP32
        p.replace(FPSTR(T_1), (String)esp_get_idf_version());
#else
        p.replace(FPSTR(T_1), (String)system_get_sdk_version());
#endif
    }
    else if (id == F("corever"))
    {
#ifdef ESP8266
        p = FPSTR(HTTP_INFO_corever);
        p.replace(FPSTR(T_1), (String)ESP.getCoreVersion());
#endif
    }
#ifdef ESP8266
    else if (id == F("bootver"))
    {
        p = FPSTR(HTTP_INFO_bootver);
        p.replace(FPSTR(T_1), (String)system_get_boot_version());
    }
#endif
    else if (id == F("cpufreq"))
    {
        p = FPSTR(HTTP_INFO_cpufreq);
        p.replace(FPSTR(T_1), (String)ESP.getCpuFreqMHz());
    }
    else if (id == F("freeheap"))
    {
        p = FPSTR(HTTP_INFO_freeheap);
        p.replace(FPSTR(T_1), (String)ESP.getFreeHeap());
    }
#ifdef ESP8266
    else if (id == F("memsketch"))
    {
        p = FPSTR(HTTP_INFO_memsketch);
        p.replace(FPSTR(T_1), (String)(ESP.getSketchSize()));
        p.replace(FPSTR(T_2), (String)(ESP.getSketchSize() + ESP.getFreeSketchSpace()));
    }
#endif
#ifdef ESP8266
    else if (id == F("memsmeter"))
    {
        p = FPSTR(HTTP_INFO_memsmeter);
        p.replace(FPSTR(T_1), (String)(ESP.getSketchSize()));
        p.replace(FPSTR(T_2), (String)(ESP.getSketchSize() + ESP.getFreeSketchSpace()));
    }
#endif
    else if (id == F("lastreset"))
    {
#ifdef ESP8266
        p = FPSTR(HTTP_INFO_lastreset);
        p.replace(FPSTR(T_1), (String)ESP.getResetReason());
#elif defined(ESP32) && defined(_ROM_RTC_H_)
        // requires #include <rom/rtc.h>
        p = FPSTR(HTTP_INFO_lastreset);
        for (int i = 0; i < 2; i++)
        {
            int reason = rtc_get_reset_reason(i);
            String tok = (String)T_ss + (String)(i + 1) + (String)T_es;
            switch (reason)
            {
            //@todo move to array
            case 1:
                p.replace(tok, F("Vbat power on reset"));
                break;
            case 3:
                p.replace(tok, F("Software reset digital core"));
                break;
            case 4:
                p.replace(tok, F("Legacy watch dog reset digital core"));
                break;
            case 5:
                p.replace(tok, F("Deep Sleep reset digital core"));
                break;
            case 6:
                p.replace(tok, F("Reset by SLC module, reset digital core"));
                break;
            case 7:
                p.replace(tok, F("Timer Group0 Watch dog reset digital core"));
                break;
            case 8:
                p.replace(tok, F("Timer Group1 Watch dog reset digital core"));
                break;
            case 9:
                p.replace(tok, F("RTC Watch dog Reset digital core"));
                break;
            case 10:
                p.replace(tok, F("Instrusion tested to reset CPU"));
                break;
            case 11:
                p.replace(tok, F("Time Group reset CPU"));
                break;
            case 12:
                p.replace(tok, F("Software reset CPU"));
                break;
            case 13:
                p.replace(tok, F("RTC Watch dog Reset CPU"));
                break;
            case 14:
                p.replace(tok, F("for APP CPU, reseted by PRO CPU"));
                break;
            case 15:
                p.replace(tok, F("Reset when the vdd voltage is not stable"));
                break;
            case 16:
                p.replace(tok, F("RTC Watch dog reset digital core and rtc module"));
                break;
            default:
                p.replace(tok, F("NO_MEAN"));
            }
        }
#endif
    }
    else if (id == F("apip"))
    {
        p = FPSTR(HTTP_INFO_apip);
        p.replace(FPSTR(T_1), WiFi.softAPIP().toString());
    }
    else if (id == F("apmac"))
    {
        p = FPSTR(HTTP_INFO_apmac);
        p.replace(FPSTR(T_1), (String)WiFi.softAPmacAddress());
    }
#ifdef ESP32
    else if (id == F("aphost"))
    {
        p = FPSTR(HTTP_INFO_aphost);
        p.replace(FPSTR(T_1), WiFi.softAPgetHostname());
    }
#endif
    else if (id == F("apssid"))
    {
        p = FPSTR(HTTP_INFO_apssid);
        p.replace(FPSTR(T_1), htmlEntities((String)WiFi_SSID()));
    }
    else if (id == F("apbssid"))
    {
        p = FPSTR(HTTP_INFO_apbssid);
        p.replace(FPSTR(T_1), (String)WiFi.BSSIDstr());
    }
    else if (id == F("staip"))
    {
        p = FPSTR(HTTP_INFO_staip);
        p.replace(FPSTR(T_1), WiFi.localIP().toString());
    }
    else if (id == F("stagw"))
    {
        p = FPSTR(HTTP_INFO_stagw);
        p.replace(FPSTR(T_1), WiFi.gatewayIP().toString());
    }
    else if (id == F("stasub"))
    {
        p = FPSTR(HTTP_INFO_stasub);
        p.replace(FPSTR(T_1), WiFi.subnetMask().toString());
    }
    else if (id == F("dnss"))
    {
        p = FPSTR(HTTP_INFO_dnss);
        p.replace(FPSTR(T_1), WiFi.dnsIP().toString());
    }
    else if (id == F("host"))
    {
        p = FPSTR(HTTP_INFO_host);
#ifdef ESP32
        p.replace(FPSTR(T_1), WiFi.getHostname());
#else
        p.replace(FPSTR(T_1), WiFi.hostname());
#endif
    }
    else if (id == F("stamac"))
    {
        p = FPSTR(HTTP_INFO_stamac);
        p.replace(FPSTR(T_1), WiFi.macAddress());
    }
    else if (id == F("conx"))
    {
        p = FPSTR(HTTP_INFO_conx);
        p.replace(FPSTR(T_1), WiFi.isConnected() ? FPSTR(S_y) : FPSTR(S_n));
    }
#ifdef ESP8266
    else if (id == F("autoconx"))
    {
        p = FPSTR(HTTP_INFO_autoconx);
        p.replace(FPSTR(T_1), WiFi.getAutoConnect() ? FPSTR(S_enable) : FPSTR(S_disable));
    }
#endif
#ifdef ESP32
    else if (id == F("temp"))
    {
        // temperature is not calibrated, varying large offsets are present, use for relative temp changes only
        p = FPSTR(HTTP_INFO_temp);
        p.replace(FPSTR(T_1), (String)temperatureRead());
        p.replace(FPSTR(T_2), (String)((temperatureRead() + 32) * 1.8));
    }
#endif
    return p;
}

/** 
 * HTTPD CALLBACK root or redirect to captive portal
 */
void AsyncWiFiManager::handleExit()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Exit"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titleexit)); // @token titleexit
    page += FPSTR(S_exiting);                      // @token exiting
    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);
    abort = true;
}

/** 
 * HTTPD CALLBACK reset page
 */
void AsyncWiFiManager::handleReset()
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Reset"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titlereset)); //@token titlereset
    page += FPSTR(S_resetting);                     //@token resetting
    page += FPSTR(HTTP_END);

    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);

    DEBUG_WM(F("Resetting ESP."));
    delay(1000);
    reboot();
}

/** 
 * HTTPD CALLBACK erase page
 */

// void AsyncWiFiManager::handleErase() {
//   handleErase(false);
// }
void AsyncWiFiManager::handleErase(boolean opt)
{
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP Erase"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titleerase)); // @token titleerase

    bool ret = erase(opt);

    if (ret)
        page += FPSTR(S_resetting); // @token resetting
    else
    {
        page += FPSTR(S_error); // @token erroroccur
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] WiFi EraseConfig failed."));
    }

    page += FPSTR(HTTP_END);
    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);

    if (ret)
    {
        delay(2000);
        DEBUG_WM(F("Resetting ESP."));
        reboot();
    }
}

/** 
 * HTTPD CALLBACK 404
 */
void AsyncWiFiManager::handleNotFound()
{
    if (captivePortal())
        return; // If captive portal redirect instead of displaying the page
    handleRequest();
    String message = FPSTR(S_notfound); // @token notfound
    message += FPSTR(S_uri);            // @token uri
    message += server->uri();
    message += FPSTR(S_method); // @token method
    message += (server->method() == HTTP_GET) ? FPSTR(S_GET) : FPSTR(S_POST);
    message += FPSTR(S_args); // @token args
    message += server->args();
    message += F("\n");

    for (uint8_t i = 0; i < server->args(); i++)
    {
        message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    }
    server->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    server->sendHeader(F("Pragma"), F("no-cache"));
    server->sendHeader(F("Expires"), F("-1"));
    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(message.length()));
    server->send(404, FPSTR(HTTP_HEAD_CT2), message);
}

/**
 * HTTPD redirector
 * Redirect to captive portal if we got a request for another domain. 
 * Return true in that case so the page handler do not try to handle the request again. 
 */
boolean AsyncWiFiManager::captivePortal()
{
    DEBUG_WM(DEBUG_DEV, "-> " + server->hostHeader());

    if (!_enableCaptivePortal)
        return false; // skip redirections

    String serverLoc = toStringIp(server->client().localIP());
    if (_httpPort != 80)
        serverLoc += ":" + (String)_httpPort;            // add port if not default
    bool doredirect = serverLoc != server->hostHeader(); // redirect if hostheader not server ip, prevent redirect loops
    // doredirect = !isIp(server->hostHeader()) // old check

    if (doredirect)
    // if (!isIp(server->hostHeader()))
    {
        DEBUG_WM(DEBUG_VERBOSE, F("<- Request redirected to captive portal."));
        server->sendHeader(F("Location"), (String)F("http://") + serverLoc, true);
        server->send(302, FPSTR(HTTP_HEAD_CT2), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server->client().stop();                     // Stop is needed because we sent no content length
        return true;
    }
    return false;
}

void AsyncWiFiManager::stopCaptivePortal()
{
    _enableCaptivePortal = false;
    // @todo maybe disable configportaltimeout(optional), or just provide callback for user
}

void AsyncWiFiManager::handleClose()
{
    stopCaptivePortal();
    DEBUG_WM(DEBUG_VERBOSE, F("<- HTTP close"));
    handleRequest();
    String page = getHTTPHead(FPSTR(S_titleclose)); // @token titleclose
    page += FPSTR(S_closing);                       // @token closing
    server->sendHeader(FPSTR(HTTP_HEAD_CL), String(page.length()));
    server->send(200, FPSTR(HTTP_HEAD_CT), page);
}

void AsyncWiFiManager::reportStatus(String &page)
{
    updateConxResult(WiFi.status());
    String str;
    if (WiFi_SSID() != "")
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            str = FPSTR(HTTP_STATUS_ON);
            str.replace(FPSTR(T_i), WiFi.localIP().toString());
            str.replace(FPSTR(T_v), htmlEntities(WiFi_SSID()));
        }
        else
        {
            str = FPSTR(HTTP_STATUS_OFF);
            str.replace(FPSTR(T_v), htmlEntities(WiFi_SSID()));
            if (_lastconxresult == WL_STATION_WRONG_PASSWORD)
            {
                // wrong password
                str.replace(FPSTR(T_c), "D"); // class
                str.replace(FPSTR(T_r), FPSTR(HTTP_STATUS_OFFPW));
            }
            else if (_lastconxresult == WL_NO_SSID_AVAIL)
            {
                // connect failed, or ap not found
                str.replace(FPSTR(T_c), "D");
                str.replace(FPSTR(T_r), FPSTR(HTTP_STATUS_OFFNOAP));
            }
            else if (_lastconxresult == WL_CONNECT_FAILED)
            {
                // connect failed
                str.replace(FPSTR(T_c), "D");
                str.replace(FPSTR(T_r), FPSTR(HTTP_STATUS_OFFFAIL));
            }
            else
            {
                str.replace(FPSTR(T_c), "");
                str.replace(FPSTR(T_r), "");
            }
        }
    }
    else
    {
        str = FPSTR(HTTP_STATUS_NONE);
    }
    page += str;
}

// PUBLIC

// METHODS

/**
 * reset wifi settings, clean stored ap password
 */

/**
 * [stopConfigPortal description]
 * @return {[type]} [description]
 */
bool AsyncWiFiManager::stopConfigPortal()
{
    if (_configPortalIsBlocking)
    {
        abort = true;
        return true;
    }
    return shutdownConfigPortal();
}

/**
 * disconnect
 * @access public
 * @since $dev
 * @return bool success
 */
bool AsyncWiFiManager::disconnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_WM(DEBUG_VERBOSE, F("Disconnecting: Not connected."));
        return false;
    }
    DEBUG_WM(F("Disconnecting."));
    return WiFi_Disconnect();
}

/**
 * reboot the device
 * @access public
 */
void AsyncWiFiManager::reboot()
{
    DEBUG_WM(F("Restarting."));
    ESP.restart();
}

/**
 * reboot the device
 * @access public
 */
bool AsyncWiFiManager::erase()
{
    return erase(false);
}

bool AsyncWiFiManager::erase(bool opt)
{
    DEBUG_WM(F("Erasing."));

#if defined(ESP32) && ((defined(WM_ERASE_NVS) || defined(nvs_flash_h)))
    // if opt true, do nvs erase
    if (opt)
    {
        DEBUG_WM(F("Erasing NVS."));
        esp_err_t err;
        err = nvs_flash_init();
        DEBUG_WM(DEBUG_VERBOSE, F("nvs_flash_init: "), err != ESP_OK ? (String)err : F("Success."));
        err = nvs_flash_erase();
        DEBUG_WM(DEBUG_VERBOSE, F("nvs_flash_erase: "), err != ESP_OK ? (String)err : F("Success."));
        return err == ESP_OK;
    }
#elif defined(ESP8266) && defined(spiffs_api_h)
    if (opt)
    {
        bool ret = false;
        if (SPIFFS.begin())
        {
            DEBUG_WM(F("Erasing SPIFFS."));
            bool ret = SPIFFS.format();
            DEBUG_WM(DEBUG_VERBOSE, F("SPIFFS erase: "), ret ? F("Success") : F("ERROR"));
        }
        else
            DEBUG_WM(F("[ERROR] Could not start SPIFFS."));
        return ret;
    }
#else
    (void)opt;
#endif

    DEBUG_WM(F("Erasing WiFi config."));
    return WiFi_eraseConfig();
}

/**
 * [resetSettings description]
 * ERASES STA CREDENTIALS
 * @access public
 */
void AsyncWiFiManager::resetSettings()
{
    DEBUG_WM(F("resetSettings"));
    WiFi_enableSTA(true, true); // must be sta to disconnect erase

    if (_resetcallback != NULL)
        _resetcallback();

#ifdef ESP32
    WiFi.disconnect(true, true);
#else
    WiFi.persistent(true);
    WiFi.disconnect(true);
    WiFi.persistent(false);
#endif
    DEBUG_WM(F("Settings erased."));
}

// SETTERS

/**
 * [setTimeout description]
 * @access public
 * @param {[type]} unsigned long seconds [description]
 */
void AsyncWiFiManager::setTimeout(unsigned long seconds)
{
    setConfigPortalTimeout(seconds);
}

/**
 * [setConfigPortalTimeout description]
 * @access public
 * @param {[type]} unsigned long seconds [description]
 */
void AsyncWiFiManager::setConfigPortalTimeout(unsigned long seconds)
{
    _configPortalTimeout = seconds * 1000;
}

/**
 * [setConnectTimeout description]
 * @access public
 * @param {[type]} unsigned long seconds [description]
 */
void AsyncWiFiManager::setConnectTimeout(unsigned long seconds)
{
    _connectTimeout = seconds * 1000;
}

/**
 * [setConnectRetries description]
 * @access public
 * @param {[type]} uint8_t numRetries [description]
 */
void AsyncWiFiManager::setConnectRetries(uint8_t numRetries)
{
    _connectRetries = constrain(numRetries, 1, 10);
}

/**
 * toggle _cleanconnect, always disconnect before connecting
 * @param {[type]} bool enable [description]
 */
void AsyncWiFiManager::setCleanConnect(bool enable)
{
    _cleanConnect = enable;
}

/**
 * [setConnectTimeout description
 * @access public
 * @param {[type]} unsigned long seconds [description]
 */
void AsyncWiFiManager::setSaveConnectTimeout(unsigned long seconds)
{
    _saveTimeout = seconds * 1000;
}

/**
 * [setDebugOutput description]
 * @access public
 * @param {[type]} boolean debug [description]
 */
void AsyncWiFiManager::setDebugOutput(boolean debug)
{
    _debug = debug;
    if (_debug && _debugLevel == DEBUG_DEV)
        debugPlatformInfo();
}

/**
 * [setAPStaticIPConfig description]
 * @access public
 * @param {[type]} IPAddress ip [description]
 * @param {[type]} IPAddress gw [description]
 * @param {[type]} IPAddress sn [description]
 */
void AsyncWiFiManager::setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn)
{
    _ap_static_ip = ip;
    _ap_static_gw = gw;
    _ap_static_sn = sn;
}

/**
 * [setSTAStaticIPConfig description]
 * @access public
 * @param {[type]} IPAddress ip [description]
 * @param {[type]} IPAddress gw [description]
 * @param {[type]} IPAddress sn [description]
 */
void AsyncWiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn)
{
    _sta_static_ip = ip;
    _sta_static_gw = gw;
    _sta_static_sn = sn;
}

/**
 * [setSTAStaticIPConfig description]
 * @since $dev
 * @access public
 * @param {[type]} IPAddress ip [description]
 * @param {[type]} IPAddress gw [description]
 * @param {[type]} IPAddress sn [description]
 * @param {[type]} IPAddress dns [description]
 */
void AsyncWiFiManager::setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn, IPAddress dns)
{
    setSTAStaticIPConfig(ip, gw, sn);
    _sta_static_dns = dns;
}

/**
 * [setMinimumSignalQuality description]
 * @access public
 * @param {[type]} int quality [description]
 */
void AsyncWiFiManager::setMinimumSignalQuality(int quality)
{
    _minimumQuality = quality;
}

/**
 * [setBreakAfterConfig description]
 * @access public
 * @param {[type]} boolean shouldBreak [description]
 */
void AsyncWiFiManager::setBreakAfterConfig(boolean shouldBreak)
{
    _shouldBreakAfterConfig = shouldBreak;
}

/**
 * setAPCallback, set a callback when softap is started
 * @access public 
 * @param {[type]} void (*func)(AsyncWiFiManager* wminstance)
 */
void AsyncWiFiManager::setAPCallback(std::function<void(AsyncWiFiManager *)> func)
{
    _apcallback = func;
}

/**
 * setWebServerCallback, set a callback after webserver is reset, and before routes are setup
 * if we set webserver handlers before wm, they are used and wm is not by esp webserver
 * on events cannot be overrided once set, and are not mutiples
 * @access public 
 * @param {[type]} void (*func)(void)
 */
void AsyncWiFiManager::setWebServerCallback(std::function<void()> func)
{
    _webservercallback = func;
}

/**
 * setSaveConfigCallback, set a save config callback after closing configportal
 * @note calls only if wifi is saved or changed, or setBreakAfterConfig(true)
 * @access public
 * @param {[type]} void (*func)(void)
 */
void AsyncWiFiManager::setSaveConfigCallback(std::function<void()> func)
{
    _savewificallback = func;
}

/**
 * setConfigResetCallback, set a callback to occur when a resetSettings() occurs
 * @access public
 * @param {[type]} void(*func)(void)
 */
void AsyncWiFiManager::setConfigResetCallback(std::function<void()> func)
{
    _resetcallback = func;
}

/**
 * setSaveParamsCallback, set a save params callback on params save in wifi or params pages
 * @access public
 * @param {[type]} void (*func)(void)
 */
void AsyncWiFiManager::setSaveParamsCallback(std::function<void()> func)
{
    _saveparamscallback = func;
}

/**
 * setPreSaveConfigCallback, set a callback to fire before saving wifi or params
 * @access public
 * @param {[type]} void (*func)(void)
 */
void AsyncWiFiManager::setPreSaveConfigCallback(std::function<void()> func)
{
    _presavecallback = func;
}

/**
 * set custom head html
 * custom element will be added to head, eg. new style tag etc.
 * @access public
 * @param char element
 */
void AsyncWiFiManager::setCustomHeadElement(const char *element)
{
    _customHeadElement = element;
}

/**
 * toggle wifiscan hiding of duplicate ssid names
 * if this is false, wifiscan will remove duplicat Access Points - defaut true
 * @access public
 * @param boolean removeDuplicates [true]
 */
void AsyncWiFiManager::setRemoveDuplicateAPs(boolean removeDuplicates)
{
    _removeDuplicateAPs = removeDuplicates;
}

/**
 * toggle configportal blocking loop
 * if enabled, then the configportal will enter a blocking loop and wait for configuration
 * if disabled use with process() to manually process webserver
 * @since $dev
 * @access public
 * @param boolean shoudlBlock [false]
 */
void AsyncWiFiManager::setConfigPortalBlocking(boolean shoudlBlock)
{
    _configPortalIsBlocking = shoudlBlock;
}

/**
 * toggle restore persistent, track internally
 * sets ESP wifi.persistent so we can remember it and restore user preference on destruct
 * there is no getter in esp8266 platform prior to https://github.com/esp8266/Arduino/pull/3857
 * @since $dev
 * @access public
 * @param boolean persistent [true]
 */
void AsyncWiFiManager::setRestorePersistent(boolean persistent)
{
    _userpersistent = persistent;
    if (!persistent)
        DEBUG_WM(F("Persistent is off."));
}

/**
 * toggle showing static ip form fields
 * if enabled, then the static ip, gateway, subnet fields will be visible, even if not set in code
 * @since $dev
 * @access public
 * @param boolean alwaysShow [false]
 */
void AsyncWiFiManager::setShowStaticFields(boolean alwaysShow)
{
    if (_disableIpFields)
        _staShowStaticFields = alwaysShow ? 1 : -1;
    else
        _staShowStaticFields = alwaysShow ? 1 : 0;
}

/**
 * toggle showing dns fields
 * if enabled, then the dns1 field will be visible, even if not set in code
 * @since $dev
 * @access public
 * @param boolean alwaysShow [false]
 */
void AsyncWiFiManager::setShowDnsFields(boolean alwaysShow)
{
    if (_disableIpFields)
        _staShowDns = alwaysShow ? 1 : -1;
    _staShowDns = alwaysShow ? 1 : 0;
}

/**
 * toggle showing password in wifi password field
 * if not enabled, placeholder will be S_passph
 * @since $dev
 * @access public
 * @param boolean alwaysShow [false]
 */
void AsyncWiFiManager::setShowPassword(boolean show)
{
    _showPassword = show;
}

/**
 * toggle captive portal
 * if enabled, then devices that use captive portal checks will be redirected to root
 * if not you will automatically have to navigate to ip [192.168.4.1]
 * @since $dev
 * @access public
 * @param boolean enabled [true]
 */
void AsyncWiFiManager::setCaptivePortalEnable(boolean enabled)
{
    _enableCaptivePortal = enabled;
}

/**
 * toggle wifi autoreconnect policy
 * if enabled, then wifi will autoreconnect automatically always
 * On esp8266 we force this on when autoconnect is called, see notes
 * On esp32 this is handled on SYSTEM_EVENT_STA_DISCONNECTED since it does not exist in core yet
 * @since $dev
 * @access public
 * @param boolean enabled [true]
 */
void AsyncWiFiManager::setWiFiAutoReconnect(boolean enabled)
{
    _wifiAutoReconnect = enabled;
}

/**
 * toggle configportal timeout wait for station client
 * if enabled, then the configportal will start timeout when no stations are connected to softAP
 * disabled by default as rogue stations can keep it open if there is no auth
 * @since $dev
 * @access public
 * @param boolean enabled [false]
 */
void AsyncWiFiManager::setAPClientCheck(boolean enabled)
{
    _apClientCheck = enabled;
}

/**
 * toggle configportal timeout wait for web client
 * if enabled, then the configportal will restart timeout when client requests come in
 * @since $dev
 * @access public
 * @param boolean enabled [true]
 */
void AsyncWiFiManager::setWebPortalClientCheck(boolean enabled)
{
    _webClientCheck = enabled;
}

/**
 * toggle wifiscan percentages or quality icons
 * @since $dev
 * @access public
 * @param boolean enabled [false]
 */
void AsyncWiFiManager::setScanDispPerc(boolean enabled)
{
    _scanDispOptions = enabled;
}

/**
 * toggle configportal if autoconnect failed
 * if enabled, then the configportal will be activated on autoconnect failure
 * @since $dev
 * @access public
 * @param boolean enabled [true]
 */
void AsyncWiFiManager::setEnableConfigPortal(boolean enable)
{
    _enableConfigPortal = enable;
}

/**
 * set the hostname (dhcp client id)
 * @since $dev
 * @access public
 * @param  char* hostname 32 character hostname to use for sta+ap in esp32, sta in esp8266
 * @return bool false if hostname is not valid
 */
bool AsyncWiFiManager::setHostname(const char *hostname)
{
    //@todo max length 32
    _hostname = hostname;
    return true;
}

/**
 * set the soft ao channel, ignored if channelsync is true and connected
 * @param int32_t   wifi channel, 0 to disable
 */
void AsyncWiFiManager::setWiFiAPChannel(int32_t channel)
{
    _apChannel = channel;
}

/**
 * set the soft ap hidden
 * @param bool   wifi ap hidden, default is false
 */
void AsyncWiFiManager::setWiFiAPHidden(bool hidden)
{
    _apHidden = hidden;
}

/**
 * toggle showing erase wifi config button on info page
 * @param boolean enabled
 */
void AsyncWiFiManager::setShowInfoErase(boolean enabled)
{
    _showInfoErase = enabled;
}

/**
 * set menu items and order
 * if param is present in menu , params will be removed from wifi page automatically
 * eg.
 *  const char * menu[] = {"wifi","setup","sep","info","exit"};
 *  AsyncWiFiManager.setMenu(menu);
 * @since $dev
 * @param uint8_t menu[] array of menu ids
 */
void AsyncWiFiManager::setMenu(const char *menu[], uint8_t size)
{
    // DEBUG_WM(DEBUG_VERBOSE,"setmenu array");
    _menuIds.clear();
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < _nummenutokens; j++)
        {
            if (menu[i] == _menutokens[j])
            {
                if ((String)menu[i] == "param")
                    _paramsInWifi = false; // param auto flag
                _menuIds.push_back(j);
            }
        }
    }
    //DEBUG_WM(getMenuOut());
}

/**
 * setMenu with vector
 * eg.
 * std::vector<const char *> menu = {"wifi","setup","sep","info","exit"};
 * AsyncWiFiManager.setMenu(menu);
 * tokens can be found in _menutokens array in strings_en.h
 * @shiftIncrement $dev
 * @param {[type]} std::vector<const char *>& menu [description]
 */
void AsyncWiFiManager::setMenu(std::vector<const char *> &menu)
{
    // DEBUG_WM(DEBUG_VERBOSE,"setmenu vector");
    _menuIds.clear();
    for (auto menuitem : menu)
    {
        for (size_t j = 0; j < _nummenutokens; j++)
        {
            if (menuitem == _menutokens[j])
            {
                if ((String)menuitem == "param")
                    _paramsInWifi = false; // param auto flag
                _menuIds.push_back(j);
            }
        }
    }
    // DEBUG_WM(getMenuOut());
}

/**
 * set params as sperate page not in wifi
 * NOT COMPATIBLE WITH setMenu! @todo scan menuids and insert param after wifi or something
 * @param bool enable 
 * @since $dev
 */
void AsyncWiFiManager::setParamsPage(bool enable)
{
    _paramsInWifi = !enable;
    setMenu(enable ? _menuIdsParams : _menuIdsDefault);
}

// GETTERS

/**
 * get config portal AP SSID
 * @since 0.0.1
 * @access public
 * @return String the configportal ap name
 */
String AsyncWiFiManager::getConfigPortalSSID()
{
    return _apName;
}

/**
 * return the last known connection result
 * logged on autoconnect and wifisave, can be used to check why failed
 * get as readable string with getWLStatusString(getLastConxResult);
 * @since $dev
 * @access public
 * @return bool return wl_status codes
 */
uint8_t AsyncWiFiManager::getLastConxResult()
{
    return _lastconxresult;
}

/**
 * check if wifi has a saved ap or not
 * @since $dev
 * @access public
 * @return bool true if a saved ap config exists
 */
bool AsyncWiFiManager::getWiFiIsSaved()
{
    return WiFi_hasAutoConnect();
}

String AsyncWiFiManager::getDefaultAPName()
{
    String hostString = String(WIFI_getChipId(), HEX);
    hostString.toUpperCase();
    // char hostString[16] = {0};
    // sprintf(hostString, "%06X", ESP.getChipId());
    return _wifissidprefix + "_" + hostString;
}

/**
 * setCountry
 * @since $dev
 * @param String cc country code, must be defined in WiFiSetCountry, US, JP, CN
 */
void AsyncWiFiManager::setCountry(String cc)
{
    _wificountry = cc;
}

/**
 * setClass
 * @param String str body class string
 */
void AsyncWiFiManager::setClass(String str)
{
    _bodyClass = str;
}

void AsyncWiFiManager::setHttpPort(uint16_t port)
{
    _httpPort = port;
}

// HELPERS

/**
 * getWiFiSSID
 * @since $dev
 * @param bool persistent
 * @return String
 */
String AsyncWiFiManager::getWiFiSSID(bool persistent)
{
    return WiFi_SSID(persistent);
}

/**
 * getWiFiPass
 * @since $dev
 * @param bool persistent
 * @return String
 */
String AsyncWiFiManager::getWiFiPass(bool persistent)
{
    return WiFi_psk(persistent);
}

// DEBUG
// @todo fix DEBUG_WM(0,0);
template <typename Generic>
void AsyncWiFiManager::DEBUG_WM(Generic text, bool cr)
{
    DEBUG_WM(DEBUG_NOTIFY, text, "", cr);
}

template <typename Generic>
void AsyncWiFiManager::DEBUG_WM(wm_debuglevel_t level, Generic text, bool cr)
{
    if (_debugLevel >= level)
        DEBUG_WM(level, text, "", cr);
    // }

    // template <typename Generic, typename Genericb>
    // void AsyncWiFiManager::DEBUG_WM(Generic text, Genericb textb)
    // {
    //     DEBUG_WM(DEBUG_NOTIFY, text, textb, true);
}

template <typename Generic, typename Genericb>
void AsyncWiFiManager::DEBUG_WM(Generic text, Genericb textb, bool cr)
{
    DEBUG_WM(DEBUG_NOTIFY, text, textb, cr);
}

template <typename Generic, typename Genericb>
void AsyncWiFiManager::DEBUG_WM(wm_debuglevel_t level, Generic text, Genericb textb, bool cr)
{
    if (!_debug || _debugLevel < level)
        return;

    if (_debugLevel >= DEBUG_MAX)
    {
        uint32_t free;
        uint16_t max;
        uint8_t frag;
#ifdef ESP8266
        // TODO: Does not exist in 2.3.0
        ESP.getHeapStats(&free, &max, &frag);
        _debugPort.printf("[MEM] free: %5d | max: %5d | frag: %3d%% \n", free, max, frag);
#elif defined ESP32
        // total_free_bytes;      ///<  Total free bytes in the heap. Equivalent to multi_free_heap_size().
        // total_allocated_bytes; ///<  Total bytes allocated to data in the heap.
        // largest_free_block;    ///<  Size of largest free block in the heap. This is the largest malloc-able size.
        // minimum_free_bytes;    ///<  Lifetime minimum free heap size. Equivalent to multi_minimum_free_heap_size().
        // allocated_blocks;      ///<  Number of (variable size) blocks allocated in the heap.
        // free_blocks;           ///<  Number of (variable size) free blocks in the heap.
        // total_blocks;          ///<  Total number of (variable size) blocks in the heap.
        multi_heap_info_t info;
        heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
        free = info.total_free_bytes;
        max = info.largest_free_block;
        frag = 100 - (max * 100) / free;
        _debugPort.printf("[MEM] free: %5d | max: %5d | frag: %3d%% %s", free, max, frag, cr ? "\n" : "");
#endif
    }
    _debugPort.print(F("*WM: "));
    if (_debugLevel == DEBUG_DEV)
        _debugPort.print("[" + (String)level + "] ");
    _debugPort.print(text);
    if (textb)
    {
        _debugPort.print(F(" "));
        _debugPort.print(textb);
    }
    _debugPort.print(cr ? "\n" : ""); // Allow no \n after line
}

/**
 * [debugSoftAPConfig description]
 * @access public
 * @return {[type]} [description]
 */
void AsyncWiFiManager::debugSoftAPConfig()
{
    // wifi_country_t country;

#ifdef ESP8266
    softap_config config;
    wifi_softap_get_config(&config);
#if !defined(WM_NOCOUNTRY)
    wifi_country_t country;
    wifi_get_country(&country);
#endif
#elif defined(ESP32)
    wifi_country_t country;
    wifi_config_t conf_config;
    esp_wifi_get_config(WIFI_IF_AP, &conf_config); // == ESP_OK
    wifi_ap_config_t config = conf_config.ap;
    esp_wifi_get_country(&country);
#endif

    DEBUG_WM(F("SoftAP Configuration:"));
    DEBUG_WM(FPSTR(D_HR));
    DEBUG_WM(F("ssid:            "), (char *)config.ssid);
    DEBUG_WM(F("password:        "), (char *)config.password);
    DEBUG_WM(F("ssid_len:        "), config.ssid_len);
    DEBUG_WM(F("channel:         "), config.channel);
    DEBUG_WM(F("authmode:        "), config.authmode);
    DEBUG_WM(F("ssid_hidden:     "), config.ssid_hidden);
    DEBUG_WM(F("max_connection:  "), config.max_connection);
#if !defined(WM_NOCOUNTRY)
    DEBUG_WM(F("country:         "), (String)country.cc);
#endif
    // DEBUG_WM(F("country:         "), (String)country.cc);
    DEBUG_WM(F("beacon_interval: "), (String)config.beacon_interval + "(ms)");
    DEBUG_WM(FPSTR(D_HR));
}

/**
 * [debugPlatformInfo description]
 * @access public
 * @return {[type]} [description]
 */
void AsyncWiFiManager::debugPlatformInfo()
{
#ifdef ESP8266
    system_print_meminfo();
    DEBUG_WM(F("getCoreVersion():         "), ESP.getCoreVersion());
    DEBUG_WM(F("system_get_sdk_version(): "), system_get_sdk_version());
    DEBUG_WM(F("system_get_boot_version():"), system_get_boot_version());
    DEBUG_WM(F("getFreeHeap():            "), (String)ESP.getFreeHeap());
#elif defined(ESP32)
    size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    DEBUG_WM(F("Free heap:       "), freeHeap);
    DEBUG_WM(F("ESP SDK version: "), esp_get_idf_version());
#endif
}

int AsyncWiFiManager::getRSSIasQuality(int RSSI)
{
    int quality = 0;

    if (RSSI <= -100)
    {
        quality = 0;
    }
    else if (RSSI >= -50)
    {
        quality = 100;
    }
    else
    {
        quality = 2 * (RSSI + 100);
    }
    return quality;
}

/** Is this an IP? */
boolean AsyncWiFiManager::isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

/** IP to String? */
String AsyncWiFiManager::toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

boolean AsyncWiFiManager::validApPassword()
{
    // check that ap password is valid, return false
    if (_apPassword == NULL)
        _apPassword = "";
    if (_apPassword != "")
    {
        if (_apPassword.length() < 8 || _apPassword.length() > 63)
        {
            DEBUG_WM(F("AccessPoint set password is INVALID or <8 chars."));
            _apPassword = "";
            return false; // @todo FATAL or fallback to empty ?
        }
        DEBUG_WM(DEBUG_VERBOSE, F("AccessPoint set password is valid."));
        DEBUG_WM(_apPassword);
    }
    return true;
}

/**
 * encode htmlentities
 * @since $dev
 * @param  string str  string to replace entities
 * @return string      encoded string
 */
String AsyncWiFiManager::htmlEntities(String str)
{
    str.replace("&", "&amp;");
    str.replace("<", "&lt;");
    str.replace(">", "&gt;");
    // str.replace("'","&#39;");
    // str.replace("\"","&quot;");
    // str.replace("/": "&#x2F;");
    // str.replace("`": "&#x60;");
    // str.replace("=": "&#x3D;");
    return str;
}

/**
 * [getWLStatusString description]
 * @access public
 * @param  {[type]} uint8_t status        [description]
 * @return {[type]}         [description]
 */
String AsyncWiFiManager::getWLStatusString(uint8_t status)
{
    if (status <= 7)
        return WIFI_STA_STATUS[status];
    return FPSTR(S_NA);
}

String AsyncWiFiManager::encryptionTypeStr(uint8_t authmode)
{
    // DEBUG_WM("enc_tye: ",authmode);
    return AUTH_MODE_NAMES[authmode];
}

String AsyncWiFiManager::getModeString(uint8_t mode)
{
    if (mode <= 3)
        return WIFI_MODES[mode];
    return FPSTR(S_NA);
}

bool AsyncWiFiManager::WiFiSetCountry()
{
    if (_wificountry == "")
        return false; // skip not set
    bool ret = false;
#ifdef ESP32
    // @todo check if wifi is init, no idea how, doesnt seem to be exposed atm ( might be now! )
    if (WiFi.getMode() == WIFI_MODE_NULL)
        ; // exception if wifi not init!
    else if (_wificountry == "US")
        ret = esp_wifi_set_country(&WM_COUNTRY_US) == ESP_OK;
    else if (_wificountry == "JP")
        ret = esp_wifi_set_country(&WM_COUNTRY_JP) == ESP_OK;
    else if (_wificountry == "CN")
        ret = esp_wifi_set_country(&WM_COUNTRY_CN) == ESP_OK;
    else
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] Country code not found."));

#elif defined(ESP8266) && !defined(WM_NOCOUNTRY)
    // if(WiFi.getMode() == WIFI_OFF); // exception if wifi not init!
    if (_wificountry == "US")
        ret = wifi_set_country((wifi_country_t *)&WM_COUNTRY_US);
    else if (_wificountry == "JP")
        ret = wifi_set_country((wifi_country_t *)&WM_COUNTRY_JP);
    else if (_wificountry == "CN")
        ret = wifi_set_country((wifi_country_t *)&WM_COUNTRY_CN);
    else
        DEBUG_WM(DEBUG_ERROR, F("([ERROR] Country code not found."));
#endif

    if (ret)
        DEBUG_WM(DEBUG_VERBOSE, "esp_wifi_set_country: " + _wificountry);
    else
        DEBUG_WM(DEBUG_ERROR, F("[ERROR] esp_wifi_set_country failed."));
    return ret;
}

// set mode ignores WiFi.persistent
bool AsyncWiFiManager::WiFi_Mode(WiFiMode_t m, bool persistent)
{
    bool ret;
#ifdef ESP8266
    if ((wifi_get_opmode() == (uint8)m) && !persistent)
    {
        return true;
    }
    ETS_UART_INTR_DISABLE();
    if (persistent)
        ret = wifi_set_opmode(m);
    else
        ret = wifi_set_opmode_current(m);
    ETS_UART_INTR_ENABLE();
    return ret;
#elif defined(ESP32)
    if (persistent && esp32persistent)
        WiFi.persistent(true);
    ret = WiFi.mode(m); // @todo persistent check persistant mode , NI
    if (persistent && esp32persistent)
        WiFi.persistent(false);
    return ret;
#endif
}
bool AsyncWiFiManager::WiFi_Mode(WiFiMode_t m)
{
    return WiFi_Mode(m, false);
}

// sta disconnect without persistent
bool AsyncWiFiManager::WiFi_Disconnect()
{
#ifdef ESP8266
    if ((WiFi.getMode() & WIFI_STA) != 0)
    {
        bool ret;
        DEBUG_WM(DEBUG_DEV, F("WIFI station disconnect."));
        ETS_UART_INTR_DISABLE(); // @todo probably not needed
        ret = wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
        return ret;
    }
#elif defined(ESP32)
    DEBUG_WM(DEBUG_DEV, F("WIFI station disconnect."));
    return WiFi.disconnect(); // not persistent atm
#endif
    return false;
}

// toggle STA without persistent
bool AsyncWiFiManager::WiFi_enableSTA(bool enable, bool persistent)
{
    DEBUG_WM(DEBUG_DEV, F("WiFi_enableSTA: "), (String)enable ? "enabled." : "disabled.");
#ifdef ESP8266
    WiFiMode_t newMode;
    WiFiMode_t currentMode = WiFi.getMode();
    bool isEnabled = (currentMode & WIFI_STA) != 0;
    if (enable)
        newMode = (WiFiMode_t)(currentMode | WIFI_STA);
    else
        newMode = (WiFiMode_t)(currentMode & (~WIFI_STA));

    if ((isEnabled != enable) || persistent)
    {
        if (enable)
        {
            if (persistent)
                DEBUG_WM(DEBUG_DEV, F("enableSTA persistent on."));
            return WiFi_Mode(newMode, persistent);
        }
        else
        {
            return WiFi_Mode(newMode, persistent);
        }
    }
    else
    {
        return true;
    }
#elif defined(ESP32)
    bool ret;
    if (persistent && esp32persistent)
        WiFi.persistent(true);
    ret = WiFi.enableSTA(enable); // @todo handle persistent when it is implemented in platform
    if (persistent && esp32persistent)
        WiFi.persistent(false);
    return ret;
#endif
}
bool AsyncWiFiManager::WiFi_enableSTA(bool enable)
{
    return WiFi_enableSTA(enable, false);
}

bool AsyncWiFiManager::WiFi_eraseConfig()
{
#ifdef ESP8266
#ifndef WM_FIXERASECONFIG
    return ESP.eraseConfig();
#else
    // erase config BUG replacement
    // https://github.com/esp8266/Arduino/pull/3635
    const size_t cfgSize = 0x4000;
    size_t cfgAddr = ESP.getFlashChipSize() - cfgSize;

    for (size_t offset = 0; offset < cfgSize; offset += SPI_FLASH_SEC_SIZE)
    {
        if (!ESP.flashEraseSector((cfgAddr + offset) / SPI_FLASH_SEC_SIZE))
        {
            return false;
        }
    }
    return true;
#endif
#elif defined(ESP32)
    bool ret;
    WiFi.mode(WIFI_AP_STA); // cannot erase if not in STA mode !
    WiFi.persistent(true);
    ret = WiFi.disconnect(true, true);
    WiFi.persistent(false);
    return ret;
#endif
}

uint8_t AsyncWiFiManager::WiFi_softap_num_stations()
{
#ifdef ESP8266
    return wifi_softap_get_station_num();
#elif defined(ESP32)
    return WiFi.softAPgetStationNum();
#endif
}

bool AsyncWiFiManager::WiFi_hasAutoConnect()
{
    return WiFi_SSID(true) != "";
}

String AsyncWiFiManager::WiFi_SSID(bool persistent) const
{

#ifdef ESP8266
    struct station_config conf;
    if (persistent)
        wifi_station_get_config_default(&conf);
    else
        wifi_station_get_config(&conf);

    char tmp[33]; //ssid can be up to 32chars, => plus null term
    memcpy(tmp, conf.ssid, sizeof(conf.ssid));
    tmp[32] = 0; //nullterm in case of 32 char ssid
    return String(reinterpret_cast<char *>(tmp));

#elif defined(ESP32)
    if (persistent)
    {
        wifi_config_t conf;
        esp_wifi_get_config(WIFI_IF_STA, &conf);
        return String(reinterpret_cast<const char *>(conf.sta.ssid));
    }
    else
    {
        if (WiFiGenericClass::getMode() == WIFI_MODE_NULL)
        {
            return String();
        }
        wifi_ap_record_t info;
        if (!esp_wifi_sta_get_ap_info(&info))
        {
            return String(reinterpret_cast<char *>(info.ssid));
        }
        return String();
    }
#endif
}

String AsyncWiFiManager::WiFi_psk(bool persistent) const
{
#ifdef ESP8266
    struct station_config conf;

    if (persistent)
        wifi_station_get_config_default(&conf);
    else
        wifi_station_get_config(&conf);

    char tmp[65]; //psk is 64 bytes hex => plus null term
    memcpy(tmp, conf.password, sizeof(conf.password));
    tmp[64] = 0; //null term in case of 64 byte psk
    return String(reinterpret_cast<char *>(tmp));

#elif defined(ESP32)
    // only if wifi is init
    if (WiFiGenericClass::getMode() == WIFI_MODE_NULL)
    {
        return String();
    }
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    return String(reinterpret_cast<char *>(conf.sta.password));
#endif
}

#ifdef ESP32
void AsyncWiFiManager::WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
    if (!_hasBegun)
    {
        // DEBUG_WM(DEBUG_VERBOSE,"[ERROR] WiFiEvent, not ready");
        return;
    }
    // DEBUG_WM(DEBUG_VERBOSE,"[EVENT]",event);
    if (event == SYSTEM_EVENT_STA_DISCONNECTED)
    {
        if (_debug)
            Serial.println();
        DEBUG_WM(DEBUG_VERBOSE, F("[EVENT] WIFI_REASON:"), info.disconnected.reason);
        if (info.disconnected.reason == WIFI_REASON_AUTH_EXPIRE || info.disconnected.reason == WIFI_REASON_AUTH_FAIL)
        {
            _lastconxresulttmp = 7; // hack in wrong password internally, sdk emit WIFI_REASON_AUTH_EXPIRE on some routers on auth_fail
        }
        else if (info.disconnected.reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT)
        {
            // Hack to reset due to ESP not connecting after flashing
            DEBUG_WM(DEBUG_VERBOSE, F("[EVENT] WIFI_REASON: WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT detected, resetting."));
            ESP.restart();
        }
        else
            _lastconxresulttmp = WiFi.status();
        if (info.disconnected.reason == WIFI_REASON_NO_AP_FOUND)
        {
            DEBUG_WM(DEBUG_VERBOSE, F("[EVENT] WIFI_REASON: NO_AP_FOUND"));
        }
#ifdef esp32autoreconnect
        DEBUG_WM(DEBUG_VERBOSE, F("[Event] SYSTEM_EVENT_STA_DISCONNECTED, reconnecting."));
        WiFi.reconnect();
#endif
    }
    else if (event == SYSTEM_EVENT_SCAN_DONE)
    {
        uint16_t scans = WiFi.scanComplete();
        WiFi_scanComplete(scans);
    }
}
#endif

void AsyncWiFiManager::WiFi_autoReconnect()
{
#ifdef ESP8266
    WiFi.setAutoReconnect(_wifiAutoReconnect);
#elif defined(ESP32)
    // if(_wifiAutoReconnect){
    // @todo move to seperate method, used for event listener now
    DEBUG_WM(DEBUG_VERBOSE, F("ESP32 event handler enabled."));
    using namespace std::placeholders;
    wm_event_id = WiFi.onEvent(std::bind(&AsyncWiFiManager::WiFiEvent, this, _1, _2));
    // }
#endif
}

#endif
