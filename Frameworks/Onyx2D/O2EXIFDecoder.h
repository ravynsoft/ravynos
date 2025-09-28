//
//  O2EXIFDecoder.h
//  AppKit
//
//  Created by Airy ANDRE on 22/03/13.
//
//

#import <Foundation/Foundation.h>

@interface O2EXIFDecoder : NSObject {
    NSMutableDictionary *_tags;
}
- (id)initWithBytes:(const uint8_t *)bytes length:(size_t)length;
- (NSMutableDictionary *)tags;
@end
