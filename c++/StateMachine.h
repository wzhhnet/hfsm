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


#include "State.h"
#include "Transition.h"

namespace utils {
namespace hfsm {

/// The sequence of State machine run-time:
///
/// State: S0, S1, S2 (S1 and S2 are both sub-states of S0)
/// Event: E1
/// Transition: T1
///
/// Scenario 1: current state transit from S1 to S2 via T1 triggered by E1.
/// 1, S1->Invoke
/// 3, T1->EventTriggered
/// 4, T1->Guard
/// Below is continue if T1->Guard return true or no guard.
/// 5, S1->Exit
/// 6, T1->Effect
/// 7, S2->Entry
///
/// Scenario 2: current state transit from S1 to S1 (self-transition)
/// 1, S1->Invoke
/// 2, T1->EventTriggered
/// 3, T1->Guard
/// Below is continue if T1->Guard return true or no guard.
/// 4, T1->Effect

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
    const size_t  MAX_EVENT_NUM = 64;
    State *cur_state_ = nullptr;
    std::unique_ptr<EventHub> evt_hub_;
    StateSet state_set_;
    SpEvent start_evt_;

  private:
    /// Disallow the copy constructor
    StateMachine(const StateMachine &) = delete;
    /// Disallow the assign constructor
    void operator=(const StateMachine &) = delete;
};

};
};

#endif //_CPP_HIERARCHICAL_FINITE_STATE_MACHINE_H

