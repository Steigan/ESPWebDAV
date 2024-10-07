#ifndef _NETWORK_H_
#define _NETWORK_H_

#define WIFI_CONNECT_TIMEOUT 30000UL
#define NTP_CLIENT_START_TRIALS 5

class Network {
public:
  Network() { initFailed = false;wifiConnecting = true;}
  void start();
  bool isConnected();
  bool isConnecting();
  void handle();
	void resetWMSettings();

private:
  void startWM();
  bool startDAVServer();
  bool ready();
  bool wifiConnected;
  bool wifiConnecting;
  bool initFailed;
};

extern Network network;

#endif
