# Take a string and write it into a header file in an uint64_t buffer array 
# with the name of the string as the variable name

import argparse
import json
import cbor2
import pycoreconf


def main():

    # Take arguments from the command line to read the .sid file
    parser = argparse.ArgumentParser(description='Convert a string to a CBOR buffer in a header file')
    parser.add_argument('sid_file', type=str, help='The name of the header file from which the CBOR buffer is generated and written to the header file')
    parser.add_argument('model_file', type=str, help='The name of the model file from which the CBOR buffer is generated and written to the header file')
    parser.add_argument('output_file', type=str, help='The name of the output file to write the CBOR buffer to')

    # Parse the arguments
    args = parser.parse_args()
    outputFile = args.output_file
    sidFile = args.sid_file
    modelFile = args.model_file

    sidJSONData = modelJSONData = {}

    # Create the model object
    ccm = pycoreconf.CORECONFModel(sidFile)

    # Read the .sid file
    with open(sidFile, 'r') as sidFile:
        sidJSONData = json.load(sidFile)

    # Read the model data
    with open(modelFile, 'r') as modelFile_:
        modelJSONData = json.load(modelFile_)

    # CORECONFModel in CBOR split the string into a list of bytes
    coreconfModel = ccm.toCORECONF(modelFile)
    coreconfModelCBORString = coreconfModel.hex()
    # Add spaces between the bytes
    coreconfModelCBORString = ' '.join(coreconfModelCBORString[i:i+2] for i in range(0, len(coreconfModelCBORString), 2))

    # Key Mapping CBOR String
    keyMapping = sidJSONData["key-mapping"]
    # keyMapping_ is a map similar to keyMapping using integer values instead of string (from json)
    keyMapping_ = {}
    for sid, sidKeys in keyMapping.items():
        keyMapping_[int(sid)] = [int(x) for x in sidKeys]

    print(coreconfModelCBORString)

    keyMappingCBORString = cbor2.dumps(keyMapping_)
    keyMappingCBORString = keyMappingCBORString.hex()

    coreconfModelCBORString = coreconfModelCBORString.split(" ")
    # remove empty strings from the list
    coreconfModelCBORString = list(filter(None, coreconfModelCBORString))
    
    # open the output file
    with open(outputFile, 'w') as file:
        # Write basic imports to the header files
        file.write("#include <stdio.h>\n")
        file.write("#include <stdint.h>\n")
        file.write("#include <stddef.h>\n")

        file.write("#define MAX_CBOR_BUFFER_SIZE 8096\n")

        # write the start of the array
        file.write("const uint8_t coreconfModelCBORBuffer"  + "[] = {")
        # loop through the list of bytes, dont write the last comma for the last byte
        lastByteIndex = len(coreconfModelCBORString) - 1
        for i in range(len(coreconfModelCBORString)):
            
            # write the byte to the file
            if i == lastByteIndex:
                file.write("0x" + coreconfModelCBORString[i])
            else:
                file.write("0x" + coreconfModelCBORString[i] + ", ")

        # write the end of the array
        file.write("};\n")

        file.write("\n")
        # Write keyMappingCBORString
        keyMappingCBORString = keyMappingCBORString.split(" ")
        file.write("const uint8_t keyMappingCBORBuffer"  + "[] = {")
        # loop through the list of bytes, dont write the last comma for the last byte
        lastByteIndex = len(keyMappingCBORString) - 1
        for i in range(len(keyMappingCBORString)):
            
            # write the byte to the file
            if i == lastByteIndex:
                file.write("0x" + keyMappingCBORString[i])
            else:
                file.write("0x" + keyMappingCBORString[i] + ", ")


        # write the end of the array
        file.write("};\n")
        file.write("\n")

if __name__ == "__main__":
    main()
