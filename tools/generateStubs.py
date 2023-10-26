# Read a SID JSON file and Generate a C header file with stubs for all the SID functions

# Import argparse and write a command line parser to accept .sid file as input
# Read the .sid file and parse it as JSON
import argparse
import json

cborTypeToCMapping = {
    "uint8": "uint8_t",
    "uint16": "uint16_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "int8": "int8_t",
    "int16": "int16_t",
    "int32": "int32_t",
    "int64": "int64_t",
    "float32": "float",
    "float64": "double",
    "decimal64": "double",
    "boolean": "bool",
    "binary" : "bool",
    "string": "char *",
    "bytes": "uint8_t *",
    "array": "uint8_t *",
    "identityref" : "char *",
    "void" : "void"
}


def formatIdentifier(identifier):
    """
    Format the identifier by replacing "/", "-", ":" with "_"
    """
    # Capitalize the letter next to "-"
    while "-" in identifier:
        index = identifier.index("-")
        identifier = identifier[:index] + identifier[index+1].upper() + identifier[index+2:]

    identifier = identifier.replace("/", "_").replace(":", "_")
    # Remove the leading "_"
    if identifier[0] == "_":
        identifier = identifier[1:]

    return identifier

class SIDItem:
    def __init__(self, namespace, identifier, sid, type_=None):
        self.namespace = namespace
        self.identifier = identifier
        self.sid = sid
        self.sidKeyItems = []
        if type_:
            # NOTE fix this later
            if type(type_) == list:
                self.type = "void"
            else:
                self.type = type_
            self.checkType()
        else:
            self.type = "void"

    def checkType(self):
        # Ideally this should come from libcbor?
        if self.type not in cborTypeToCMapping:
            raise Exception("Invalid type: " + self.type)
        
    def addSidKey(self, sidKeyItem):
        self.sidKeyItems.append(sidKeyItem)
    
    def generateCGetMethods(self):
        """
        Generate C code for this item, the sidKeys will be passed as parameters to the function
        """
        getString = "get_"
        # Don't do anything if the namespace is not "data"
        if self.namespace != "data":
            return ""
        
        # generate function name from the self.identifier by replacing "/" with "_"
        functionName = formatIdentifier(self.identifier)
        # generate C function argument string from the sidKeyItems
        functionArgs = ""
        functionBody = "{\n\t}"

        # If no sidKeyItems are found directly return the function string
        if not self.sidKeyItems:
            functionString = cborTypeToCMapping[self.type] + " " + getString + functionName + "()" + functionBody + "\n"
            return functionString

        lastSidKeyItem = self.sidKeyItems[-1]
        for sidKeyItem in self.sidKeyItems:
            # check if sidKeyItem is the last item in the list
            if sidKeyItem == lastSidKeyItem:
                # if yes, then don't add the comma
                functionArgs += cborTypeToCMapping[sidKeyItem.type] + " " +  formatIdentifier(sidKeyItem.identifier)
            else:
                functionArgs += cborTypeToCMapping[sidKeyItem.type] + " " +  formatIdentifier(sidKeyItem.identifier) + ", "

        # generate C function return type from the self.type
        functionReturnType = cborTypeToCMapping[self.type]
        functionString = functionReturnType + " " + getString + functionName + "(" + functionArgs + ")" + functionBody + "\n"
        return functionString


def findIdentifierBlock(sid, items):
    """
    Iterate through items and return the dictionary which matches the sid number
    """
    for item in items:
        if item["sid"] == sid:
            return item
    return {}

def main():
    # write a command line parser to accept .sid file as input
    parser = argparse.ArgumentParser(description='Generate a C header file with stubs for all the SID functions')
    parser.add_argument('input', metavar='input', type=str, help='The input .sid file')
    #parser.add_argument('output', metavar='output', type=str, help='The output .h file')
    args = parser.parse_args()

    includeString = "#include <stdlib.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include <string.h>\n"

    # read the .sid file and parse it as JSON
    with open(args.input, 'r') as f:
        sidJSON = json.load(f)

    # Read "key-mapping" from sidJSON
    keyMapping = sidJSON["key-mapping"]

    # Iterate through items and find items with namespace "data"
    dataItems = []
    for item in sidJSON["items"]:
        if item["namespace"] == "data":
            # check if item is in keyMapping
            itemSID = item["sid"]
            if itemSID in keyMapping:
                # Find out the keys for this item
                sidKeys = keyMapping[itemSID]

            dataItems.append(item)


    # Read "items" from sidJSON
    items = sidJSON["items"]

    # Iterate through dataItems and generate C code for each item
    cCode = ""
    for item in dataItems:
        # Generate C code for this item
        sidItem = SIDItem(item["namespace"], item["identifier"], item["sid"], item.get("type"))
        # Find out the keys for this item
        sidKeys = keyMapping.get(str(item["sid"]), [])
        for sidKey in sidKeys:
            # Find the sidKeyItem from items
            sidKeyItemMap = findIdentifierBlock(sidKey, items)
            if not sidKeyItemMap:
                raise Exception("No item found for sid key: " + str(sidKey["sid"]))
            sidKeyItem = SIDItem(sidKeyItemMap["namespace"], sidKeyItemMap["identifier"], sidKeyItemMap["sid"], sidKeyItemMap.get("type"))
            sidItem.addSidKey(sidKeyItem)

        cCode += sidItem.generateCGetMethods() + "\n"

    # print the C code to stdout
    print(includeString)
    print(cCode)

if __name__ == "__main__":
    main()