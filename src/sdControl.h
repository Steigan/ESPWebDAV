#ifndef _SD_CONTROL_H_
#define _SD_CONTROL_H_

#define SPI_BLOCKOUT_PERIOD	20000UL

#define LED_ON				{digitalWrite(2, LOW);}
#define LED_OFF				{digitalWrite(2, HIGH);}

class SDControl {
public:
  SDControl() { }
  static void setup();
  static void takeBusControl();
  static void relinquishBusControl();
  static bool canWeTakeBus();
 	static bool isBusBusy();

private:
	static void CSSenseInterrupt();
  static volatile unsigned long _spiBlockoutTime;
  static bool _weTookBus;
};

extern SDControl sdcontrol;

#endif
