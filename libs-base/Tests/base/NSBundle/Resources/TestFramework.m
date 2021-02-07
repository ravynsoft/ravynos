#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

@interface TestFramework : NSObject
{

}
-(NSString *)test;
@end

@implementation TestFramework
-(NSString *)test
{
  return @"Something";
}
@end
