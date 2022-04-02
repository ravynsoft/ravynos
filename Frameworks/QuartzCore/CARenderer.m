#import <QuartzCore/CARenderer.h>
#import <QuartzCore/CALayer.h>
#import <QuartzCore/CAAnimation.h>
#import <QuartzCore/CAMediaTimingFunction.h>
#import <CoreVideo/CoreVideo.h>
#import <OpenGL/OpenGL.h>
#import <Onyx2D/O2Surface.h>

@interface CALayer(private)
-(void)_setContext:(CALayerContext *)context;
-(void)_setTextureId:(NSNumber *)value;
-(NSNumber *)_textureId;
@end

@implementation CARenderer

-(CGRect)bounds {
   return _bounds;
}

-(void)setBounds:(CGRect)value {
   _bounds=value;
}

@synthesize layer=_rootLayer;

-initWithCGLContext:(void *)cglContext options:(NSDictionary *)options {
   _cglContext=cglContext;
   _bounds=CGRectZero;
   _rootLayer=nil;
   return self;
}

+(CARenderer *)rendererWithCGLContext:(void *)cglContext options:(NSDictionary *)options {
   return [[[self alloc] initWithCGLContext:cglContext options:options] autorelease];
}

static void startAnimationsInLayer(CALayer *layer,CFTimeInterval currentTime){
   NSArray *keys=[layer animationKeys];
   
   for(NSString *key in keys){
    CAAnimation *check=[layer animationForKey:key];
    
    if([check beginTime]==0.0)
     [check setBeginTime:currentTime];
    if(currentTime>[check beginTime]+[check duration]){
     [layer removeAnimationForKey:key];
    }
    
   }
   
   for(CALayer *child in layer.sublayers)
    startAnimationsInLayer(child,currentTime);
}

-(void)beginFrameAtTime:(CFTimeInterval)currentTime timeStamp:(CVTimeStamp *)timeStamp {
   startAnimationsInLayer(_rootLayer,currentTime);
}

static inline float cubed(float value){
   return value*value*value;
}

static inline float squared(float value){
   return value*value;
}

static float applyMediaTimingFunction(CAMediaTimingFunction *function,float t){
   float result;
   float cp1[2];
   float cp2[2];
   
   [function getControlPointAtIndex:1 values:cp1];
   [function getControlPointAtIndex:2 values:cp2];
   
   double x=cubed(1.0-t)*0.0+3*squared(1-t)*t*cp1[0]+3*(1-t)*squared(t)*cp2[0]+cubed(t)*1.0;
   double y=cubed(1.0-t)*0.0+3*squared(1-t)*t*cp1[1]+3*(1-t)*squared(t)*cp2[1]+cubed(t)*1.0;
   
// this is wrong
   return y;
}

static float mediaTimingScale(CAAnimation *animation,CFTimeInterval currentTime){
   CFTimeInterval begin=[animation beginTime];
   CFTimeInterval duration=[animation duration];
   CFTimeInterval delta=currentTime-begin;
   double         zeroToOne=delta/duration;
   CAMediaTimingFunction *function=[animation timingFunction];
   
   if(function==nil)
    function=[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];

   return applyMediaTimingFunction(function,zeroToOne);
}

static float interpolateFloatInLayerKey(CALayer *layer,NSString *key,CFTimeInterval currentTime){
   CAAnimation *animation=[layer animationForKey:key];
   
   if(animation==nil)
    return [[layer valueForKey:key] floatValue];
   
   if([animation isKindOfClass:[CABasicAnimation class]]){
    CABasicAnimation *basic=(CABasicAnimation *)animation;
    
    id fromValue=[basic fromValue];
    id toValue=[basic toValue];
    
    if(toValue==nil)
     toValue=[layer valueForKey:key];
    
    float fromFloat=[fromValue floatValue];
    float toFloat=[toValue floatValue];
        
    float        resultFloat;
    double timingScale=mediaTimingScale(animation,currentTime);
    
    resultFloat=fromFloat+(toFloat-fromFloat)*timingScale;
        
    return resultFloat;
   }
   
   return 0;
}

