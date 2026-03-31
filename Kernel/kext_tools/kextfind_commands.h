/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#ifndef _KEXTFIND_COMMANDS_H_
#define _KEXTFIND_COMMANDS_H_

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>

#include "kextfind_main.h"

void printKext(
    OSKextRef theKext,
    PathSpec relativePath,
    Boolean extra_info,
    char lineEnd);

void printKextProperty(
    OSKextRef theKext,
    CFStringRef propKey,
    char lineEnd);
void printKextMatchProperty(
    OSKextRef theKext,
    CFStringRef propKey,
    char lineEnd);
void printKextArches(
    OSKextRef theKext,
    char lineEnd,
    Boolean printLineEnd);

void printKextDependencies(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd);
void printKextDependents(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd);
void printKextPlugins(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd);

void printKextInfoDictionary(
    OSKextRef theKext,
    PathSpec pathSpec,
    char lineEnd);
void printKextExecutable(
    OSKextRef theKext,
    PathSpec pathSpec,
    char lineEnd);

CFStringRef copyPathForKext(
    OSKextRef theKext,
    PathSpec  pathSpec);

CFStringRef copyKextInfoDictionaryPath(
    OSKextRef theKext,
    PathSpec pathSpec);
CFStringRef copyKextExecutablePath(
    OSKextRef theKext,
    PathSpec pathSpec);

#endif /* _KEXTFIND_COMMANDS_H_ */
