#ifndef	INCLUDED_GSURLSESSIONTASKBODYSOURCE_H
#define	INCLUDED_GSURLSESSIONTASKBODYSOURCE_H

#import "common.h"
#import "GSDispatch.h"

@class	NSFileHandle;
@class	NSInputStream;

typedef NS_ENUM(NSUInteger, GSBodySourceDataChunk) {
    GSBodySourceDataChunkData,
    // The source is depleted.
    GSBodySourceDataChunkDone,
    // Retry later to get more data.
    GSBodySourceDataChunkRetryLater,
    GSBodySourceDataChunkError
};

/*
 * A (non-blocking) source for body data.
 */
@protocol GSURLSessionTaskBodySource <NSObject>

/*
 * Get the next chunck of data.
 */
- (void) getNextChunkWithLength: (NSInteger)length
              completionHandler: (void (^)(GSBodySourceDataChunk chunk, NSData *data))completionHandler;

@end

@interface GSBodyStreamSource : NSObject <GSURLSessionTaskBodySource>

- (instancetype) initWithInputStream: (NSInputStream*)inputStream;

@end

@interface GSBodyDataSource : NSObject <GSURLSessionTaskBodySource>

- (instancetype)initWithData:(NSData *)data;

@end

@interface GSBodyFileSource : NSObject <GSURLSessionTaskBodySource>

- (instancetype) initWithFileURL: (NSURL*)fileURL
                       workQueue: (dispatch_queue_t)workQueue
            dataAvailableHandler: (void (^)(void))dataAvailableHandler;

@end

#endif
