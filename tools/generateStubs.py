# Read a SID JSON file and Generate a C header file with stubs for all the SID functions

# Import argparse and write a command line parser to accept .sid file as input
# Read the .sid file and parse it as JSON
import argparse
import json
import cbor2
import pycoreconf
import os
from jinja2 import Environment, FileSystemLoader

# Set up Jinja2 environment
def get_jinja_env():
    template_dir = os.path.join(os.path.dirname(__file__), 'templates')
    return Environment(
        loader=FileSystemLoader(template_dir),
        trim_blocks=True,
        lstrip_blocks=True
    )

cborTypeToCMapping = {
    "uint8": "uint64_t",   # Always use 64-bit to avoid union member access issues
    "uint16": "uint64_t",  # Always use 64-bit to avoid union member access issues
    "uint32": "uint64_t",  # Always use 64-bit to avoid union member access issues
    "uint64": "uint64_t",
    "int8": "int64_t",     # Always use 64-bit to avoid union member access issues
    "int16": "int64_t",    # Always use 64-bit to avoid union member access issues
    "int32": "int64_t",    # Always use 64-bit to avoid union member access issues
    "int64": "int64_t",
    "float32": "float",
    "float64": "double",
    "decimal64": "double",
    "boolean": "bool",
    "binary": "bool",
    "string": "char *",
    "bytes": "uint8_t *",
    "array": "uint8_t *",
    "enum": "enum",
    "identityref": "char *",  # Map identityref to a char* since we dont have identityMap
    "void": "void",
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
    "binary": "false",
    "string": "NULL",
    "bytes": "NULL",
    "array": "NULL",
    "enum": "enum",
    "identityref": "NULL",
    "void": "NULL",
}

# Map cbor types to CoreconfValueT constructor functions
coreconfTypeConstructors = {
    "uint8": "createCoreconfUint8",
    "uint16": "createCoreconfUint16",
    "uint32": "createCoreconfUint32",
    "uint64": "createCoreconfUint64",
    "int8": "createCoreconfInt8",
    "int16": "createCoreconfInt16",
    "int32": "createCoreconfInt32",
    "int64": "createCoreconfInt64",
    "boolean": "createCoreconfBoolean",
    "string": "createCoreconfString",
    "float32": "createCoreconfReal",
    "float64": "createCoreconfReal",
    "decimal64": "createCoreconfReal",
}

# Map cbor types to CORECONF_* enum values
coreconfTypeEnums = {
    "uint8": "CORECONF_UINT_8",
    "uint16": "CORECONF_UINT_16",
    "uint32": "CORECONF_UINT_32",
    "uint64": "CORECONF_UINT_64",
    "int8": "CORECONF_INT_8",
    "int16": "CORECONF_INT_16",
    "int32": "CORECONF_INT_32",
    "int64": "CORECONF_INT_64",
    "boolean": "CORECONF_TRUE",  # Will check value at runtime
    "string": "CORECONF_STRING",
    "float32": "CORECONF_REAL",
    "float64": "CORECONF_REAL",
    "decimal64": "CORECONF_REAL",
}

# Map cbor types to CoreconfValueT data union field names
coreconfTypeDataFields = {
    "uint8": "u8",
    "uint16": "u16",
    "uint32": "u32",
    "uint64": "u64",
    "int8": "i8",
    "int16": "i16",
    "int32": "i32",
    "int64": "i64",
    "boolean": "u8",  # Boolean is stored as u8
    "string": "string_value",
    "float32": "real_value",
    "float64": "real_value",
    "decimal64": "real_value",
}

# TODO: Perhaps refactor to not use globals in the future?
enumTypes = {}
functionNameWithEnumTypes = {}


def formatIdentifier(identifier, max_words=2):
    """
    Format the identifier by replacing "/", "-", ":" with "_"

    Args:
        identifier: The SID identifier path
        max_words: Maximum number of words to use from the end of the path (default: 2)
    """
    # Capitalize the letter next to "-"
    while "-" in identifier:
        index = identifier.index("-")
        identifier = (
            identifier[:index] + identifier[index + 1].upper() + identifier[index + 2 :]
        )

    identifier = identifier.replace("/", "_").replace(":", "_")
    # Remove the leading "_"
    if identifier[0] == "_":
        identifier = identifier[1:]

    # Shorten it, take the last max_words words and use that.
    # The hope is that for most cases this is unique enough
    id_name = "_".join([item for item in identifier.split("_")[-max_words:]])

    return id_name


