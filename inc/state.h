/*
 * Hierarchical finite state machine
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

#ifndef _HFSM_STATE_H
#define _HFSM_STATE_H

#include "event_hub.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HFSM_MAX_STATES     (64)
typedef unsigned char state_id;
typedef struct action_t {
    void (*entry)(void* /*!< userdata */);
    void (*exit)(void* /*!< userdata */);
    state_id (*process)(const event_t* /*!< event */, void* /*!< userdata */);
} action_t;

typedef struct state_t {
    state_id id;
    struct state_t *parent;
    struct action_t action;
} state_t;


#ifdef __cplusplus
}
#endif

#endif /*! _HFSM_STATE_H */
