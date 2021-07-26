/* Copyright (c) 2008 John Engelhart

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSAssertionHandler.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSString.h>

@implementation NSAssertionHandler

+ (NSAssertionHandler *)currentHandler
{
  id currentHandlerForThread = [[[NSThread currentThread] threadDictionary] objectForKey:[self className]];

  if((currentHandlerForThread == NULL) && ((currentHandlerForThread = [[self alloc] init]) != NULL)) {
    [[[NSThread currentThread] threadDictionary] setObject:currentHandlerForThread forKey:[self className]];
  }
  
  return(currentHandlerForThread);
}

- (void)handleFailureInMethod:(SEL)selector object:(id)object file:(NSString *)fileName lineNumber:(NSInteger)line description:(NSString *)format,...
{
  NSLog(@"*** Assertion failure in %c[%@ %@], %@:%ld", (object == [object class]) ? '+' : '-', [object className], NSStringFromSelector(selector), fileName, (long)line);

  va_list arguments;  
  va_start(arguments, format);
  [NSException raise:NSInternalInconsistencyException format:format arguments:arguments];
  va_end(arguments);
}

- (void)handleFailureInFunction:(NSString *)functionName file:(NSString *)fileName lineNumber:(NSInteger)line description:(NSString *)format,...
{
  NSLog(@"*** Assertion failure in %@, %@:%ld", functionName, fileName, (long)line);

  va_list arguments;
  va_start(arguments, format);
  [NSException raise:NSInternalInconsistencyException format:format arguments:arguments];
  va_end(arguments);  
}

@end

