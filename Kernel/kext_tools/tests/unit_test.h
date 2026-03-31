/*
 *  unit_test.h
 *  kext_tools
 *
 *  Copyright 2017 Apple Inc. All rights reserved.
 *
 */
#pragma once

#define TEST_START(name) \
do { \
    printf("==================================================\n"); \
    printf("[TEST] %s\n", name); \
    printf("==================================================\n"); \
} while(0)

#define TEST_RESULT(name, cond) \
do { \
    if ((cond)) \
        printf("[PASS] %s\n", (name)); \
    else \
        printf("[FAIL] %s\n", (name)); \
} while (0)

#define TEST_REQUIRE(name, cond, errVar, errVal, label) \
do { \
    TEST_RESULT(name, cond); \
    if (!(cond)) { \
        (errVar) = (errVal); \
        goto label; \
    } \
} while (0)

#define TEST_CASE(name, cond) \
do { \
    printf("[BEGIN] %s\n", (name)); \
    TEST_RESULT(name, cond); \
} while (0)

#define TEST_LOG(fmt, ...) \
do { \
    printf("[LOG] %s.%d: " fmt "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__); \
} while (0)
