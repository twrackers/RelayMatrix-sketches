# RelayMatrix-sketches
Arduino sketch for Relay Matrix Controller

This sketch is written to run on an Arduino Micro, although it will probably work on an Adafruit Pro Trinket as well.

Commands are received by Ethernet via an ENC28J60 Ethernet module which connects to the processor via SPI.  
The processor connects to up to 8 MCP23017 I2C port expanders.  Each expander has 16 GPIO pins which can drive 4 banks
of 8 relay modules each, giving a capacity of up to 32 track blocks.

Received messages are echoed exactly back to the sender.  The message format is 4 bytes long.

byte|label|description
----|-----|-----------
0|signature|ASCII 'R'
1|sequence|checked by sender when message is echoed
2|block|block number, 0 to highest block number
3|select|throttle channel select, 0 to 7, or 255 to deselect all throttles

The sketch RelayMatrix.ino is written to support a full set of 8 MCP23017 I2C port expanders, with addresses set to 0x20 through 0x27.  If fewer than 8 are being used, their addresses should be consecutive starting at 0x20.  Also, symbol NUM_MCP must be changed to reflect the number of port expanders on the I2C bus.

Source and header files Block.cpp and Block.h define the Block class.  A Block object is the interface to a single track block's bank of up to 8 relay modules.  The Block class provides methods to enable and disable a block, and to select a throttle channel to which the block will be switched.  In effect, the Block is driving a double-pole-8-throw (DP8T) switch with a 9th no-connection state.  This class uses the BlockControl class.

Source and header files BlockControl.cpp and BlockControl.h define the BlockControl class.  A BlockControl object is a low-level interface to one group of 4 consecutive GPIO pins on a single port expander chip.  The pins are assigned to groups as 15-12, 11-8, 7-4, and 3-0.  So four Block objects can use the same BlockControl object.  This class uses the Adafruit_MCP23017 library.

My StateMachine library is on GitHub at https://github.com/twrackers/StateMachine-library.

The lowest level interface to the port expander devices is the Adafruit_MCP23017 library which is available on GitHub at https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library.

The Arduino library for the ENC28J60 Ethernet module is located at https://github.com/UIPEthernet/UIPEthernet.
