# Example python3 generateCBORDumps.py ../samples/simple_yang/sensor_instance.json ../samples/simple_yang/sensor@unknown.sid cborDumpsHeader.h
import cbor2
import pycoreconf
import argparse
import os
from jinja2 import Environment, FileSystemLoader


def get_jinja_env():
    """Get Jinja2 environment with templates directory"""
    template_dir = os.path.join(os.path.dirname(__file__), 'templates')
    return Environment(
        loader=FileSystemLoader(template_dir),
        trim_blocks=True,
        lstrip_blocks=True
    )


def recursivelyConvertJSONToPy(jsonData):
    coreconf = {}
    for key in jsonData:
        if isinstance(jsonData[key], dict):
            coreconf[int(key)] = recursivelyConvertJSONToPy(jsonData[key])
        elif isinstance(jsonData[key], list):
            coreconf[int(key)] = []
            for i in range(len(jsonData[key])):
                if isinstance(jsonData[key][i], dict):
                    coreconf[int(key)].append(
                        recursivelyConvertJSONToPy(jsonData[key][i])
                    )
                else:
                    coreconf[int(key)].append(jsonData[key][i])
        else:
            coreconf[int(key)] = jsonData[key]
    return coreconf


def main():
    parser = argparse.ArgumentParser(
        description="""A tool which generates CBOR dumps of the given input
        file passed as a command line argument, the dump is later saved as
        uint8_t array in a .h file"""
    )
    parser.add_argument(
        "-i",
        "--input",
        required=True,
        type=str,
        help="Instance data in json format",
    )
    parser.add_argument(
        "-s",
        "--sid",
        nargs="+",
        required=False,
        type=str,
        help="List of input .sid files",
    )
    parser.add_argument(
        "-o",
        "--output",
        required=True,
        type=str,
        help="Output header file name",
    )
    args = parser.parse_args()

    sensorJsonData = args.input
    sensorSIDs = args.sid
    outputFileName = args.output

    # Make CORECONF representation of the JSON
    if not isinstance(sensorSIDs, list):
        sensorSIDs = [sensorSIDs]
    coreconfModel = pycoreconf.CORECONFModel(sensorSIDs)
    cborData = coreconfModel.encode_json(sensorJsonData)

    print("CORECONF Model")
    print(cborData)

    # Print hex string of cbor data
    print("CBOR Data")
    print(cborData.hex())

    # Extract the key mapping table from the SID file
    keyMap = coreconfModel.key_mapping

    # Key mapping table found
    print("Key Mapping Table found in the SID file:")
    print(keyMap)

    # Convert that into pure python data structure
    keyMap = recursivelyConvertJSONToPy(keyMap)
    # Convert it into a CBOR dump
    keyMappingCborData = cbor2.dumps(keyMap)

    # Print the hex string of the key mapping data
    print("Key Mapping Data Hex String:")
    print(keyMappingCborData.hex())

    # Render template with both buffers
    env = get_jinja_env()
    template = env.get_template('cbor_dumps.h.jinja')

    context = {
        'coreconf_bytes': list(cborData),
        'key_mapping_bytes': list(keyMappingCborData)
    }

    output = template.render(context)

    # Save to file
    with open(outputFileName, "w") as file:
        print(f"Data saved to {outputFileName}")
        file.write(output)


if __name__ == "__main__":
    main()
