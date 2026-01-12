#import <CoreFoundation/CFSocket.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSSocket.h>
#import <Foundation/NSCFTypeID.h>

#define NSSocketCast(_) ((NSSocket *)_)

@implementation NSSocket(CFSocket)

+(CFSocketRef)createWithAllocator:(CFAllocatorRef)allocator protocolFamily:(CFInteger)protocolFamily type:(CFInteger)socketType protocol:(CFInteger)protocol options:(CFOptionFlags)flags callback:(CFSocketCallBack)callback context:(const CFSocketContext *)context {
   NSInvalidAbstractInvocation();
   return 0;
}

+(CFSocketRef)createConnectedWithAllocator:(CFAllocatorRef)allocator signature:(const CFSocketSignature *)signature options:(CFOptionFlags)flags callback:(CFSocketCallBack)callback context:(const CFSocketContext *)context timeout:(CFTimeInterval)timeout {
   NSInvalidAbstractInvocation();
   return 0;
}

+(CFSocketRef)createWithAllocator:(CFAllocatorRef)allocator native:(CFSocketNativeHandle)native options:(CFOptionFlags)flag callback:(CFSocketCallBack)callback context:(const CFSocketContext *)context {
   NSInvalidAbstractInvocation();
   return 0;
}

+(CFSocketRef)createWithAllocator:(CFAllocatorRef)allocator signature:(const CFSocketSignature *)signature options:(CFOptionFlags)flags callback:(CFSocketCallBack)callback context:(const CFSocketContext *)context {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFSocketError)connectToAddress:(CFDataRef)address timeout:(CFTimeInterval)timeout {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFDataRef)copyAddress {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFDataRef)copyPeerAddress {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFRunLoopSourceRef)createRunLoopSourceWithAllocator:(CFAllocatorRef)allocator order:(CFIndex)order {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)disableCallBacks:(CFOptionFlags)mask {
   NSInvalidAbstractInvocation();
}

-(void)enableCallBacks:(CFOptionFlags)mask {
   NSInvalidAbstractInvocation();
}

-(void)getContext:(CFSocketContext *)context {
   NSInvalidAbstractInvocation();
}

-(CFSocketNativeHandle)native {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFOptionFlags)socketFlags {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)invalidate {
   NSInvalidAbstractInvocation();
}

-(Boolean)isValid {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFSocketError)sendToAddress:(CFDataRef)address data:(CFDataRef)data timeout:(CFTimeInterval)timeout {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CFSocketError)setAddress:(CFDataRef)address {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)setSocketFlags:(CFOptionFlags)flags {
   NSInvalidAbstractInvocation();
}

CFTypeID CFSocketGetTypeID() {
   return kNSCFTypeSocket;
}

CFSocketRef CFSocketCreate(CFAllocatorRef allocator,CFInteger protocolFamily,CFInteger socketType,CFInteger protocol,CFOptionFlags flags,CFSocketCallBack callback,const CFSocketContext *context) {
   return [NSSocket createWithAllocator:allocator protocolFamily:protocolFamily type:socketType protocol:protocol options:flags callback:callback context:context];
}

CFSocketRef CFSocketCreateConnectedToSocketSignature(CFAllocatorRef allocator,const CFSocketSignature *signature,CFOptionFlags flags,CFSocketCallBack callback,const CFSocketContext *context,CFTimeInterval timeout) {
   return [NSSocket createConnectedWithAllocator:allocator signature:signature options:flags callback:callback context:context timeout:timeout];
}

CFSocketRef CFSocketCreateWithNative(CFAllocatorRef allocator,CFSocketNativeHandle native,CFOptionFlags flags,CFSocketCallBack callback,const CFSocketContext *context) {
   return [NSSocket createWithAllocator:allocator native:native options:flags callback:callback context:context];
}

CFSocketRef CFSocketCreateWithSocketSignature(CFAllocatorRef allocator,const CFSocketSignature *signature,CFOptionFlags flags,CFSocketCallBack callback,const CFSocketContext *context) {
   return [NSSocket createWithAllocator:allocator signature:signature options:flags callback:callback context:context];
}

CFSocketError CFSocketConnectToAddress(CFSocketRef self,CFDataRef address,CFTimeInterval timeout) {
   return [NSSocketCast(self) connectToAddress:address timeout:timeout];
}

CFDataRef CFSocketCopyAddress(CFSocketRef self) {
   return [NSSocketCast(self) copyPeerAddress];
}

CFDataRef CFSocketCopyPeerAddress(CFSocketRef self) {
   return [NSSocketCast(self) copyPeerAddress];
}

CFRunLoopSourceRef CFSocketCreateRunLoopSource(CFAllocatorRef allocator,CFSocketRef self,CFIndex order) {
   return [NSSocketCast(self) createRunLoopSourceWithAllocator:allocator order:order];
}

void CFSocketDisableCallBacks(CFSocketRef self,CFOptionFlags flags) {
   [NSSocketCast(self) disableCallBacks:flags];
}

void CFSocketEnableCallBacks(CFSocketRef self,CFOptionFlags flags) {
   [NSSocketCast(self) enableCallBacks:flags];
}

void CFSocketGetContext(CFSocketRef self,CFSocketContext *context) {
   [NSSocketCast(self) getContext:context];
}

CFSocketNativeHandle CFSocketGetNative(CFSocketRef self) {
   return [NSSocketCast(self) native];
}

CFOptionFlags CFSocketGetSocketFlags(CFSocketRef self) {
   return [NSSocketCast(self) socketFlags];
}

void CFSocketInvalidate(CFSocketRef self) {
   [NSSocketCast(self) invalidate];
}

Boolean CFSocketIsValid(CFSocketRef self) {
   return [NSSocketCast(self) isValid];
}

CFSocketError CFSocketSendData(CFSocketRef self,CFDataRef address,CFDataRef data,CFTimeInterval timeout) {
   [NSSocketCast(self) sendToAddress:address data:data timeout:timeout];
   return 0;
}

CFSocketError CFSocketSetAddress(CFSocketRef self,CFDataRef address) {
   [NSSocketCast(self) setAddress:address];
   return 0;
}

void CFSocketSetSocketFlags(CFSocketRef self,CFOptionFlags flags) {
   [NSSocketCast(self) setSocketFlags:flags];
}

@end
