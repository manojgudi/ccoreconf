/* main.c - minimal ccoreconf demo over the generated "test-schc" model.
 *
 * Every SID / key-mapping fact used below comes from include_model/test-schc.h
 * (generated). No .sid or .yang file is read, and no Python is involved: the
 * instance is decoded by ccoreconf via nanocbor.
 */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "coreconfManipulation.h"   /* PathNodeT, printPathNode, DynamicLongListT */
#include "test-schc.h"              /* testSchcModelDesc + SID_* macros */

/* ---- 1. top-level key/value map of the decoded instanceCBOR ------------- */
static void showInstanceMap(CoreconfModelT *model) {
    printf("=== 1. instanceCBOR top-level key/value map ===\n");
    if (model->root == NULL || model->root->type != CORECONF_HASHMAP) {
        printf("  <root is not a map>\n\n");
        return;
    }
    printCoreconfMap(model->root->data.map_value);
    printf("\n\n");
}

/* ---- 2. value of a SID that sits directly under parentSID --------------- */
static void showValueOfSID(CoreconfModelT *model, uint64_t parentSID, uint64_t childSID) {
    const char *id = ccoreconfModelLookupIdentifier(model, childSID);
    printf("=== 2. value of %" PRIu64 "%s%s%s ===\n", childSID,
           id ? "  " : "", id ? id : "(unknown identifier)", "");

    if (model->root == NULL || model->root->type != CORECONF_HASHMAP) {
        printf("  <root is not a map>\n\n");
        return;
    }
    /* Top-level keys are absolute SIDs. */
    CoreconfValueT *parent = getCoreconfHashMap(model->root->data.map_value, parentSID);
    if (parent == NULL) {
        printf("  parent %" PRIu64 " is not in this instance\n\n", parentSID);
        return;
    }
    if (parent->type != CORECONF_HASHMAP) {
        printf("  parent %" PRIu64 " is not a map\n\n", parentSID);
        return;
    }
    /* Nested keys are CBOR deltas relative to the parent SID. */
    CoreconfValueT *value = getCoreconfHashMap(parent->data.map_value, childSID - parentSID);
    if (value == NULL) {
        printf("  no value stored at %" PRIu64 "\n\n", childSID);
        return;
    }
    printCoreconf(value);
    printf("\n\n");
}

/* ---- 3 + 4. clookup-derived path, for a valid and an invalid SID --------- */
static void showPathToSID(CoreconfModelT *model, uint64_t SID) {
    const char *id = ccoreconfModelLookupIdentifier(model, SID);
    printf("=== path to %" PRIu64 " %s===\n", SID, id ? id : "(no such SID in this model) ");

    PathNodeT *path = ccoreconfModelFindRequirementForSID(model, SID);
    if (path == NULL) {
        /* The library already logged the reason on stderr. */
        printf("  no path: %" PRIu64 " is absent from the clookup table\n\n", SID);
        return;
    }
    printPathNode(path);
    freePathNode(path);
    printf("\n");
}

/* ------------------------------------------------------------------------- *
 * Deep-traversal check: fetch the innermost nodes of the instance and compare
 * them with the values schc.json is known to hold there.
 *
 * Reaching these means walking three nested YANG lists --
 *   60096 rule -> 60100 entry -> 60113 target-value
 * -- so the list keys must select the correct entry at every level.
 * ------------------------------------------------------------------------- */

/* requestKeys is consumed as a stack (popLong takes from the back), so the
 * values are listed in REVERSE of the order the path visits them:
 *   visit order = 60130, 60129 | 60106, 60108, 60105 | 60114
 *   list  order = 60114, 60105, 60108, 60106, 60129, 60130               */
typedef struct {
    const char *name;
    uint64_t    leafSID;
    int         nKeys;
    long        requestKeys[6];
    const char *expected;      /* "<not found>" for an unsatisfiable query */
} QueryCase;

