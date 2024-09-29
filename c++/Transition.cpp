/*
 * Hierarchical finite state machine
 * Implemented by C++
 *
 * Author wanch
 * Date 2023/6/8
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
#include <EventHub.h>

#include "log.h"
#include "State.h"
#include "Transition.h"

namespace utils {
namespace hfsm {

/// System event used to first transiton
class InitEvent : public Event
{
  public:
    static const uint32_t INIT_EVT_ID = 0xF0F0F0F0;

  public:
    InitEvent() {}
    virtual ~InitEvent() {}
    virtual uint32_t ID() const {
        return INIT_EVT_ID;
    }
    virtual const char* Name() const {
        return "InitEvent";
    }
    virtual EvtPriority Priority() const {
        return EvtPriority::kEvtPriHigh;
    }
};

/// Initial transition used to transit to initial state.
class InitTransition : public Transition
{
  public:
    InitTransition(const SpState &target)
      : Transition(nullptr, target) {}
    virtual ~InitTransition() {}

  private:
    virtual void Effect(StateMachine *sm) override final {}
    virtual bool Triggered(const SpEvent &evt, StateMachine *sm) override final {
        return (evt->ID() == InitEvent::INIT_EVT_ID);
    }
};

Transition::Transition(const SpState &source, const SpState &target)
  : src_(source), tar_(target)
{
}

Transition::~Transition()
{
}

bool Transition::operator==(const Transition& other) const
{
    return (src_ == other.src_)
        && (tar_ == other.tar_);
}

SpState Transition::Source() const
{
    return src_;
}

SpState Transition::Target() const
{
    return tar_;
}

SpTrans Transition::CreateInitialTransition(const SpState &target)
{
    return SpTrans(new InitTransition(target));
}

SpEvent Transition::CreateInitialEvent()
{
    return SpEvent(new InitEvent());
}

SpState Transition::Transit(StateMachine *sm)
{
    /*! State self-transition */
    if (src_ == tar_) {
        Effect(sm);
        return tar_;
    }

    /*! fetch all parents of current state */
    std::list<SpState> from_list;
    auto cur = src_;
    while (cur) {
        from_list.push_back(cur);
        cur = cur->Parent();
    }

    /*! fetch all parents of target state */
    std::list<SpState> to_list;
    cur = tar_;
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
        (*it)->Exit(sm);
    }

    /*! invoke effect action */
    Effect(sm);

    /*! invoke entry action */
    for (auto it = to_list.rbegin();
            it != to_list.rend(); ++it) {
        (*it)->Entry(sm);
    }

    return tar_;
}

}
}

