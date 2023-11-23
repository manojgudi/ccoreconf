#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <coreconf.h>


/*
    This is an autogenerated function associated to 
    SID: 1007
    Module: data 
    Identifier: /sensor:sensorHealth
    function params: 
    Stable: false
*/
void get_sensorHealth(){
	}


/*
    This is an autogenerated function associated to 
    SID: 1008
    Module: data 
    Identifier: /sensor:sensorHealth/healthReadings
    function params: /sensor:sensorHealth/healthReadings/readingIndex
    Stable: false
*/
json_t * get_healthReadings(json_t *instance, SIDModelT *sidModel, uint8_t readingIndex){
    int64_t keys[] = {1011};
    size_t keyLength = 1;
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = INT64_MIN;
    sidIdentifier->identifier = "/sensor:sensorHealth/healthReadings";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(coreconfModel, sidModel, sidIdentifier, keys, keyLength);
    free(sidIdentifier);
    return traversedJSON_;
}
        


/*
    This is an autogenerated function associated to 
    SID: 1010
    Module: data 
    Identifier: /sensor:sensorHealth/healthReadings/healthValue
    function params: /sensor:sensorHealth/healthReadings/readingIndex
    Stable: false
*/
json_t * get_healthValue(json_t *instance, SIDModelT *sidModel, uint8_t readingIndex){
    int64_t keys[] = {1011};
    size_t keyLength = 1;
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = INT64_MIN;
    sidIdentifier->identifier = "/sensor:sensorHealth/healthReadings/healthValue";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(coreconfModel, sidModel, sidIdentifier, keys, keyLength);
    free(sidIdentifier);
    return traversedJSON_;
}
        


/*
    This is an autogenerated function associated to 
    SID: 1012
    Module: data 
    Identifier: /sensor:sensordata
    function params: 
    Stable: false
*/
void get_sensordata(){
	}


/*
    This is an autogenerated function associated to 
    SID: 1013
    Module: data 
    Identifier: /sensor:sensordata/dataReadings
    function params: /sensor:sensordata/dataReadings/readingIndex
    Stable: false
*/
json_t * get_dataReadings(json_t *instance, SIDModelT *sidModel, uint8_t readingIndex){
    int64_t keys[] = {1015};
    size_t keyLength = 1;
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = INT64_MIN;
    sidIdentifier->identifier = "/sensor:sensordata/dataReadings";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(coreconfModel, sidModel, sidIdentifier, keys, keyLength);
    free(sidIdentifier);
    return traversedJSON_;
}
        


/*
    This is an autogenerated function associated to 
    SID: 1014
    Module: data 
    Identifier: /sensor:sensordata/dataReadings/dataValue
    function params: /sensor:sensordata/dataReadings/readingIndex
    Stable: false
*/
json_t * get_dataValue(json_t *instance, SIDModelT *sidModel, uint8_t readingIndex){
    int64_t keys[] = {1015};
    size_t keyLength = 1;
    IdentifierSIDT *sidIdentifier = malloc(sizeof(IdentifierSIDT));
    sidIdentifier->sid = INT64_MIN;
    sidIdentifier->identifier = "/sensor:sensordata/dataReadings/dataValue";
    json_t *traversedJSON_ = traverseCORECONFWithKeys(coreconfModel, sidModel, sidIdentifier, keys, keyLength);
    free(sidIdentifier);
    return traversedJSON_;
}
        


