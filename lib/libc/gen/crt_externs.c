/*
 * Implementation of Apple crt_externs.h functions for ravynOS/FreeBSD
 *
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
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

#include <crt_externs.h>
#include <mach/mach.h>
__FBSDID("$ravynOS$");

char ***__argv = 0;
int *__argc = 0;
extern char **environ;
extern char *__progname;

char ***_NSGetArgv(void)
{
    return __argv;
}

int *_NSGetArgc(void)
{
    return __argc;
}

char ***_NSGetEnviron(void)
{
    return &environ;
}

char **_NSGetProgname(void)
{
    return &__progname;
}

struct mach_header *_NSGetMachExecuteHeader(void)
{
    return NULL;
}
