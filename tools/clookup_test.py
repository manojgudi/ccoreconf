import json
import pprint

pp = pprint.PrettyPrinter(indent=4)
SID_NEW_FORMAT  = "NEW"
SID_OLD_FORMAT  = "OLD"


def createChumpLookup(cc, leafLookup, parent):
    """
    Flat lookup
        {"leaf" : [parent1, parent2]}
    """
    if type(cc) == dict:
        for ks, vs in cc.items():
            ks = int(ks)
            ksFull = ks + parent
            leafLookup.setdefault(ksFull, set([])).add(parent)
            leafLookup = createChumpLookup(vs, leafLookup, ksFull)

    elif type(cc) == list:
        for x in cc:
            leafLookup = createChumpLookup(x, leafLookup, parent)
    else:
        # Leaf
        pass

    return leafLookup
            
            
def findKeyRequirements(sid, ccLookup, keyMap, depth=10):
    """
    For a particular SID how many keys are required to access it
    """
    if sid not in ccLookup:
        print("SID %s not a valid leaf or a key", sid)
        return
    
    # Until currentSID is not 0, recurse throughout
    # traverseParents and keys [parent, keys]
    traverseParents = []
    currentSID = sid
    while ((depth!=0) and (currentSID != 0)):
        parent = ccLookup[currentSID].pop()
        keys = keyMap.get(str(parent), [])

        if (parent!=0):
            traverseParents.append([parent, keys])

        currentSID = parent
        depth -= 1

    return traverseParents

def examineCoreconf(cc, keyMap, sid, keys, traverseParents):
    """
    Based on chump lookup examine the cc
    """
    # Check len(sidKeys)

    # Traverse through to find the leaf
    traverseParents.reverse()
    subTree = cc
    previousSID = 0
    for parentSID, sidKeys in  traverseParents:
        subTree = subTree[str(parentSID - previousSID)]
        previousSID = parentSID
        if sidKeys == []:
            continue

        # For a given key, assign element to a subTree only if element[sidDiff] == valueCheck
        for element in subTree:
            keysClone = list(keys)
            sidKeyValueMatchSet = set()
            for sidKey in sidKeys:
                sidDiff = str(sidKey - parentSID)
                valueCheck = keysClone.pop(0)
                # Should there be a else?
                if element[sidDiff] == valueCheck:
                    sidKeyValueMatchSet.add(sidKey)

            # Check if all sidKey exist in sidKeyValueMatchSet, if yes, then subTree = element if not continue
            if set(sidKeys) == sidKeyValueMatchSet:
                subTree = element
                keys = list(keysClone)
                break
            
    returnMap = {parentSID : subTree}
    return returnMap

def getParents(identifier, formatSID, identifierToSIDMap):
    """
    """
    parents = []
    if formatSID == SID_NEW_FORMAT:
        while(identifier.rfind("/")) != -1:
            parent = identifier[ : identifier.rfind("/") ]
            # If parent is empty string, then ignore
            identifier = parent
            if not parent:
                continue

            # Find the SID of the parent
            parents.append( identifierToSIDMap[parent] )
    print(parents)
    return parents


def findDependencies(sid, keyMap, identifierToSIDMap, sidToIdentifierMap):
    """
    """
    parentSIDKeyTuples = []
    
    identifier    = sidToIdentifierMap[sid]
    parentsSID    = getParents(identifier, SID_NEW_FORMAT, identifierToSIDMap)

    requiredKeyCount = 0
    for parentSID in parentsSID:
        keys = keyMap.get(str(parentSID), [])
        requiredKeyCount += len(keys)
        parentSIDKeyTuples.append( (parentSID, keys)  )

    print("Req Count", requiredKeyCount)
    print(parentSIDKeyTuples)
    return parentSIDKeyTuples


def traverseCoreconf(cc, keyMap, sid, keys, parent, subTree):
    """
    """

    if type(cc) == dict:
        for ks, vs in cc.items():
            ks = int(ks)
            ksFull = ks + parent

            keysRequired = keyMap

            if ksFull == sid:
                return { ksFull : vs }
            else:
                return traverseCoreconf(vs, keyMap, sid, keys, ksFull)
                
    elif type(cc) == list:
        jsonList = []
        for x in cc:
            result = traverseCoreconf(x, keyMap, sid, keys, parent)
            if result:
                jsonList.append(result)

        return jsonList

    else:
        # cc is a leaf
        if (parent == 0):
            print("Something is wrong")
            return
        if (parent == sid):
            return {parent : cc }


def buildSIDMaps(items):
    """
    Return two maps
    sidToIdentifierMap, identifierToSIDMap
    """
    sidToIdentifierMap = {}
    identifierToSIDMap = {}
    for item in items:
        if item["namespace"] != "data":
            continue

        identifier = item["identifier"]
        sid        = item["sid"] 
        sidToIdentifierMap[sid] = identifier
        identifierToSIDMap[identifier] = sid
    
    return sidToIdentifierMap, identifierToSIDMap


def main():

    # DONT use the 2022-10-09 version of the sidModel as SIDs conflict
    sidModel = json.load(open("./samples/sid_examples/ietf-schc@2022-12-19.sid"))
    sidItems = sidModel["items"]

    sidToIdentifierMap, identifierToSIDMap = buildSIDMaps(sidItems)

    ccModel = json.load(open("./samples/sid_examples/coreconf.json"))
    keyMap = sidModel["key-mapping"]


    #Example1
    sid = 1000095
    keys = [5, 3]
    #Example2
    #sid = 1000113
    #keys = [5, 3, 1000073, 1, 1000018]
    #Example3
    #sid = 1000115
    #keys = [5, 3, 1000073, 1, 1000018,0]
    #Example4
    #sid = 1000113
    #keys = [5, 3, 1000057, 1, 1000018]
    #Example5
    #sid = 1000115
    #keys = [5, 3, 1000057, 1, 1000018, 1]
    
    identifier = sidToIdentifierMap[sid]
    parentSIDKeyTuples = findDependencies(sid, keyMap, identifierToSIDMap, sidToIdentifierMap)

    leafLookup = {}
    leafLookup = createChumpLookup(ccModel, leafLookup, 0)
    pp.pprint(leafLookup)
    keyRequirements = findKeyRequirements(sid, leafLookup, keyMap, depth=10)
    keyRequirements.insert(0, [sid, keyMap.get(sid, [])])
    
    pprint.pprint(("Subtree ", examineCoreconf(ccModel, keyMap, sid, keys, keyRequirements)))


if __name__ == "__main__":
    main()
