#include <Wire.h>
#include <UIPEthernet.h>

#include <StateMachine.h>
#include <Pulse.h>

#include "BlockControl.h"
#include "Block.h"

// Define MAC and IP addresses for the Ethernet adapter.
byte mac[] = {
  0x01, 0x23, 0x45, 0x67, 0x89, 0x23
};
IPAddress ipLocal(10, 0, 0, 123);

// Port to receive commands
unsigned int portLocal = 4023;

// Receive/transmit buffer
byte packetBuffer[16];

// Actual packet size
#define PKT_SIZE 4

// Each MCP23017 I2C port expander has 16 GPIO pins.
// These 16 pins are divided up into 4 consecutive "ports"
// of 4 pins each.  Each port drives a single relay matrix
// decoder/driver.
// MCP23017 port expander can be set to 8 different I2C addresses,
// 0x20 through 0x27.  If fewer than 8 port expanders are in use,
// their addresses must be consecutive (although not necessarily
// physically arranged in order) starting with 0x20.

// Number of ports per MCP23017
#define PORTS_PER_MCP 4

// Number of port expander devices in use
// Max 8 may be used, all must have consecutive
// I2C addresses starting at 0x20.
#define NUM_MCP 8

// Number of blocks = # expanders * # ports per expander
// Each 4-bit port controls one block.
#define NUM_BLK (NUM_MCP * PORTS_PER_MCP)

// Packet format:
// byte 0:  signature   82, ASCII code of 'R'
// byte 1:  sequence    0-255, rolls over to 0
// byte 2:  block       0 to highest block number
// byte 3:  select      0 to 7, 255 deselects all

// Ethernet UDP device driver
EthernetUDP udp;

// I2C bus expanders, 16 GPIO per device
Adafruit_MCP23017* mcps[NUM_MCP];

// Block controllers, 1 per expander
BlockControl* rs[NUM_MCP];

// Blocks, 4 per controller
Block* blocks[NUM_BLK];

// Pulse length on built-in LED when packet received
#define PULSE_MSEC 40

// Pulse object attached to built-in LED
Pulse led(LED_BUILTIN, HIGH, PULSE_MSEC);

// GPIO pin connected to Ethernet adapter RESET pin
#define ETH_RESET 4

void setup()
{
  // Create the collections of objects used to control
  // the relay matrix.
  for (byte mcp = 0; mcp < NUM_MCP; ++mcp) {
    // Create each port-expander object.
    mcps[mcp] = new Adafruit_MCP23017;
    // Create a BlockControl object for each port expander.
    // This allows the port expander to be accessed as 4
    // 4-bit ports.
    rs[mcp] = new BlockControl(*mcps[mcp], mcp);
    // For each BlockControl object, create 4 Block objects.
    // The port expander at address 0x20 will handle blocks
    // 0-3, address 0x21 will handle blocks 4-7, and so on.
    for (byte port = 0; port < PORTS_PER_MCP; ++port) {
      byte blk = mcp * PORTS_PER_MCP + port;
      blocks[blk] = new Block(*rs[mcp], port);
    }
  }
  
  // Reset the Ethernet module.
  // Using delay() in setup() is okay because real-time
  // processes only happen within loop().
  pinMode(ETH_RESET, OUTPUT);
  digitalWrite(ETH_RESET, LOW);
  delay(100);
  digitalWrite(ETH_RESET, HIGH);
  delay(1000);

  // Start Ethernet module, then UDP processing.
  Ethernet.begin(mac, ipLocal);
  udp.begin(portLocal);

  // Start I2C on port expanders.
  for (BlockControl* r : rs) {
    r->begin();
  }
  // Disable all blocks.
  for (Block* b : blocks) {
    b->enable(false);
  }
}

void loop()
{
  // Has a packet arrived?
  int packetSize = udp.parsePacket();
  if (packetSize) {

    // If so, read the packet.
    udp.read(packetBuffer, sizeof packetBuffer);

    // Echo the packet back to the sender.
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(packetBuffer, packetSize);
    udp.endPacket();

    // If the packet is of the correct size and has the
    // correct signature byte...
    if (packetSize == PKT_SIZE && packetBuffer[0] == 'R') {
      // Skip sequence byte, get block and select bytes.
      byte b = packetBuffer[2];
      byte s = packetBuffer[3];
      // Is block within range?
      if (b < NUM_BLK) {
        // Trigger the LED pulse.
        led.trigger();
        // If the select byte is NONE (0xFF or 255)...
        if (s == NONE) {
          // ... disable block...
          blocks[b]->enable(false);
        } else {
          // ... otherwise, enable block and select channel.
          blocks[b]->enable(true);
          blocks[b]->select(s & 0x7);
        }
      }
    }
    
  }

  // Update the Pulse and Block state machines.
  led.update();
  StateMachine::updateAll(blocks, NUM_BLK);
}
