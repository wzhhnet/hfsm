/*
 * Loger for HFSM
 *
 * Author wanch
 * Date 2021/12/27
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

#include <log.h>
#include <stdarg.h>
#include <stdio.h>

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

