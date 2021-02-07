#import "GSURLSessionTaskBodySource.h"
#import "Foundation/NSData.h"
#import "Foundation/NSStream.h"


@implementation GSBodyStreamSource
{
  NSInputStream  *_inputStream;
}

- (instancetype) initWithInputStream: (NSInputStream*)inputStream 
{
  if (nil != (self = [super init])) 
    {
      ASSIGN(_inputStream, inputStream);
      if ([_inputStream streamStatus] == NSStreamStatusNotOpen) 
        {
          [_inputStream open];
        }
  }

  return self;
}

- (void) dealloc
{
  DESTROY(_inputStream);
  [super dealloc];
}

- (void) getNextChunkWithLength: (NSInteger)length
              completionHandler: (void (^)(GSBodySourceDataChunk, NSData*))completionHandler 
{
  if (nil == completionHandler) 
    {
      return;
    }

  if (![_inputStream hasBytesAvailable]) 
    {
      completionHandler(GSBodySourceDataChunkDone, nil);
      return;
    }

  uint8_t   buffer[length];
  NSInteger readBytes;
  NSData    *data;
  
  readBytes = [_inputStream read: buffer maxLength: length];
  if (readBytes > 0) 
    {
      data = AUTORELEASE([[NSData alloc] initWithBytes: buffer 
                                                length: readBytes]);
      completionHandler(GSBodySourceDataChunkData, data);
    } 
  else if (readBytes == 0) 
    {
      completionHandler(GSBodySourceDataChunkDone, nil);
    } 
  else 
    {
      completionHandler(GSBodySourceDataChunkError, nil);
    }
}

@end

@implementation GSBodyDataSource
{
  NSData  *_data;
}

- (instancetype) initWithData: (NSData*)data 
{
    if (nil != (self = [super init]))
      {
        ASSIGN(_data, data);
      }
    return self;
}

- (void) dealloc
{
  DESTROY(_data);
  [super dealloc];
}

- (void) getNextChunkWithLength: (NSInteger)length
              completionHandler: (void (^)(GSBodySourceDataChunk, NSData*))completionHandler 
{
  if (nil == completionHandler) 
    {
      return;
    }
  
  NSUInteger remaining = [_data length];
  if (remaining == 0) 
    {
      completionHandler(GSBodySourceDataChunkDone, nil);
    } 
  else if (remaining <= length) 
    {
      NSData *r = AUTORELEASE([[NSData alloc] initWithData: _data]);
      DESTROY(_data);
      completionHandler(GSBodySourceDataChunkData, r);
    } 
  else 
    {
      NSData *chunk = [_data subdataWithRange: NSMakeRange(0, length)];
      NSData *remainder = [_data subdataWithRange: 
        NSMakeRange(length - 1, [_data length] - length)];
      ASSIGN(_data, remainder);
      completionHandler(GSBodySourceDataChunkData, chunk);
    }
}

@end

@implementation GSBodyFileSource
{
  NSURL             *_fileURL;
  dispatch_queue_t  _workQueue;
  void              (^_dataAvailableHandler)(void);
}

- (instancetype) initWithFileURL: (NSURL*)fileURL
                       workQueue: (dispatch_queue_t)workQueue
            dataAvailableHandler: (void (^)(void))dataAvailableHandler
{
  if (nil != (self = [super init]))
    {
      ASSIGN(_fileURL, fileURL);
      _workQueue = workQueue;
      _dataAvailableHandler = dataAvailableHandler;
    }

  return self;
}

- (void) dealloc
{
  DESTROY(_fileURL);
  [super dealloc];
}

- (void) getNextChunkWithLength: (NSInteger)length
              completionHandler: (void (^)(GSBodySourceDataChunk chunk, NSData *data))completionHandler
{
  //FIXME
}

@end
