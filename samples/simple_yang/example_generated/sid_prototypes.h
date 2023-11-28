#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define  SID_BATTERY             1009
#define  SID_HEALTHVALUE         1010
#define  SID_READINGINDEX        1011
#define  SID_DATAVALUE           1014
#define  SID_READINGINDEX1       1015


#define read_sensorHealth        read_1007
#define read_healthReadings      read_1008
#define read_healthValue         read_1010
#define read_sensordata          read_1012
#define read_dataReadings        read_1013
#define read_dataValue           read_1014


void read_sensorHealth(void);
void read_healthReadings(uint8_t readingIndex);
uint32_t read_healthValue(uint8_t readingIndex);
void read_sensordata(void);
void read_dataReadings(uint8_t readingIndex);
uint16_t read_dataValue(uint8_t readingIndex);
