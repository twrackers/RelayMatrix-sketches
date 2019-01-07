#ifndef _BLOCK_CONTROL__H_
#define _BLOCK_CONTROL__H_

#include <Arduino.h>
#include <Adafruit_MCP23017.h>

class BlockControl
{
  private:
    Adafruit_MCP23017& m_mcp;
    const byte m_addr;

  public:
    BlockControl(Adafruit_MCP23017& mcp, const byte addr);

    void begin();
    void write(const byte chan, const byte bits);
    byte read(const byte chan);
};

#endif
