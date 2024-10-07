#include <EEPROM.h>
#include "consts.h"
#include "config.h"
#include "serial_min.h"

void Config::setToDefaults() {
  memset(data.hostname, '\0', sizeof(data));
  strncpy(data.hostname, DEFAULT_HOSTNAME, WIFI_HOSTNAME_LEN - 1);
  data.ntpSource = DEFAULT_NTPSOURCE;
  strncpy(data.ntpServer, DEFAULT_NTPSERVER, WIFI_NTPSERVER_LEN - 1);
}

bool Config::load() {
  uint8_t checkSum = 0;
  EEPROM.begin(EEPROM_SIZE);
  uint8_t* p = (uint8_t*)(&data);
  unsigned int i = 0;
  for (; i < sizeof(data); i++) {
    *(p + i) = EEPROM.read(i);
    checkSum += *(p + i);
  }
  // Read stored checkSum
  uint8_t storedCheckSum = EEPROM.read(i);
  EEPROM.commit();
  if (checkSum == storedCheckSum)
    return true;

  // Set default values
	setToDefaults();
  return false;
}

void Config::save() {
  uint8_t checkSum = 0;
  EEPROM.begin(EEPROM_SIZE);
  uint8_t* p = (uint8_t*)(&data);
  unsigned int i = 0;
  for (; i < sizeof(data); i++) {
    EEPROM.write(i, *(p + i));
    checkSum += *(p + i);
  }
  EEPROM.write(i, checkSum);
  EEPROM.commit();
}

void Config::save(const char* hostname, uint8_t ntpSource, const char* ntpServer) {
	// Set config values
  memset(data.hostname, '\0', sizeof(data));
  bool valueInvalid = strlen(hostname) < WIFI_HOSTNAME_MIN_LEN;
  strncpy(data.hostname, valueInvalid ? DEFAULT_HOSTNAME : hostname, WIFI_HOSTNAME_LEN - 1);
  if (valueInvalid)
    SERIAL_ECHOLN("HOSTNAME not valid -> set to default");

  if (ntpSource > 2) {
    data.ntpSource = DEFAULT_NTPSOURCE;
    SERIAL_ECHOLN("NTP source not valid -> set to default");
  } else
	  data.ntpSource = ntpSource;

  valueInvalid = strlen(ntpServer) < WIFI_NTPSERVER_MIN_LEN;
  strncpy(data.ntpServer, valueInvalid ? DEFAULT_NTPSERVER : ntpServer, WIFI_NTPSERVER_LEN - 1);
  if (valueInvalid)
    SERIAL_ECHOLN("NTPSERVER not valid -> set to default");

	save();
}

char* Config::hostname() {
  return data.hostname;
}

void Config::hostname(const char* hostname) {
  if (hostname == NULL || strlen(hostname) < WIFI_HOSTNAME_MIN_LEN) return;
  strncpy(data.hostname, hostname, WIFI_HOSTNAME_LEN);
}

uint8_t Config::ntpSource() {
  return data.ntpSource;
}

void Config::ntpSource(uint8_t ntpSource) {
	// data.ntpSource = ntpSource > WIFI_NTPSOURCE_MAX ? DEFAULT_NTPSOURCE : ntpSource;
	if (ntpSource > WIFI_NTPSOURCE_MAX) return;
	data.ntpSource = ntpSource;
}

char* Config::ntpServer() {
  return data.ntpServer;
}

void Config::ntpServer(const char* ntpServer) {
  if (ntpServer == NULL || strlen(ntpServer) < WIFI_NTPSERVER_MIN_LEN) return;
  strncpy(data.ntpServer, ntpServer, WIFI_NTPSERVER_LEN);
}

Config config;
