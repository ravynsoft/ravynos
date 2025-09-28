/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSNibConnector.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@implementation NSNibConnector

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _source=[[keyed decodeObjectForKey:@"NSSource"] retain];
    _destination=[[keyed decodeObjectForKey:@"NSDestination"] retain];
    _label=[[keyed decodeObjectForKey:@"NSLabel"] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-(void)dealloc {
   [_source release];
   [_destination release];
   [_label release];
   [super dealloc];
}

-source {
   return _source;
}

-destination {
   return _destination;
}

-(NSString *)label {
   return _label;
}

-(void)setSource:source {
   source=[source retain];
   [_source release];
   _source=source;
}

-(void)setDestination:destination {
   destination=[destination retain];
   [_destination release];
   _destination=destination;
}

-(void)setLabel:(NSString *)label {
   label=[label copy];
   [_label release];
   _label=label;
}

-(void)replaceObject:original withObject:replacement {
   if(original==_source)
    [self setSource:replacement];
   if(original==_destination)
    [self setDestination:replacement];
}

-(void)establishConnection {
   NSInvalidAbstractInvocation();
}

@end
