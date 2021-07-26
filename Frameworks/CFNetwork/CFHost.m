#import <CFNetwork/CFHost.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSDebug.h>
#ifdef WINDOWS
#import <Foundation/NSHandleMonitor_win32.h>
#undef WINVER
#define WINVER 0x501
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#else
#include <sys/param.h>
#endif

#include <pthread.h>

#if defined(WIN32) || defined(LINUX)
#define MAXHOSTNAMELEN 512
#endif

typedef enum {
   CFHostRequestInQueue,
   CFHostRequestInProgress,
   CFHostRequestDone,
   CFHostRequestDeallocate,
} CFHostRequestState;

typedef struct {
  CFHostRequestState _state;
   char             *_name;
   struct addrinfo  *_addressList;
#ifdef WINDOWS
   HANDLE            _event;
#endif
} CFHostRequest;

@interface __CFHost : NSObject {
   CFStringRef          _name;
   CFHostClientCallBack _callback;
   CFHostClientContext  _context;
   Boolean              _hasResolvedAddressing;
   CFArrayRef           _addressing;
   CFHostRequest       *_request;
#ifdef WINDOWS
   HANDLE                 _event;
   NSHandleMonitor_win32 *_monitor;
#endif
}

@end
#ifdef __clang__
// has to be in sync with the __CFHost interface
struct __CFHost {
   CFStringRef          _name;
   CFHostClientCallBack _callback;
   CFHostClientContext  _context;
   Boolean              _hasResolvedAddressing;
   CFArrayRef           _addressing;
   CFHostRequest       *_request;
#ifdef WINDOWS
   HANDLE                 _event;
   NSHandleMonitor_win32 *_monitor;
#endif
};
#endif

@implementation __CFHost

#ifdef WINDOWS

typedef struct  {
   CRITICAL_SECTION queueLock;
   HANDLE           queueEvent;
   int              queueCapacity,queueCount;
   CFHostRequest  **queue;
} CFAddressResolverThreadInfo;

static int preXP_getaddrinfo(const char *host,const char *service,const struct addrinfo *hints,struct addrinfo **result){
   struct addrinfo *list=NULL;
   struct addrinfo *current=NULL;
   struct hostent  *hp;

   if((hp=gethostbyname(host))==NULL)
    return EAI_FAIL;

   switch(hp->h_addrtype){

    case AF_INET:;
     uint32_t **addr_list;

     addr_list=(uint32_t **)hp->h_addr_list;
     for(;*addr_list!=NULL;addr_list++){
      struct addrinfo    *node=NSZoneCalloc(NULL,1,sizeof(struct addrinfo));
      struct sockaddr_in *ipv4=NSZoneCalloc(NULL,1,sizeof(struct sockaddr_in));

      node->ai_family=AF_INET;
      node->ai_addrlen=sizeof(struct sockaddr_in);
      node->ai_addr=(struct sockaddr *)ipv4;
      ipv4->sin_family=AF_INET;
      ipv4->sin_addr.s_addr=**addr_list;

      if(list==NULL)
       list=current=node;
      else {
       current->ai_next=node;
       current=node;
      }
     }
     break;
   }

   *result=list;
   return 0;
}

static void preXP_freeaddrinfo(struct addrinfo *info){
   struct addrinfo *next;

   for(;info!=NULL;info=next){
    next=info->ai_next;
    NSZoneFree(NULL,info->ai_addr);
    NSZoneFree(NULL,info);
   }
}

static int any_getaddrinfo(const char *host,const char *service,const struct addrinfo *hints,struct addrinfo **result){
   HANDLE               library=LoadLibrary("WS2_32");
   typeof(getaddrinfo) *function=(typeof(getaddrinfo) *)GetProcAddress(library,"getaddrinfo");

   if(function==NULL){
    return preXP_getaddrinfo(host,service,hints,result);
   }
   else {
    return function(host,service,hints,result);
   }
}

static void any_freeaddrinfo(struct addrinfo *info){
   HANDLE              library=LoadLibrary("WS2_32");
   typeof(freeaddrinfo) *function=(typeof(freeaddrinfo) *)GetProcAddress(library,"freeaddrinfo");

   if(function==NULL){
    return preXP_freeaddrinfo(info);
   }
   else {
    return function(info);
   }
}

