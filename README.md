# CCORECONF
Open-source library which implements CORECONF (CoAP Management Interface) specified in the [ietf-draft-](https://datatracker.ietf.org/doc/draft-ietf-core-comi/).

## What is CORECONF?
Briefly, CoAP Management Interface (CORECONF) protocol is a network management interface for constrained devices and networks. It is based on the Constrained Application Protocol (CoAP) and uses the YANG data modeling language to describe the data that can be managed. CORECONF is designed to be lightweight and efficient, making it suitable for use in resource-constrained environments.

CORECONF can be used to manage a wide range of devices, including sensors, actuators, and other IoT devices. It can also be used to manage networks, such as wireless sensor networks and mesh networks.

## What does this library do?

This library intends to provide tools for IoT projects to implement this protocol for communicating with their IoT devices. These tools are basically functional interfaces which mainly allow:
1. To express data models in CORECONF format: Assuming you have a YANG model, and it's associated generated SID file (using [pyang tool](https://github.com/alex-fddz/pycoreconf#requirements--setup)).
2. To query CORECONF model: Query your model using SID, or SID-keys or combination of both to quickly find a particular node in your model as explained in this [youtube-tutorial](https://www.youtube.com/watch?v=pE5NI83k3Xk&t=437s) by @ltn22 
3. To manipulate CORECONF model: Once you can query the node, you should be able to modify/update the node using the functional interfaces.

## Enough talk, show me some code.
[YES.](https://github.com/manojgudi/ccoreconf/blob/main/examples/demo_functionalities.c)