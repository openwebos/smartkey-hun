/* @@@LICENSE
*
*      Copyright (c) 2011-2013 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2013 LG Electronics
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#include "PerfTimer.h"
#include <glib.h>


/**
* PerfTimer
*/
PerfTimer::PerfTimer()
{
}

/**
* ~PerfTimer
*/
void PerfTimer::start()
{
    _start = gettime();
}

/**
* stop
*/
void PerfTimer::stop()
{
    _stop = gettime();
}

/**
* print
*/
void PerfTimer::print(const char* msg)
{
    if (msg)
        g_debug("%s (Took %g msec)", msg, (_stop - _start) * 1000.0);
    else
        g_debug("Interval took %g msec", (_stop - _start) * 1000.0);
}

/**
* gettime
*
* @return double
*   time
*/
double PerfTimer::gettime()
{
    struct timespec curTime;
    clock_gettime(CLOCK_REALTIME, &curTime);

    return static_cast<double>(curTime.tv_sec) +
           static_cast<double>(curTime.tv_nsec) / 1000000000.0f;
}

