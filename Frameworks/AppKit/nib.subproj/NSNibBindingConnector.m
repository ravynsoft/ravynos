/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSNibBindingConnector.h"
#import "../NSKeyValueBinding/NSObject+BindingSupport.h"

@implementation NSNibBindingConnector
-(void)dealloc
{
	[_binding release];
	[_keyPath release];
	[_options release];
	[super dealloc];
}

-(id)initWithCoder:(NSCoder *)coder
{
	if((self = [super initWithCoder:coder]))
	{
		int version=[coder decodeIntForKey:@"NSNibBindingConnectorVersion"];
		if(version != 2)
			[NSException raise:NSInvalidArgumentException format:@"-[%@ %s] unknown connector version %i",isa,sel_getName(_cmd), version];
		
		_binding=[[coder decodeObjectForKey:@"NSBinding"] retain];
		_keyPath=[[coder decodeObjectForKey:@"NSKeyPath"] retain];
		_options=[[coder decodeObjectForKey:@"NSOptions"] retain];
	}
	else {
		[NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
	}
	return self;
}

-(void)establishConnection
{
	//NSLog(@"binding between %@.%@ and %@.%@ options=%@", [_source className], _binding, [_destination className], _keyPath,_options);

	[_source bind:_binding toObject:_destination withKeyPath:_keyPath options:_options];
}
@end
