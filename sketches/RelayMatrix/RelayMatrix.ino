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

// Define port to receive commands.
unsigned int portLocal = 4023;

// Allocate receive/transmot buffer.
byte packetBuffer[16];

// Define actual packet size.
#define PKT_SIZE 4

// Define # port expander devices
#define NUM_MCP 8

// Define number of ports per MCP23017
#define PORTS_PER_MCP 4

// Define number of blocks (4 blocks per expander device)
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
#define ETH_RESET 12

void setup()
{
  for (byte mcp = 0; mcp < NUM_MCP; ++mcp) {
    mcps[mcp] = new Adafruit_MCP23017;
    rs[mcp] = new BlockControl(*mcps[mcp], mcp);
    for (byte port = 0; port < PORTS_PER_MCP; ++port) {
      byte blk = mcp * PORTS_PER_MCP + port;
      blocks[blk] = new Block(*rs[mcp], port);
    }
  }
  
  // Reset the Ethernet module.
  pinMode(ETH_RESET, OUTPUT);
  digitalWrite(ETH_RESET, LOW);
  delay(100);
  digitalWrite(ETH_RESET, HIGH);
  delay(1000);

  // Start Ethernet module, then UDP processing.
  Ethernet.begin(mac, ipLocal);
  udp.begin(portLocal);

  // Start I2C on bus expander.
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
      // Trigger the LED pulse.
      led.trigger();
      // If the select byte is 0xFF (255)...
      if (s == 0xFF) {
        // ... disable block...
        blocks[b]->enable(false);
      } else {
        // ... otherwise, enable block and select channel.
        blocks[b]->enable(true);
        blocks[b]->select(s & 0x7);
      }
    }
    
  }

  // Update the Pulse and Block state machines.
  led.update();
  StateMachine::updateAll(blocks, NUM_BLK);
}
