#ifndef CONSTS_H
#define CONSTS_H

#define APP_TITLE "WiFi SD Card"
#ifndef APP_VERSION
  #define APP_VERSION "1.2.0"
#endif
#define APP_GITHUB "https://github.com/Steigan/ESPWebDAV/"

#define DEFAULT_HOSTNAME "WIFI_SDCARD"
#define DEFAULT_NTPSOURCE 0
#define DEFAULT_NTPSERVER "pool.ntp.org"

#define FALLBACK_SSID "WIFI_SDCARD_AP"

#define WM_SERVER_PORT		80
#define DAV_SERVER_PORT		8080

#define STR_(X) #X
#define STR(X) STR_(X)

#endif   // CONSTS_H