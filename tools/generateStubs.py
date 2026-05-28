# Read a SID JSON file and Generate a C header file with stubs for all the SID functions

# Import argparse and write a command line parser to accept .sid file as input
# Read the .sid file and parse it as JSON
import argparse
import json
import cbor2
import pycoreconf

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
    "enum" : "enum",
    "identityref" : "char *",   # Map identityref to a char* since we dont have identityMap
    "void" : "void"
}


# Define default/NULL values for each type
defaultTypeValue = {
    "uint8": "0",
    "uint16": "0",
    "uint32": "0",
    "uint64": "0",
    "int8": "0",
    "int16": "0",
    "int32": "0",
    "int64": "0",
    "float32": "0",
    "float64": "0",
    "decimal64": "0",
    "boolean": "false",
    "binary" : "false",
    "string": "NULL",
    "bytes": "NULL",
    "array": "NULL",
    "enum" : "enum",
    "identityref" : "NULL",
    "void" : "NULL"
}

enumTypes = {}
functionNameWithEnumTypes = {}

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

    # Shorten it, take the last two words and use that.
    # The hope is that for most cases this is unique enough
    id_name = "_".join([item for item in identifier.split("_")[-2:]])

    return id_name


def generateSIDPreprocessors(model):
    """
    Take pycoreconf generated model as input and generate C headers for all the sids
    For each item which has a valid type, add a line
        # define LEAF LEAF_SID_NUMBER
    """
    # Map of leafIdentifier to number of times it has been referred
    leafIdentifierCount = {}
    cHeaders = ""

    for identifier, sid in model.sids.items():
        if identifier not in model.types:
            continue

        itemType = model.types[identifier]
        formattedItemIdentifier = formatIdentifier(identifier)

        # Resolve Enumeration type
        if isinstance(itemType, dict):
            functionName = formattedItemIdentifier
            enumDefinition = ", ".join(f'{value} = {key}' for key, value in itemType.items())
            # Signal the code generator that this is an enum type
            # TODO: How do we do this now? item["type"] = "enum"
            # Populate the enumTypes TODO: Remove these globals when refactoring
            enumTypeName = functionName.title() + "Enum"
            enumTypes[enumTypeName] = enumDefinition
            functionNameWithEnumTypes[functionName] = enumTypeName

            # Add an alias in the header
            cHeaders += "#define  SID_" + formattedItemIdentifier.upper() + " " + str(sid) + "\n"

            # Continue the next section
            continue
        elif itemType in cborTypeToCMapping:

            cHeaders += "#define  SID_" + formattedItemIdentifier.upper() + " " + str(sid) + "\n"

    return cHeaders

def generateFunctionPreprocessors(functionPrefix, sid, identifier):
    """
    Construct function name and its SID and put them as synonyms in the preprocessor
    """
    functionName = formatIdentifier(identifier)
    return "#define " + functionPrefix + functionName + " " + functionPrefix + str(sid) + "\n"

