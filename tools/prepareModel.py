# Generate the C model data for one CORECONF model.
#
# Input:  one or more .sid files (pyang --sid-extension) and an instance JSON.
# Output: <name>.h / <name>.c holding the static tables and the instance CBOR,
#         bundled in one `const CoreconfModelDescT <name>ModelDesc` that
#         ccoreconfModelLoadDesc() consumes.
#
# Dependencies: see tools/requirements.txt (install into a venv)

import argparse
import json
import os
import re

import pycoreconf
from jinja2 import Environment, FileSystemLoader

NAMESPACES = {"module": "SID_NS_MODULE", "identity": "SID_NS_IDENTITY", "data": "SID_NS_DATA"}


def get_jinja_env():
    template_dir = os.path.join(os.path.dirname(__file__), "templates")
    return Environment(loader=FileSystemLoader(template_dir), trim_blocks=True, lstrip_blocks=True,
                       keep_trailing_newline=True)


def arrayRows(values, perRow, fmt="{}"):
    """C array initializer lines: `perRow` values each, comma-terminated"""
    return [", ".join(fmt.format(v) for v in values[i:i + perRow]) + ","
            for i in range(0, len(values), perRow)]


def camelCase(name):
    """sensor-model -> sensorModel; a leading digit gets a "model" prefix"""
    words = [w for w in re.split(r"[^0-9A-Za-z]+", name) if w]
    if not words:
        raise SystemExit(f"invalid model name: {name!r}")
    if words[0][0].isdigit():
        words.insert(0, "model")
    return words[0][0].lower() + words[0][1:] + "".join(w[0].upper() + w[1:] for w in words[1:])


def macroWord(text):
    return re.sub(r"[^0-9A-Za-z]+", "_", text).strip("_").upper()


def readSIDFile(path):
    """Return (module name, items, key-mapping) of one .sid file (RFC 9595 or legacy layout)"""
    with open(path) as f:
        obj = json.load(f)
    if len(obj) == 1 and next(iter(obj)).endswith("sid-file"):
        obj = next(iter(obj.values()))
    items = obj.get("item") or obj.get("items") or []
    keyMapping = obj.get("key-mapping")
    if keyMapping is None:
        raise SystemExit(f"{path}: no key-mapping; regenerate it with pyang --sid-extension")
    return obj.get("module-name", "unknown"), items, keyMapping


def collectEntries(sidFiles):
    """All SID items of all files, plus the merged key-mapping {listSID: [keySID, ...]}"""
    entries = []
    keyMapping = {}
    skipped = {}
    for path in sidFiles:
        module, items, km = readSIDFile(path)
        for item in items:
            # SIDNamespace in coreconfModel.h has no value for e.g. "feature" yet
            if item["namespace"] not in NAMESPACES:
                skipped[item["namespace"]] = skipped.get(item["namespace"], 0) + 1
                continue
            identifier = item["identifier"]
            # Identities are module-qualified, as pycoreconf does
            if item["namespace"] == "identity":
                identifier = f"{module}:{identifier}"
            entries.append({
                "identifier": identifier,
                "sid": int(item["sid"]),
                "namespace": item["namespace"],
            })
        keyMapping.update({int(k): [int(s) for s in v] for k, v in km.items()})
    for namespace, count in skipped.items():
        print(f"Note: skipped {count} '{namespace}' item(s); coreconfModel.h has no SIDNamespace for them")
    entries.sort(key=lambda e: e["sid"])
    return entries, keyMapping


def schemaParents(entries):
    """{childSID: parentSID} for data nodes: the closest ancestor path that is itself a data node (0 at the top)"""
    dataSIDs = {e["identifier"]: e["sid"] for e in entries if e["namespace"] == "data"}
    parents = {}
    for path, sid in dataSIDs.items():
        parent = path.rsplit("/", 1)[0]
        while parent and parent not in dataSIDs:
            parent = parent.rsplit("/", 1)[0]
        parents[sid] = dataSIDs.get(parent, 0)
    return parents


