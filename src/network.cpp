#include "network.h"
#include "serial_min.h"
#include "config.h"
#include "consts.h"
#include "pins.h"
#include "ESP8266WiFi.h"
#include "ESPWebDAV.h"
#include "sdControl.h"
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// Wifi Manager
WiFiManager WM;
String ntpSourcePara_str;
WiFiManagerParameter hostnamePara;
WiFiManagerParameter ntpSourcePara;
WiFiManagerParameter ntpServerPara;

String getWMParam(String name);
void saveWMConfigCallback();
void errorBlink();

// NTP client
WiFiUDP ntpUDP;
NTPClient* timeClient;

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void dateTime(uint16_t* date, uint16_t* time) {
	unsigned long Dtime = timeClient->getEpochTime();
	uint16_t Tyear = year(Dtime);
	uint8_t Tmonth = month(Dtime);
	uint8_t Tday = day(Dtime);
	uint8_t Thour = hour(Dtime);
	uint8_t Tminute = minute(Dtime);
	uint8_t Tsecond = second(Dtime);

	*date = FAT_DATE(Tyear, Tmonth, Tday);
	*time = FAT_TIME(Thour, Tminute, Tsecond);
}

void Network::start() {
  wifiConnected = false;
  wifiConnecting = true;
  
  SERIAL_ECHOLN("");
  SERIAL_ECHOLN("Going to set WiFi connection");
  
  // Set hostname first
  // WiFi.hostname(config.hostname());
  
	// Reduce startup surge current
  WiFi.setAutoConnect(false);
  WiFi.mode(WIFI_STA);
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  
	startWM();

  wifiConnected = true;

  if (!WM.getWiFiHostname().equals(config.hostname())) {
		// catched first normal connection
    SERIAL_ECHOLN("Reboot...");
	  ESP.restart();
  }
  SERIAL_ECHO("Connected to "); SERIAL_ECHOLN(WM.getWiFiSSID());
  SERIAL_ECHO("RSSI: "); SERIAL_ECHOLN(WiFi.RSSI());
  SERIAL_ECHO("Mode: "); SERIAL_ECHOLN(WiFi.getPhyMode());

  SERIAL_ECHOLN("");
  SERIAL_ECHOLN("Going to start Web portal");
	WM.setConfigPortalBlocking(false);
	WM.setHttpPort(WM_SERVER_PORT);
  WM.startWebPortal();
  SERIAL_ECHO("Asscess to Web portal: http:\\\\"); SERIAL_ECHO(WiFi.localIP()); SERIAL_ECHO(":"); SERIAL_ECHOLN(WM_SERVER_PORT);

	// Use mdns for host name resolution
  SERIAL_ECHOLN("");
  SERIAL_ECHOLN("Going to start mDNS");
	if (!MDNS.begin(config.hostname())) {
		SERIAL_ECHOLN("Error setting up mDNS responder!");
	} else {
		SERIAL_ECHO("mDNS started: "); SERIAL_ECHO(config.hostname()); SERIAL_ECHOLN(".local");
	}

	if (config.ntpSource()) {
		SERIAL_ECHOLN("");
		SERIAL_ECHOLN("Going to start NTP client");
		SERIAL_ECHO("NTP server: ");
		if (config.ntpSource() == 1) {
			SERIAL_ECHOLN(WiFi.gatewayIP().toString().c_str());
			timeClient = new NTPClient(ntpUDP, WiFi.gatewayIP());
		} else {
			SERIAL_ECHOLN(config.ntpServer());
			timeClient = new NTPClient(ntpUDP, config.ntpServer());
		}
		timeClient->begin();
		for (int i = 0; i < NTP_CLIENT_START_TRIALS; i++) {
			if (timeClient->forceUpdate()) {
				SERIAL_ECHO("UTC time: "); SERIAL_ECHOLN(timeClient->getFormattedTime().c_str());
				break;
			}
			if (i < NTP_CLIENT_START_TRIALS) {
				delay(250);
			} else {
				SERIAL_ECHOLN("No answer from NTP server");
			}
		}
		SdFile::dateTimeCallback(dateTime);
	}
  SERIAL_ECHOLN("");
  SERIAL_ECHOLN("Going to start DAV server");
}

