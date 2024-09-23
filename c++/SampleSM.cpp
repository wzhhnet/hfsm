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

///   ------
///  |  S0  |------|-------------|--------------|
///   ------       |             |              |
///                |             |              |
///             ------         ------         ------
///            |  S1  |--E1-->|  S2  |--E2-->|  S3  |
///             ------         ------         ------

class SampleSM : public StateMachine
{
  public:
    SampleSM() : s0_(), s1_(), s2_(), s3_()
    {
    }
    virtual ~SampleSM() {}
    void s0_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void s0_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool s0_invoke(const SpEvent &evt)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        return true;
    }

    void s1_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void s1_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool s1_invoke(const SpEvent &evt)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        if (evt->ID() == 1) {
            return true;
        }
        return false;
    }
    bool s1_trans_to_s2_triggered(const SpEvent &evt) {
        if (evt->ID() == 1) {
            LOGD("%s() trigger evt = %s", __FUNCTION__, evt->Name());
            return true;
        }
        return false;
    }
    bool s1_trans_to_s2_guard() {
        LOGD("%s() guard = %d", __FUNCTION__, s1_to_s2_guard_);
        bool ret = s1_to_s2_guard_;
        s1_to_s2_guard_ = !s1_to_s2_guard_;
        return ret;
    }
    void s1_trans_to_s2_effect()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void s2_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void s2_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool s2_invoke(const SpEvent &evt)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        return false;
    }
    bool s2_trans_to_s3_triggered(const SpEvent &evt) {
        if (evt->ID() == 2) {
            LOGD("%s() trigger evt = %s", __FUNCTION__, evt->Name());
            return true;
        }
        return false;
    }
    void s2_trans_to_s3_effect()
    {
        LOGD("%s()", __FUNCTION__);
    }

    void s3_enter()
    {
        LOGD("%s()", __FUNCTION__);
    }
    void s3_exit()
    {
        LOGD("%s()", __FUNCTION__);
    }
    bool s3_invoke(const SpEvent &evt)
    {
        LOGD("%s() evt = %s", __FUNCTION__, evt->Name());
        return false;
    }
    void Setup()
    {
        /// configure state0 as root state.
        StateImpl<SampleSM>::StateAction action_0 = {
            &SampleSM::s0_enter,
            &SampleSM::s0_exit,
            &SampleSM::s0_invoke
        };
        s0_.SetAction(&action_0);

        /// configure state1 as sub-state of state0.
        StateImpl<SampleSM>::StateAction action_1 = {
            &SampleSM::s1_enter,
            &SampleSM::s1_exit,
            &SampleSM::s1_invoke
        };
        s1_.SetParent(&s0_);
        s1_.SetAction(&action_1);

        /// configure state2 as sub-state of state0.
        StateImpl<SampleSM>::StateAction action_2 = {
            &SampleSM::s2_enter,
            &SampleSM::s2_exit,
            &SampleSM::s2_invoke
        };
        s2_.SetParent(&s0_);
        s2_.SetAction(&action_2);

        /// configure state3 as sub-state of state0.
        StateImpl<SampleSM>::StateAction action_3 = {
            &SampleSM::s3_enter,
            &SampleSM::s3_exit,
            &SampleSM::s3_invoke
        };
        s3_.SetParent(&s0_);
        s3_.SetAction(&action_3);

        /// configure transition form state1 to state2.
        /// trigger: event1
        /// guard: s1_to_s2_guard_
        TransitionImpl<SampleSM>::TransAction trans_1_to_2_act = {
            .guard = &SampleSM::s1_trans_to_s2_guard,
            .effect = &SampleSM::s1_trans_to_s2_effect,
            .triggered = &SampleSM::s1_trans_to_s2_triggered
        };
        std::shared_ptr<TransitionImpl<SampleSM>> trans_1_2(
            new TransitionImpl<SampleSM>(&s1_, &s2_, trans_1_to_2_act));
        s1_.AddTransition(trans_1_2);

        /// configure transition form state2 to state3.
        /// trigger: event2
        /// guard: no guard
        TransitionImpl<SampleSM>::TransAction trans_2_to_3_act = {
            .guard = nullptr,
            .effect = &SampleSM::s2_trans_to_s3_effect,
            .triggered = &SampleSM::s2_trans_to_s3_triggered
        };
        std::shared_ptr<TransitionImpl<SampleSM>> trans_2_3(
            new TransitionImpl<SampleSM>(&s2_, &s3_, trans_2_to_3_act));
        s2_.AddTransition(trans_2_3);

        /// set state1 as initial state
        Start(&s1_);
    }

  private:
    StateImpl<SampleSM> s0_;
    StateImpl<SampleSM> s1_;
    StateImpl<SampleSM> s2_;
    StateImpl<SampleSM> s3_;
    bool s1_to_s2_guard_ = false;
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

    SpEvent evt1(new SampleEvent(1, "event1", EvtPriority::kEvtPriMid));
    SpEvent evt2(new SampleEvent(2, "event2", EvtPriority::kEvtPriMid));
    SpEvent evt3(new SampleEvent(3, "event3", EvtPriority::kEvtPriMid));

    /// enter initial state(1)
    sm.Setup();

    /// triggered transiton from state(1) to state(2)
    /// but failed due to guard is false
    sm.SendEvent(evt1);

    /// set guard to true and retrigger to transit to state(2)
    evt1.reset(new SampleEvent(1, "event1", EvtPriority::kEvtPriMid));
    sm.SendEvent(evt1);

    /// triggered transiton from state(2) to state(3)
    sm.SendEvent(evt2);

    /// invoke event(3) on state(3)
    sm.SendEvent(evt3);

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


