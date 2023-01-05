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

StateMachine::StateMachine()
  : cur_state_(nullptr), evt_hub_(), state_set_(), start_evt_()
{
}

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
    LOGD("%s() evt = %d", __FUNCTION__, evt->ID());
    if (evt == nullptr) return;
    if (evt->ID() == StartEvent::EVENT_ID) {
        LOGD("%s()", "OnEvent state start");
        const StartEvent *e =
            dynamic_cast<const StartEvent*>(evt.get());
        cur_state_ = e->StartState();
        if (cur_state_) {
            cur_state_->Enter(this);
        }
    } else {
        State *cur = cur_state_;
        if (!cur) {
            LOGE("%s() not start", __FUNCTION__);
        }
        while (cur) {
            State *tar = cur; // transition target
            LOGD("%s()", "OnEvent state invoke");
            bool done = cur->Invoke(evt, this, &tar);
            if (done) {
                if (tar && tar != cur) {
                    /// transite to target state
                    if (tar->Guard(this) && Transit(tar)) {
                        cur_state_ = tar;
                        LOGD("%s()", "OnEvent state transition");
                    }
                }
                break;
            }
            cur = cur->Parent();
        }
    }
}

bool StateMachine::SendEvent(const SpEvent &evt)
{
    if (evt_hub_ == nullptr) {
        LOGD("%s()", __FUNCTION__);
        return false;
    }

    return evt_hub_->Send(evt);
}

bool StateMachine::Transit(const State *to)
{
    if (!cur_state_ || !to) {
        return false;
    }

    const StateSet &states = GetStateSet();
    const auto it = states.find(const_cast<State*>(to));
    if (it == states.end()) {
        return false;
    }

    /*! fetch all parents of current state */
    std::list<State*> from_list;
    State *cur = cur_state_;
    while (cur) {
        from_list.push_back(cur);
        cur = cur->Parent();
    }

    /*! fetch all parents of target state */
    std::list<State*> to_list;
    cur = const_cast<State*>(to);
    while (cur) {
        to_list.push_back(cur);
        cur = cur->Parent();
    }

    /*! remove the common tail */
    while (from_list.size() && to_list.size()
        && from_list.back() == to_list.back()) {
        from_list.pop_back();
        to_list.pop_back();
    }

    /*! invoke exit action */
    for (auto it = from_list.begin();
            it != from_list.end(); ++it) {
        (*it)->Exit(this);
    }

    /*! invoke entry action */
    for (auto it = to_list.rbegin();
            it != to_list.rend(); ++it) {
        (*it)->Enter(this);
    }

    return true;
}

StateMachine::StartEvent::StartEvent(State *state)
  : state_(state)
{
}

StateMachine::StartEvent::~StartEvent()
{
}

uint32_t StateMachine::StartEvent::ID() const
{
    return EVENT_ID;
}

const char* StateMachine::StartEvent::Name() const
{
    return NAME;
}

EvtPriority StateMachine::StartEvent::Priority() const
{
    return EvtPriority::kEvtPriHigh;
}

State* StateMachine::StartEvent::StartState() const
{
    return state_;
}

};
};

