/*
 * Unit test for HFSM
 *
 * Author wanch
 * Date 2021/12/23
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

#include <gtest/gtest.h>

#include <unistd.h>
#include <gtest/gtest.h>
#include <hfsm.c>
#include <log.c>

hfsm_handle ghfsm = NULL;

enum {
    TEST_STATE_1 = 1,
    TEST_STATE_2,
    TEST_STATE_3
};

enum {
    TEST_EVENT_AT_STATE2 = EVENT_ID_USER_BASE+1,
    TEST_EVENT_TRANS_TO_STATE3,
    TEST_EVENT_AT_STATE3
};
class GlobalTest : public testing::Environment
{
  public:
    virtual void SetUp()
    {
        hfsm_param param = {
            .max_states = 4,
            .userdata = NULL
        };

        hfsm_create(&ghfsm, &param);
    }

    virtual void TearDown()
    {
        hfsm_destroy(&ghfsm);
    }

};

GTEST_API_ int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::Environment *env = new GlobalTest();
    testing::AddGlobalTestEnvironment(env); 
    return RUN_ALL_TESTS();
}

TEST(hfsm, hfsm_create)
{
    ASSERT_NE(ghfsm, nullptr);
}

TEST(hfsm, hfsm_new_state)
{
    state_t *s1, *s2, *s3, *s4, *s5;
    s1 = hfsm_new_state(ghfsm);
    EXPECT_NE(s1, nullptr);
    s2 = hfsm_new_state(ghfsm);
    EXPECT_NE(s2, nullptr);
    s3 = hfsm_new_state(ghfsm);
    EXPECT_NE(s3, nullptr);
    s4 = hfsm_new_state(ghfsm);
    EXPECT_NE(s4, nullptr);
    s5 = hfsm_new_state(ghfsm);
    EXPECT_EQ(s5, nullptr);

    int s;
    struct state_info_t *info;
    struct hfsm_t *handle = (struct hfsm_t*)ghfsm;

    info = container_of(s1, struct state_info_t, state);
    s = ALLOCATOR_FREE(state, &handle->pool, info);
    EXPECT_EQ(s, UTILS_SUCC);

    info = container_of(s2, struct state_info_t, state);
    s = ALLOCATOR_FREE(state, &handle->pool, info);
    EXPECT_EQ(s, UTILS_SUCC);

    info = container_of(s3, struct state_info_t, state);
    s = ALLOCATOR_FREE(state, &handle->pool, info);
    EXPECT_EQ(s, UTILS_SUCC);

    info = container_of(s4, struct state_info_t, state);
    s = ALLOCATOR_FREE(state, &handle->pool, info);
    EXPECT_EQ(s, UTILS_SUCC);
}

void s1_entry(void *userdata)
{
    LOGD("%s()", __FUNCTION__);
}
void s1_exit(void *userdata)
{
    LOGD("%s()", __FUNCTION__);
}
bool s1_process(const event_t *event, void *userdata, state_id *pstate)
{
    LOGD("%s() id = %u state = %u", __FUNCTION__, event->id, *pstate);
    return true;
}
void s2_entry(void *userdata)
{
    LOGD("%s()", __FUNCTION__);
}
void s2_exit(void *userdata)
{
    LOGD("%s()", __FUNCTION__);
}
bool s2_process(const event_t *event, void *userdata, state_id *pstate)
{
    LOGD("%s() id = %u state = %u", __FUNCTION__, event->id, *pstate);
    event_t *evt = (event_t*)event;
    if (evt->id == TEST_EVENT_TRANS_TO_STATE3) {
        LOGD("%s() try to transit to state[%lu]", __FUNCTION__, TEST_STATE_3);
        *pstate = TEST_STATE_3;
        return true;
    }

    return false;
}
void s3_entry(void *userdata)
{
    LOGD("%s()", __FUNCTION__);
}
void s3_exit(void *userdata)
{
    LOGD("%s()", __FUNCTION__);
}
bool s3_process(const event_t *event, void *userdata, state_id *pstate)
{
    LOGD("%s() id = %u state = %u", __FUNCTION__, event->id, *pstate);
    event_t *evt = (event_t*)event;
    if (evt->param) {
        return false;   //need parent processing
    } else {
        return true;
    }
}

TEST(hfsm, hfsm_add_state)
{
    int s;
    state_t *s1, *s2, *s3;
    s1 = hfsm_new_state(ghfsm);
    ASSERT_NE(s1, nullptr);
    s2 = hfsm_new_state(ghfsm);
    ASSERT_NE(s2, nullptr);
    s3 = hfsm_new_state(ghfsm);
    ASSERT_NE(s3, nullptr);

    s1->id = TEST_STATE_1;
    s1->parent = NULL;
    s1->action.entry = s1_entry;
    s1->action.exit = s1_exit;
    s1->action.process = s1_process;
    s = hfsm_add_state(ghfsm, s1);
    EXPECT_EQ(s, HFSM_SUCC);

    s2->id = TEST_STATE_2;
    s2->parent = s1;
    s2->action.entry = s2_entry;
    s2->action.exit = s2_exit;
    s2->action.process = s2_process;
    s = hfsm_add_state(ghfsm, s2);
    EXPECT_EQ(s, HFSM_SUCC);

    s3->id = TEST_STATE_3;
    s3->parent = s1;
    s3->action.entry = s3_entry;
    s3->action.exit = s3_exit;
    s3->action.process = s3_process;
    s = hfsm_add_state(ghfsm, s3);
    EXPECT_EQ(s, HFSM_SUCC);
}

TEST(hfsm, hfsm_start)
{
    int s = hfsm_start(ghfsm, TEST_STATE_2);
    EXPECT_EQ(s, HFSM_SUCC);
}

TEST(hfsm, hfsm_send_event)
{
    event_t evt = {
        .id = TEST_EVENT_AT_STATE2,
        .priority = 1,
        .param = NULL
    };

    int s = hfsm_send_event(ghfsm, &evt);
    EXPECT_EQ(s, HFSM_SUCC);

    evt.id = TEST_EVENT_TRANS_TO_STATE3;
    s = hfsm_send_event(ghfsm, &evt);
    EXPECT_EQ(s, HFSM_SUCC);

    evt.id = TEST_EVENT_AT_STATE3;
    s = hfsm_send_event(ghfsm, &evt);
    EXPECT_EQ(s, HFSM_SUCC);

    evt.param = (void*)1;
    s = hfsm_send_event(ghfsm, &evt);
    EXPECT_EQ(s, HFSM_SUCC);
}

TEST(hfsm, hfsm_destroy)
{
    usleep(1000); // wait for other cases compeleted
    int s = hfsm_destroy(&ghfsm);
    EXPECT_EQ(s, HFSM_SUCC);
    EXPECT_EQ(ghfsm, nullptr);
}


