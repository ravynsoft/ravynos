#import <Foundation/Foundation.h>

@interface Foo: NSObject
+(int)foo;
+(int)bar;
@end

@implementation Foo

+(int)foo {
  return 0;
}

+(int)bar {
  return 0;
}
@end

int main() {
  printf("%d\n", [Foo foo]);
  printf("%d\n", [Foo bar]);
}
