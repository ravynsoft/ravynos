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

#include <assert.h>
#include "json.h"

namespace json {

static bool
context_is_value(context c)
{
    return c == root || c == array_value || c == object_value;
}

writer::writer(FILE *f)
: _file(f)
, _context(root)
, _depth(0)
, _needs_comma(false)
{
}

writer::~writer()
{
    fputc('\n', _file);
    fflush(_file);
}

void
writer::begin_value(int sep)
{
    if (_needs_comma) {
        _needs_comma = false;
        if (sep) {
            fprintf(_file, ", %c\n", sep);
            return;
        }
        fputs(",\n", _file);
    }
    if (_context == array_value || _context == object_key) {
        fprintf(_file, "%*s", _depth * 2, "");
    }
    if (sep) {
        fprintf(_file, "%c\n", sep);
    }
}

void
writer::advance(context c)
{
    switch (c) {
    case root:
        _context = done;
        _needs_comma = false;
        break;
    case array_value:
        _context = array_value;
        _needs_comma = true;
        break;
    case object_value:
        _context = object_key;
        _needs_comma = true;
        break;
    case object_key:
        _context = object_value;
        _needs_comma = false;
        break;
    case done:
        assert(false);
        break;
    }
}

void
writer::key(const char *key)
{
    assert(_context == object_key);

    begin_value();
    fprintf(_file, "\"%s\": ", key);
    advance(_context);
}

void
writer::object(std::function<void()> f)
{
    context old = _context;
    assert(context_is_value(old));

    begin_value('{');

    _depth++;
    _context = object_key;
    _needs_comma = false;
    f();

    _depth--;
    fprintf(_file, "\n%*s}", _depth * 2, "");
    advance(old);
}

void
writer::object(const char *k, std::function<void()> f)
{
    key(k);
    object(f);
}

void
writer::array(std::function<void()> f)
{
    context old = _context;
    assert(context_is_value(old));

    begin_value('[');

    _depth++;
    _context = array_value;
    _needs_comma = false;
    f();

    _depth--;
    fprintf(_file, "\n%*s]", _depth * 2, "");
    advance(old);
}

void
writer::array(const char *k, std::function<void()> f)
{
    key(k);
    array(f);
}

void
writer::boolean(bool value)
{
    assert(context_is_value(_context));
    begin_value();
    fputs(value ? "true" : "false", _file);
    advance(_context);
}

void
writer::boolean(const char *k, bool value)
{
    key(k);
    boolean(value);
}

void
writer::number(uint64_t value)
{
    assert(context_is_value(_context));
    begin_value();
    fprintf(_file, "%lld", value);
    advance(_context);
}

void
writer::number(const char *k, uint64_t value)
{
    key(k);
    number(value);
}

void
writer::string(const char *s)
{
    assert(context_is_value(_context));
    begin_value();
    fprintf(_file, "\"%s\"", s);
    advance(_context);
}

void
writer::string(const char *k, const char *s)
{
    key(k);
    string(s);
}

void
writer::stringf(const char *fmt, ...)
{
    va_list ap;

    assert(context_is_value(_context));
    begin_value();
    fputc('"', _file);
    va_start(ap, fmt);
    vfprintf(_file, fmt, ap);
    va_end(ap);
    fputc('"', _file);
    advance(_context);
}

void
writer::stringf(const char *k, const char *fmt, ...)
{
    va_list ap;

    key(k);

    assert(context_is_value(_context));
    begin_value();
    fputc('"', _file);
    va_start(ap, fmt);
    vfprintf(_file, fmt, ap);
    va_end(ap);
    fputc('"', _file);
    advance(_context);
}

} // json
