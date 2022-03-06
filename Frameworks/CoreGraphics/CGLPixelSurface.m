#import <CoreGraphics/CGLPixelSurface.h>
#import <CoreGraphics/CGWindow.h>
#import <Onyx2D/O2Image.h>
#import <GLES2/gl2.h>
#import <GLES2/gl2ext.h>
#if defined(__AIRYX__)
#import <Onyx2D/O2Surface.h>
#else
#import <AppKit/O2Surface_DIBSection.h>
#endif

@implementation CGLPixelSurface

-initWithSize:(O2Size)size {
    _width=size.width;
    _height=size.height;
    _validBuffers=NO;
    _numberOfBuffers=0;
    _bufferObjects=NULL;
    _readPixels=NULL;
    _staticPixels=NULL;
    return self;
}

-(void)dealloc {
   [_surface release];
   [super dealloc];
}

-(void)setFrameSize:(O2Size)value {
    _width=value.width;
    _height=value.height;
   
   _validBuffers=NO;
}

-(void)setOpaque:(BOOL)value {
    _isOpaque=value;
}

-(void)validateBuffersIfNeeded {
   int i;

   if(_validBuffers)
    return;

// 0's are silently ignored per spec.
   if(_numberOfBuffers>0 && _bufferObjects!=NULL) // nVidia driver will crash if bufferObjects is NULL, does not conform to spec.
    CGLDeleteBuffers(_numberOfBuffers,_bufferObjects);
      
   if(_bufferObjects!=NULL)
    free(_bufferObjects);
    
   if(_readPixels!=NULL)
    free(_readPixels);
    
   if(_staticPixels!=NULL)
    free(_staticPixels);
   
   [_surface release];
   
   _validBuffers=YES;
   _numberOfBuffers=1;
   _rowsPerBuffer=(_height+(_numberOfBuffers-1))/_numberOfBuffers;
   _bufferObjects=malloc(_numberOfBuffers*sizeof(GLuint));
   _readPixels=malloc(_numberOfBuffers*sizeof(void *));
   _staticPixels=malloc(_numberOfBuffers*sizeof(void *));
   _surface=[[O2Surface alloc] initWithWidth:_width height:-_height compatibleWithDeviceContext:nil];
   
   for(i=0;i<_numberOfBuffers;i++){
    _bufferObjects[i]=0;
    _readPixels[i]=NULL;
    _staticPixels[i]=NULL;
   }

  // CGLGenBuffers(_numberOfBuffers,_bufferObjects);

   int row=0,bytesPerRow=_width*4;
   
   for(i=0;i<_numberOfBuffers;i++){    
    _staticPixels[i]=((uint8_t *)[_surface pixelBytes])+row*bytesPerRow;

    if(_bufferObjects[i]==0){
     _readPixels[i]=_staticPixels[i];
    }
    else {
     _readPixels[i]=NULL;
     CGLBindBuffer(GL_ARRAY_BUFFER, _bufferObjects[i]);
     CGLBufferData(GL_ARRAY_BUFFER, _width*_rowsPerBuffer*4, NULL,GL_STREAM_DRAW);
     CGLBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    row+=_rowsPerBuffer;
   }
}

//#define RGBA_NOT_BGRA 1

#ifdef RGBA_NOT_BGRA
#define PIXEL_FORMAT GL_RGBA
#else
#define PIXEL_FORMAT GL_BGRA_EXT
#endif

static inline uint32_t setAlpha255(uint32_t value){
#ifdef RGBA_NOT_BGRA
   unsigned int a=0xFF;
   unsigned int b=(value>>16)&0xFF;
   unsigned int g=(value>>8)&0xFF;
   unsigned int r=(value>>0)&0xFF;

   value=a<<24;
   value|=r<<16;
   value|=g<<8;
   value|=b;
   
   return value;
#else
   return value|=0xFF000000;
#endif
}

static inline uint32_t premultiplyPixel(uint32_t value){
#ifdef RGBA_NOT_BGRA
   unsigned int a=(value>>24)&0xFF;
   unsigned int b=(value>>16)&0xFF;
   unsigned int g=(value>>8)&0xFF;
   unsigned int r=(value>>0)&0xFF;
#else
   unsigned int a=(value>>24)&0xFF;
   unsigned int r=(value>>16)&0xFF;
   unsigned int g=(value>>8)&0xFF;
   unsigned int b=(value>>0)&0xFF;
#endif
   
   value&=0xFF000000;
   value|=O2Image_8u_mul_8u_div_255(r,a)<<16;
   value|=O2Image_8u_mul_8u_div_255(g,a)<<8;
   value|=O2Image_8u_mul_8u_div_255(b,a);
          
   return value;
}

-(O2Surface *)validSurface { 
    [self validateBuffersIfNeeded];
    return _surface;
}

-(void)readBuffer {
    
   [self validateBuffersIfNeeded];

   int bytesPerRow=_width*4;
   int i,row=0;

   if(glGetError()!=GL_NO_ERROR)
    return;
#if 0
   glPixelStorei(GL_PACK_ALIGNMENT, 4);
   glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   glPixelStorei(GL_PACK_SKIP_ROWS, 0);
   glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
#endif

   // Technically shouldn't need unbind, but to be safe
   BOOL unbind=NO;
   
   for(i=0;i<_numberOfBuffers;i++){
    int rowCount=MIN(_height-row,_rowsPerBuffer);

    if(_bufferObjects[i]==0)
     glReadPixels(0,row,_width,rowCount,PIXEL_FORMAT, GL_UNSIGNED_BYTE,_readPixels[i]);
    else {
     CGLBindBuffer(GL_ARRAY_BUFFER,_bufferObjects[i]);
     unbind=YES;
     
     glReadPixels(0,row,_width,rowCount,PIXEL_FORMAT, GL_UNSIGNED_BYTE, _readPixels[i]);
    }
    
    GLenum error=glGetError();
    if(error!=GL_NO_ERROR){
     NSLog(@"glReadPixels error=%d",error);
    }
    row+=rowCount;
   }
   
   if(unbind)
    CGLBindBuffer(GL_ARRAY_BUFFER,0);          

   row=0;
   unbind=NO;
      
   for(i=0;i<_numberOfBuffers;i++){
    int            r,rowCount=MIN(_height-row,_rowsPerBuffer);
    unsigned char *inputRow;
    unsigned char *outputRow=_staticPixels[i];
    
    //if(_bufferObjects[i]==0)
     inputRow=_readPixels[i];
    //else {
    // unbind=YES;
    // CGLBindBuffer(GL_ARRAY_BUFFER,_bufferObjects[i]); 
    // inputRow=(GLubyte*)CGLMapBuffer(GL_ARRAY_BUFFER,GL_READ_ONLY);
    //}
    
    if(_isOpaque){
     // Opaque contexts ignore alpha so we set it to 0xFF to get proper results when blending
     // E.g. application clears context with color and zero alpha, this will display as the color on OS X
     // reading back will give us 0 alpha, premultiplying will give us black, which would be wrong.
     
     for(r=0;r<rowCount;r++,inputRow+=bytesPerRow,outputRow+=bytesPerRow){
      int c;
     
      for(c=0;c<bytesPerRow;c+=4){
       uint32_t pixel=*((uint32_t *)(inputRow+c));
       
       pixel=setAlpha255(pixel);
       
       *((uint32_t *)(outputRow+c))=pixel;
       
      }
     }
    }
    else {
     for(r=0;r<rowCount;r++,inputRow+=bytesPerRow,outputRow+=bytesPerRow){
      int c;
     
      for(c=0;c<bytesPerRow;c+=4){
       uint32_t pixel=*((uint32_t *)(inputRow+c));
       
       pixel=premultiplyPixel(pixel);
       
       *((uint32_t *)(outputRow+c))=pixel;
       
      }
     }
    }
    
    //if(_bufferObjects[i]!=0){
    // CGLUnmapBuffer(GL_ARRAY_BUFFER);
    //}
    
    row+=rowCount;
   }
   
   if(unbind)
    CGLBindBuffer(GL_ARRAY_BUFFER,0);          
      
#if 0    
   if(_usePixelBuffer){
    CGLBindBuffer(GL_ARRAY_BUFFER,0);
    if(inputBytes!=NULL){
     CGLUnmapBuffer(GL_ARRAY_BUFFER);
}
   }
#endif
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %p:size={  %d %d } surface=%@",isa,self,_width,_height,_surface];
}

@end
