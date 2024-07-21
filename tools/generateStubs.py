# Read a SID JSON file and Generate a C header file with stubs for all the SID functions

# Import argparse and write a command line parser to accept .sid file as input
# Read the .sid file and parse it as JSON
import argparse
import json
import cbor2

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

    # Shorten it, take the last word after "_" and use it as the identifier
    identifier = identifier.split("_")[-1]

    return identifier

# Function to generate CBOR mapping of key-mapping
def generateCBORMapping(keyMapping, identifierSIDKeyMapping):
    """
    Replace the string identifiers in keyMapping into SIDs
    and dump it into CBOR mapping
    """
    # Replace identifiers in key, values of keyMapping with SIDs
    sidKeyMapping = {}
    for identifierKey, keyList in keyMapping.items():
        # Find SID for identifierKey
        sidKey = identifierKey # identifierSIDKeyMapping[identifierKey]
        sidKeyMapping[sidKey] = []

        for key in keyList:
            sidKeyMapping[sidKey].append(key)
    
    # Dump the sidKeyMapping into CBOR mapping
    cborMapping = cbor2.dumps(sidKeyMapping)
    # Format the string to store as bytestrings in C
    cborMapping = str(cborMapping).replace("b'","").replace("'","")
    return cborMapping


def generateSIDPreprocessors(items):
    """
    Take items as input and generate C headers for all the items
    For each item which has a valid type, add a line
        # define LEAF LEAF_SID_NUMBER
    """
    # Map of leafIdentifier to number of times it has been referred
    leafIdentifierCount = {}
    cHeaders = ""

    for item in items:
        itemType = item.get("type")
        formattedItemIdentifier =  formatIdentifier(item["identifier"])
            
        # Resolve Enumeration type
        if type(itemType) == dict:
            functionName = formattedItemIdentifier
            enumDefinition = ", ".join(f'{value} = {key}' for key, value in itemType.items()) 
            # Signal the code generator that this is an enum type
            item["type"] = "enum"
            # Populate the enumTypes
            enumTypeName = functionName.title() + "Enum"
            enumTypes[enumTypeName] = enumDefinition
            functionNameWithEnumTypes[functionName] = enumTypeName

            # Add an alias in the header
            cHeaders += "#define  SID_" + formattedItemIdentifier.upper() + " " * calculateSpaces + str(item["sid"]) + "\n"

            # Continue the next section
            continue

        if itemType in cborTypeToCMapping:
            # Check if formattedItemIdentifier is already in leafIdentifierCount
            if formattedItemIdentifier in leafIdentifierCount:
                leafIdentifierCount[formattedItemIdentifier] += 1
                formattedItemIdentifier += str(leafIdentifierCount[formattedItemIdentifier])
            else:
                leafIdentifierCount[formattedItemIdentifier] = 0

            # Take absolute value of the difference between 20 and len(identifier)
            calculateSpaces = abs( 19 - len( formattedItemIdentifier) ) + 1 # +1 in case the difference is 0
            cHeaders += "#define  SID_" + formattedItemIdentifier.upper() + " " * calculateSpaces + str(item["sid"]) + "\n"
    return cHeaders

def generateFunctionPreprocessors(functionPrefix, item):
    """
    Construct function name and its SID and put them as synonyms in the preprocessor
    """
    functionName = formatIdentifier(item["identifier"])
    functionSID = item["sid"]
    calculateSpaces = abs( 19 - len(functionName) ) + 1 # +1 in case the difference is 0
    return "#define " + functionPrefix + functionName + " "*calculateSpaces + functionPrefix + str(functionSID) + "\n"

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
            if type(type_) == list:
                self.type = "void"
            else:
                self.type = type_
            self.checkType()
        else:
            self.type = "void"

    def checkType(self):
        # Ideally this should come from libcbor?
        # Check if self.type is an ENUM
        if type(self.type) != dict:
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
    function params: %s
    Stable: %s
