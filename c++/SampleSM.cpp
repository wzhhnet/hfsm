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

#include <unistd.h>
#include <stdarg.h>

#include "log.h"
#include "StateMachine.h"

using namespace utils;
using namespace hfsm;

class SampleSM : public StateMachine
{
  public:
    SampleSM() : state_0_(), state_1_(), state_2_()
    {
    }
    virtual ~SampleSM() {}
    void state_0_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void state_0_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool state_0_invoke(const SpEvent &evt, State **ppnext)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        return true;
    }
    void state_1_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void state_1_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool state_1_invoke(const SpEvent &evt, State **ppnext)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        if (evt->ID() == 1) {
            return true;
        } else if (evt->ID() == 3) {
            *ppnext = &state_2_;    // transit to state 2
            return true;
        }
        return false;
    }
    void state_2_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void state_2_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool state_2_invoke(const SpEvent &evt, State **ppnext)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        if (evt->ID() == 3) {
            return true;
        }
        return false;
    }

    void Setup()
    {
        StateImpl<SampleSM>::StateAction action_0 = {
            nullptr,
            &SampleSM::state_0_enter,
            &SampleSM::state_0_exit,
            &SampleSM::state_0_invoke
        };
        state_0_.SetAction(&action_0);

        StateImpl<SampleSM>::StateAction action_1 = {
            nullptr,
            &SampleSM::state_1_enter,
            &SampleSM::state_1_exit,
            &SampleSM::state_1_invoke
        };
        state_1_.SetParent(&state_0_);
        state_1_.SetAction(&action_1);

        StateImpl<SampleSM>::StateAction action_2 = {
            nullptr,
            &SampleSM::state_2_enter,
            &SampleSM::state_2_exit,
            &SampleSM::state_2_invoke
        };
        state_2_.SetParent(&state_0_);
        state_2_.SetAction(&action_2);

        Start(&state_1_);
    }

    virtual const StateSet& GetStateSet() const
    {
        return STATES;
    }

  private:
    const StateSet STATES = {&state_0_, &state_1_, &state_2_};
    StateImpl<SampleSM> state_0_;
    StateImpl<SampleSM> state_1_;
    StateImpl<SampleSM> state_2_;
};


class SampleEvent : public Event
{
  public:
    SampleEvent(uint32_t id, const char *name, EvtPriority pri)
      : id_(id), name_(name), pri_(pri) {}
    virtual ~SampleEvent() {}

    virtual uint32_t ID() const{ return id_;}
    virtual const char* Name() const { return name_.c_str(); }
    virtual EvtPriority Priority() const { return pri_; }

  private:
    uint32_t id_;
    std::string name_;
    EvtPriority pri_;
};

int main(int argc, char **argv)
{
    SampleSM sm;

    SpEvent evt1(new SampleEvent(1, "event1", EvtPriority::kEvtPriHigh));
    SpEvent evt2(new SampleEvent(2, "event2", EvtPriority::kEvtPriMid));
    SpEvent evt3(new SampleEvent(3, "event3", EvtPriority::kEvtPriMid));
    SpEvent evt4(new SampleEvent(4, "event4", EvtPriority::kEvtPriLow));

    sm.Setup();

    sm.SendEvent(evt1);
    sm.SendEvent(evt2);
    sm.SendEvent(evt3);
    sm.SendEvent(evt4);

    while(1)
    usleep(1000);

    printf("main thread end\n");
    return 0;
}

void hfsm_trace(const char *format, ...) {
    int n;
    char buf[4096];
    
    va_list ap;
    va_start(ap, format);

    n = vsnprintf(buf, sizeof(buf)-1, format, ap);
    if (n > 0 && n < (int)(sizeof(buf)-1)) {
        fprintf(stderr, "%s\n" RST, buf);
    }
    va_end (ap);
}


