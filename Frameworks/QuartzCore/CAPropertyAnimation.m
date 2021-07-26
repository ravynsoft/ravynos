#import <QuartzCore/CAAnimation.h>

@implementation CAPropertyAnimation

-initWithKeyPath:(NSString *)keyPath {
   [super init];
   _keyPath=[keyPath copy];
   _additive=NO;
   _cumulative=NO;
   return self;
}

+animationWithKeyPath:(NSString *)keyPath {
   return [[[self alloc] initWithKeyPath:keyPath] autorelease];
}

-(NSString *)keyPath {
   return _keyPath;
}

-(void)setKeyPath:(NSString *)value {
   value=[value copy];
   [_keyPath release];
   _keyPath=value;
}

-(BOOL)isAdditive {
   return _additive;
}

-(void)setAdditive:(BOOL)value {
   _additive=value;
}

-(BOOL)isCumulative {
   return _cumulative;
}

-(void)setCumulative:(BOOL)value {
   _cumulative=value;
}

@end
