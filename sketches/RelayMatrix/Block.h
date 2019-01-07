#ifndef _BLOCK__H_
#define _BLOCK__H_

#include <Arduino.h>
#include <StateMachine.h>

#include "BlockControl.h"

class Block : public StateMachine
{
  private:
    BlockControl& m_ctrl;
    const byte m_chan;
    byte m_sel;
    byte m_sw;
    bool m_ena;
    enum {
      eDisabled, eEnabled, eSwitch
    } m_state;

    void x_write4();

  public:
    Block(BlockControl& ctrl, const byte chan);

    virtual bool update();
    void enable(const bool ena);
    void select(const byte sel);
};

#endif
