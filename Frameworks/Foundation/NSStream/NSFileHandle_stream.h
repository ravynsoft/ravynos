#import <Foundation/NSFileHandle.h>

@class NSInputStream, NSOutputStream, NSSocket, NSMutableData;

@interface NSFileHandle_stream : NSFileHandle {
    NSInputStream *_inputStream;
    NSOutputStream *_outputStream;
    BOOL _closeOnDealloc;
    int _asyncState;
    NSArray *_modes;
    NSMutableData *_endOfFileBuffer;
}

- initWithSocket:(NSSocket *)socket closeOnDealloc:(BOOL)closeOnDealloc;

@end
