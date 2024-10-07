#include <ESP8266WiFi.h>
#include "sdControl.h"
#include "pins.h"

volatile unsigned long SDControl::_spiBlockoutTime = 0;
bool SDControl::_weTookBus = false;

void IRAM_ATTR SDControl::CSSenseInterrupt() {
		if(!_weTookBus)
			_spiBlockoutTime = millis() + SPI_BLOCKOUT_PERIOD;
}

void SDControl::setup() {
  // Free SPI bus
	relinquishBusControl();
	pinMode(CS_SENSE, INPUT);

	_spiBlockoutTime = millis() + SPI_BLOCKOUT_PERIOD;

	// Detect when other master uses SPI bus
	attachInterrupt(CS_SENSE, CSSenseInterrupt, FALLING);

	// wait for other master to assert SPI bus first
	LED_ON;
	// while (!canWeTakeBus())
	// 	delay(1000);
	// LED_OFF;
}

// ------------------------
void SDControl::takeBusControl()	{
// ------------------------
	_weTookBus = true;
	LED_ON;
	pinMode(MISO_PIN, SPECIAL);	
	pinMode(MOSI_PIN, SPECIAL);	
	pinMode(SCLK_PIN, SPECIAL);	
	pinMode(SD_CS, OUTPUT);
}

// ------------------------
void SDControl::relinquishBusControl()	{
// ------------------------
	pinMode(MISO_PIN, INPUT);	
	pinMode(MOSI_PIN, INPUT);	
	pinMode(SCLK_PIN, INPUT);	
	pinMode(SD_CS, INPUT);
	LED_OFF;
	_weTookBus = false;
}

bool SDControl::canWeTakeBus() {
	if(millis() < _spiBlockoutTime) {
    return false;
  }
  return true;
}

bool SDControl::isBusBusy() {
	if (_weTookBus) {
		return true;
	}
	if(millis() < _spiBlockoutTime) {
    return true;
  }
  return false;
}