static CGPoint interpolatePointInLayerKey(CALayer *layer,NSString *key,CFTimeInterval currentTime){
   CAAnimation *animation=[layer animationForKey:key];
   
   if(animation==nil)
    return [[layer valueForKey:key] pointValue];
   
   if([animation isKindOfClass:[CABasicAnimation class]]){
    CABasicAnimation *basic=(CABasicAnimation *)animation;
    
    id fromValue=[basic fromValue];
    id toValue=[basic toValue];
    
    if(toValue==nil)
     toValue=[layer valueForKey:key];
    
    CGPoint fromPoint=[fromValue pointValue];
    CGPoint toPoint=[toValue pointValue];
        
    CGPoint        resultPoint;
    double timingScale=mediaTimingScale(animation,currentTime);
    
    resultPoint.x=fromPoint.x+(toPoint.x-fromPoint.x)*timingScale;
    resultPoint.y=fromPoint.y+(toPoint.y-fromPoint.y)*timingScale;
        
    return resultPoint;
   }
   
   return CGPointMake(0,0);
}

static CGRect interpolateRectInLayerKey(CALayer *layer,NSString *key,CFTimeInterval currentTime){
   CAAnimation *animation=[layer animationForKey:key];

   if(animation==nil){
    return [[layer valueForKey:key] rectValue];
   }
   
   if([animation isKindOfClass:[CABasicAnimation class]]){
    CABasicAnimation *basic=(CABasicAnimation *)animation;
    
    id fromValue=[basic fromValue];
    id toValue=[basic toValue];
    
    if(toValue==nil)
     toValue=[layer valueForKey:key];
    
    CGRect fromRect=[fromValue rectValue];
    CGRect toRect=[toValue rectValue];
    
    double timingScale=mediaTimingScale(animation,currentTime);
    
    CGRect        resultRect;
    
    resultRect.origin.x=fromRect.origin.x+(toRect.origin.x-fromRect.origin.x)*timingScale;
    resultRect.origin.y=fromRect.origin.y+(toRect.origin.y-fromRect.origin.y)*timingScale;
    resultRect.size.width=fromRect.size.width+(toRect.size.width-fromRect.size.width)*timingScale;
    resultRect.size.height=fromRect.size.height+(toRect.size.height-fromRect.size.height)*timingScale;
        
    return resultRect;
   }
   
   return CGRectMake(0,0,0,0);
}

static GLint interpolationFromName(NSString *name){
   if(name==kCAFilterLinear)
    return GL_LINEAR;
   else if(name==kCAFilterNearest)
    return GL_NEAREST;
   else if([name isEqualToString:kCAFilterLinear])
    return GL_LINEAR;
   else if([name isEqualToString:kCAFilterNearest])
    return GL_NEAREST;
   else
    return GL_LINEAR;   
}

void CATexImage2DCGImage(CGImageRef image){
    size_t            imageWidth=CGImageGetWidth(image);
    size_t            imageHeight=CGImageGetHeight(image);
    CGBitmapInfo      bitmapInfo=CGImageGetBitmapInfo(image);
   
    CGDataProviderRef provider=CGImageGetDataProvider(image);
    CFDataRef         data=CGDataProviderCopyData(provider);
    const uint8_t    *pixelBytes=CFDataGetBytePtr(data);
   
   
    GLenum glFormat=GL_BGRA_EXT;
    GLenum glType=GL_UNSIGNED_BYTE;
   
    CGImageAlphaInfo alphaInfo=bitmapInfo&kCGBitmapAlphaInfoMask;
    CGBitmapInfo     byteOrder=bitmapInfo&kCGBitmapByteOrderMask;
   
    switch(alphaInfo){
   
        case kCGImageAlphaNone:
            break;

        case kCGImageAlphaPremultipliedLast:
            if(byteOrder==kO2BitmapByteOrder32Big){
                glFormat=GL_RGBA;
                glType=GL_UNSIGNED_BYTE;
            }
            break;

        case kCGImageAlphaPremultipliedFirst: // ARGB
            if(byteOrder==kCGBitmapByteOrder32Little){
                glFormat=GL_BGRA_EXT;
                glType=GL_UNSIGNED_BYTE;
            }
            break;

        case kCGImageAlphaLast:
            break;

        case kCGImageAlphaFirst:
            break;

        case kCGImageAlphaNoneSkipLast:
            break;

        case kCGImageAlphaNoneSkipFirst:
            break;

        case kCGImageAlphaOnly:
            break;
    }

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,imageWidth,imageHeight,0,glFormat,glType,pixelBytes);
}


