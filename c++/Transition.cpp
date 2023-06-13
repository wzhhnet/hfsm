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

#include "State.h"
#include "Transition.h"

namespace utils {
namespace hfsm {

Transition::Transition(State *source, State *target)
  : src_(source), tar_(target)
{
}

Transition::~Transition()
{
}

State* Transition::Transit(StateMachine *sm)
{
    if (!tar_)
        return nullptr;

    if (src_ == tar_) {
        Effect(sm);
        return tar_;
    }

    /*! fetch all parents of current state */
    std::list<State*> from_list;
    State *cur = src_;
    while (cur) {
        from_list.push_back(cur);
        cur = cur->Parent();
    }

    /*! fetch all parents of target state */
    std::list<State*> to_list;
    cur = const_cast<State*>(tar_);
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

};
};

