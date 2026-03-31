/*
 *  fork_program.h
 *  kext_tools
 *
 *  Created by nik on 5/11/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#ifndef _FORK_PROGRAM
#define _FORK_PROGRAM

#include <CoreFoundation/CoreFoundation.h>
#include <unistd.h>
int fork_program(
    const char * argv0,
    char * const argv[],
    Boolean      wait);

#endif /* _FORK_PROGRAM */
