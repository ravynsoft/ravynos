/*
 *  testlist.c
 *  corecrypto
 *
 *  Created on 09/13/2012
 *
 *  Copyright (c) 2012,2015 Apple Inc. All rights reserved.
 *
 */

#include "testmore.h"
#include "testlist.h"

#define ONE_TEST(x) {.name=#x, .entry=x, .executed=0},
#define DISABLED_ONE_TEST(x)
struct one_test_s testlist[] = {
#include "testlistInc.h"
    { .name=NULL, .entry=NULL, .executed=0},
};
#undef ONE_TEST
#undef DISABLED_ONE_TEST


