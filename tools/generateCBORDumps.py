import cbor2
import pycoreconf
import argparse

# Header preprocessor string"
preprocessorString = """
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Default definitions of buffer sizes used by RIOT application, safe to ignore
#define MAX_CORECONF_BUFFER_SIZE 4096
#define MAX_KEY_MAPPING_SIZE 128
#define MAX_CBOR_REQUEST_PAYLOAD_SIZE 32
#define MAX_CBOR_RESPONSE_PAYLOAD_SIZE 128
#define MAX_PERMISSIBLE_TRAVERSAL_REQUESTS 5
\n\n\n
"""


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

    # Save the data into a file
    # Fix the output file name, its saved as .h file?
    with open(outputFileName, "w") as file:
        print("Data saved to " + outputFileName)
        file.write(preprocessorString)
        file.write("const uint8_t coreconfModelCBORBuffer" + "[] = {")
        # Write the data into the file, if the last element, don't add a comma
        for i in range(len(cborData)):
            x = str(hex(cborData[i]))[2:]
            # If x is less than 2 digits, add a 0 to the front
            if len(x) == 1:
                x = "0" + x
            if i == len(cborData) - 1:
                file.write("0x" + x)
            else:
                file.write("0x" + x + ", ")

        file.write("};\n\n\n")

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

    # Save the data into a the output file
    with open(outputFileName, "a") as file:
        print("Key mapping saved to " + outputFileName)

        file.write("const uint8_t keyMappingCBORBuffer" + "[] = {")
        # Write the data into the file, if the last element, don't add a comma
        for i in range(len(keyMappingCborData)):
            x = str(hex(keyMappingCborData[i]))[2:]
            # If x is less than 2 digits, add a 0 to the front
            if len(x) == 1:
                x = "0" + x
            if i == len(keyMappingCborData) - 1:
                file.write("0x" + x)
            else:
                file.write("0x" + x + ", ")

        file.write("};\n")


if __name__ == "__main__":
    main()
