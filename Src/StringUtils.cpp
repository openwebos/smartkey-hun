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
 * StringUtils.cpp
 *
 *  Created on: Jan 18, 2010
 *      Author: Anthony D'Auria
 */

#include "StringUtils.h"
#include <wctype.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <string.h>
#include <glib.h>
#include <stdio.h>

using namespace SmartKey;

/**
* chomp
*
* @param *str
*   input string
*
* @param numChars
*   number of chars in string
*/
void StringUtils::chomp (char* str, size_t numChars)
{
    while (*str != '\0' && numChars--)
    {
        if (*str == '\n' || *str == '\r')
        {
            *str = '\0';
            return;
        }
        str++;
    }
}

/**
* compare strings
*
* @param first
*   first string
*
* @param second
*   second string
*
* @return bool
*   true if first string is less than second
*/
bool StringUtils::compareStrings (const std::string& first, const std::string& second)
{
    UnicodeString uFirst  = StringUtils::utf8StringToUnicodeString(first);
    UnicodeString uSecond = StringUtils::utf8StringToUnicodeString(second);

    StringUtils::transliterate(uFirst);
    StringUtils::transliterate(uSecond);

    uFirst.toLower();
    uSecond.toLower();

    return uFirst < uSecond;
}

/**
* transliterate
*
* @param str
*   input unicode string
*
* @return bool
*   true if done
*/
bool StringUtils::transliterate (UnicodeString& str)
{
    UParseError parseError = { 0 };
    UErrorCode lStatus    = U_ZERO_ERROR;
    static icu::Transliterator* pTransliterator = 0;

    if (pTransliterator == NULL)
    {
        pTransliterator = icu::Transliterator::createInstance("NFD; [:M:] Remove; NFC;", UTRANS_FORWARD, parseError, lStatus);
        if ((pTransliterator == 0) || U_FAILURE(lStatus))
        {
            return false;
        }
        else
        {
            icu::Transliterator::registerInstance(pTransliterator);
        }
    }

    pTransliterator->transliterate(str);
    return true;
}

/**
* convert string utf8 to lower
*
* @param str
*   input string
*
* @return std::string
*   converted string
*/
std::string StringUtils::utf8tolower (const std::string& str)
{
    std::string   lowerStr;
    lowerStr.reserve(str.size());
    gchar buffer[6];    // for individual conversions from gunichar to utf8
    const gchar * cstr = str.c_str();
    while (*cstr)
    {
        gunichar c = g_unichar_tolower(g_utf8_get_char(cstr));
        if (!c)
            break;
        gint count = g_unichar_to_utf8(c, buffer);
        lowerStr.append(buffer, count);
        cstr = g_utf8_next_char(cstr);
    }
    return lowerStr;
}

/**
* convert utf8 string to unicode string
*
* @param str
*   input string
*
* @return UnicodeString
*   converted string
*/
UnicodeString StringUtils::utf8StringToUnicodeString (const std::string& str)
{
    if (str.empty())
        return UnicodeString();

    glong len(0);
    auto_g_free_array<gunichar2> utf16 = g_utf8_to_utf16(str.c_str(), -1, NULL, &len, NULL);
    if (utf16 != NULL)
    {
        UnicodeString ustr(utf16.p);
        return ustr;
    }
    else
    {
        return UnicodeString();
    }
}

/**
* print string
*
* @param *format
*   format of output
*
* @param ...
*   float number of parameters
*
* @return std::string
*   return string
*/
std::string string_printf(const char *format, ...)
{
    if (format == 0)
        return "";
    va_list args;
    va_start(args, format);
    char stackBuffer[1024];
    int result = vsnprintf(stackBuffer, G_N_ELEMENTS(stackBuffer), format, args);
    if (result > -1 && result < (int) G_N_ELEMENTS(stackBuffer))
    {
        // stack buffer was sufficiently large. Common case with no temporary dynamic buffer.
        va_end(args);
        return std::string(stackBuffer, result);
    }

    int length = result > -1 ? result + 1 : G_N_ELEMENTS(stackBuffer) * 3;
    char * buffer = 0;
    do
    {
        if (buffer)
        {
            delete[] buffer;
            length *= 3;
        }
        buffer = new char[length];
        result = vsnprintf(buffer, length, format, args);
    }
    while (result == -1 && result < length);
    va_end(args);
    std::string	str(buffer, result);
    delete[] buffer;
    return str;
}


