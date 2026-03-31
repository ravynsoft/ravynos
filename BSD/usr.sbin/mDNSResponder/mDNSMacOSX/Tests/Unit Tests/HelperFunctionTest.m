//
//  HelperFunctionTest.m
//  Tests
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "unittest_common.h"

@interface HelperFunctionTest : XCTestCase

@end

@implementation HelperFunctionTest

- (void)setUp {
    // It is empty for now.
}

- (void)tearDown {
    // It is empty for now.
}

- (void)testCFStringToDomainLabel {
    // test_cstring[i][0] is the input
    // test_cstring[i][1] is the expected correct output
    static const char * const test_cstring[][2] = {
        {"short", "short"},
        {"this-is-a-normal-computer-name", "this-is-a-normal-computer-name"},
        {"", ""},
        {"This is an ascii string whose length is more than 63 bytes, where it takes one byte to store every character", "This is an ascii string whose length is more than 63 bytes, whe"},
        {"यह एक एस्सी स्ट्रिंग है जिसकी लंबाई साठ तीन बाइट्स से अधिक है, जहां यह हर चरित्र को संग्रहीत करने के लिए एक बाइट लेता है", "यह एक एस्सी स्ट्रिंग है "}, // "यह एक एस्सी स्ट्रिंग है " is 62 byte, and "यह एक एस्सी स्ट्रिंग है जि" is more than 63, so the result is expected to truncated to 62 bytes instead of 63 bytes
        {"वितीय टेस्ट ट्राई टी॰वी॰", "वितीय टेस्ट ट्राई टी॰वी"},
        {"这是一个超过六十三比特的其中每个中文字符占三比特的中文字符串", "这是一个超过六十三比特的其中每个中文字符占"},
        {"🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝", "🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝"} // "🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝" is 60 bytes, and "🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝🃝" is more than 63 bytes so the result is expected to be truncated to 60 bytes instead of 64 bytes
    };

    for (int i = 0, n = sizeof(test_cstring) / sizeof(test_cstring[0]); i < n; i++) {
        // construct CFString from input
        CFStringRef name_ref = CFStringCreateWithCString(kCFAllocatorDefault, test_cstring[i][0], kCFStringEncodingUTF8);
        XCTAssertTrue(name_ref != NULL, @"unit test internal error. {descrption=\"name_ref should be non-NULL.\"}");

        // call the function being tested
        domainlabel label;
        mDNSDomainLabelFromCFString_ut(name_ref, &label);

        // Check if the result is correct
        XCTAssertEqual(label.c[0], strlen(test_cstring[i][1]),
                       @"name length is not equal. {expect=%d,actual=%d}", strlen(test_cstring[i][1]), label.c[0]);
        XCTAssertTrue(memcmp(label.c + 1, test_cstring[i][1], label.c[0]) == 0,
                      @"name is not correctly decoded. {expect='%s',actual='%s'}", test_cstring[i][1], label.c + 1);

        CFRelease(name_ref);
    }
}

@end
