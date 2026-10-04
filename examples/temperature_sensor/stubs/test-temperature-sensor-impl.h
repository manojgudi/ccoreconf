#ifndef __TEST_TEMPERATURE_SENSOR_IMPL_H__
#define __TEST_TEMPERATURE_SENSOR_IMPL_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <coreconfTypes.h>
#include <coreconfManipulation.h>
#include <hashmap.h>
#define read_TemperatureSensor_TemperatureSensorObject read_60001
#define write_TemperatureSensor_TemperatureSensorObject write_60001
CoreconfValueT* read_TemperatureSensor_TemperatureSensorObject(void);
int write_TemperatureSensor_TemperatureSensorObject(CoreconfValueT* value);
#define read_TemperatureSensorObject_batteryLevel read_60002
#define write_TemperatureSensorObject_batteryLevel write_60002
uint64_t read_TemperatureSensorObject_batteryLevel(void);
int write_TemperatureSensorObject_batteryLevel(uint64_t value);
#define read_TemperatureSensorObject_statusLED read_60003
#define write_TemperatureSensorObject_statusLED write_60003
enum Temperaturesensorobject_StatusledEnum read_TemperatureSensorObject_statusLED(void);
int write_TemperatureSensorObject_statusLED(enum Temperaturesensorobject_StatusledEnum value);
#define read_TemperatureSensorObject_temperature read_60004
#define write_TemperatureSensorObject_temperature write_60004
CoreconfValueT* read_TemperatureSensorObject_temperature(void);
int write_TemperatureSensorObject_temperature(CoreconfValueT* value);
#define read_temperature_voltageADC read_60005
#define write_temperature_voltageADC write_60005
double read_temperature_voltageADC(void);
int write_temperature_voltageADC(double value);
enum Temperaturesensorobject_StatusledEnum {Temperaturesensorobject_StatusledEnum_green = 0, Temperaturesensorobject_StatusledEnum_yellow = 1, Temperaturesensorobject_StatusledEnum_red = 2};


// User-facing read/write function prototypes
// Implement these functions in your code

#endif