def macroNames(entries, modelName):
    """SID_<MODEL>_<shortest unique path suffix>, e.g. SID_SENSOR_HEALTHVALUE, SID_SENSOR_IDENTITY_HIGH_LEVEL"""
    def words(e):
        if e["namespace"] == "data":
            # "/sensor:sensor/sensorHealth" -> ["SENSOR", "SENSORHEALTH"]
            return [macroWord(seg.split(":")[-1]) for seg in e["identifier"].strip("/").split("/")]
        return [e["namespace"].upper(), macroWord(e["identifier"].split(":")[-1])]

    allWords = [words(e) for e in entries]
    # Modules and identities always keep their MODULE_/IDENTITY_ prefix
    depth = [1 if e["namespace"] == "data" else 2 for e in entries]
    while True:
        names = ["_".join(w[-d:]) for w, d in zip(allWords, depth)]
        seen = {}
        for i, n in enumerate(names):
            seen.setdefault(n, []).append(i)
        clashes = [i for idx in seen.values() if len(idx) > 1 for i in idx if depth[i] < len(allWords[i])]
        if not clashes:
            break
        for i in clashes:
            depth[i] += 1
    prefix = "SID_" + macroWord(modelName)
    return [f"{prefix}_{n}" for n in names]


def main():
    parser = argparse.ArgumentParser(description="Generate <name>.c/.h model data (tables + instance CBOR) from SID files and an instance JSON")
    parser.add_argument("--sid-files", nargs="+", required=True, help="Input .sid files (generated with pyang --sid-extension)")
    parser.add_argument("--instance", required=True, help="Instance data in JSON (RFC 7951)")
    parser.add_argument("--name", required=True, help="Model name; output files are <name>.c and <name>.h")
    parser.add_argument("--output-dir", default=".", help="Directory for the generated files (default: .)")
    args = parser.parse_args()

    entries, keyMapping = collectEntries(args.sid_files)
    instanceCBOR = pycoreconf.CORECONFModel(args.sid_files).encode_json(args.instance)

    # Flat arrays of key SIDs and parent SIDs; table rows point into them
    keySIDs = []
    keyRows = []
    for listSID in sorted(keyMapping):
        keyRows.append({"listSID": listSID, "offset": len(keySIDs), "count": len(keyMapping[listSID])})
        keySIDs.extend(keyMapping[listSID])
    keyOffset = {row["listSID"]: row for row in keyRows}

    parents = schemaParents(entries)
    parentSIDs = []
    clookupRows = []
    for child in sorted(parents):
        clookupRows.append({"childSID": child, "offset": len(parentSIDs), "count": 1})
        parentSIDs.append(parents[child])

    sidRows = []
    for e, macro in zip(entries, macroNames(entries, args.name)):
        row = keyOffset.get(e["sid"])
        sidRows.append({
            "identifier": json.dumps(e["identifier"]),
            "sid": e["sid"],
            "namespace": NAMESPACES[e["namespace"]],
            "isList": int(row is not None),
            "keyOffset": row["offset"] if row else None,
            "keyCount": row["count"] if row else 0,
            "macro": macro,
        })

    context = {
        "name": args.name,
        "descName": camelCase(args.name) + "ModelDesc",
        "guard": "CCORECONF_MODEL_" + macroWord(args.name) + "_H",
        "inputs": ", ".join(os.path.basename(p) for p in args.sid_files + [args.instance]),
        "instanceRows": arrayRows(list(instanceCBOR), 12, "0x{:02x}"),
        "keySIDs": arrayRows(keySIDs, 8),
        "keyRows": keyRows,
        "parentSIDs": arrayRows(parentSIDs, 8),
        "clookupRows": clookupRows,
        "sidRows": sidRows,
    }

    env = get_jinja_env()
    os.makedirs(args.output_dir, exist_ok=True)
    for template, suffix in (("model.h.jinja", ".h"), ("model.c.jinja", ".c")):
        path = os.path.join(args.output_dir, args.name + suffix)
        with open(path, "w") as f:
            f.write(env.get_template(template).render(context))
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
