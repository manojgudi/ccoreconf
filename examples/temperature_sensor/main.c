/* main.c - small ccoreconf walk-through over the generated TemperatureSensor model.
 *
 * The model (instance CBOR, SID table, clookup table) is compiled in from
 * include_model/test-temperature-sensor.c. No .sid, .yang or .json file is
 * read at run time.
 *
 * The instance (instance/TemperatureSensor.json) looks like this:
 *
 *   TemperatureSensorObject        60001  container
 *     batteryLevel                 60002  uint8         87
 *     statusLED                    60003  enumeration   green (0)
 *     temperature                  60004  container
 *       voltageADC                 60005  decimal64     1.8
 */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "coreconfManipulation.h"         /* PathNodeT, DynamicLongListT */
#include "test-temperature-sensor.h"      /* testTemperatureSensorModelDesc + SID_* macros */

/* Last part of a YANG identifier, without the module prefix:
 * "/TemperatureSensor:TemperatureSensorObject/temperature" -> "temperature". */
static const char *shortName(CoreconfModelT *model, uint64_t SID) {
    const char *id = ccoreconfModelLookupIdentifier(model, SID);
    if (id == NULL) {
        return "?";
    }
    const char *slash = strrchr(id, '/');
    const char *name = slash ? slash + 1 : id;
    const char *colon = strchr(name, ':');
    return colon ? colon + 1 : name;
}

/* Print a map as an indented tree. Keys are deltas from parentSID
 * (parentSID is 0 at the top level, where keys are absolute SIDs). */
static void printTree(CoreconfModelT *model, CoreconfHashMapT *map, uint64_t parentSID, int depth) {
    for (size_t i = 0; i < HASHMAP_TABLE_SIZE; i++) {
        for (CoreconfObjectT *entry = map->table[i]; entry != NULL; entry = entry->next) {
            uint64_t SID = parentSID + entry->key;
            printf("  %*s%-*s SID %" PRIu64 " (key %" PRIu64 ")",
                   depth * 2, "", 24 - depth * 2, shortName(model, SID), SID, entry->key);
            if (entry->value->type == CORECONF_HASHMAP) {
                printf("\n");
                printTree(model, entry->value->data.map_value, SID, depth + 1);
            } else {
                printf(" = ");
                printCoreconf(entry->value);
                printf("\n");
            }
        }
    }
}

/* ---- 1. the whole instance, as decoded from CBOR ------------------------ */
static void showInstance(CoreconfModelT *model) {
    printf("=== 1. whole instance ===\n");
    printTree(model, model->root->data.map_value, 0, 0);
    printf("\n");
}

/* ---- 2. look up a value by its absolute SID ---------------------------- *
 * SID -> path (clookup) -> value. Always the absolute SID: ccoreconf turns it
 * into the deltas used inside the CBOR tree. No list in this model, so no
 * keys are needed.                                                          */
static void showValueBySID(CoreconfModelT *model, uint64_t SID) {
    PathNodeT *path = ccoreconfModelFindRequirementForSID(model, SID);
    if (path == NULL) {
        printf("  SID %" PRIu64 ": not in this model\n\n", SID);
        return;
    }
    /* The path lists every node from the top down to the leaf, and ends
     * with a node whose SID is 0. */
    printf("  SID %" PRIu64 ", path:", SID);
    for (PathNodeT *node = path; node != NULL && node->parentSID != 0; node = node->nextPathNode) {
        printf(" %s%s", node == path ? "" : "-> ", shortName(model, (uint64_t)node->parentSID));
    }
    printf("\n");

    DynamicLongListT *keys = createDynamicLongList();
    /* Returns { SID: <node borrowed from the model> }: free the wrapper with
     * freeExaminedCoreconfValue(), never with freeCoreconf(). */
    CoreconfValueT *hit = ccoreconfModelExamineCoreconfValue(model, keys, path);
    CoreconfValueT *leaf = hit ? getCoreconfHashMap(hit->data.map_value, SID) : NULL;

    printf("    value: ");
    if (leaf != NULL) {
        printCoreconf(leaf);
    } else {
        printf("<not found>");
    }
    printf("\n\n");

    freeExaminedCoreconfValue(hit);
    freeDynamicLongList(keys);
    freePathNode(path);
}

/* ---- 3. look up a value by its YANG identifier ------------------------- *
 * identifier -> SID (SID table), then the same lookup as above.             */
static void showValueByIdentifier(CoreconfModelT *model, const char *identifier) {
    uint64_t SID = ccoreconfModelLookupSID(model, identifier);
    printf("  %s\n", identifier);
    if (SID == 0) {
        printf("    not in the SID table\n\n");
        return;
    }
    showValueBySID(model, SID);
}

/* ---- 4. update a value by its absolute SID ----------------------------- *
 * Same SID -> path step as the lookup. navigateToParentContainer() then finds
 * the map that holds the leaf and gives back the key the leaf has inside it,
 * so the caller never works out a delta.                                    */
static int updateValueBySID(CoreconfModelT *model, uint64_t SID, CoreconfValueT *newValue) {
    PathNodeT *path = ccoreconfModelFindRequirementForSID(model, SID);
    if (path == NULL) {
        freeCoreconf(newValue, true);
        return -1;
    }

    uint64_t keyInParent = 0;
    CoreconfValueT *parent = navigateToParentContainer(model->root, NULL, path, SID, &keyInParent);
    freePathNode(path);
    if (parent == NULL) {
        freeCoreconf(newValue, true);
        return -1;
    }

    /* Replaces the old value and frees it: any pointer to the old leaf
     * fetched earlier is no longer valid. On success the model owns newValue. */
    if (insertCoreconfHashMap(parent->data.map_value, keyInParent, newValue) != 0) {
        freeCoreconf(newValue, true);
        return -1;
    }
    return 0;
}

int main(void) {
    CoreconfModelT *model = ccoreconfModelLoadDesc(&testTemperatureSensorModelDesc);
    if (model == NULL) {
        fprintf(stderr, "failed to load the TemperatureSensor model\n");
        return 1;
    }
    if (model->root == NULL || model->root->type != CORECONF_HASHMAP) {
        fprintf(stderr, "instance root is not a map\n");
        ccoreconfModelFree(model);
        return 1;
    }

    showInstance(model);

    printf("=== 2. look up values by absolute SID ===\n");
    showValueBySID(model, 60002);                                    /* batteryLevel */
    showValueBySID(model, SID_TEST_TEMPERATURE_SENSOR_VOLTAGEADC);   /* 60005, macro from the generated header */
    showValueBySID(model, 60099);                                    /* not in the model */

    printf("=== 3. look up values by YANG identifier ===\n");
    showValueByIdentifier(model, "/TemperatureSensor:TemperatureSensorObject/statusLED");
    showValueByIdentifier(model, "/TemperatureSensor:TemperatureSensorObject/temperature/voltageADC");
    showValueByIdentifier(model, "/TemperatureSensor:TemperatureSensorObject/humidity");   /* not in the model */

    printf("=== 4. update voltageADC, then fetch it again ===\n");
    showValueBySID(model, SID_TEST_TEMPERATURE_SENSOR_VOLTAGEADC);
    /* The YANG range is 0.5..3.3; ccoreconf does not check it, the caller must. */
    if (updateValueBySID(model, SID_TEST_TEMPERATURE_SENSOR_VOLTAGEADC, createCoreconfReal(2.5)) != 0) {
        fprintf(stderr, "update of voltageADC failed\n");
    }
    showValueBySID(model, SID_TEST_TEMPERATURE_SENSOR_VOLTAGEADC);

    ccoreconfModelFree(model);
    return 0;
}
