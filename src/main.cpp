// Using the WebDAV server with Rigidbot 3D printer.
// Printer controller is a variation of Rambo running Marlin firmware

#include <Arduino.h>
#include "serial_min.h"
#include "config.h"
#include "consts.h"
#include "network.h"
#include "sdControl.h"
#include "pins.h"

// LED is connected to GPIO2 on this board
#define INIT_LED			{pinMode(2, OUTPUT);}
#define LED_ON				{digitalWrite(2, LOW);}
#define LED_OFF				{digitalWrite(2, HIGH);}
#define LED_SET(state)	{digitalWrite(2, state ? LOW : HIGH);}

// ------------------------
void blink();
void statusBlink();
void handleBootButton(void);

// ------------------------
void setup() {
	pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
	SERIAL_INIT(115200);
	INIT_LED;
	blink();
  SERIAL_ECHOLN("\n\n**********************************************");
  SERIAL_ECHO(APP_TITLE); SERIAL_ECHO(" Version "); SERIAL_ECHOLN(APP_VERSION);
  SERIAL_ECHOLN("**********************************************\n");
  SERIAL_ECHO("Loading additional settings from EEPROM : ");
  SERIAL_ECHOLN(config.load() ? "Success" : "Fail! Set default values");

  SERIAL_ECHOLN("Turning on SPI bus control. Module will not occupy the bus for at least 20 seconds");
	sdcontrol.setup();

  network.start();
}

// ------------------------
void loop() {
  // handle the request
	network.handle();

  // blink
  statusBlink();

  // handle boot button
	handleBootButton();
}

// ------------------------
void blink()	{
// ------------------------
	LED_ON; 
	delay(100); 
	LED_OFF; 
	delay(400);
}

// ------------------------
void statusBlink() {
// ------------------------
  static unsigned long time = 0;
  if(millis() > time + 1000 ) {
    if(network.isConnecting()) {
      LED_ON; 
  		delay(50); 
  		LED_OFF; 
    }
    else if(network.isConnected()) {
			LED_SET(sdcontrol.isBusBusy());
    }
    else {
      LED_ON;
    }
    time = millis();
  }

  // SPI bus not ready
	//if(millis() < spiBlockoutTime)
	//	blink();
}

// The flash button is used to reset the WiFi settings
// ------------------------
void handleBootButton(void) {
// ------------------------
  static unsigned long last_millis = millis();
  static byte last_state = 0;
  if (digitalRead(BOOT_BUTTON_PIN) == 0) {
    if (!last_state) {
      last_state = 1;
      last_millis = millis();
      return;
    }
    if ((millis() - last_millis > 5000) && (last_state == 1)) {
      last_state = 2;
      SERIAL_ECHOLN("\nBOOT button pressed for 5 seconds");

      // Let the LED blinking 3 times
      for (int i = 0; i < 3; i++) {
        blink();
      }

      SERIAL_ECHOLN("Reset additional settings");
			config.setToDefaults();
			config.save();

      SERIAL_ECHOLN("Reset WiFi settings");
      network.resetWMSettings();

      SERIAL_ECHOLN("Reboot...");
      ESP.restart();
    }
  } else {
    if (last_state) {
      last_state = 0;
    }
  }
}