def generateSIDPreprocessors(model, max_words=2):
    """
    Take pycoreconf generated model as input and generate C headers for all the sids using Jinja2

    Args:
        model: The pycoreconf model
        max_words: Maximum number of words to use from identifier path
    """
    sid_defines = []

    for identifier, sid in model.sids.items():
        if identifier not in model.types:
            continue

        itemType = model.types[identifier]
        formattedItemIdentifier = formatIdentifier(identifier, max_words)

        # Resolve Enumeration type
        if isinstance(itemType, dict):
            functionName = formattedItemIdentifier
            # Create enum type name first
            enumTypeName = functionName.title() + "Enum"

            # Prefix each enum value with the enum type name to avoid conflicts
            # Format: EnumTypeName_value = numeric_value
            # Sanitize value names: replace dashes with underscores for valid C identifiers
            enumDefinition = ", ".join(
                f"{enumTypeName}_{value.replace('-', '_')} = {key}" for key, value in itemType.items()
            )

            # Signal the code generator that this is an enum type
            enumTypes[enumTypeName] = enumDefinition
            functionNameWithEnumTypes[functionName] = enumTypeName

            sid_defines.append({
                'name': formattedItemIdentifier.upper(),
                'sid': sid
            })
        elif itemType in cborTypeToCMapping:
            sid_defines.append({
                'name': formattedItemIdentifier.upper(),
                'sid': sid
            })

    # Render template
    env = get_jinja_env()
    template = env.get_template('sid_defines.h.jinja')
    return template.render(sid_defines=sid_defines)


def generateFunctionPreprocessors(functionPrefix, sid, identifier, max_words=2):
    """
    Construct function name and its SID and put them as synonyms in the preprocessor using Jinja2

    Args:
        functionPrefix: Prefix for the function name
        sid: The SID value
        identifier: The identifier path
        max_words: Maximum number of words to use from identifier path
    """
    functionName = formatIdentifier(identifier, max_words)

    env = get_jinja_env()
    template = env.get_template('function_alias.h.jinja')
    return template.render(
        function_name=functionPrefix + functionName,
        function_sid=functionPrefix + str(sid)
    )


