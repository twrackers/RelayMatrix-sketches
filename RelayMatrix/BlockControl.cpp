#include "BlockControl.h"

BlockControl::BlockControl(Adafruit_MCP23X17& mcp, const byte addr) :
m_mcp(mcp), m_addr(addr & 0x07)
{
}

void BlockControl::begin()
{
  m_mcp.begin_I2C(m_addr);
  m_mcp.writeGPIOAB(0xFFFF);
  for (byte i = 0; i < 16; ++i) {
    m_mcp.pinMode(i, OUTPUT);
  }
}

void BlockControl::write(const byte chan, const byte bits)
{
  if (chan < 4) {
    uint16_t cur = m_mcp.readGPIOAB();
    byte shift = chan << 2;
    cur &= ~(0xF << shift);
    cur |= (bits & 0xF) << shift;
    m_mcp.writeGPIOAB(cur);
  }
}

byte BlockControl::read(const byte chan)
{
  if (chan < 4) {
    uint16_t cur = m_mcp.readGPIOAB();
    byte shift = chan << 2;
    cur = (cur >> shift) & 0xF;
    return (byte) cur;
  } else {
    return NONE;
  }
}
