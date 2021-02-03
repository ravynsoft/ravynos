/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#if !defined(CF_EXCLUDE_CSTD_HEADERS)

#include <sys/types.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__STDC_VERSION__)

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#endif

#ifndef MAXFLOAT
#define MAXFLOAT ((float)3.40282346638528860e+38)
#endif

#endif

#import <CoreFoundation/CFAttributedString.h>
#import <CoreFoundation/CFBag.h>
#import <CoreFoundation/CFBinaryHeap.h>
#import <CoreFoundation/CFBitVector.h>
#import <CoreFoundation/CFBundle.h>
#import <CoreFoundation/CFByteOrder.h>
#import <CoreFoundation/CFCalendar.h>
#import <CoreFoundation/CFCharacterSet.h>
#import <CoreFoundation/CFData.h>
#import <CoreFoundation/CFDate.h>
#import <CoreFoundation/CFDateFormatter.h>
#import <CoreFoundation/CFDictionary.h>
#import <CoreFoundation/CFError.h>
#import <CoreFoundation/CFFileDescriptor.h>
#import <CoreFoundation/CFLocale.h>
#import <CoreFoundation/CFMachPort.h>
#import <CoreFoundation/CFMessagePort.h>
#import <CoreFoundation/CFNotificationCenter.h>
#import <CoreFoundation/CFNumber.h>
#import <CoreFoundation/CFNumberFormatter.h>
#import <CoreFoundation/CFPlugIn.h>
#import <CoreFoundation/CFPreferences.h>
#import <CoreFoundation/CFPropertyList.h>
#import <CoreFoundation/CFRunLoop.h>
#import <CoreFoundation/CFSet.h>
#import <CoreFoundation/CFSocket.h>
#import <CoreFoundation/CFStream.h>
#import <CoreFoundation/CFString.h>
#import <CoreFoundation/CFStringTokenizer.h>
#import <CoreFoundation/CFTimeZone.h>
#import <CoreFoundation/CFTree.h>
#import <CoreFoundation/CFURL.h>
#import <CoreFoundation/CFUUID.h>
#import <CoreFoundation/CFUserNotification.h>
#import <CoreFoundation/CFXMLNode.h>
#import <CoreFoundation/CFXMLParser.h>
