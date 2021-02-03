#import <CoreFoundation/CoreFoundation.h>
#import <CFNetwork/CFNetworkExport.h>

typedef struct __CFHost *CFHostRef;

enum CFHostInfoType {
    kCFHostAddresses = 0,
    kCFHostNames = 1,
    kCFHostReachability = 2,
};
typedef enum CFHostInfoType CFHostInfoType;

typedef void (*CFHostClientCallBack)(CFHostRef host, CFHostInfoType infoType, const CFStreamError *streamError, void *info);

typedef struct CFHostClientContext {
    CFIndex version;
    void *info;
    CFAllocatorRetainCallBack retain;
    CFAllocatorReleaseCallBack release;
    CFAllocatorCopyDescriptionCallBack copyDescription;
} CFHostClientContext;

CFNETWORK_EXPORT CFTypeID CFHostGetTypeID();
CFNETWORK_EXPORT CFHostRef CFHostCreateCopy(CFAllocatorRef alloc, CFHostRef self);
CFNETWORK_EXPORT CFHostRef CFHostCreateWithAddress(CFAllocatorRef allocator, CFDataRef address);
CFNETWORK_EXPORT CFHostRef CFHostCreateWithName(CFAllocatorRef allocator, CFStringRef name);
CFNETWORK_EXPORT CFArrayRef CFHostGetAddressing(CFHostRef self, Boolean *hasBeenResolved);
CFNETWORK_EXPORT CFArrayRef CFHostGetNames(CFHostRef self, Boolean *hasBeenResolved);
CFNETWORK_EXPORT CFDataRef CFHostGetReachability(CFHostRef self, Boolean *hasBeenResolved);
CFNETWORK_EXPORT Boolean CFHostSetClient(CFHostRef self, CFHostClientCallBack callback, CFHostClientContext *context);

CFNETWORK_EXPORT Boolean CFHostStartInfoResolution(CFHostRef self, CFHostInfoType infoType, CFStreamError *streamError);
CFNETWORK_EXPORT void CFHostCancelInfoResolution(CFHostRef theHost, CFHostInfoType infoType);

CFNETWORK_EXPORT void CFHostScheduleWithRunLoop(CFHostRef self, CFRunLoopRef runLoop, CFStringRef mode);
CFNETWORK_EXPORT void CFHostUnscheduleFromRunLoop(CFHostRef self, CFRunLoopRef runLoop, CFStringRef mode);
