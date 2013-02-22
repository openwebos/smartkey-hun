/* @@@LICENSE
*
*      Copyright (c) 2010-2013 Hewlett-Packard Development Company, L.P.
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
/*
 * StringUtils.h
 *
 *  Created on: Jan 18, 2010
 *      Author: Anthony D'Auria
 */

#ifndef StringUtils_h
#define StringUtils_h

#include <string>
#include <unicode/translit.h>
#include <sys/times.h>
#include <unistd.h>
#include "glib.h"

namespace SmartKey
{

class StringUtils
{
public :

    static void chomp (char* str, size_t numChars);
    static UnicodeString utf8StringToUnicodeString (const std::string& str);
    static std::string utf8tolower (const std::string& str);
    static bool transliterate (UnicodeString& str);
    static bool compareStrings (const std::string& first, const std::string& second);
};

// mini wrapper around a g_lib returned array that needs to be freed using g_free()
template <class T> class auto_g_free_array
{
public:
    auto_g_free_array(T * init = NULL) : p(init) {}
    ~auto_g_free_array()
    {
        if (p)
            g_free(p);
    }
    auto_g_free_array<T> & operator=(T * new_p)
    {
        if (p)
            g_free(p);
        p = new_p;
        return *this;
    }

    operator T *()
    {
        return p;
    }
    template <class otherT> const otherT * as() const
    {
        return reinterpret_cast<const otherT *>(p);
    }
    template <class otherT> otherT * as()
    {
        return reinterpret_cast<otherT *>(p);
    }

    T * p;

private:
    void	operator= (auto_g_free_array<T> & rhs);	// declare private, don't define: SHOULD NEVER BE USED BY ANYONE!!!
};

class PerfMonitor
{
public:
    PerfMonitor(const char * text = NULL, GLogLevelFlags logLevel = G_LOG_LEVEL_DEBUG) : m_text(text), m_logLevel(logLevel), m_sys_timeFirst(0), m_user_timeFirst(0), m_sys_timeLast(0), m_user_timeLast(0), m_wall_time(0)
    {
        reset();
    }
    void trace(const char * message, GLogLevelFlags logLevel = (GLogLevelFlags) 0)
    {
        if (logLevel == 0)
            logLevel = m_logLevel;
        guint64 sys_time, user_time;
        if (takeTime(sys_time, user_time))
        {
            traceTime("-- ", message, logLevel, sys_time, user_time, m_sys_timeLast, m_user_timeLast, m_wall_time);
            m_sys_timeLast = sys_time;
            m_user_timeLast = user_time;
        }
    }

    void reset()
    {
        if (takeTime(m_sys_timeFirst, m_user_timeFirst))
        {
            m_sys_timeLast = m_sys_timeFirst;
            m_user_timeLast = m_user_timeFirst;
        }
        else
            m_sys_timeLast = m_sys_timeFirst = m_user_timeLast = m_user_timeFirst = 0;
        m_wall_time = bootTime();
    }

    ~PerfMonitor()
    {
        guint64 sys_time, user_time;
        if (m_text && takeTime(sys_time, user_time))
            traceTime(">> ", m_text, m_logLevel, sys_time, user_time, m_sys_timeFirst, m_user_timeFirst, m_wall_time);
    }
private:
    inline bool takeTime(guint64 & sysTime, guint64 & userTime)
    {
        guint64 persec = sysconf(_SC_CLK_TCK);
        struct tms	cputimes;
        if (times(&cputimes) != -1)
        {
            sysTime = guint64(cputimes.tms_stime) * 1000LLU / persec;
            userTime = guint64(cputimes.tms_utime) * 1000LLU / persec;
            return true;
        }
        return false;
    }
    static void traceTime(const char * step, const char * message, GLogLevelFlags logLevel, const guint64 & sysTime, const guint64 & userTime, const guint64 & sysTimeRef, const guint64 & userTimeRef, const guint64 & beginWallTime)
    {
        guint64 sys_time = sysTime - sysTimeRef;
        guint64 user_time = userTime - userTimeRef;
        guint64 wallTime = bootTime() - beginWallTime;
//		if (sys_time + user_time > 20)
        g_log(G_LOG_DOMAIN, logLevel, "%s%s: System: %Lums, User: %Lums, Total: %Lums, Wall time: %Lums.", step, message, sys_time, user_time, sys_time + user_time, wallTime);
    }

    static guint64 bootTime()
    {
        struct timespec currTime;
        clock_gettime(CLOCK_MONOTONIC, &currTime);

        return ((guint64) currTime.tv_sec * 1000 +
                (guint64) currTime.tv_nsec / 1000000);
    }

    const char *    m_text;
    GLogLevelFlags  m_logLevel;
    guint64         m_sys_timeFirst;
    guint64         m_user_timeFirst;
    guint64         m_sys_timeLast;
    guint64         m_user_timeLast;
    guint64         m_wall_time;
};

std::string string_printf(const char *format, ...) G_GNUC_PRINTF(1, 2);

}  // namespace SmartKey

using SmartKey::StringUtils;

#endif // StringUtils_h
