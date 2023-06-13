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
#ifndef _CPP_STATE_H
#define _CPP_STATE_H

#include "Transition.h"

namespace utils {
namespace hfsm {

//class Transition;
class StateMachine;
/// Abstract basic class for state
class State
{
  friend Transition;
  friend StateMachine;

  public:
    /// Constructor
    State() {}
    State(State *parent) : parent_(parent) {}
    /// Deconstructor
    virtual ~State() {}
    void SetParent(State* parent) { parent_ = parent; }
    State* Parent() { return parent_; }
    void AddTransition(SpTrans trans) { trans_.push_back(trans); }

  protected:
    /**
     * @brief Invoked while entering this state.
     *        If a state transit to itself, Entry will not be called.
     *
     * @param[in] Base class of state machine.
     * @return None.
     */
    virtual void Entry(StateMachine *sm) = 0;
    /**
     * @brief Invoked while exiting this state.
     *        If a state transit to itself, Exit will not be called.
     *
     * @param[in] Base class of state machine.
     * @return None.
     */
    virtual void Exit(StateMachine *sm) = 0;
    /**
     * @brief Invoked while receiving event on this state.
     *
     * @param[in] evt Invoked event on this state.
     * @param[in] sm  Base class of state machine.
     *
     * @return false if user expects event to continue to be invoked by the parent state,
     *         otherwise return true.
     *
     * NOTICE: if invoked event is done(true is returned) or triggered a successful transition on the state,
     *         it will not be continue to invoked by the parent state.
     */
    virtual bool Invoke(const SpEvent &evt, StateMachine *sm) = 0;

  private:
    /**
     * @brief Get all transitions which source as this state.
     *       called by state-machine
     *
     * @return reference of transition container.
     */
    const TransSet& GetTransitions() const { return trans_; }

  private:
    State *parent_ = nullptr;
    TransSet trans_;
};

template <typename SM>
class StateImpl final : public State
{
  public:
    using FnCommon = void(SM::*)();
    using FnInvoke = bool(SM::*)(const SpEvent&);
    struct StateAction {
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
    virtual void Entry(StateMachine *sm) override;
    virtual void Exit(StateMachine *sm) override;
    virtual bool Invoke(const SpEvent &evt, StateMachine *sm) override;

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
void StateImpl<SM>::Entry(StateMachine *sm)
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
bool StateImpl<SM>::Invoke(const SpEvent &evt, StateMachine *sm)
{
    if (action_.invoke) {
        SM *obj = dynamic_cast<SM*>(sm);
        return (obj->*action_.invoke)(evt);
    }
    return false;
}

using SpState = std::shared_ptr<State>;

};
};

#endif // _CPP_STATE_H

