# RelayMatrix-sketches
Arduino sketch for Relay Matrix Controller

This sketch is written to run on an Arduino Micro, although it will probably work on an Adafruit Pro Trinket as well.

Commands are received by Ethernet via an ENC28J60 Ethernet module which connects to the processor via SPI.  
The processor connects to up to 8 MCP23017 I2C port expanders.  Each expander has 16 GPIO pins which can drive 4 banks
of 8 relay modules each.

Received messages are echoed exactly back to the sender.  The message format is 4 bytes long.

byte|label|description
----|-----|-----------
0|signature|ASCII 'R'
1|sequence|checked by sender when message is echoed
2|block|block number, 0 to highest block number
3|select|throttle channel select, 0 to 7, or 255 to deselect all throttles
