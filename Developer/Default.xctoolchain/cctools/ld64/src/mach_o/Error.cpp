/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <errno.h>
#include <TargetConditionals.h>

#include <mach/machine.h>
#include <mach-o/fat.h>

#include "Error.h"

namespace mach_o {


Error::Error(Error&& other)
{
	_buffer = other._buffer;
	other._buffer = nullptr;
}

Error& Error::operator=(Error&& other)
{
	_buffer = other._buffer;
	other._buffer = nullptr;
	return *this;
}

Error::~Error()
{
   if ( _buffer )
		free(_buffer);
	_buffer = nullptr;
}

Error::Error(const char* format, ...)
{
	va_list    list;
	va_start(list, format);
	vasprintf(&_buffer, format, list);
	va_end(list);
}

Error::Error(const char* format, va_list list)
{
	vasprintf(&_buffer, format, list);
}

const char* Error::message() const
{
	return _buffer;
}

bool Error::messageContains(const char* subString) const
{
	if ( _buffer == nullptr )
		return false;
	return (strstr(_buffer, subString) != nullptr);
}

} // namespace mach_o

