/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2005-2010 Apple Inc. All rights reserved.
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

#ifndef __BITCODE_HPP__
#define __BITCODE_HPP__

#include <unistd.h>

namespace ld {

class Bitcode {
public:
    Bitcode(const uint8_t* content, uint32_t size) : _content(content), _size(size) { }
    virtual ~Bitcode() { }

    virtual bool isMarker() const                               { return _size <= 1 ; }
    virtual const uint8_t* getContent() const                   { return _content; }
    virtual uint32_t getSize() const                            { return _size; }
private:
    const uint8_t* _content;
    uint32_t _size;
};

class LLVMBitcode : public Bitcode {
public:
    LLVMBitcode(const uint8_t* content, uint32_t size, const uint8_t* cmd, uint32_t cmdSize) :
        Bitcode(content, size), _cmdline(cmd), _cmdSize(cmdSize)    { }

    virtual const uint8_t* getCmdline() const                   { return _cmdline; }
    virtual uint32_t getCmdSize() const                         { return _cmdSize; }
    virtual const char* getBitcodeName() const                  { return "llvm"; }
private:
    const uint8_t* _cmdline;
    uint32_t _cmdSize;
};

class ClangBitcode : public LLVMBitcode {
public:
    ClangBitcode(const uint8_t* content, uint32_t size, const uint8_t* cmd, uint32_t cmdSize) :
        LLVMBitcode(content, size, cmd, cmdSize)    { }
    virtual const char* getBitcodeName() const override         { return "clang"; }
};

class SwiftBitcode : public LLVMBitcode {
public:
    SwiftBitcode(const uint8_t* content, uint32_t size, const uint8_t* cmd, uint32_t cmdSize) :
        LLVMBitcode(content, size, cmd, cmdSize)    { }
    virtual const char* getBitcodeName() const override         { return "swift"; }
};

class AsmBitcode : public Bitcode {
public:
    AsmBitcode(const uint8_t* content, uint32_t size) : Bitcode(content, size) { }

    virtual bool isMarker() const override                      { return false; }
};

class BundleBitcode : public Bitcode {
public:
    BundleBitcode(const uint8_t* content, uint32_t size) :
        Bitcode(content, size)  { }
};

}


#endif /* defined(__BITCODE_HPP__) */