class SIDItem:
    def __init__(self, namespace, identifier, sid, type_=None, stable=False):
        self.namespace = namespace
        self.identifier = identifier
        self.sid = sid
        self.stable = stable
        self.sidKeyItems = []
        self.functionPrototype = ""

        if type_:
            # NOTE fix this later
            if isinstance(type_, list):
                self.type = "void"
            else:
                self.type = type_
            self.checkType()
        else:
            self.type = "void"

    def checkType(self):
        # Ideally this should come from libcbor?
        # Check if self.type is an ENUM
        if not isinstance(self.type, dict):
            # Check if self.type in cborType
            if (self.type not in cborTypeToCMapping):
                print("Invalid type: " + self.type)
                # NOTE treat Invalid types as string
                self.type = "string"

    def addSidKey(self, sidKeyItem):
        self.sidKeyItems.append(sidKeyItem)

    def addFunctionPrototype(self, functionPrototype):
        self.functionPrototype = functionPrototype

    def generateDocStrings(self):
        """
        Generate C documentation String for the sidKey of the form:
            /*
            This is an autogenerated function associated to
            SID:
            Module:
            Identifier:
            function params: (ruleIdValue, ruleIdLength)
            Stable:
            */
        """
        docStringTemplate = """
/*
    This is an autogenerated function associated to
    SID: %s
    Module: %s
    Identifier: %s
    function params:%s
    Stable: %s
*/\n"""

        docString = docStringTemplate % (self.sid, self.namespace, self.identifier, " , ".join([x.identifier for x in self.sidKeyItems]), str(self.stable))
        return docString

    def generateFunctionBody(self):
        """
        Generate function body to be placed inside the prototype functions
        """
        leafInitialization = ""
        leafReturn = ""
        functionName = formatIdentifier(self.identifier)
        instanceName = formatIdentifier(self.identifier) + "Instance"

        # If the type is enum, then initialize it differently
        if self.type == "enum":
            leafInitialization = cborTypeToCMapping[self.type] + " " + functionNameWithEnumTypes.get(functionName, "") + " " + instanceName + ";\n\t";
            leafReturn = "// Return the leaf \n"+ "    return " + instanceName + ";"

        # If the type is void, then don't initialize the leaf, else initialize with default value
        elif self.type != "void":
            leafInitialization = cborTypeToCMapping[self.type] + " " +  instanceName + "  = " + defaultTypeValue[self.type] + ";\n\t"
            leafReturn = "// Return the leaf \n"+ "    return " + instanceName + ";"

        functionBodyTemplate = """{
    // Initialize the leaf if it has a return type with a default value;
    %s
    // Do something with the leaf
    %s
}
        """

        functionBody = functionBodyTemplate % (leafInitialization, leafReturn);
        return functionBody

    def generateCGetMethods(self):
        """
        Generate C code for this item, the sidKeys will be passed as parameters to the function
        """
        readString = "read_"
        # Don't do anything if the namespace is not "data"
        if self.namespace != "data":
            return ""

        # generate docstrings
        docString = self.generateDocStrings()

        # generate function name from the self.identifier by replacing "/" with "_"
        functionName = formatIdentifier(self.identifier)
        # generate C function argument string from the sidKeyItems
        ### MODIFY FROM HERE, function args should contain any keys if its required
        functionArgs = ""
        functionBody = self.generateFunctionBody()

        # If no sidKeyItems are found directly return the function string
        if not self.sidKeyItems:
            functionPrototype = cborTypeToCMapping[self.type] + " " + functionNameWithEnumTypes.get(functionName, "")  + " " + readString + functionName + "(void)"
            functionString = docString + functionPrototype + functionBody + "\n"

            # Add the function prototype to the list of function prototypes
            self.addFunctionPrototype("%s;"%(functionPrototype))
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
        functionPrototype = functionReturnType + " " + functionNameWithEnumTypes.get(functionName, "") + " " + readString + functionName + "(" + functionArgs + ")"
        functionString = docString + functionPrototype + functionBody + "\n"

        # Add the function prototype to the list of function prototypes
        self.addFunctionPrototype("%s;"%(functionPrototype))
        return functionString


def findKeysForLeavesBySID(itemSID, model):
    """
    Returns a list of keys for a leafSID
    """

    itemIdentifier = model.ids[itemSID]
    requiredSIDKeys = []

    print("DEETS", itemSID, itemIdentifier)

    # If itemIdentifier is itself in keyMapping, then add its keys to requiredSIDKeys
    if str(itemSID) in model.key_mapping:
        sidKeys = model.key_mapping[itemIdentifier]
        for sidKey in sidKeys:
            requiredSIDKeys.append(sidKey)

    # Check if its parents are in key_mapping and add their keys to requiredSIDKeys
    identifier = itemIdentifier
    while (identifier):
        identifier = identifier.rsplit("/", 1)[0]

        if not identifier:
            continue
        if identifier not in model.sids:
            continue

        currentItemSID = model.sids[identifier]

        if currentItemSID in model.key_mapping:
            sidKeys = model.key_mapping[identifier]
            for sidKey in sidKeys:
                requiredSIDKeys.append(sidKey)

    return requiredSIDKeys


