#include "Block.h"

// Private method to write 4-bit group
void Block::x_write4()
{
  m_ctrl.write(m_chan, (m_ena ? 0x0 : 0x8) | m_sel);
}

// Constructor
Block::Block(BlockControl& ctrl, const byte chan) :
StateMachine(10, false),
m_ctrl(ctrl), m_chan(chan),
m_sw(0), m_sel(0), m_ena(false),
m_state(eDisabled)
{
}

// Update method, overrides StateMachine::update
bool Block::update()
{
  // Time to update?
  if (StateMachine::update()) {
    if (m_state == eDisabled) {
      // Now disabled, is enable requested?
      if (m_ena) {
        // If so, update selection in case it was changed
        // while disabled, then write to GPIO port.
        m_sel = m_sw;
        x_write4();
        // Update state to enabled.
        m_state = eEnabled;
      }
    } else if (m_state == eEnabled) {
      // Now enabled, is disable requested?
      if (!m_ena) {
        // If so, update GPIO port.
        x_write4();
        // Update state to disabled.
        m_state = eDisabled;
      } else {
        // if still enabled, has the requested selectin changed?
        if (m_sw != m_sel) {
          // If so, disable the port...
          m_ena = false;
          x_write4();
          // ... then change the state to switching,
          m_state = eSwitch;
          // On the next call to update(), the switching step
          // will complete.
        }
      }
    } else /* (m_state == eSwitch) */ {
      // Switching state was entered on the previous call to
      // update, so it is completed here.
      // Re-enable the port and update to the new selection
      // before writing to the GPIO port.
      m_ena = true;
      m_sel = m_sw;
      x_write4();
      // Update status to enabled, switching is complete.
      m_state = eEnabled;
    }
    // This object has been updated.
    return true;
  }
  // Not time to update yet.
  return false;
}

void Block::enable(const bool ena)
{
  // Change enabled status on next update.
  m_ena = ena;
}

void Block::select(const byte sel)
{
  // Change requested selection on next update.
  // The change takes 2 update passes to complete.
  // When switching, the port is disabled for 10 msec
  // to provide break-before-make on the relays' contacts.
  m_sw = sel;
}
