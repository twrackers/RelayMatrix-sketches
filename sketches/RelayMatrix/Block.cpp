#include "Block.h"

void Block::x_write4()
{
  m_ctrl.write(m_chan, (m_ena ? 0x0 : 0x8) | m_sel);
}

Block::Block(BlockControl& ctrl, const byte chan) :
StateMachine(10, false),
m_ctrl(ctrl), m_chan(chan),
m_sw(0), m_sel(0), m_ena(false),
m_state(eDisabled)
{
}

bool Block::update()
{
  if (StateMachine::update()) {
    if (m_state == eDisabled) {
      if (m_ena) {
        m_sel = m_sw;
        x_write4();
        m_state = eEnabled;
      }
    } else if (m_state == eEnabled) {
      if (!m_ena) {
        x_write4();
        m_state = eDisabled;
      } else {
        if (m_sw != m_sel) {
          m_ena = false;
          x_write4();
          m_state = eSwitch;
        }
      }
    } else /* (m_state == eSwitch) */ {
      m_ena = true;
      m_sel = m_sw;
      x_write4();
      m_state = eEnabled;
    }
    return true;
  }
  return false;
}

void Block::enable(const bool ena)
{
  m_ena = ena;
}

void Block::select(const byte sel)
{
  m_sw = sel;
}
