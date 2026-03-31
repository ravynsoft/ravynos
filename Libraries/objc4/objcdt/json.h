/*
* Copyright (c) 2019 Apple Inc.  All Rights Reserved.
*
* @APPLE_LICENSE_HEADER_START@
*
* This file contains Original Code and/or Modifications of Original Code
* as defined in and that are subject to the Apple Public Source License
* Version 2.0 (the 'License'). You may not use this file except in
* compliance with the License. Please obtain a copy of the License at
* http://www.opensource.apple.com/apsl/ and read it before using this
* file.
*
* The Original Code and all software distributed under the License are
* distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
* EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
* INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
* Please see the License for the specific language governing rights and
* limitations under the License.
*
* @APPLE_LICENSE_HEADER_END@
*/

#ifndef _OBJC_OBJCDT_JSON_H_
#define _OBJC_OBJCDT_JSON_H_

#include <cstdint>
#include <cstdbool>
#include <stdio.h>
#include <functional>

namespace json {

enum context: uint8_t {
    root,
    array_value,
    object_value,
    object_key,
    done,
};

class writer {
private:
    FILE *_file;
    context _context;
    int _depth;
    bool _needs_comma;

    void begin_value(int sep = '\0');
    void advance(context old);
    void key(const char *key);

public:

    writer(FILE *f);
    ~writer();

    void object(std::function<void()>);
    void object(const char *key, std::function<void()>);

    void array(std::function<void()>);
    void array(const char *key, std::function<void()>);

    void boolean(bool value);
    void boolean(const char *key, bool value);

    void number(uint64_t value);
    void number(const char *key, uint64_t value);

    void string(const char *s);
    void string(const char *key, const char *s);

    __printflike(2, 3)
    void stringf(const char *fmt, ...);

    __printflike(3, 4)
    void stringf(const char *key, const char *fmt, ...);
};

}

#endif /* _OBJC_OBJCDT_JSON_H_ */