static const QueryCase deepCases[] = {
    /* target-value/value  of field-id 60061 (fid-ipv6-flowlabel) */
    { "Q1", 60115, 6, { 0, 60018, 1, 60061, 3, 5 }, "h'023456'"          },
    /* target-value/value  of field-id 60068 (fid-ipv6-version)  */
    { "Q2", 60115, 6, { 0, 60018, 1, 60068, 3, 5 }, "h'06'"              },
    /* target-value/index  of field-id 60055 (fid-coap-version)  */
    { "Q3", 60114, 6, { 0, 60018, 1, 60055, 3, 5 }, "0"                  },
    /* target-value/index  of field-id 60068 (fid-ipv6-version)  */
    { "Q4", 60114, 6, { 0, 60018, 1, 60068, 3, 5 }, "0"                  },
    /* target-value[1]/value of field-id 60060 (fid-ipv6-devprefix) */
    { "Q5", 60115, 6, { 1, 60018, 1, 60060, 3, 5 }, "h'fe80000000000000'" },

    /* Unsatisfiable queries.  pycoreconf's datastore returns None for each of
     * these, so ccoreconf must report "not found" too -- not crash, and not
     * silently hand back some other entry's value.
     *
     * WARNING: N1 and N2 currently FAIL that expectation.  When a YANG list's
     * keys select no element, examineCoreconfValue() keeps the whole ARRAY in
     * `subTree` and then dereferences it as a CoreconfHashMapT, reading ~800
     * bytes past the allocation.  Under -fsanitize=address these abort with
     * heap-buffer-overflow at coreconfTypes.c:237; without a sanitizer they
     * only *appear* to pass because the out-of-bounds read happens to yield
     * NULL.  See pending/BUGS_ENCOUNTERED.md, Bug 5.  N3 is well behaved:
     * the missing node sits under a container, so the lookup returns a clean
     * NULL. */
    { "N1 target-value index 9 does not exist",   60115, 6, { 9, 60018, 1, 60060, 3, 5 }, "<not found>" },
    { "N2 rule with id-length 9 does not exist",   60115, 6, { 9, 60018, 1, 60068, 3, 9 }, "<not found>" },
    { "N3 ticks-numbers absent from the instance", 60120, 2, { 3, 5 },                     "<not found>" },
};
#define DEEP_CASES (sizeof(deepCases) / sizeof(deepCases[0]))

/* Render a scalar exactly the way the expectations above are written. */
static void valueToString(const CoreconfValueT *v, char *buf, size_t n) {
    if (v == NULL) {
        snprintf(buf, n, "<not found>");
    } else if (v->type == CORECONF_BYTES) {
        size_t o = (size_t)snprintf(buf, n, "h'");
        for (size_t i = 0; i < v->data.bytes_value.length && o + 2 < n; i++) {
            o += (size_t)snprintf(buf + o, n - o, "%02x", v->data.bytes_value.data[i]);
        }
        snprintf(buf + o, n - o, "'");
    } else if (isTypeUint(v->type) || isTypeInt(v->type)) {
        snprintf(buf, n, "%llu",
                 (unsigned long long)getCoreconfValueAsUint64((CoreconfValueT *)v));
    } else {
        snprintf(buf, n, "<type %d>", v->type);
    }
}

static int runDeepQueries(CoreconfModelT *model) {
    int passed = 0;
    printf("=== 5. deep queries (expected values taken from schc.json) ===\n");
    for (size_t i = 0; i < DEEP_CASES; i++) {
        const QueryCase *tc = &deepCases[i];
        char got[64] = "";

        PathNodeT *path = ccoreconfModelFindRequirementForSID(model, tc->leafSID);
        if (path == NULL) {
            printf("  %s: FAIL  no path for %" PRIu64 "\n", tc->name, tc->leafSID);
            continue;
        }

        DynamicLongListT *keys = createDynamicLongList();
        for (int k = 0; k < tc->nKeys; k++) {
            addLong(keys, tc->requestKeys[k]);
        }

        /* Returns { leafSID: <node borrowed from the model tree> }.  The node
         * is not ours, so this wrapper must NOT be handed to freeCoreconf. */
        CoreconfValueT *hit = ccoreconfModelExamineCoreconfValue(model, keys, path);
        CoreconfValueT *leaf = hit ? getCoreconfHashMap(hit->data.map_value, tc->leafSID) : NULL;

        valueToString(leaf, got, sizeof(got));
        int ok = strcmp(got, tc->expected) == 0;
        printf("  %s: %s  leaf %" PRIu64 "  got %-20s expected %s\n",
               tc->name, ok ? "PASS" : "FAIL", tc->leafSID, got, tc->expected);

        freeDynamicLongList(keys);
        freePathNode(path);
        passed += ok;
    }
    printf("  %d/%d deep queries matched\n\n", passed, (int)DEEP_CASES);
    return passed;
}

int main(void) {
    CoreconfModelT *model = ccoreconfModelLoadDesc(&testSchcModelDesc);
    if (model == NULL) {
        fprintf(stderr, "failed to load the test-schc model\n");
        return 1;
    }

    showInstanceMap(model);
    showValueOfSID(model, 60095, SID_TEST_SCHC_RULE);   /* 60096, delta 1 */
    showPathToSID(model, 60120);
    showPathToSID(model, 60900);                         /* deliberately invalid */

    /* Manuel tests go here*/
    int deepPassed = runDeepQueries(model);

    ccoreconfModelFree(model);
    return deepPassed == (int)DEEP_CASES ? 0 : 1;
}
