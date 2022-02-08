#include "Runtime/Input/CInputGenerator.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/CArchitectureQueue.hpp"

namespace metaforce {

void CInputGenerator::Update(float dt, CArchitectureQueue& queue) {
  if (m_firstFrame) {
    m_firstFrame = false;
    return;
  }

  const CFinalInput& kbInput = getFinalInput(0, dt);
  queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, kbInput));

  /* Dolphin controllers next */
//  for (int i = 0; i < 4; ++i) {
//    bool connected;
//    EStatusChange change = m_dolphinCb.getStatusChange(i, connected);
//    if (change != EStatusChange::NoChange)
//      queue.Push(MakeMsg::CreateControllerStatus(EArchMsgTarget::Game, i, connected));
//    if (connected) {
//      CFinalInput input = m_dolphinCb.getFinalInput(i, dt, m_leftDiv, m_rightDiv);
//      if (i == 0) /* Merge KB input with first controller */
//      {
//        input |= kbInput;
//        kbUsed = true;
//      }
//      m_lastUpdate = input;
//      queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, input));
//    }
//  }

//  /* Send straight keyboard input if no first controller present */
//  if (!kbUsed) {
//    m_lastUpdate = kbInput;
//  }
}

} // namespace metaforce
