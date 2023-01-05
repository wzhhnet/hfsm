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

#ifndef _CPP_HIERARCHICAL_FINITE_STATE_MACHINE_H
#define _CPP_HIERARCHICAL_FINITE_STATE_MACHINE_H

#include <set>
#include <functional>
#include <EventHub.h>

namespace utils {
namespace hfsm {

class StateMachine;
class State
{
  public:
    State() {}
    State(State *parent) : parent_(parent) {}
    virtual ~State() {}
    void SetParent(State* parent) { parent_ = parent; }
    State* Parent() { return parent_; }

  protected:
    /// Transition is allowed if guard is true
    virtual bool Guard(StateMachine *sm) = 0;
    /// Entry action on current state
    virtual void Enter(StateMachine *sm) = 0;
    /// Exit action on current state
    virtual void Exit(StateMachine *sm) = 0;
    /// Event processing action on current state
    virtual bool Invoke(const SpEvent &evt, StateMachine *sm, State **to) = 0;

  private:
    State *parent_ = nullptr;
    friend StateMachine;
};

template <typename SM>
class StateImpl final : public State
{
  public:
    using FnGuard = bool(SM::*)();
    using FnCommon = void(SM::*)();
    using FnInvoke = bool(SM::*)(const SpEvent&, State**);
    struct StateAction {
        FnGuard   guard;
        FnCommon  enter;
        FnCommon  exit;
        FnInvoke  invoke;
    };

  public:
    /// Constructor
    StateImpl();
    StateImpl(State *parent);
    StateImpl(StateAction &action);
    StateImpl(State *parent, StateAction &action);
    /// Deconstructor
    virtual ~StateImpl();
    void SetAction(const StateAction *action);

  protected:
    virtual bool Guard(StateMachine *sm) override;
    virtual void Enter(StateMachine *sm) override;
    virtual void Exit(StateMachine *sm) override;
    virtual bool Invoke(const SpEvent &evt, StateMachine *sm, State **to) override;

  private:
    StateAction action_ = {};
};

template <typename SM>
StateImpl<SM>::StateImpl() {}

template <typename SM>
StateImpl<SM>::StateImpl(State *parent) : State(parent) {}

template <typename SM>
StateImpl<SM>::StateImpl(StateAction &action) : action_(action) {}

template <typename SM>
StateImpl<SM>::StateImpl(State *parent, StateAction &action)
  : State(parent), action_(action) {}

template <typename SM>
StateImpl<SM>::~StateImpl() {}

template <typename SM>
void StateImpl<SM>::SetAction(const StateAction *action)
{
    if (action)
        action_ = *action;
}

template <typename SM>
bool StateImpl<SM>::Guard(StateMachine *sm)
{
    if (action_.guard) {
        SM *obj = dynamic_cast<SM*>(sm);
        return (obj->*action_.guard)();
    }
    return true;
}

template <typename SM>
void StateImpl<SM>::Enter(StateMachine *sm)
{
    if (action_.enter) {
        SM *obj = dynamic_cast<SM*>(sm);
        (obj->*action_.enter)();
    }
}

template <typename SM>
void StateImpl<SM>::Exit(StateMachine *sm)
{
    if (action_.exit) {
        SM *obj = dynamic_cast<SM*>(sm);
        (obj->*action_.exit)();
    }
}

template <typename SM>
bool StateImpl<SM>::Invoke(const SpEvent &evt, StateMachine *sm, State **to)
{
    if (action_.invoke) {
        SM *obj = dynamic_cast<SM*>(sm);
        return (obj->*action_.invoke)(evt, to);
    }
    return false;
}

using SpState = std::shared_ptr<State>;

class StateMachine : public EventHandler
{
  public:
    using StateSet = std::set<State*>;

  public:
    StateMachine();
    virtual ~StateMachine();
    void Start(State *init_state);
    bool SendEvent(const SpEvent &evt);
    virtual const StateSet& GetStateSet() const = 0;

  protected:
    virtual void OnEvent(const SpEvent evt) override final;

  private:
    class StartEvent : public Event
    {
      public:
        StartEvent(State *state);
        virtual ~StartEvent();
        virtual uint32_t ID() const;
        virtual const char* Name() const;
        virtual EvtPriority Priority() const;
        State* StartState() const;
      public:
        static const uint32_t EVENT_ID = 0xF0F0F0F0;

      private:
        const char *NAME = "Start";
        State *state_ = nullptr;
    };

  private:
    bool Transit(const State *to);
    /// Disallow the copy constructor
    StateMachine(const StateMachine &) = delete;
    /// Disallow the assign constructor
    void operator=(const StateMachine &) = delete;

  private:
    const size_t  MAX_EVENT_NUM = 64;
    State *cur_state_ = nullptr;
    std::unique_ptr<EventHub> evt_hub_;
    StateSet state_set_;
    SpEvent start_evt_;
};

};
};

#endif //_CPP_HIERARCHICAL_FINITE_STATE_MACHINE_H