void Network::startWM() {
  new (&hostnamePara) WiFiManagerParameter("hostname", "Hostname", config.hostname(), 40, "placeholder=\"Hostname\"");
  ntpSourcePara_str =
      "<hr><div style='margin:0 0 10px 0;padding:0;'><b>Time synchronization</b></div>"
			"<label for='ntpsource'>NTP Source</label><div><input type='radio' name='ntpsource' value='0'{0}> "
      "None<br><input type='radio' name='ntpsource' value='1'{1}> Station gateway<br><input type='radio' "
      "name='ntpsource' value='2'{2}> NTP Server specified below</div>";
  for (uint8_t i = 0; i < 3; i++) {
    String token = "{" + String(i) + "}";
    ntpSourcePara_str.replace(token, (config.ntpSource() == i ? " checked" : ""));
  }
  new (&ntpSourcePara) WiFiManagerParameter(ntpSourcePara_str.c_str());
  new (&ntpServerPara)
      WiFiManagerParameter("ntpserver", "NTP Server name", config.ntpServer(), 40, "placeholder=\"NTP Server name\"");
  WM.addParameter(&hostnamePara);
  WM.addParameter(&ntpSourcePara);
  WM.addParameter(&ntpServerPara);

  WM.setBreakAfterConfig(true);
  WM.setSaveConfigCallback(saveWMConfigCallback);

  WM.setTitle(APP_TITLE);
  WM.setShowInfoUpdate(false);
  WM.setShowInfoErase(false);

  // custom menu
  std::vector<const char*> menu = {"wifi", "wifinoscan", "info", "sep", "update", "sep", "restart"};
  WM.setMenu(menu);

  // set dark theme
  WM.setClass("invert");

  WM.setHostname(config.hostname());

	// Connecting
  WM.autoConnect(FALLBACK_SSID);
}

bool Network::startDAVServer() {
  while (!sdcontrol.canWeTakeBus()) {
	  SERIAL_ECHOLN("SPI bus busy...");
    return false;
  }
  sdcontrol.takeBusControl();
  
  // start the SD DAV server
  if(!dav.init(SD_CS, SPI_FULL_SPEED, DAV_SERVER_PORT))   {
    SERIAL_ECHO("ERROR: "); SERIAL_ECHOLN("Failed to initialize SD Card");
    // indicate error on LED
    errorBlink();
    initFailed = true;
  }
  
  sdcontrol.relinquishBusControl();
  SERIAL_ECHOLN("WebDAV server started");
  SERIAL_ECHO("Asscess to SD: http:\\\\"); SERIAL_ECHO(WiFi.localIP()); SERIAL_ECHO(":"); SERIAL_ECHOLN(DAV_SERVER_PORT);
  wifiConnecting = false;
  return true;
}

bool Network::isConnected() {
  return wifiConnected;
}

bool Network::isConnecting() {
  return wifiConnecting;
}

// a client is waiting and FS is ready and other SPI master is not using the bus
bool Network::ready() {
  if(!isConnected()) return false;
  
  // do it only if there is a need to read FS
	if(!dav.isClientWaiting())	return false;
	
	if(initFailed) {
	  dav.rejectClient("Failed to initialize SD Card");
	  return false;
	}
	
	// has other master been using the bus in last few seconds
	if(!sdcontrol.canWeTakeBus()) {
		dav.rejectClient("Printer is reading from SD card");
		return false;
	}

	return true;
}

void Network::resetWMSettings()
{
	WM.resetSettings();
}

void Network::handle() {
	static unsigned long lastDAVInitTry = 0;
	if (!network.isConnected())
		return;

	MDNS.update();
	if (config.ntpSource())
		timeClient->update();
	WM.process();

	if (network.isConnecting()) {
		if ((millis() - lastDAVInitTry >= 1000) || lastDAVInitTry == 0) {
			startDAVServer();
			lastDAVInitTry = millis();
		}
		return;
	}

  if (network.ready()) {
	  sdcontrol.takeBusControl();
	  dav.handleClient();
	  sdcontrol.relinquishBusControl();
	}
}

String getWMParam(String name) {
  // read parameter from server, for customhmtl input
  String value;
  if (WM.server->hasArg(name)) {
    value = WM.server->arg(name);
  }
  return value;
}

// callback notifying us of the need to save config
void saveWMConfigCallback() {
  SERIAL_ECHOLN("Going to save additional settings (except SSID and PASSWORD)");
	config.save(hostnamePara.getValue(), getWMParam("ntpsource")[0] - 48, ntpServerPara.getValue());
  if (network.isConnecting())
    return;
  SERIAL_ECHOLN("Reboot...");
  ESP.restart();
}

void errorBlink()	{
	for(int i = 0; i < 100; i++)	{
		LED_ON; 
		delay(50); 
		LED_OFF; 
		delay(50);
	}
}

Network network;