class SIDItem:
    def __init__(self, namespace, identifier, sid, type_=None, stable=False, isList=False, max_words=2):
        self.namespace = namespace
        self.identifier = identifier
        self.sid = sid
        self.stable = stable
        self.sidKeyItems = []
        self.functionPrototype = ""
        self.isList = isList
        self.max_words = max_words

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
        if isinstance(self.type, dict):
            # Enum types are stored as dicts - convert to "enum" string
            self.type = "enum"
        elif self.type not in cborTypeToCMapping:
            # Check if self.type in cborType
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

        docString = docStringTemplate % (
            self.sid,
            self.namespace,
            self.identifier,
            " , ".join([x.identifier for x in self.sidKeyItems]),
            str(self.stable),
        )
        return docString

    def generateFunctionBody(self):
        """
        Generate function body to be placed inside the prototype functions
        """
        leafInitialization = ""
        leafReturn = ""
        functionName = formatIdentifier(self.identifier, self.max_words)
        instanceName = formatIdentifier(self.identifier, self.max_words) + "Instance"

        # If the type is enum, then initialize it differently
        if self.type == "enum":
            leafInitialization = (
                cborTypeToCMapping[self.type]
                + " "
                + functionNameWithEnumTypes.get(functionName, "")
                + " "
                + instanceName
                + ";\n\t"
            )
            leafReturn = "// Return the leaf \n" + "    return " + instanceName + ";"

        # If the type is void, then don't initialize the leaf, else initialize with default value
        elif self.type != "void":
            leafInitialization = (
                cborTypeToCMapping[self.type]
                + " "
                + instanceName
                + "  = "
                + defaultTypeValue[self.type]
                + ";\n\t"
            )
            leafReturn = "// Return the leaf \n" + "    return " + instanceName + ";"

        functionBodyTemplate = """{
    // Initialize the leaf if it has a return type with a default value;
    %s
    // Do something with the leaf
    %s
}
        """

        functionBody = functionBodyTemplate % (leafInitialization, leafReturn)
        return functionBody

    def generateCGetMethods(self):
        """
        Generate C read stub function using Jinja2 template
        """
        # Don't do anything if the namespace is not "data"
        if self.namespace != "data":
            return ""

        functionName = formatIdentifier(self.identifier, self.max_words)

        # Prepare keys context
        keys = []
        if self.sidKeyItems:
            keys = [
                {
                    'name': formatIdentifier(k.identifier, self.max_words),
                    'c_type': cborTypeToCMapping[k.type]
                }
                for k in self.sidKeyItems
            ]

        # Determine if this is a container or list (both use CoreconfValueT*)
        is_container_or_list = self.isList or self.type == "void"

        # Determine return type
        enumType = functionNameWithEnumTypes.get(functionName, "")
        if is_container_or_list:
            returnType = "CoreconfValueT*"
        elif self.type == "enum" and enumType:
            returnType = "enum " + enumType
        else:
            returnType = cborTypeToCMapping[self.type]

        # Prepare context
        context = {
            'docstring': self.generateDocStrings(),
            'return_type': returnType,
            'function_name': "read_" + functionName,
            'identifier': self.identifier,
            'keys': keys,
            'is_container_or_list': is_container_or_list
        }

        # Render template
        env = get_jinja_env()
        template = env.get_template('read_stub.c.jinja')
        functionString = template.render(context)

        # Build function prototype for header
        keyArgs = ", ".join([f"{k['c_type']} {k['name']}" for k in keys])
        if keys:
            functionPrototype = f"{returnType} read_{functionName}({keyArgs});"
        else:
            functionPrototype = f"{returnType} read_{functionName}(void);"

        self.addFunctionPrototype(functionPrototype)
        return functionString

    def generateCSetMethods(self):
        """
        Generate C write stub function using Jinja2 template
        """
        # Don't do anything if the namespace is not "data"
        if self.namespace != "data":
            return ""

        # Skip void types (unless it's a list or container)
        # Containers (void types) should be treated like lists - they return CoreconfValueT*
        # if self.type == "void" and not self.isList:
        #     return ""

        functionName = formatIdentifier(self.identifier, self.max_words)

        # Prepare keys context
        keys = []
        if self.sidKeyItems:
            keys = [
                {
                    'name': formatIdentifier(k.identifier, self.max_words),
                    'c_type': cborTypeToCMapping[k.type]
                }
                for k in self.sidKeyItems
            ]

        # Determine value type
        enumType = functionNameWithEnumTypes.get(functionName, "")
        if self.isList:
            valueType = "CoreconfValueT*"
        elif self.type == "void":
            # Containers (void type) accept CoreconfValueT* like lists
            valueType = "CoreconfValueT*"
        elif self.type == "enum" and enumType:
            valueType = "enum " + enumType
        else:
            valueType = cborTypeToCMapping[self.type]

        # Prepare context
        # Treat void types (containers) like lists - they work with CoreconfValueT*
        is_container_or_list = self.isList or self.type == "void"
        context = {
            'docstring': self.generateDocStrings(),
            'function_name': "write_" + functionName,
            'keys': keys,
            'value_type': valueType,
            'is_list': is_container_or_list
        }

        # Render template
        env = get_jinja_env()
        template = env.get_template('write_stub.c.jinja')
        functionString = template.render(context)

        # Build function prototype for header
        keyArgs = ", ".join([f"{k['c_type']} {k['name']}" for k in keys])
        if keys:
            functionPrototype = f"int write_{functionName}({keyArgs}, {valueType} value);"
        else:
            functionPrototype = f"int write_{functionName}({valueType} value);"

        self.addFunctionPrototype(functionPrototype)
        return functionString

    def generateReadHandlerWrapper(self):
        """
        Generate handler wrapper function for reads using Jinja2 template
        Signature: CoreconfValueT* handler_read_<sid>(SidHandlerContext *ctx)
        """
        # Don't do anything if the namespace is not "data"
        if self.namespace != "data":
            return ""

        # Determine if this is a container or list (both use CoreconfValueT*)
        is_container_or_list = self.isList or self.type == "void"

        # Skip if type not supported in handlers yet (unless it's a list or container)
        if not is_container_or_list and self.type not in coreconfTypeConstructors:
            return ""

        functionName = formatIdentifier(self.identifier, self.max_words)
        userFunctionName = f"read_{functionName}"

        # Prepare template context
        context = {
            'sid': self.sid,
            'user_function': userFunctionName,
            'is_container_or_list': is_container_or_list,
            'return_type': "CoreconfValueT*" if is_container_or_list else cborTypeToCMapping.get(self.type, "void"),
            'constructor_function': coreconfTypeConstructors.get(self.type, ""),
            'keys': []
        }

        # Add keys if present
        if self.sidKeyItems:
            context['keys'] = [
                {
                    'name': formatIdentifier(k.identifier, self.max_words),
                    'c_type': cborTypeToCMapping[k.type]
                }
                for k in self.sidKeyItems
            ]

        # Render template
        env = get_jinja_env()
        template = env.get_template('read_handler.c.jinja')
        return template.render(context) + "\n"

    def generateWriteHandlerWrapper(self):
        """
        Generate handler wrapper function for writes using Jinja2 template
        Signature: int handler_write_<sid>(SidHandlerContext *ctx, CoreconfValueT *value)
        """
        # Don't do anything if the namespace is not "data"
        if self.namespace != "data":
            return ""

        # Skip void types (unless it's a list or container)
        # Containers should be treated like lists
        # if self.type == "void" and not self.isList:
        #     return ""

        functionName = formatIdentifier(self.identifier, self.max_words)
        userFunctionName = f"write_{functionName}"

        # Prepare keys context
        keys = []
        if self.sidKeyItems:
            keys = [
                {
                    'name': formatIdentifier(k.identifier, self.max_words),
                    'c_type': cborTypeToCMapping[k.type],
                    'sid': k.sid
                }
                for k in self.sidKeyItems
            ]

        env = get_jinja_env()

        # Determine if this is a container or list (both use CoreconfValueT*)
        is_container_or_list = self.isList or self.type == "void"

        # For lists and containers, use the list-specific template
        if is_container_or_list:
            context = {
                'sid': self.sid,
                'user_function': userFunctionName,
                'keys': keys
            }
            # Add first key info for updateCoreconfArrayByKey call
            if self.sidKeyItems:
                context['first_key_name'] = keys[0]['name']
                context['first_key_sid'] = keys[0]['sid']

            template = env.get_template('write_handler_list.c.jinja')
            return template.render(context) + "\n"

        # Skip if type not supported in handlers yet
        if self.type not in coreconfTypeEnums or self.type not in coreconfTypeDataFields:
            return ""

        # For regular (non-list) nodes, use the standard write template
        context = {
            'sid': self.sid,
            'user_function': userFunctionName,
            'type': self.type,
            'type_enum': coreconfTypeEnums[self.type],
            'c_type': cborTypeToCMapping[self.type],
            'data_field': coreconfTypeDataFields[self.type],
            'keys': keys
        }

        template = env.get_template('write_handler.c.jinja')
        return template.render(context) + "\n"