static struct addrinfo *blockingRequest(CFHostRequest *request){
   struct addrinfo *result;

   if(any_getaddrinfo(request->_name,NULL,NULL,&result)!=0)
    return NULL;

   return result;
}

static __stdcall unsigned addressResolverThread(void *arg){
   CFAddressResolverThreadInfo *info=(CFAddressResolverThreadInfo *)arg;

   while(YES){
    Boolean queueEmpty;

    EnterCriticalSection(&(info->queueLock));
     queueEmpty=(info->queueCount==0)?TRUE:FALSE;
    LeaveCriticalSection(&(info->queueLock));

    if(queueEmpty){
     NSCooperativeThreadBlocking();
     WaitForSingleObject(info->queueEvent,INFINITE);
     NSCooperativeThreadWaiting();
    }

    CFHostRequest *request=NULL;

    EnterCriticalSection(&(info->queueLock));

    while(info->queueCount>0 && request==NULL){
     request=info->queue[0];

     info->queueCount--;

     int i;
     for(i=0;i<info->queueCount;i++)
      info->queue[i]=info->queue[i+1];
    }
    if(request!=NULL)
     request->_state=CFHostRequestInProgress;

    LeaveCriticalSection(&(info->queueLock));

    if(request!=NULL){
     struct addrinfo *addressList=blockingRequest(request);

     HANDLE event=NULL;

     EnterCriticalSection(&(info->queueLock));
      request->_addressList=addressList;

      if(request->_state==CFHostRequestInProgress){
       request->_state=CFHostRequestDone;
       event=request->_event;
       request=NULL;
      }
     LeaveCriticalSection(&(info->queueLock));

     if(request!=NULL){
      if(request->_addressList!=NULL)
       any_freeaddrinfo(request->_addressList);
      NSZoneFree(NULL,request->_name);
      NSZoneFree(NULL,request);
     }

     if(event!=NULL){
      SetEvent(event);
     }
    }
   }
   return 0;

}

-(void)handleMonitorIndicatesSignaled:(NSHandleMonitor_win32 *)monitor {

   if(_request==NULL){
    // cancelled
    return;
   }

   CloseHandle(_request->_event);
   _request->_event=NULL;
   [_monitor invalidate];
   [_monitor setDelegate:nil];
   [_monitor autorelease];
   _monitor=nil;

   if(_addressing!=NULL){
    CFRelease(_addressing);
    _addressing=NULL;
   }
   if(_request->_addressList==NULL){
    if(NSDebugEnabled)
     NSLog(@"Host %@ did not resolve",_name);
   }
   else {
    int i;

    _addressing=CFArrayCreateMutable(NULL,0,&kCFTypeArrayCallBacks);

    struct addrinfo *check=_request->_addressList,*next;
    for(;check!=NULL;check=next){
     next=check->ai_next;

     CFDataRef data=CFDataCreate(NULL,(void *)check->ai_addr,check->ai_addrlen);

     CFArrayAppendValue(_addressing,data);
     CFRelease(data);
    }
   }
   if(_request->_addressList!=NULL)
    any_freeaddrinfo(_request->_addressList);
   NSZoneFree(NULL,_request->_name);
   NSZoneFree(NULL,_request);
   _request=NULL;

   if(_callback!=NULL)
    _callback(self,kCFHostAddresses,NULL,_context.info);
}

-(void)handleMonitorIndicatesAbandoned:(NSHandleMonitor_win32 *)monitor {
}

static pthread_mutex_t              asyncCreationLock=PTHREAD_MUTEX_INITIALIZER;
static CFAddressResolverThreadInfo *asyncInfo;

static CFAddressResolverThreadInfo *startResolverThreadIfNeeded(){
   pthread_mutex_lock(&asyncCreationLock);

  if(asyncInfo==NULL){
   asyncInfo=NSZoneMalloc(NULL,sizeof(CFAddressResolverThreadInfo));

   InitializeCriticalSection(&(asyncInfo->queueLock));
   asyncInfo->queueEvent=CreateEvent(NULL,FALSE,FALSE,NULL);

   asyncInfo->queueCapacity=1;
   asyncInfo->queueCount=0;
   asyncInfo->queue=NSZoneMalloc(NULL,sizeof(CFHostRequest *)*asyncInfo->queueCapacity);

   unsigned threadAddr;
   _beginthreadex(NULL,0,addressResolverThread,asyncInfo,0,&threadAddr);
  }
  pthread_mutex_unlock(&asyncCreationLock);

  return asyncInfo;
}

