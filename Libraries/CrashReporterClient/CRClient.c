/*
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <CrashReporterClient.h>

#define _crc_make_getter(attr) ((const char *)(unsigned long)gCRAnnotations.attr)
#define _crc_make_setter(attr, arg) (gCRAnnotations.attr = (uint64_t)(unsigned long)(arg))

void _crc_collect_info(void);

CRASH_REPORTER_CLIENT_HIDDEN
void _crc_collect_info(void)
{
    /* FIXME: this is intended to collect backtrace & thread info, then
              fill in the rest of the CR attributes */
    gCRAnnotations.signature_string = 0x5241564E; /* RAVN */
}

const char *CRGetCrashLogMessage(void)
{
    return _crc_make_getter(message);
}

void CRSetCrashLogMessage(const char *crashString)
{
    _crc_make_setter(message, crashString);
    _crc_collect_info();
}

void CRSetCrashLogMessage2(const char *crashString)
{
    _crc_make_setter(message2, crashString);
    _crc_collect_info();
}

CRASH_REPORTER_CLIENT_HIDDEN
struct crashreporter_annotations_t gCRAnnotations
    __attribute__((section("__DATA," CRASHREPORTER_ANNOTATIONS_SECTION)))
    = { CRASHREPORTER_ANNOTATIONS_VERSION, 0, 0, 0, 0, 0, 0, 0 };
