#ifndef _BLOCK_CONTROL__H_
#define _BLOCK_CONTROL__H_

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

#define NONE ((byte) 0xFF)

class BlockControl
{
  private:
    Adafruit_MCP23X17& m_mcp; // port expander object
    const byte m_addr;        // 0 to 7 (I2C address 0x20 to 0x27)

  public:
    // Constructor
    // addr in range 0 to 7
    BlockControl(Adafruit_MCP23X17& mcp, const byte addr);

    // Start interface on I2C bus.
    void begin();
  
    // Write low 4 bits to port expander
    // chan = 0 writes to bits 3-0
    // chan = 1 writes to bits 7-4
    // chan = 2 writes to bits 11-8
    // chan = 3 writes to bits 15-12
    // otherwise no action taken
    void write(const byte chan, const byte bits);
  
    // Read 4 bits from port expander,
    // returns in low 4 bits of return value
    // chan as described for write method
    // returns NONE (0xFF) if chan > 3 
    byte read(const byte chan);
};

#endif