#if 1
#define SYNCHRONOUS 0
#else
#warning disable
#define SYNCHRONOUS 1
#endif

static void queueHostToAddressResolver(CFHostRef host){
   if(SYNCHRONOUS){
    int addressCount=0;
    struct addrinfo *addressList=blockingRequest(host->_request);

    host->_request->_state=CFHostRequestDone;
    host->_request->_addressList=addressList;

    SetEvent(host->_request->_event);
   }
   else {
   CFAddressResolverThreadInfo *info=startResolverThreadIfNeeded();

   EnterCriticalSection(&(info->queueLock));
   if(info->queueCount+1>=info->queueCapacity){
    info->queueCapacity*=2;
    info->queue=NSZoneRealloc(NULL,info->queue,sizeof(CFHostRequest *)*info->queueCapacity);
   }
   info->queue[info->queueCount++]=host->_request;
   LeaveCriticalSection(&(info->queueLock));

   SetEvent(info->queueEvent);
   }
}

static void cancelHostInAddressResolverIfNeeded(CFHostRef self){

   if(self->_request==NULL)
    return;

   if(SYNCHRONOUS){
   }
   else {
   CFAddressResolverThreadInfo *info;

   if((info=asyncInfo)==NULL)
    return;

   EnterCriticalSection(&(info->queueLock));

    if(self->_request->_state==CFHostRequestInProgress){
     self->_request->_state=CFHostRequestDeallocate;
     self->_request=NULL;
    }
    else {
     int i;

     for(i=0;i<info->queueCount;i++)
      if(info->queue[i]==self->_request){
       info->queueCount--;
       for(;i<info->queueCount;i++)
        info->queue[i]=info->queue[i+1];
       break;
      }
     }

   LeaveCriticalSection(&(info->queueLock));

   if(self->_request!=NULL){
    NSZoneFree(NULL,self->_request->_name);
    NSZoneFree(NULL,self->_request);
    self->_request=NULL;
   }
   }
}

#else
static void queueHostToAddressResolver(CFHostRef host){
}
static void cancelHostInAddressResolverIfNeeded(CFHostRef host){
}
#endif

CFTypeID CFHostGetTypeID() {
   NSUnimplementedFunction();
   return 0;
}

