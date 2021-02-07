#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

@interface TestBundle : NSObject
{

}
-(NSString *)test;
@end

@implementation TestBundle
-(NSString *)test
{
  return @"Something"; 
}
@end
