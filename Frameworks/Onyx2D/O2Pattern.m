/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Pattern.h>

@implementation O2Pattern

-initWithInfo:(void *)info bounds:(O2Rect)bounds matrix:(O2AffineTransform)matrix xstep:(O2Float)xstep ystep:(O2Float)ystep tiling:(O2PatternTiling)tiling isColored:(BOOL)isColored callbacks:(const O2PatternCallbacks *)callbacks {
   _info=info;
   _bounds=bounds;
   _matrix=matrix;
   _xstep=xstep;
   _ystep=ystep;
   _tiling=tiling;
   _isColored=isColored;
   _callbacks=*callbacks;
   return self;
}

-(void)dealloc {
   if(_info!=NULL && _callbacks.releaseInfo!=NULL)
    _callbacks.releaseInfo(_info);
   [super dealloc];
}

-(O2Rect)bounds {
   return _bounds;
}

-(O2AffineTransform)matrix {
	return _matrix;
}

-(O2Float)xstep {
	return _xstep;
}

-(O2Float)ystep {
	return _ystep;
}

-(void)drawInContext:(O2ContextRef)context {
   if(_callbacks.drawPattern!=NULL)
    _callbacks.drawPattern(_info,context);
}

O2PatternRef O2PatternCreate(void *info,O2Rect bounds,O2AffineTransform matrix,O2Float xStep,O2Float yStep,O2PatternTiling tiling,bool isColored,const O2PatternCallbacks *callbacks) {
   return [[O2Pattern alloc] initWithInfo:info bounds:bounds matrix:matrix xstep:xStep ystep:yStep tiling:tiling isColored:isColored callbacks:callbacks];
}

O2PatternRef O2PatternRetain(O2PatternRef self) {
   return [self retain];
}

void O2PatternRelease(O2PatternRef self) {
   [self release];
}

@end
