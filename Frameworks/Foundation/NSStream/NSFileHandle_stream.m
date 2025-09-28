#import "NSFileHandle_stream.h"
#import "NSInputStream_socket.h"
#import "NSOutputStream_socket.h"
#import <Foundation/NSException.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotificationCenter.h>

enum {
 NSFileHandleStateNone,
 NSFileHandleStateRead,
 NSFileHandleStateReadToEndOfFile,
 NSFileHandleStateAccept,
 NSFileHandleStateWait
};

@implementation NSFileHandle_stream

-initWithSocket:(NSSocket *)socket closeOnDealloc:(BOOL)closeOnDealloc {
   _inputStream=[[NSInputStream_socket alloc] initWithSocket:socket streamStatus:NSStreamStatusOpen];
   [_inputStream setDelegate:self];
   _outputStream=[[NSOutputStream_socket alloc] initWithSocket:socket streamStatus:NSStreamStatusOpen];
   [_outputStream setDelegate:self];
   _closeOnDealloc=closeOnDealloc;
   _asyncState=NSFileHandleStateNone;
   return self;
}

-(void)dealloc {
   if(_closeOnDealloc)
    [self closeFile];

   [_inputStream setDelegate:nil];
   [_inputStream release];
   [_outputStream setDelegate:nil];
   [_outputStream release];
   [_modes release];
   [_endOfFileBuffer release];
   [super dealloc];
}

-(int)fileDescriptor {
   if([_inputStream respondsToSelector:@selector(fileDescriptor)])
    return [(id)_inputStream fileDescriptor];

   if([_outputStream respondsToSelector:@selector(fileDescriptor)])
    return [(id)_outputStream fileDescriptor];

   return -1;
}

-(void)closeFile {
   [_inputStream close];
   [_outputStream close];
}

-(void)synchronizeFile {
   [NSException raise:NSFileHandleOperationException format:@"-[%@ %s]: Operation not supported",isa,sel_getName(_cmd)];
}

-(uint64_t)offsetInFile {
   [NSException raise:NSFileHandleOperationException format:@"-[%@ %s]: Illegal seek",isa,sel_getName(_cmd)];
   return 0;
}

-(void)seekToFileOffset:(uint64_t)offset {
   [NSException raise:NSFileHandleOperationException format:@"-[%@ %s]: Illegal seek",isa,sel_getName(_cmd)];
}

-(uint64_t)seekToEndOfFile {
   [NSException raise:NSFileHandleOperationException format:@"-[%@ %s]: Illegal seek",isa,sel_getName(_cmd)];
   return 0;
}

-(NSData *)readDataOfLength:(NSUInteger)length {
   void   *bytes=NSZoneMalloc(NULL,length);
   NSInteger bytesRead=[_inputStream read:bytes maxLength:length];

// FIX, should raise exception
   if(bytesRead==-1)
    return nil;

   return [NSData dataWithBytesNoCopy:bytes length:bytesRead freeWhenDone:YES];
}

-(NSData *)readDataToEndOfFile {
   NSMutableData *result=[NSMutableData data];
   NSData        *chunk;

   while([(chunk=[self readDataOfLength:8192]) length]>0)
    [result appendData:chunk];

   return result;
}

-(NSData *)availableData {
   return [self readDataOfLength:8192];
}


- (void)writeData:(NSData *)data
{
    NSInteger check = [_outputStream write:[data bytes] maxLength:[data length]];

    if (check != [data length]) {
        // FIX, should raise exception
    }
}


-(void)truncateFileAtOffset:(uint64_t)offset {
   [self doesNotRecognizeSelector:_cmd];
}

-(void)_setAsyncState:(int)state forModes:(NSArray *)modes {
   NSInteger i,count;

// NSFileHandle will retain itself if asked to do an async activity
   if(_asyncState==NSFileHandleStateNone && state!=NSFileHandleStateNone)
    [self retain];
   else if(state==NSFileHandleStateNone && _asyncState!=NSFileHandleStateNone)
    [self autorelease];

   _asyncState=state;

   count=[_modes count];
   for(i=0;i<count;i++)
    [_inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:[_modes objectAtIndex:i]];

   [_modes release];
   _modes=[modes copy];

   count=[_modes count];
   for(i=0;i<count;i++)
    [_inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:[_modes objectAtIndex:i]];
}


-(void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)streamEvent {
   if(stream==_inputStream){
    NSString     *notificationName=nil;
    NSDictionary *userInfo=nil;

    int eventState=_asyncState;


    switch(eventState){

     case NSFileHandleStateNone:
      // do nothing, shouldn't get here
      break;

     case NSFileHandleStateRead:{
       NSData *data=[self availableData];

       notificationName=NSFileHandleReadCompletionNotification;
       // FIX, need error key too
       userInfo=[NSDictionary dictionaryWithObject:data forKey:NSFileHandleNotificationDataItem];
       [self _setAsyncState:NSFileHandleStateNone forModes:nil];
      }
      break;

     case NSFileHandleStateReadToEndOfFile:{
       NSData *data=[self availableData];

       if([data length]>0){
        if(_endOfFileBuffer==nil)
         _endOfFileBuffer=[NSMutableData new];

        [_endOfFileBuffer appendData:data];
       }
       else {
        notificationName=NSFileHandleReadToEndOfFileCompletionNotification;
        // FIX, need error key too
        userInfo=[NSDictionary dictionaryWithObject:_endOfFileBuffer forKey:NSFileHandleNotificationDataItem];
        [_endOfFileBuffer release];
        _endOfFileBuffer=nil;

        [self _setAsyncState:NSFileHandleStateNone forModes:nil];
       }
      }
      break;

     case NSFileHandleStateAccept:{
       NSError  *error;
       NSSocket *socket=[[(NSInputStream_socket *)_inputStream socket] acceptWithError:&error];

       notificationName=NSFileHandleConnectionAcceptedNotification;

       if(socket==nil){
        // FIX, need error key
       }
       else {
        NSFileHandle *other=[[[NSFileHandle_stream alloc] initWithSocket:socket closeOnDealloc:YES] autorelease];

        userInfo=[NSDictionary dictionaryWithObject:other forKey:NSFileHandleNotificationFileHandleItem];
       }
       [self _setAsyncState:NSFileHandleStateNone forModes:nil];
      }
      break;

     case NSFileHandleStateWait:
      notificationName=NSFileHandleDataAvailableNotification;
      [self _setAsyncState:NSFileHandleStateNone forModes:nil];
      break;

    }

    if(notificationName!=nil)
     [[NSNotificationCenter defaultCenter] postNotificationName:notificationName object:self userInfo:userInfo];

   }
}

-(void)readInBackgroundAndNotifyForModes:(NSArray *)modes {
   [self _setAsyncState:NSFileHandleStateRead forModes:modes];
}

-(void)readToEndOfFileInBackgroundAndNotifyForModes:(NSArray *)modes {
   [self _setAsyncState:NSFileHandleStateReadToEndOfFile forModes:modes];
}

-(void)acceptConnectionInBackgroundAndNotifyForModes:(NSArray *)modes {
   [self _setAsyncState:NSFileHandleStateAccept forModes:modes];
}

-(void)waitForDataInBackgroundAndNotifyForModes:(NSArray *)modes {
   [self _setAsyncState:NSFileHandleStateWait forModes:modes];
}

@end
