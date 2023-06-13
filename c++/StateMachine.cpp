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

#include <list>

#include "StateMachine.h"
#include "log.h"

namespace utils {
namespace hfsm {

/// System event used to start HFSM.
class StartEvent : public Event
{
  public:
    StartEvent(State *state) : state_(state) {}
    virtual ~StartEvent() {}
    virtual uint32_t ID() const { return EVENT_ID; }
    virtual const char* Name() const { return NAME; }
    virtual EvtPriority Priority() const { return EvtPriority::kEvtPriHigh; }
    State* StartState() const { return state_; }

  public:
    static const uint32_t EVENT_ID = 0xF0F0F0F0;

  private:
    const char *NAME = "Start";
    State *state_ = nullptr;
};

/// System transition used to transit to initial state.
class InitTransition : public Transition
{
  public:
    InitTransition(State *source, State *target)
        : Transition(source, target) {}
    virtual ~InitTransition() {}

  private:
    virtual bool Guard(StateMachine *sm) override { return true; }
    virtual void Effect(StateMachine *sm) override {}
    virtual bool EventTriggered(const SpEvent &evt, StateMachine *sm) override {
        return true;
    }
};

/// Constructor
StateMachine::StateMachine()
  : cur_state_(nullptr), evt_hub_(), state_set_(), start_evt_()
{
}

/// Deconstructor
StateMachine::~StateMachine()
{
}

void StateMachine::Start(State *init_state)
{
    evt_hub_.reset(new EventHub(this, MAX_EVENT_NUM));
    if (evt_hub_ != nullptr) {
        start_evt_.reset(new StartEvent(init_state));
        evt_hub_->Send(start_evt_);
    }
}

void StateMachine::OnEvent(const SpEvent evt)
{
    if (evt == nullptr) return;
    if (evt->ID() == StartEvent::EVENT_ID) {
        /// Transit to initial state
        const StartEvent *e =
            dynamic_cast<const StartEvent*>(evt.get());
        InitTransition t(nullptr, e->StartState());
        cur_state_ = t.Transit(this);
    } else {
        State *cur = cur_state_;
        /// Invoke event on current state and parents.
        while (cur) {
            bool done = cur->Invoke(evt, this);
            /// Check if transition is happened on currrent state.
            /// An event can trigger only one transition.
            const TransSet transet = cur->GetTransitions();
            for (const auto &trans : transet) {
                /// Check trigger and guard
                if (trans->EventTriggered(evt, this)
                    && trans->Guard(this)) {
                    State *tar = trans->Transit(this);
                    /// Transition happened
                    if (tar != nullptr) {
                        done = true;
                        cur_state_ = tar;
                    }
                    break;
                }
            }
            /// break if event is done or transition happened.
            if (done)
                break;
            cur = cur->Parent(); // continue if event is not done.
        }
    }
}

bool StateMachine::SendEvent(const SpEvent &evt)
{
    if (evt_hub_ == nullptr)
        return false;

    return evt_hub_->Send(evt);
}

};
};