def main():
    # write a command line parser to accept .sid file as input
    parser = argparse.ArgumentParser(description='Generate a C header file with stubs for all the SID functions')
    parser.add_argument('-f', '--file', metavar='input', nargs='+', required=False, type=str, help='List of input .sid files')
    parser.add_argument('proto', metavar='proto', type=str, help='Two files will be generated, one with the function stubs and the other with the headers')
    args = parser.parse_args()

    headersFile = "./%s" %args.proto + ".h"
    stubsFile = "./%s" %args.proto + ".c"

    # This will be put in the header file
    functionPrototypes = []

    cIncludeString = "#include <stdlib.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include <string.h>\n#include \"%s\"\n"%(args.proto + ".h")
    hIncludeString = "#include <stdlib.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include <string.h>\n#include <cbor.h>\n\n"

    file_list = args.file
    if not isinstance(file_list, list):
        file_list = [file_list]

    ccm = pycoreconf.CORECONFModel(file_list)
    keyMappingKeysList = [key for sublist in ccm.key_mapping.values() for key in sublist]

    # Contain all the contents of the H & C file
    hCode = ""
    cCode = ""

    # Iterate through dataItems and generate preprocessor directives for each item
    preprocessorDirectives = generateSIDPreprocessors(ccm)
    hCode += preprocessorDirectives + "\n\n"

    # Iterate through dataItems and generate C code for each item
    hFunctionPrototypes = ""
    for identifier, sid in ccm.sids.items():
        if identifier not in ccm.types:
            continue

        itemType = ccm.types[identifier]
        # Ignore items which are keys
        if sid in keyMappingKeysList:
            continue

        hCode += generateFunctionPreprocessors("read_", sid, identifier)

        # Generate C code for this item
        # TODO: Do we need to add back in support for stable/unstable?
        sidItem = SIDItem(ccm.namespace[identifier],
                          identifier,
                          sid,
                          itemType,
                          False)

        # Generate C code for this item
        # Find all additional keys that this sid requires based on key-mapping
        sidKeys = findKeysForLeavesBySID(sid, ccm)

        # Find item block for corresponding sidKeys
        for sidKey in sidKeys:
            if sidKey not in ccm.ids:
                raise Exception("No item found for sid key: " + sidKey)

            # TODO: Do we need to handle stable/unstable?
            sidKeyItem = SIDItem(ccm.namespace[ccm.ids[sidKey]], ccm.ids[sidKey], sidKey, ccm.types[ccm.ids[sidKey]], False)
            sidItem.addSidKey(sidKeyItem)

        cCode += sidItem.generateCGetMethods() + "\n"
        hFunctionPrototypes += sidItem.functionPrototype + "\n"

    # Add CBOR mapping to the header file
    # Dump the key_mapping into CBOR mapping
    cborMapping = cbor2.dumps(ccm.key_mapping)
    # Format the string to store as bytestrings in C
    cborMapping = str(cborMapping).replace("b'","").replace("'","")
    hCode += "\nchar* keyMapping = \"%s\";\n"%(cborMapping)

    # Add enum type
    for enumTypeName, enumDefinition in enumTypes.items():
        hCode += "enum %s {%s};\n"%(enumTypeName, enumDefinition)

    # Add the function prototypes to the header file
    hCode += "\n\n" + hFunctionPrototypes

    # print the H code to stdout
    print("//Headers\n//-----------\n")
    print(hIncludeString)
    print(hCode)
    # Write the H code to headersFile
    with open(headersFile, 'w') as f:
        f.write(hIncludeString)
        f.write(hCode)

    # print the C code to stdout
    print("//Code File\n//-----------\n")
    print(cIncludeString)
    print(cCode)

    # Write the C code to stubsFile
    with open(stubsFile, 'w') as f:
        f.write(cIncludeString)
        f.write(cCode)



if __name__ == "__main__":
    main()
