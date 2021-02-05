#import "NSKeyObserver.h"
#import <Foundation/NSString.h>

@implementation NSKeyObserver

-initWithObject:object key:(NSString *)key keyPathObserver:(NSKeyPathObserver *)keyPathObserver restOfPath:(NSString *)restOfPath {
   _object=object;
   _key=[key copy];
   _keyPathObserver=[keyPathObserver retain];
   _branchPath=[restOfPath copy];
   _isValid=YES;
   return self;
}

-(void)dealloc {
   [_key release];
   [_keyPathObserver release];
   [_branchPath release];
   [_branchObserver release];
   [_dependantKeyObservers release];
   [super dealloc];
}

-(BOOL)isValid {
   return _isValid;
}

-(void)invalidate {
   _isValid=NO;
}

-object {
   return _object;
}

-(NSString *)key {
   return _key;
}

-(NSKeyPathObserver *)keyPathObserver {
   return _keyPathObserver;
}

-(NSString *)restOfPath {
   return _branchPath;
}

-(NSKeyObserver *)restOfPathObserver {
   return _branchObserver;
}

-(void)setRestOfPathObserver:(NSKeyObserver *)keyObserver {
   keyObserver=[keyObserver retain];
   [_branchObserver release];
   _branchObserver=keyObserver;
}

-(NSArray *)dependantKeyObservers {
   return _dependantKeyObservers;
}

-(void)setDependantKeyObservers:(NSArray *)keyObservers {
   keyObservers=[keyObservers retain];
   
   [_dependantKeyObservers release];
   _dependantKeyObservers=keyObservers;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %x _object: %@ _key: %@ _branchPath: %@>",isa,self,_object,_key,_branchPath];
}

@end