def findKeysForLeavesBySID(itemSID, model):
    """
    Returns a list of keys for a leafSID
    """

    itemIdentifier = model.ids[itemSID]
    requiredSIDKeys = []

    print("DEETS", itemSID, itemIdentifier)

    # If itemSID is itself in keyMapping, then add its keys to requiredSIDKeys
    if str(itemSID) in model.key_mapping:
        sidKeys = model.key_mapping[str(itemSID)]
        for sidKey in sidKeys:
            requiredSIDKeys.append(sidKey)

    # Check if its parents are in key_mapping and add their keys to requiredSIDKeys
    identifier = itemIdentifier
    while identifier:
        identifier = identifier.rsplit("/", 1)[0]

        if not identifier:
            continue
        if identifier not in model.sids:
            continue

        currentItemSID = model.sids[identifier]

        if currentItemSID in model.key_mapping:
            sidKeys = model.key_mapping[str(currentItemSID)]
            for sidKey in sidKeys:
                requiredSIDKeys.append(sidKey)

    return requiredSIDKeys


def main():
    # write a command line parser to accept .sid file as input
    parser = argparse.ArgumentParser(
        description="Generate a C header file with stubs for all the SID functions"
    )
    parser.add_argument(
        "-f",
        "--file",
        metavar="input",
        nargs="+",
        required=False,
        type=str,
        help="List of input .sid files",
    )
    parser.add_argument(
        "proto",
        metavar="proto",
        type=str,
        help="Two files will be generated, one with the function stubs and the other with the headers",
    )
    parser.add_argument(
        "--max-identifier-words",
        type=int,
        default=2,
        help="Maximum number of words to use from identifier path when generating function names (default: 2)",
    )
    args = parser.parse_args()

    # User implementation files (stubs that user modifies)
    implHeaderFile = "./%s-impl-template.h" % args.proto
    implSourceFile = "./%s-impl-template.c" % args.proto

    # Handler wrapper files (auto-generated, don't modify)
    handlerHeaderFile = "./%s-handlers.h" % args.proto
    handlerSourceFile = "./%s-handlers.c" % args.proto

    # Generate include strings using templates
    env = get_jinja_env()
    guard_name = args.proto.upper().replace('-', '_')

    impl_header_template = env.get_template('impl_header.h.jinja')
    implHIncludeString = impl_header_template.render(guard_name=guard_name)

    impl_includes_template = env.get_template('impl_includes.c.jinja')
    implCIncludeString = impl_includes_template.render(proto=args.proto)

    handler_header_template = env.get_template('handler_header.h.jinja')
    handlerHIncludeString = handler_header_template.render(guard_name=guard_name)

    handler_includes_template = env.get_template('handler_includes.c.jinja')
    handlerCIncludeString = handler_includes_template.render(proto=args.proto)

    file_list = args.file
    if not isinstance(file_list, list):
        file_list = [file_list]

    ccm = pycoreconf.CORECONFModel(file_list)
    keyMappingKeysList = [
        key for sublist in ccm.key_mapping.values() for key in sublist
    ]

    # Get list of list SIDs (keys in keyMapping)
    listSIDs = set()
    for listSid, sidKeys in ccm.key_mapping.items():
        if int(listSid) in ccm.ids:
            listSIDs.add(int(listSid))

    # Contain all the contents of the H & C file
    hCode = ""
    cCode = ""

    # Iterate through dataItems and generate preprocessor directives for each item
    preprocessorDirectives = generateSIDPreprocessors(ccm, args.max_identifier_words)
    hCode += preprocessorDirectives + "\n\n"

    # Iterate through dataItems and generate C code for each item
    implHCode = ""  # User implementation prototypes
    implCCode = ""  # User implementation stubs

    # Add CBOR mapping to the implementation source file
    # Dump the key_mapping into CBOR mapping
    cborMapping = cbor2.dumps(ccm.key_mapping)
    # Format the string to store as bytestrings in C
    cborMapping = str(cborMapping).replace("b'", "").replace("'", "")
    implCCode += '\nchar* keyMapping = "%s";\n' % (cborMapping)

    handlerHCode = ""  # Handler wrapper prototypes
    handlerCCode = ""  # Handler wrapper implementations

    registrationCalls = []
    processedSIDs = set()

    for identifier, sid in ccm.sids.items():
        # Skip if already processed
        if sid in processedSIDs:
            continue

        # Check if this is a list SID (appears as key in keyMapping)
        isListSID = sid in listSIDs

        # For list SIDs, we always want to generate handlers even without a simple type
        if not isListSID:
            # Ignore items which are keys
            if sid in keyMappingKeysList:
                continue
            # For non-list items, skip if no type and has child elements
            # (We want to generate handlers for containers that have no children)
            if identifier not in ccm.types:
                # Check if this is a container with children by seeing if any other
                # identifiers start with this one
                has_children = any(other_id.startswith(identifier + "/")
                                   for other_id in ccm.sids.keys() if other_id != identifier)
                if has_children:
                    # This is a container with children, still generate handlers for it
                    pass
                else:
                    # This might be an intermediate node we don't need
                    continue

        itemType = ccm.types.get(identifier, "void")

        implHCode += generateFunctionPreprocessors("read_", sid, identifier, args.max_identifier_words) + "\n"
        implHCode += generateFunctionPreprocessors("write_", sid, identifier, args.max_identifier_words) + "\n"

        # Generate C code for this item
        # TODO: Do we need to add back in support for stable/unstable?
        sidItem = SIDItem(ccm.namespace[identifier], identifier, sid, itemType, False, isList=isListSID, max_words=args.max_identifier_words)
        processedSIDs.add(sid)

        # Generate C code for this item
        # Find all additional keys that this sid requires based on key-mapping
        sidKeys = findKeysForLeavesBySID(sid, ccm)

        # Find item block for corresponding sidKeys
        for sidKey in sidKeys:
            if sidKey not in ccm.ids:
                raise Exception("No item found for sid key: " + sidKey)

            # TODO: Do we need to handle stable/unstable?
            sidKeyItem = SIDItem(
                ccm.namespace[ccm.ids[sidKey]],
                ccm.ids[sidKey],
                sidKey,
                ccm.types[ccm.ids[sidKey]],
                False,
                max_words=args.max_identifier_words
            )
            sidItem.addSidKey(sidKeyItem)

        # Generate user implementation stubs (read/write functions)
        implCCode += sidItem.generateCGetMethods() + "\n"
        implHCode += sidItem.functionPrototype + "\n"

        writeStub = sidItem.generateCSetMethods()
        if writeStub:
            implCCode += writeStub + "\n"

        # Generate handler wrappers (handler_read_*/handler_write_*)
        readWrapper = sidItem.generateReadHandlerWrapper()
        writeWrapper = sidItem.generateWriteHandlerWrapper()

        if readWrapper:
            handlerCCode += readWrapper + "\n"
            handlerHCode += f"CoreconfValueT* handler_read_{sid}(SidHandlerContext *ctx);\n"

        if writeWrapper:
            handlerCCode += writeWrapper + "\n"
            handlerHCode += f"int handler_write_{sid}(SidHandlerContext *ctx, CoreconfValueT *value);\n"

        # Build registration call if we have either handler
        if readWrapper or writeWrapper:
            readHandler = f"handler_read_{sid}" if readWrapper else "NULL"
            writeHandler = f"handler_write_{sid}" if writeWrapper else "NULL"
            registrationCalls.append(
                f'    registerSidHandler({sid}, {readHandler}, {writeHandler}, "{identifier}", "{itemType}");'
            )

    # Add enum type
    for enumTypeName, enumDefinition in enumTypes.items():
        implHCode += "enum %s {%s};\n" % (enumTypeName, enumDefinition)

    # Finalize implementation header
    implHCode += "\n\n// User-facing read/write function prototypes\n"
    implHCode += "// Implement these functions in your code\n\n"
    implHCode += '#endif\n'

    # Finalize handler header
    handlerHCode += "\n// Handler registration function\n"
    handlerHCode += "void registerGeneratedHandlers(void);\n"
    handlerHCode += '#endif\n'

    # Generate the registration function using template
    handlers = []
    if readWrapper or writeWrapper:
        # Build handler list from registration calls
        for call in registrationCalls:
            # Parse the registration call to extract handler info
            # Format: registerSidHandler(sid, read_handler, write_handler, "identifier", "type");
            import re
            match = re.search(r'registerSidHandler\((\d+), (.+?), (.+?), "(.+?)", "(.+?)"\)', call)
            if match:
                handlers.append({
                    'sid': match.group(1),
                    'read_handler': match.group(2),
                    'write_handler': match.group(3),
                    'identifier': match.group(4),
                    'type': match.group(5)
                })

    env = get_jinja_env()
    template = env.get_template('registration.c.jinja')
    registrationFunction = template.render(handlers=handlers)

    handlerCCode += registrationFunction

    # Generate GET handler stub and declaration
    get_stub_template = env.get_template('get_stub.c.jinja')
    get_stub = get_stub_template.render()
    implCCode += "\n" + get_stub + "\n"

    get_handler_template = env.get_template('get_handler.c.jinja')
    get_handler_decl = get_handler_template.render()
    handlerHCode += "\n" + get_handler_decl + "\n"

    # Generate PUT handler stub and declaration
    put_stub_template = env.get_template('put_stub.c.jinja')
    put_stub = put_stub_template.render()
    implCCode += "\n" + put_stub + "\n"

    put_handler_template = env.get_template('put_handler.c.jinja')
    put_handler_decl = put_handler_template.render()
    handlerHCode += "\n" + put_handler_decl + "\n"

    # Write implementation files
    print("//Implementation Header\n//-----------\n")
    print(implHIncludeString + implHCode)
    with open(implHeaderFile, "w") as f:
        f.write(implHIncludeString)
        f.write(implHCode)

    print("\n//Implementation Source\n//-----------\n")
    print(implCIncludeString + implCCode)
    with open(implSourceFile, "w") as f:
        f.write(implCIncludeString)
        f.write(implCCode)

    # Write handler wrapper files
    print("\n//Handler Header\n//-----------\n")
    print(handlerHIncludeString + handlerHCode)
    with open(handlerHeaderFile, "w") as f:
        f.write(handlerHIncludeString)
        f.write(handlerHCode)

    print("\n//Handler Source\n//-----------\n")
    print(handlerCIncludeString + handlerCCode)
    with open(handlerSourceFile, "w") as f:
        f.write(handlerCIncludeString)
        f.write(handlerCCode)


if __name__ == "__main__":
    main()
