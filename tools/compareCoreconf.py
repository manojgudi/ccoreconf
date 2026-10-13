# Check that a CORECONF CBOR payload carries the same data as an instance JSON.
#
# Decodes the CBOR with pycoreconf and compares the result with the JSON.
# Map key order is ignored (CBOR maps are unordered), so ccoreconf output
# matches pycoreconf output whenever the data is the same.
# Exit code 0 = same data, 1 = different (the differing paths are printed).
#
# Dependencies: see tools/requirements.txt (install into a venv)

import argparse
import json
import sys

import pycoreconf


def differences(expected, actual, path=""):
    """Yield one line per difference between two decoded JSON trees"""
    if isinstance(expected, dict) and isinstance(actual, dict):
        for key in expected.keys() - actual.keys():
            yield f"{path}/{key}: missing"
        for key in actual.keys() - expected.keys():
            yield f"{path}/{key}: unexpected"
        for key in expected.keys() & actual.keys():
            yield from differences(expected[key], actual[key], f"{path}/{key}")
    elif isinstance(expected, list) and isinstance(actual, list):
        if len(expected) != len(actual):
            yield f"{path}: {len(expected)} entries expected, {len(actual)} found"
        for i, (e, a) in enumerate(zip(expected, actual)):
            yield from differences(e, a, f"{path}[{i}]")
    elif expected != actual:
        yield f"{path or '/'}: expected {expected!r}, found {actual!r}"


def main():
    parser = argparse.ArgumentParser(description="Check that a CORECONF CBOR payload has the same data as an instance JSON")
    parser.add_argument("--sid-files", nargs="+", required=True, help="The .sid files of the model")
    parser.add_argument("--json", required=True, help="Expected instance data (JSON)")
    parser.add_argument("--cbor", required=True, help="CBOR payload to check (e.g. ccoreconf output)")
    args = parser.parse_args()

    model = pycoreconf.CORECONFModel(args.sid_files)
    with open(args.json) as f:
        expected = json.load(f)
    with open(args.cbor, "rb") as f:
        actual = json.loads(model.decode_to_json(f.read()))

    diffs = list(differences(expected, actual))
    if diffs:
        print(f"DIFFERENT: {len(diffs)} difference(s)")
        for line in diffs[:20]:
            print("  " + line)
        return 1
    print("SAME DATA")
    return 0


if __name__ == "__main__":
    sys.exit(main())
