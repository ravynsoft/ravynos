#import <Foundation/Foundation.h>

@interface Foo: NSObject
+(int)foo;
@end

@implementation Foo

+(int)foo {
  return 0;
}
@end

int main() {
  printf("%d\n", [Foo foo]);
}