*/\n"""

        docString = docStringTemplate % (self.sid, self.namespace, self.identifier, ", ".join([x.identifier for x in self.sidKeyItems]), str(self.stable))
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
        #functionReturnType = cborTypeToCMapping[self.type]
        functionReturnType = cborTypeToCMapping[self.type]
        functionPrototype = functionReturnType + " " + functionNameWithEnumTypes.get(functionName, "") + " " + readString + functionName + "(" + functionArgs + ")"
        functionString = docString + functionPrototype + functionBody + "\n"
        
        # Add the function prototype to the list of function prototypes
        self.addFunctionPrototype("%s;"%(functionPrototype))
        return functionString

def findIdentifierBlockBySID(sid, items):
    """
    Iterate through items and return the dictionary which matches the sid number
    """
    for item in items:
        if item["sid"] == sid:
            return item
    return {}

def findIdentifierBlockByIdentifier(identifier, items):
    """
    Iterate through items and return the dictionary which matches the identifier
    """
    for item in items:
        if item["identifier"] == identifier:
            return item
    return {}

def generateMapping(items):
    """
    NOTE items is not just dataItems, it is all the items in the sid file
    Generate a dictionary of the form:
    {
        "identifier1": sid1,
        "identifier2": sid2
    }
    """
    identifierSIDKeyMapping = {}
    sidKeyIdentifierMapping = {}
    for item in items:
        identifierSIDKeyMapping[item["identifier"]] = item["sid"]
        sidKeyIdentifierMapping[item["sid"]] = item["identifier"]
    return identifierSIDKeyMapping, sidKeyIdentifierMapping

def getSetOfKeysOfKeyMapping(keyMapping, identifierSIDKeyMapping):
    """
    Returns a set of all the keys in keyMapping
    """
    keySet = set()
    for identifierKey, keyList in keyMapping.items():
        # Find SID for identifierKey
        print(identifierSIDKeyMapping)
        sidKey = identifierSIDKeyMapping[identifierKey]

        for key in keyList:
            keySet.add( identifierSIDKeyMapping[key])

    # print keySet
    print(keySet)
    return keySet

def findKeysForLeaves(itemIdentifier, keyMapping, identifierSIDKeyMapping):
    """
    Returns a list of keys for a leafIdentifier
    """
    itemSID = identifierSIDKeyMapping[itemIdentifier]
    requiredSIDKeys = []

    # If itemIdentifier is itself in keyMapping, then add its keys to requiredSIDKeySet
    if itemIdentifier in keyMapping:
        sidKeys = keyMapping[itemIdentifier]
        for sidKey in sidKeys:
            requiredSIDKeys.append(identifierSIDKeyMapping[sidKey])

    # Check if its parents are in keyMapping and add their keys to requiredSIDKeySet
    identifier = itemIdentifier
    while (identifier):
        identifier = identifier.rsplit("/", 1)[0]
        if identifier in keyMapping:
            # Should you keep the sidKey : keyList or just the keyList in this set?
            # Add the sidKey to requiredSIDKeys
            sidKeys = keyMapping[identifier]
            for sidKey in sidKeys:
                requiredSIDKeys.append( identifierSIDKeyMapping[sidKey] )

    return requiredSIDKeys

def main():
    # write a command line parser to accept .sid file as input
    parser = argparse.ArgumentParser(description='Generate a C header file with stubs for all the SID functions')
    parser.add_argument('input', metavar='input', type=str, help='The input .sid file')
    parser.add_argument('proto', metavar='proto', type=str, help='Two files will be generated, one with the function stubs and the other with the headers')
    args = parser.parse_args()

    headersFile = "./%s" %args.proto + ".h"
    stubsFile = "./%s" %args.proto + ".c"

    # This will be put in the header file
    functionPrototypes = []

    cIncludeString = "#include <stdlib.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include <string.h>\n#include \"%s\"\n"%(args.proto + ".h")
    hIncludeString = "#include <stdlib.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include <string.h>\n#include <cbor.h>\n\n"

    # read the .sid file and parse it as JSON
    with open(args.input, 'r') as f:
        sidJSON = json.load(f)

    # Read "key-mapping" from sidJSON
    keyMapping = sidJSON["key-mapping"]

    # Read "item" from sidJSON, add compatibility check as well 
    items = sidJSON.get("item")
    if not items:
        items = sidJSON["items"]

    identifierSIDKeyMapping, sidKeyIdentifierMapping = generateMapping(items)
    # keySet is a set of all the keys which have been used in keyMapping
    sidKeySet = keyMapping.keys()
    #sidKeySet = getSetOfKeysOfKeyMapping(keyMapping, identifierSIDKeyMapping)


    # Iterate through items and find items with namespace "data"
    dataItems = []
    for item in items:
        if item["namespace"] == "data":
            # check if item is in keyMapping
            itemSID = item["sid"]
            itemIdentifier = item["identifier"]
            # NOTE Remember key-mapping now has identifiers in instead of SIDs
            if itemIdentifier in keyMapping:
                # Find out the keys for this item
                sidKeys = keyMapping[itemIdentifier]

            dataItems.append(item)

    # Contain all the contents of the H & C file
    hCode = ""
    cCode = ""

    # Iterate through dataItems and generate preprocessor directives for each item
    preprocessorDirectives = generateSIDPreprocessors(dataItems)
    hCode += preprocessorDirectives + "\n\n"

    # Iterate through dataItems and generate C code for each item
    hFunctionPrototypes = ""
    for item in dataItems:
        itemIdentifier = item["identifier"]
        itemSID = item["sid"]
        itemType = item.get("type")
        # Ignore items which are keys
        if itemSID in sidKeySet:
            continue

        
        hCode += generateFunctionPreprocessors("read_", item)

        # Generate C code for this item
        sidItem = SIDItem(item["namespace"], itemIdentifier, itemSID, itemType, item.get("stable", "false"))
        # Find out the keys for this item

        # Generate C code for this item
        sidKeys = findKeysForLeaves(itemIdentifier, keyMapping, identifierSIDKeyMapping)
       
        # Find item block for corresponding sidKeys
        for sidKey in sidKeys:
            # Find the sidKeyItem from items
            sidKeyItemMap = findIdentifierBlockBySID(sidKey, items)
            if not sidKeyItemMap:
                raise Exception("No item found for sid key: " + str(sidKey["sid"]))
            sidKeyItem = SIDItem(sidKeyItemMap["namespace"], sidKeyItemMap["identifier"], sidKeyItemMap["sid"], sidKeyItemMap.get("type"), item.get("stable"))
            sidItem.addSidKey(sidKeyItem)

        cCode += sidItem.generateCGetMethods() + "\n"
        hFunctionPrototypes += sidItem.functionPrototype + "\n"

    # Add CBOR mapping to the header file
    cborMapping = generateCBORMapping(keyMapping, identifierSIDKeyMapping)
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
