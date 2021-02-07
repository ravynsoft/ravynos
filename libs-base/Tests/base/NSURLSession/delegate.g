#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

@interface      MyDelegate : NSObject <NSURLSessionDelegate>
{
  BOOL                  _finished;
  NSMutableArray        *_order;
  NSURLResponse         *_response;
  NSData                *_taskData;
  NSString              *_taskText;
  NSError               *_taskError;
}
- (BOOL) finished;
- (void) reset;
@end

@implementation MyDelegate
- (void) dealloc
{
  [self reset];
  [super dealloc];
}

- (BOOL) finished
{
  return _finished;
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      _order = [NSMutableArray new];
    }
  return self;
}

- (NSMutableArray*) order
{
  return _order;
}

- (void) reset
{
  DESTROY(_order);
  DESTROY(_response);
  DESTROY(_taskData);
  DESTROY(_taskError);
  DESTROY(_taskText);
  _finished = NO;
}

- (NSURLResponse*) response
{
  return _response;
}
- (NSData*) taskData
{
  return _taskData;
}
- (NSError*) taskError
{
  return _taskError;
}
- (NSString*) taskText
{
  return _taskText;
}

- (void) URLSession: (NSURLSession*)session
           dataTask: (NSURLSessionDataTask*)dataTask
 didReceiveResponse: (NSURLResponse*)response
  completionHandler: (void (^)(NSURLSessionResponseDisposition disposition))completionHandler
{
  [_order addObject: NSStringFromSelector(_cmd)];
  ASSIGN(_response, response);
  completionHandler(NSURLSessionResponseAllow);
}
 
- (void) URLSession: (NSURLSession*)session
           dataTask: (NSURLSessionDataTask*)dataTask
     didReceiveData: (NSData*)data
{
  [_order addObject: NSStringFromSelector(_cmd)];

  NSString      *text;

  ASSIGN(_taskData, data);
  text = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
  if (nil != text)
    {
      ASSIGN(_taskText, text);
    }
  RELEASE(text);
}

- (void) URLSession: (NSURLSession*)session
               task: (NSURLSessionTask*)task
didCompleteWithError: (NSError*)error
{
  [_order addObject: NSStringFromSelector(_cmd)];
  _finished = YES;

  if (error == nil)
    {
      NSLog(@"Download is Succesfull");
    }
  else
    {
      NSLog(@"Error %@", [error userInfo]);
    }
  ASSIGN(_taskError, error);
}
@end

