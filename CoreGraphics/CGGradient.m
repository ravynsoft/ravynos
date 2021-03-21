#import <CoreGraphics/CGGradient.h>
#import <CoreGraphics/CGColor.h>
#import <Foundation/NSString.h>

CGGradientRef CGGradientCreateWithColorComponents(CGColorSpaceRef colorSpace,const CGFloat components[],const CGFloat locations[],size_t count) {
   return NULL;
}

CGGradientRef CGGradientCreateWithColors(CGColorSpaceRef colorSpace,CFArrayRef colors,const CGFloat locations[]) {
   CFIndex i,count=CFArrayGetCount(colors);
   size_t  numberOfComponents=CGColorSpaceGetNumberOfComponents(colorSpace)+1;
   CGFloat components[count*numberOfComponents];
   
   for(i=0;i<count;i++){
    CGColorRef color=(CGColorRef)CFArrayGetValueAtIndex(colors,i);
    size_t     checkNumberOfComponents=CGColorGetNumberOfComponents(color);
    size_t     j;
    
    if(checkNumberOfComponents==numberOfComponents){
     const CGFloat *copy=CGColorGetComponents(color);
     
     for(j=0;j<numberOfComponents;j++)
      components[i*numberOfComponents+j]=components[j];
    }
    else {
     NSLog(@"CGGradientCreateWithColors, color spaces don't match, conversion not implemented");
     for(j=0;j<numberOfComponents;j++)
      components[i*numberOfComponents+j]=0;
    }
    
   }
   
   return CGGradientCreateWithColorComponents(colorSpace,components,locations,count);
}

void CGGradientRelease(CGGradientRef self) {
   if(self==NULL)
    return;
   
   CFRelease(self);
}

CGGradientRef CGGradientRetain(CGGradientRef self) {
   if(self==NULL)
    return NULL;
   
   return (CGGradientRef)CFRetain(self);
}
