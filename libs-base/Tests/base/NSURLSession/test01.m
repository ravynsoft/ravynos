#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

#if GS_HAVE_NSURLSESSION
@interface      MyDelegate : NSObject <NSURLSessionDelegate>
{
@public
  BOOL          responseCompletion;
  BOOL          didComplete;
  NSString      *taskText;
  NSError       *taskError;
}
@end

@implementation MyDelegate
- (void) dealloc
{
  RELEASE(taskText);
  RELEASE(taskError);
  [super dealloc];
}

- (void) URLSession: (NSURLSession*)session
           dataTask: (NSURLSessionDataTask*)dataTask
 didReceiveResponse: (NSURLResponse*)response
  completionHandler: (void (^)(NSURLSessionResponseDisposition disposition))completionHandler
{
  responseCompletion = YES; 
  if (NO == didComplete)
    {
      NSLog(@"### handler 1 before didComplete...");
    }
  else
    {
      NSLog(@"### handler 1 after didComplete...");
    }
  completionHandler(NSURLSessionResponseAllow);
}
 
- (void) URLSession: (NSURLSession*)session
           dataTask: (NSURLSessionDataTask*)dataTask
     didReceiveData: (NSData*)data
{
  NSString      *text;

  text = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
  if (nil == text)
    {
      NSLog(@"Received non-utf8 %@", data);
    }
  else
    {
      ASSIGN(taskText, text);
      NSLog(@"Received String %@", text);
    }
  RELEASE(text);
}
- (void) URLSession: (NSURLSession*)session
               task: (NSURLSessionTask*)task
didCompleteWithError: (NSError*)error
{
  if (error == nil)
    {
      NSLog(@"Download is Succesfull");
    }
  else
    {
      NSLog(@"Error %@", [error userInfo]);
    }
  didComplete = YES;
  ASSIGN(taskError, error);
}
@end
#endif

int main()
{
  START_SET("NSURLSession test01")

#if !GS_HAVE_NSURLSESSION
    SKIP("library built without NSURLSession support")
#else
  NSURLSessionConfiguration     *defaultConfigObject;
  NSURLSession                  *defaultSession;
  NSURLSessionDataTask          *dataTask;
  NSMutableURLRequest           *urlRequest;
  NSURL                         *url;
  NSOperationQueue              *mainQueue;
  NSString                      *params;
  MyDelegate                    *object;

  object = AUTORELEASE([MyDelegate new]);
  mainQueue = [NSOperationQueue mainQueue];
  defaultConfigObject = [NSURLSessionConfiguration defaultSessionConfiguration];
  defaultSession = [NSURLSession sessionWithConfiguration: defaultConfigObject
                                                 delegate: object
                                            delegateQueue: mainQueue];
  url = [NSURL URLWithString:
    @"http://localhost:12345/not-here"];
  urlRequest = [NSMutableURLRequest requestWithURL: url];
  [urlRequest setHTTPMethod: @"POST"];
  params = @"name=Ravi&loc=India&age=31&submit=true";
  [urlRequest setHTTPBody: [params dataUsingEncoding: NSUTF8StringEncoding]];
  if ([urlRequest respondsToSelector: @selector(setDebug:)])
    {
      [urlRequest setDebug: YES];
    }

  dataTask = [defaultSession dataTaskWithRequest: urlRequest];
  [dataTask resume];

  NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 60.0];
  while (object->didComplete == NO
    && [limit timeIntervalSinceNow] > 0.0)
    {
      ENTER_POOL
      NSDate    *when = [NSDate dateWithTimeIntervalSinceNow: 0.1];

      [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                               beforeDate: when];
      LEAVE_POOL
    }

  PASS(YES == object->didComplete, "request completed")
  PASS([object->taskError code] == NSURLErrorCannotConnectToHost,
    "unable to connect to host")

#endif
  END_SET("NSURLSession test01")
  return 0;
}
