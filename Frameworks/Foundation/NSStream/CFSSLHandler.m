#import <Foundation/CFSSLHandler.h>

@implementation CFSSLHandler

-initWithProperties:(CFDictionaryRef )properties {

   return self;
}

-(BOOL)isHandshaking {
   return NO;
}

-(NSInteger)writePlaintext:(const uint8_t *)buffer maxLength:(NSUInteger)length {
   return -1;
}

-(NSInteger)writeBytesAvailable {
   return -1;
}

-(BOOL)wantsMoreIncoming {
   return NO;
}

-(NSInteger)readEncrypted:(uint8_t *)buffer maxLength:(NSUInteger)length {
   return -1;
}

-(NSInteger)writeEncrypted:(const uint8_t *)buffer maxLength:(NSUInteger)length {
   return -1;
}

-(NSInteger)readBytesAvailable {
   return -1;
}

-(NSInteger)readPlaintext:(uint8_t *)buffer maxLength:(NSUInteger)length {
   return -1;
}

-(NSInteger)transferOneBufferFromSSLToSocket:(NSSocket *)socket {
   return -1;
}

-(NSInteger)transferOneBufferFromSocketToSSL:(NSSocket *)socket {
   return -1;
}

-(void)runHandshakeIfNeeded:(NSSocket *)socket {
   return;
}

-(void)runWithSocket:(NSSocket *)socket {
   return;
}

@end
