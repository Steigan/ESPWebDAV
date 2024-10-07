#ifndef _CONFIG_H_
#define _CONFIG_H_

#define WIFI_HOSTNAME_LEN 32
#define WIFI_NTPSERVER_LEN 64
#define WIFI_NTPSOURCE_MAX 2

#define WIFI_HOSTNAME_MIN_LEN 4
#define WIFI_NTPSERVER_MIN_LEN 4
#define WIFI_NTPSERVER_MIN_LEN 4

#define EEPROM_SIZE 512

typedef struct config_type
{
  char hostname[WIFI_HOSTNAME_LEN];
  uint8_t ntpSource;
  char ntpServer[WIFI_NTPSERVER_LEN];
}CONFIG_TYPE;

class Config	{
public:
	void setToDefaults();
	bool load();
  void save();
	void save(const char* hostname, uint8_t ntpSource, const char* ntpServer);
  char* hostname();
  void hostname(const char* hostname);
  uint8_t ntpSource();
  void ntpSource(uint8_t ntpSource);
  char* ntpServer();
  void ntpServer(const char* ntpServer);

protected:
  CONFIG_TYPE data;
};

extern Config config;

#endif
