# Take a string and write it into a header file in an uint64_t buffer array 
# with the name of the string as the variable name
import sys


def main():
    # take header file name as commandline arg
    if len(sys.argv) != 2:
        print("Usage: python3 stringToCBORBuffer.py <header_file_name>")
        return
    # get the header file name
    output_file = sys.argv[1]

    # CORECONFModel in CBOR split the string into a list of bytes
    coreconfModelCBORString = "a1 1a 00 0f 42 9f a1 01 88 a4 04 98 1a a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 84 08 01 07 04 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 81 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7d 08 01 07 14 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a6 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 80 08 01 07 10 09 1a 00 0f 42 94 a7 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 0f 42 7f 08 01 07 08 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7e 08 01 07 08 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 52 06 1a 00 0f 42 7c 08 01 07 18 40 09 1a 00 0f 42 95 0d 83 a2 01 00 02 00 a2 01 01 02 00 a2 01 02 02 00 a7 01 1a 00 0f 42 4c 05 1a 00 0f 42 52 06 1a 00 0f 42 7b 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 52 06 1a 00 0f 42 79 08 01 07 18 40 09 1a 00 0f 42 95 0d 83 a2 01 00 02 00 a2 01 01 02 00 a2 01 02 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 78 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a8 01 1a 00 0f 42 4d 05 1a 00 0f 42 52 06 1a 00 0f 42 89 08 01 07 10 09 1a 00 0f 42 96 0a 81 a2 01 00 02 00 0d 81 a2 01 00 02 00 a8 01 1a 00 0f 42 4d 05 1a 00 0f 42 52 06 1a 00 0f 42 86 08 01 07 10 09 1a 00 0f 42 96 0a 81 a2 01 00 02 00 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 8a 08 01 07 10 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 88 08 01 07 10 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 77 08 01 07 02 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 0f 42 76 08 01 07 02 09 1a 00 0f 42 94 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 0f 42 74 08 01 07 04 09 1a 00 0f 42 94 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 0f 42 57 08 01 07 08 09 1a 00 0f 42 94 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 0f 42 5a 08 01 07 10 09 1a 00 0f 42 94 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 0f 42 75 08 01 07 1a 00 0f 42 8c 09 1a 00 0f 42 94 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 54 06 1a 00 0f 42 71 08 01 07 1a 00 0f 42 8d 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 54 06 1a 00 0f 42 71 08 02 07 1a 00 0f 42 8d 09 1a 00 0f 42 95 0d 82 a2 01 00 02 00 a2 01 01 02 00 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 54 06 1a 00 0f 42 71 08 03 07 1a 00 0f 42 8d 09 1a 00 0f 42 94 a6 01 1a 00 0f 42 50 05 1a 00 0f 42 54 06 1a 00 0f 42 71 08 04 07 1a 00 0f 42 8d 09 1a 00 0f 42 94 a8 01 1a 00 0f 42 4d 05 1a 00 0f 42 54 06 1a 00 0f 42 73 08 01 07 1a 00 0f 42 8d 09 1a 00 0f 42 96 0a 81 a2 01 00 02 00 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 53 06 1a 00 0f 42 5f 08 01 07 1a 00 0f 42 8d 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 18 21 03 18 23 1a 00 0f 42 98 18 22 05 a4 04 8f a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 84 08 01 07 04 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 81 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7d 08 01 07 14 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a6 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 80 08 01 07 10 09 1a 00 0f 42 94 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7f 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7e 08 01 07 08 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 52 06 1a 00 0f 42 7c 08 01 07 18 40 09 1a 00 0f 42 95 0d 83 a2 01 00 02 00 a2 01 01 02 00 a2 01 02 02 00 a7 01 1a 00 0f 42 4c 05 1a 00 0f 42 52 06 1a 00 0f 42 7b 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 52 06 1a 00 0f 42 79 08 01 07 18 40 09 1a 00 0f 42 95 0d 83 a2 01 00 02 00 a2 01 01 02 00 a2 01 02 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 78 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 1e 84 86 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 1e 84 83 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 1e 84 82 08 01 07 10 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 1e 84 84 08 01 07 10 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 50 05 1a 00 0f 42 52 06 1a 00 1e 84 85 08 01 07 10 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 18 21 03 18 23 1a 00 0f 42 98 18 22 06 a9 03 02 02 1a 00 0f 42 53 15 1a 00 0f 42 8f 14 03 18 1d 1a 00 0f 42 9c 18 21 03 18 23 1a 00 0f 42 99 18 22 01 18 26 07 a9 03 02 02 1a 00 0f 42 54 15 1a 00 0f 42 8f 14 03 18 1d 1a 00 0f 42 9c 18 21 03 18 23 1a 00 0f 42 99 18 22 02 18 26 07 a8 03 02 02 1a 00 0f 42 53 15 1a 00 0f 42 91 14 03 18 1d 1a 00 0f 42 9c 18 21 03 18 23 1a 00 0f 42 99 18 22 03 a8 03 02 02 1a 00 0f 42 54 15 1a 00 0f 42 91 14 03 18 1d 1a 00 0f 42 9c 18 21 03 18 23 1a 00 0f 42 99 18 22 04 a9 03 02 02 1a 00 0f 42 54 15 1a 00 0f 42 8e 14 03 18 1d 1a 00 0f 42 9c 18 21 03 18 23 1a 00 0f 42 99 18 22 07 18 26 01 a4 04 97 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 84 08 01 07 04 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 81 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7d 08 01 07 14 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a6 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 80 08 01 07 10 09 1a 00 0f 42 94 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7f 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7e 08 01 07 08 09 1a 00 0f 42 94 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7c 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 7b 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 79 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 78 08 01 07 18 40 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 89 08 01 07 10 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 86 08 01 07 10 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a6 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 8a 08 01 07 10 09 1a 00 0f 42 94 a6 01 1a 00 0f 42 4b 05 1a 00 0f 42 52 06 1a 00 0f 42 88 08 01 07 10 09 1a 00 0f 42 94 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 77 08 01 07 02 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 53 06 1a 00 0f 42 76 08 01 07 02 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 54 06 1a 00 0f 42 76 08 01 07 02 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 74 08 01 07 04 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 54 06 1a 00 0f 42 57 08 01 07 08 09 1a 00 0f 42 95 0d 82 a2 01 00 02 00 a2 01 01 02 00 a7 01 1a 00 0f 42 4e 05 1a 00 0f 42 53 06 1a 00 0f 42 57 08 01 07 08 09 1a 00 0f 42 95 0d 82 a2 01 00 02 00 a2 01 01 02 00 a8 01 1a 00 0f 42 4d 05 1a 00 0f 42 52 06 1a 00 0f 42 5a 08 01 07 10 09 1a 00 0f 42 96 0a 81 a2 01 00 02 00 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 53 06 1a 00 0f 42 71 08 01 07 08 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 a7 01 1a 00 0f 42 4f 05 1a 00 0f 42 52 06 1a 00 0f 42 5f 08 01 07 1a 00 0f 42 8d 09 1a 00 0f 42 93 0d 81 a2 01 00 02 00 18 21 08 18 23 1a 00 0f 42 98 18 22 18 64"

    # Key Mapping CBOR String    
    keyMappingCBORString = "a5 1a 00 0f 42 a0 82 1a 00 0f 42 c2 1a 00 0f 42 c1 1a 00 0f 42 ae 81 1a 00 0f 42 af 1a 00 0f 42 b1 81 1a 00 0f 42 b2 1a 00 0f 42 a4 83 1a 00 0f 42 aa 1a 00 0f 42 ac 1a 00 0f 42 a9 1a 00 0f 42 a6 81 1a 00 0f 42 a7"

    coreconfModelCBORString = coreconfModelCBORString.split(" ")
    # remove empty strings from the list
    coreconfModelCBORString = list(filter(None, coreconfModelCBORString))
    
    # open the output file
    with open(output_file, 'w') as file:
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
