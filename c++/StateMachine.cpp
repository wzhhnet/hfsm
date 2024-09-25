/*
 * Hierarchical finite state machine
 * Implemented by C++
 *
 * Author wanch
 * Date 2022/12/20
 * Email wzhhnet@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm> // for find_if

#include "StateMachine.h"
#include "log.h"

namespace utils {
namespace hfsm {

/// System event used to start HFSM.
class StartEvent : public Event
{
  public:
    StartEvent() {}
    virtual ~StartEvent() {}
    virtual uint32_t ID() const {
        return InitTransition::INIT_EVT_ID;
    }
    virtual const char* Name() const {
        return NAME;
    }
    virtual EvtPriority Priority() const {
        return EvtPriority::kEvtPriHigh;
    }

  private:
    const char *NAME = "StartEvent";
};

/// Constructor
StateMachine::StateMachine()
{
}

/// Deconstructor
StateMachine::~StateMachine()
{
}

void StateMachine::Start()
{
    if (running_) {
        LOGE("%s SM is running", __func__);
        return;
    }
    evt_hub_.reset(new EventHub(this, MAX_EVENT_NUM));
    evt_hub_->Send(SpEvent(new StartEvent()));
}

bool StateMachine::AddTransition(const SpTrans &trans)
{
    if (running_) {
        LOGE("%s SM is running", __func__);
        return false;
    }
    auto it = std::find_if(trans_list_.begin(), trans_list_.end(),
        [trans](const SpTrans &sp) -> bool {
            return (*sp == *trans);
        });
    if (it == trans_list_.end()) {
        trans_list_.emplace_back(trans);
        return true;
    }
    LOGE("%s duplicated transition", __func__);
    return false;
}

void StateMachine::OnEvent(const SpEvent evt)
{
    if (evt == nullptr) return;
    if (TransTriggered(evt)) {
        /// Transition occerred
        if (!cur_state_) {
            trans_list_.clear();
            running_ = false;
        } else {
            running_ = true;
        }
    } else {
        /// Invoke event on current state and parents.
        auto cur = cur_state_;
        /// continue if event invoked not done.
        while (cur && !cur->Invoke(evt, this)) {
            cur = cur->Parent();
        }
    }
}

bool StateMachine::TransTriggered(const SpEvent &evt)
{
    /// Check if transition is happened on currrent state.
    /// An event can trigger only one transition.
    for (const auto &trans : trans_list_) {
        /// Check transition(source, trigger and guard)
        if (cur_state_ == trans->Source()
            && trans->EventTriggered(evt, this)
            && trans->Guard(this)) {
            /// Transition happened
            cur_state_ = trans->Transit(this);
            return true;
        }
    }
    return false;
}

bool StateMachine::SendEvent(const SpEvent &evt)
{
    if (evt_hub_ == nullptr)
        return false;

    return evt_hub_->Send(evt);
}

}
}

