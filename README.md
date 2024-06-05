# CCORECONF
Open-source library which implements CORECONF (CoAP Management Interface) specified in this [ietf-draft](https://datatracker.ietf.org/doc/draft-ietf-core-comi/).
This library is one of fruits of the ietf-draft: [SID Extension to efficiently manipulate YANG Data Models](https://datatracker.ietf.org/doc/draft-toutain-t2t-sid-extension/)

## What is CORECONF?
Briefly, CoAP Management Interface (CORECONF) protocol is a network management interface for constrained devices and networks. It is based on the Constrained Application Protocol (CoAP) and uses the YANG data modeling language to describe the data that can be managed. CORECONF is designed to be lightweight and efficient, making it suitable for use in resource-constrained environments.

CORECONF can be used to manage a wide range of devices, including sensors, actuators, and other IoT devices. It can also be used to manage networks, such as wireless sensor networks and mesh networks.

## What does this library do?

This library intends to provide tools for IoT projects to implement this protocol for communicating with their IoT devices. These tools are basically functional interfaces which mainly allow:
1. To express data models in CORECONF format: Assuming you have a YANG model, and it's associated generated SID file (using [pyang tool](https://github.com/alex-fddz/pycoreconf#requirements--setup)).
2. To query CORECONF model: Query your model using SID, or SID-keys or combination of both to quickly find a particular node in your model as explained in this [youtube-tutorial](https://www.youtube.com/watch?v=pE5NI83k3Xk&t=437s) by [@ltn22](https://github.com/ltn22/)
3. To manipulate CORECONF model: Once you can query the node, you should be able to modify/update the node using the functional interfaces.

## How to set it up?
This is a VSCode project. It depends on libraries such as: [nanocbor](https://github.com/bergzand/NanoCBOR). Once these library/libraries have been setup, update the tasks.json and c_cpp_properties.json files in .vscode/ to be able to build any examples.

## Enough talk, show me some code.
[YES.](https://github.com/manojgudi/ccoreconf/blob/main/examples/demo_functionalities_coreconf.c)

## How do I use the CCORECONF with RIOT OS?
Take a look at this [RIOT-OS README](https://github.com/manojgudi/RIOT/blob/coreconf-integration/examples/coreconf_simple/README.md)

## What are some tools built for CORECONF and included in this library?
Yes, ideally those python stub/code generation tools should be moved out, which will happen soon..
Until then, here's the [Stub-generation README](https://github.com/manojgudi/ccoreconf/blob/stub_generation/tools/README.md)

## Further Reading
This project uses SID identifiers as specified in [RFC-9254](https://datatracker.ietf.org/doc/rfc9254/). Some other relevant projects are [pycoreconf](https://github.com/alex-fddz/pycoreconf) and [openschc](https://github.com/openschc/openschc)
