#include "hashmap.h"
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

json_t *readJSON(const char *filePath);