/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <pthread.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>
#include <SystemConfiguration/SystemConfiguration.h>

static SCNetworkReachabilityRef g_reachability = NULL;
static CFURLRef g_url = NULL;
static CFReadStreamRef g_rstream = NULL;

static char *
string2CString(CFStringRef str)
{
    UInt8 *buffer;
    CFIndex clen;
    CFRange r = CFRangeMake(0, CFStringGetLength(str));

    if (CFStringGetBytes(str, r, kCFStringEncodingASCII, 0, false, NULL, 0, &clen) > 0) {
	buffer = (UInt8 *)CFAllocatorAllocate(kCFAllocatorDefault, (clen + 1) * sizeof(UInt8), 0);

	if (buffer != NULL) {
	    if (CFStringGetBytes(str, r, kCFStringEncodingASCII, 0, false, buffer, clen, NULL)) {
		buffer[clen] = '\0';
		return (char *)buffer;
	    }
	    CFAllocatorDeallocate(kCFAllocatorDefault, buffer);
	}
    }

    return NULL;
}

static void
printReachabilityFlags(const char *source, SCNetworkReachabilityFlags flags)
{
    printf("[%s] Reachability flags (%x):\n", source, flags);
    if (flags & kSCNetworkReachabilityFlagsTransientConnection) {
	printf("[%s]  transient\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsReachable) {
	printf("[%s]  reachable\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsConnectionRequired) {
	printf("[%s]  connection required\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) {
	printf("[%s]  connection on traffic\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsInterventionRequired) {
	printf("[%s]  intervention required\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsConnectionOnDemand) {
	printf("[%s]  connection on demand\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsIsLocalAddress) {
	printf("[%s]  local address\n", source);
    }
    if (flags & kSCNetworkReachabilityFlagsIsDirect) {
	printf("[%s]  direct\n", source);
    }
#if TARGET_OS_IPHONE
    if (flags & kSCNetworkReachabilityFlagsIsWWAN) {
	printf("[%s]  wwan\n", source);
    }
#endif
}

static void
handleReachabilityUpdate(
	SCNetworkReachabilityRef target,
	SCNetworkReachabilityFlags flags,
	void *info)
{
    printReachabilityFlags("RunLoop", flags);
}

static SCNetworkReachabilityRef
createReachabilityWithCFHost(CFHostRef theHost)
{
    SCNetworkReachabilityRef reachRef = NULL;
    Boolean resolved = FALSE;
    CFArrayRef addrs = CFHostGetAddressing(theHost, &resolved);

    if (resolved && addrs != NULL && CFArrayGetCount(addrs) > 0) {
	CFDataRef addr = (CFDataRef)CFArrayGetValueAtIndex(addrs, 0);

	reachRef = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (struct sockaddr *)CFDataGetBytePtr(addr));
    } else {
	CFArrayRef names = CFHostGetNames(theHost, NULL);

	if (names != NULL && CFArrayGetCount(names) > 0) {
	    CFStringRef host = (CFStringRef)CFArrayGetValueAtIndex(names, 0);
	    char *chost = string2CString(host);

	    reachRef = SCNetworkReachabilityCreateWithName(kCFAllocatorDefault, chost);

	    CFAllocatorDeallocate(kCFAllocatorDefault, chost);
	}
    }

    if (reachRef != NULL) {
	SCNetworkReachabilityContext reach_ctx = { 0, NULL, NULL, NULL, NULL };
	SCNetworkReachabilitySetCallback(reachRef, handleReachabilityUpdate,
					 &reach_ctx);
	CFShow(reachRef);
    } else {
	fprintf(stderr, "Failed to create a reachability object\n");
    }

    return reachRef;
}

static void
handleDownload(CFReadStreamRef rstream, CFStreamEventType eventType, void *info)
{
    Boolean done = FALSE;

    if (eventType == kCFStreamEventHasBytesAvailable) {
	UInt8 buffer[1024];

	while (CFReadStreamHasBytesAvailable(rstream)) {
	    CFIndex count = CFReadStreamRead(rstream, buffer, sizeof(buffer));
	    if (count == 0) {
		done = TRUE;
	    }
	}
    } else if (eventType == kCFStreamEventEndEncountered) {
	printf("Download completed\n");
	done = TRUE;
    } else if (eventType == kCFStreamEventErrorOccurred) {
	printf("Download error\n");
	done = TRUE;
    } else {
	printf("Got stream event: %lu\n", eventType);
    }

    if (!done) {
	return;
    }

    CFReadStreamUnscheduleFromRunLoop(rstream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFReadStreamClose(rstream);
    CFRelease(rstream);

    g_rstream = NULL;
}

static void
startDownload(void)
{
    CFHTTPMessageRef request = CFHTTPMessageCreateRequest(kCFAllocatorDefault, CFSTR("GET"), g_url, kCFHTTPVersion1_1);
    CFReadStreamRef rstream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, request);
    CFStreamClientContext ctx = { 0, NULL, NULL, NULL, NULL };

    printf("Starting download\n");

    CFReadStreamSetClient(rstream,
	    		  kCFStreamEventEndEncountered | kCFStreamEventErrorOccurred | kCFStreamEventHasBytesAvailable, 
			  handleDownload,
			  &ctx);

    CFReadStreamScheduleWithRunLoop(rstream, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

    CFReadStreamOpen(rstream);

    g_rstream = rstream;

    CFRelease(request);
}

static void
downloadTimerFired(CFRunLoopTimerRef timer, void *info)
{
    if (g_rstream != NULL) {
	handleDownload(g_rstream, kCFStreamEventErrorOccurred, NULL);
    }


    SCNetworkReachabilityUnscheduleFromRunLoop(g_reachability, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

    startDownload();

    SCNetworkReachabilityScheduleWithRunLoop(g_reachability, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
}

static void
startDownloadLoop(void)
{
    CFRunLoopTimerRef timer;
    CFRunLoopTimerContext ctx = { 0, NULL, NULL, NULL, NULL };
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();

    timer = CFRunLoopTimerCreate(kCFAllocatorDefault, 
	    			 now + 0.1,
				 7.0,
				 0,
				 0,
				 downloadTimerFired,
				 &ctx);

    CFRunLoopAddTimer(CFRunLoopGetMain(), timer, kCFRunLoopCommonModes);
}

static void *
reachabilityLoop(void *arg)
{
    while (1) {
	SCNetworkReachabilityFlags flags;
	SCNetworkReachabilityGetFlags(g_reachability, &flags);

	printReachabilityFlags("thread", flags);

	sleep(1);
    }

    return NULL;
}

static void
startReachabilityThread(void)
{
    pthread_attr_t tattr;
    pthread_t th;

    pthread_attr_init(&tattr);
    pthread_create(&th, &tattr, reachabilityLoop, NULL);
    pthread_attr_destroy(&tattr);
}

static void
addressResolutionCallback(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError *error, void *info)
{
    g_reachability = createReachabilityWithCFHost(theHost);

    if (g_reachability != NULL) {
	startDownloadLoop();
	startReachabilityThread();
    }

    CFRelease(theHost);
}

static void
startAddressResolution(Boolean resolve)
{
    CFStringRef hostStr = CFURLCopyHostName(g_url);
    CFHostRef cfhost = CFHostCreateWithName(kCFAllocatorDefault, hostStr);
    CFHostClientContext ctx = { 0, NULL, NULL, NULL, NULL };
    CFStreamError err;

    if (resolve) {
	CFHostSetClient(cfhost, addressResolutionCallback, &ctx);
	CFHostScheduleWithRunLoop(cfhost, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
	CFHostStartInfoResolution(cfhost, kCFHostAddresses, &err);
    } else {
	addressResolutionCallback(cfhost, kCFHostNames, NULL, NULL);
    }

    CFRelease(hostStr);
}

int
main(int argc, char *argv[])
{
    CFStringRef urlStr;
    Boolean resolve = TRUE;

    if (argc < 2) {
	fprintf(stderr, "usage: %s <url> [-byname]", argv[0]);
	return 1;
    }

    urlStr = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingASCII);
    g_url = CFURLCreateWithString(kCFAllocatorDefault, urlStr, NULL);

    CFRelease(urlStr);

    if (argc > 2 && !strcmp(argv[2], "-byname")) {
	resolve = FALSE;
    }

    startAddressResolution(resolve);

    CFRunLoopRun();

    CFRelease(g_url);

    SCNetworkReachabilityUnscheduleFromRunLoop(g_reachability, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    CFRelease(g_reachability);

    return 0;
}
