#import <Foundation/Foundation.h>

@interface Foo: NSObject
+(int)aaa;
+(int)aab;
+(int)aac;

+(int)aa;
+(int)ab;
+(int)ac;
+(int)ad;

@end

@implementation Foo

+(int)aaa {
  return 0;
}

+(int)aab {
  return 0;
}

+(int)aac {
  return 0;
}

+(int)aa {
  return 0;
}

+(int)ab {
  return 0;
}

+(int)ac {
  return 0;
}

+(int)ad {
  return 0;
}

@end

int main() {
  printf("%d\n", [Foo aaa]);
  printf("%d\n", [Foo aab]);
  printf("%d\n", [Foo aac]);
  printf("%d\n", [Foo aa]);
  printf("%d\n", [Foo ab]);
  printf("%d\n", [Foo ac]);
  printf("%d\n", [Foo ad]);
}
