#ifndef __SERIAL_MIN_H__
#define __SERIAL_MIN_H__

#include <HardwareSerial.h>

#define SERIAL_INIT(...)		{ Serial.begin(__VA_ARGS__); }

#define SERIAL_ECHO(x)      Serial.print(x)
#define SERIAL_ECHOLN(x)    Serial.println(x)
#define SERIAL_PRINTF(args...) Serial.printf(args)

#endif // __SERIAL_MIN_H__
