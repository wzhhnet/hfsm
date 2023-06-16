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
#ifndef _HFSM_CPP_TRANSITION_H
#define _HFSM_CPP_TRANSITION_H

namespace utils {
namespace hfsm {

class State;
class StateMachine;
class Transition
{
  friend StateMachine;

  public:
    Transition(State *source, State *target);
    virtual ~Transition();

  protected:
    virtual bool Guard(StateMachine *sm) = 0;
    virtual void Effect(StateMachine *sm) = 0;
    virtual bool EventTriggered(const SpEvent &evt, StateMachine *sm) = 0;

  private:
    State* Transit(StateMachine *sm);

  private:
    State *src_;
    State *tar_;
};

using SpTrans = std::shared_ptr<Transition>;
using TransSet = std::vector<SpTrans>;

template <typename SM>
class TransitionImpl final : public Transition
{
  public:
    using FnGuard = bool(SM::*)();
    using FnEffect = void(SM::*)();
    using FnTriggered = bool(SM::*)(const SpEvent&);
    struct TransAction {
        FnGuard guard;
        FnEffect effect;
        FnTriggered triggered;
    };

  public:
    TransitionImpl(State *source, State *target, TransAction &action);
    virtual ~TransitionImpl();

  protected:
    virtual bool Guard(StateMachine *sm) override;
    virtual void Effect(StateMachine *sm) override;
    virtual bool EventTriggered(const SpEvent &evt, StateMachine *sm) override;

  private:
    TransAction action_ = {};
};

template <typename SM>
TransitionImpl<SM>::TransitionImpl(State *source, State *target, TransAction &action)
  : Transition(source, target), action_(action)
{
}

template <typename SM>
TransitionImpl<SM>::~TransitionImpl() {}

template <typename SM>
bool TransitionImpl<SM>::Guard(StateMachine *sm)
{
    if (action_.guard) {
        SM *obj = dynamic_cast<SM*>(sm);
        return (obj->*action_.guard)();
    }
    return true;
}

template <typename SM>
void TransitionImpl<SM>::Effect(StateMachine *sm)
{
    if (action_.effect) {
        SM *obj = dynamic_cast<SM*>(sm);
        (obj->*action_.effect)();
    }
}

template <typename SM>
bool TransitionImpl<SM>::EventTriggered(const SpEvent &evt, StateMachine *sm)
{
    if (action_.triggered) {
        SM *obj = dynamic_cast<SM*>(sm);
        return (obj->*action_.triggered)(evt);
    }
    return false;
}


};
};

#endif // _HFSM_CPP_TRANSITION_H

