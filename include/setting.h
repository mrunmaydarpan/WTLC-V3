/*------------sensor setting-----------------*/

#define SENSOR_1 true // sonar
// #define SENSOR_2 true // UART
// #define SENSOR_3 true // PWM

/*------------wifi setting-------------------*/

#define WM_SET // (wifimanager) this turns on/off the wifi feature

/*------------API setting--------------------*/

#ifdef WM_SET
#define HA_INIT false // if using with home assistant {turn on WM_SET to enable this feature}
#endif

/*-------------------------------------------*/

#define SW_TEST true
#define OLED true // use OLED display
#define debugData false
#define Buzzer true

#define on_delay 500
#define off_delay 1000