-(void)_renderLayer:(CALayer *)layer z:(float)z currentTime:(CFTimeInterval)currentTime {
   NSNumber *textureId=[layer _textureId];
   GLuint    texture=[textureId unsignedIntValue];
   GLboolean loadPixelData=GL_FALSE;
   
   if(texture==0)
    loadPixelData=GL_TRUE;
   else {
   
    if(glIsTexture(texture)==GL_FALSE){
     loadPixelData=GL_TRUE;
    }
    glBindTexture(GL_TEXTURE_2D,texture);
    
   }

    if(loadPixelData){
        CGImageRef image=layer.contents;
    
        CATexImage2DCGImage(image);

        GLint minFilter=interpolationFromName(layer.minificationFilter);
        GLint magFilter=interpolationFromName(layer.magnificationFilter);
   
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minFilter);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,magFilter);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
   }
   
   CGPoint anchorPoint=interpolatePointInLayerKey(layer,@"anchorPoint",currentTime);
   CGPoint position=interpolatePointInLayerKey(layer,@"position",currentTime);
   CGRect  bounds=interpolateRectInLayerKey(layer,@"bounds",currentTime);
   float   opacity=interpolateFloatInLayerKey(layer,@"opacity",currentTime);
   
   GLfloat textureVertices[4*2];
   GLfloat vertices[4*3];
   
   textureVertices[0]=0;
   textureVertices[1]=1;
   textureVertices[2]=1;
   textureVertices[3]=1;
   textureVertices[4]=0;
   textureVertices[5]=0;
   textureVertices[6]=1;
   textureVertices[7]=0;

   vertices[0]=0;
   vertices[1]=0;
   vertices[2]=z;
   
   vertices[3]=bounds.size.width;
   vertices[4]=0;
   vertices[5]=z;
   
   vertices[6]=0;
   vertices[7]=bounds.size.height;
   vertices[8]=z;

   vertices[9]=bounds.size.width;
   vertices[10]=bounds.size.height;
   vertices[11]=z;
   

   glPushMatrix();
 //  glTranslatef(width/2,height/2,0);
   glTexCoordPointer(2, GL_FLOAT, 0, textureVertices);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   
   
   glTranslatef(position.x-(bounds.size.width*anchorPoint.x),position.y-(bounds.size.height*anchorPoint.y),0);
  // glTranslatef(position.x,position.y,0);
  // glScalef(bounds.size.width,bounds.size.height,1);
   
 //  glRotatef(1,0,0,1);
   glColor4f(opacity,opacity,opacity,opacity);

   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   for(CALayer *child in layer.sublayers)
    [self _renderLayer:child z:z+1 currentTime:currentTime];

   glPopMatrix();
}

-(void)render {
   //glMatrixMode(GL_MODELVIEW);                                           
   glLoadIdentity();

   glClearColor(0, 0, 0, 0);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);					

   //glEnable( GL_TEXTURE_2D );
   //glEnableClientState(GL_VERTEX_ARRAY);
   //glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   glEnable (GL_BLEND);
   glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
   
   glAlphaFunc ( GL_GREATER, 0 ) ;
   //glEnable ( GL_ALPHA_TEST ) ;
  
   [self _renderLayer:_rootLayer z:0 currentTime:CACurrentMediaTime()];

   glFlush();
}

-(void)endFrame {
}

@end