CFHostRef  CFHostCreateCopy(CFAllocatorRef alloc,CFHostRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFHostRef  CFHostCreateWithAddress(CFAllocatorRef allocator,CFDataRef address) {
   NSUnimplementedFunction();
   return 0;
}


CFHostRef CFHostCreateWithName(CFAllocatorRef allocator, CFStringRef name)
{
    CFHostRef result = (CFHostRef)[__CFHost allocWithZone:NULL];

    result->_name = CFStringCreateCopy(allocator, name);

    return result;
}


-(void)dealloc {
   CFRelease(_name);
   if(self->_context.info!=NULL && self->_context.release!=NULL)
    self->_context.release(self->_context.info);
   CFRelease(_addressing);
#ifdef WINDOWS
   if(self->_event!=NULL)
    CloseHandle(self->_event);

   [self->_monitor setDelegate:nil];
   [self->_monitor invalidate];
   [self->_monitor release];
#endif
   [super dealloc];
}

CFArrayRef CFHostGetAddressing(CFHostRef self,Boolean *hasBeenResolved) {
   if(hasBeenResolved!=NULL)
    *hasBeenResolved=self->_hasResolvedAddressing;

   return self->_addressing;
}

CFArrayRef CFHostGetNames(CFHostRef self,Boolean *hasBeenResolved) {
   NSUnimplementedFunction();
   return 0;
}

CFDataRef  CFHostGetReachability(CFHostRef self,Boolean *hasBeenResolved) {
   NSUnimplementedFunction();
   return 0;
}

Boolean    CFHostSetClient(CFHostRef self,CFHostClientCallBack callback,CFHostClientContext *context) {
   if(self->_context.info!=NULL && self->_context.release!=NULL)
    self->_context.release(self->_context.info);

   self->_callback=callback;
   if(context!=NULL)
    self->_context=*context;
   else {
    self->_context.version=0;
    self->_context.info=NULL;
    self->_context.retain=NULL;
    self->_context.release=NULL;
    self->_context.copyDescription=NULL;
   }

   if(self->_callback!=NULL){
    if(self->_context.info!=NULL && self->_context.retain!=NULL)
     self->_context.info=(void *)self->_context.retain(self->_context.info);
   }

   return TRUE;
}

static void CFHostCreateEventIfNeeded(CFHostRef self){
#ifdef WINDOWS
   if(self->_event==NULL){
    self->_event=CreateEvent(NULL,FALSE,FALSE,NULL);
    self->_monitor=[[NSHandleMonitor_win32 handleMonitorWithHandle:self->_event] retain];
    [self->_monitor setDelegate:self];
    [self->_monitor setCurrentActivity:Win32HandleSignaled];
   }
#endif
}

Boolean CFHostStartInfoResolution(CFHostRef self,CFHostInfoType infoType,CFStreamError *streamError) {

   switch(infoType){

    case kCFHostAddresses:
     if(self->_hasResolvedAddressing){
      NSLog(@"CFHostStartInfoResolution, addressing already resolved");
      return TRUE;
     }
     if(self->_callback!=NULL){
      if(self->_request!=NULL){
       NSLog(@"CFHostStartInfoResolution already started");
       return FALSE;
      }
      char *cStringName=NSZoneMalloc(NULL,MAXHOSTNAMELEN+1);

// this encoding is probably wrong but CFStringGetCString can do it
      if(!CFStringGetCString(self->_name,cStringName,MAXHOSTNAMELEN,kCFStringEncodingISOLatin1)){
        NSLog(@"CFStringGetCString failed for CFHostRef name %@",self->_name);
        NSZoneFree(NULL,cStringName);
        return FALSE;
      }

      self->_request=NSZoneMalloc(NULL,sizeof(CFHostRequest));
      self->_request->_state=CFHostRequestInQueue;
      self->_request->_name=cStringName;
      self->_request->_addressList=NULL;
      CFHostCreateEventIfNeeded(self);
#ifdef WINDOWS
      self->_request->_event=self->_event;
#endif

      queueHostToAddressResolver(self);
      return TRUE;
     }
     else {
      NSUnimplementedFunction();
      return FALSE;
     }

    case kCFHostNames:
     NSUnimplementedFunction();
     return FALSE;

    case kCFHostReachability:
     NSUnimplementedFunction();
     return FALSE;

    default:
     [NSException raise:NSInvalidArgumentException format:@"CFHostStartInfoResolution CFHostInfoType is not valid (%d)",infoType];
     return FALSE;
   }
}

void CFHostCancelInfoResolution(CFHostRef self,CFHostInfoType infoType) {
   switch(infoType){

    case kCFHostAddresses:
     cancelHostInAddressResolverIfNeeded(self);
     break;

    case kCFHostNames:
     NSUnimplementedFunction();
     break;

    case kCFHostReachability:
     NSUnimplementedFunction();
     break;

    default:
     [NSException raise:NSInvalidArgumentException format:@"CFHostCancelInfoResolution CFHostInfoType is not valid (%d)",infoType];
     break;
   }
}

void CFHostScheduleWithRunLoop(CFHostRef self,CFRunLoopRef runLoop,CFStringRef mode) {
   if(runLoop!=CFRunLoopGetCurrent())
    NSUnimplementedFunction();

   CFHostCreateEventIfNeeded(self);
#ifdef WINDOWS
   [(NSRunLoop *)runLoop addInputSource:self->_monitor forMode:(NSString *)mode];
#endif
}

void CFHostUnscheduleFromRunLoop(CFHostRef self,CFRunLoopRef runLoop,CFStringRef mode) {
   if(runLoop!=CFRunLoopGetCurrent())
     NSUnimplementedFunction();

#ifdef WINDOWS
   [(NSRunLoop *)runLoop removeInputSource:self->_monitor forMode:(NSString *)mode];
#endif
}

@end

