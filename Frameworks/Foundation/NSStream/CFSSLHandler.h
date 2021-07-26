#import <Foundation/NSObject.h>
#import <CoreFoundation/CFDictionary.h>

@class NSSocket;

@interface CFSSLHandler : NSObject {
}

- initWithProperties:(CFDictionaryRef)properties;

- (BOOL)isHandshaking;

- (NSInteger)writePlaintext:(const uint8_t *)buffer maxLength:(NSUInteger)length;
- (NSInteger)writeBytesAvailable;
- (BOOL)wantsMoreIncoming;
- (NSInteger)readEncrypted:(uint8_t *)buffer maxLength:(NSUInteger)length;

- (NSInteger)writeEncrypted:(const uint8_t *)buffer maxLength:(NSUInteger)length;
- (NSInteger)readBytesAvailable;
- (NSInteger)readPlaintext:(uint8_t *)buffer maxLength:(NSUInteger)length;

- (NSInteger)transferOneBufferFromSSLToSocket:(NSSocket *)socket;
- (NSInteger)transferOneBufferFromSocketToSSL:(NSSocket *)socket;

- (void)runHandshakeIfNeeded:(NSSocket *)socket;
- (void)runWithSocket:(NSSocket *)socket;

@end
