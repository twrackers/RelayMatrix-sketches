#ifndef _BLOCK__H_
#define _BLOCK__H_

#include <Arduino.h>
#include <StateMachine.h>

#include "BlockControl.h"

class Block : public StateMachine
{
  private:
    BlockControl& m_ctrl; // low-level interface
    const byte m_chan;    // channel (0 to 3)
    byte m_sel;           // current selected output
    byte m_sw;            // new selected output
    bool m_ena;           // enabled state
    enum {
      eDisabled,          // block is disabled
      eEnabled,           // block is enabled
      eSwitch             // block is switching
    } m_state;

    // Private 4-bit write method
    void x_write4();

  public:
    // Constructor
    // ctrl: low-level interface to MCP23017
    // chan: 4-bit channel on MCP23017, 0 to 3
    Block(BlockControl& ctrl, const byte chan);

    // Update, overrides StateMachine::update
    // returns true if update occurred
    virtual bool update();
    
    // Set enabled state
    // ena: new enabled state
    //   true enables block
    //   false disables block
    void enable(const bool ena);
    
    // Select relay to activate
    // sel = 0 to 7
    // If sel out of range, no action occurs
    void select(const byte sel);
};

#endif
