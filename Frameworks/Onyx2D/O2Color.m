/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Color.h>
#import <Onyx2D/O2ColorSpace.h>

@implementation O2Color

-initWithColorSpace:(O2ColorSpaceRef)colorSpace pattern:(O2Pattern *)pattern components:(const O2Float *)components {
   int i;
   
   _colorSpace=[colorSpace retain];
   _pattern=[pattern retain];
   _numberOfComponents=O2ColorSpaceGetNumberOfComponents(_colorSpace)+1;
   _components=NSZoneMalloc([self zone],sizeof(O2Float)*_numberOfComponents);
   for(i=0;i<_numberOfComponents;i++)
    _components[i]=components[i];
    
   return self;
}

O2ColorRef O2ColorInitWithColorSpace(O2ColorRef self,O2ColorSpaceRef colorSpace,const O2Float *components) {
   int i;
   
   self->_colorSpace=[colorSpace retain];
   self->_pattern=nil;
   self->_numberOfComponents=O2ColorSpaceGetNumberOfComponents(self->_colorSpace)+1;
   self->_components=NSZoneMalloc([self zone],sizeof(O2Float)*self->_numberOfComponents);
   for(i=0;i<self->_numberOfComponents;i++)
    self->_components[i]=components[i];
    
   return self;
}

-initWithDeviceGray:(O2Float)gray alpha:(O2Float)alpha {
   O2Float components[2]={gray,alpha};
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceGray();
   O2ColorInitWithColorSpace(self,colorSpace,components);
   [colorSpace release];
   return self;
}

-initWithDeviceRed:(O2Float)red green:(O2Float)green blue:(O2Float)blue alpha:(O2Float)alpha {
   O2Float components[4]={red,green,blue,alpha};
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   O2ColorInitWithColorSpace(self,colorSpace,components);
   [colorSpace release];
   return self;
}

-initWithDeviceCyan:(O2Float)cyan magenta:(O2Float)magenta yellow:(O2Float)yellow black:(O2Float)black alpha:(O2Float)alpha {
   O2Float components[5]={cyan,magenta,yellow,black,alpha};
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceCMYK();
   O2ColorInitWithColorSpace(self,colorSpace,components);
   [colorSpace release];
   return self;
}

O2ColorRef O2ColorCreate(O2ColorSpaceRef colorSpace,const O2Float *components) {
   return O2ColorInitWithColorSpace([O2Color alloc],colorSpace,components);
}

O2ColorRef O2ColorCreateGenericGray(O2Float gray,O2Float a) {
   return [[O2Color alloc] initWithDeviceGray:gray alpha:a];
}

O2ColorRef O2ColorCreateGenericRGB(O2Float r,O2Float g,O2Float b,O2Float a) {
   return [[O2Color alloc] initWithDeviceRed:r green:g blue:b alpha:a];
}

O2ColorRef O2ColorCreateGenericCMYK(O2Float c,O2Float m,O2Float y,O2Float k,O2Float a) {
   return [[O2Color alloc] initWithDeviceCyan:c magenta:m yellow:y black:k alpha:a];
}

O2ColorRef O2ColorCreateWithPattern(O2ColorSpaceRef colorSpace,O2PatternRef pattern,const O2Float *components) {
   return [[O2Color alloc] initWithColorSpace:colorSpace pattern:pattern components:components];
}

-init {
   O2ColorSpaceRef gray=O2ColorSpaceCreateDeviceGray();
   O2Float       components[2]={0,1};
   
   O2ColorInitWithColorSpace(self,gray,components);
   [gray release];
   return self;
}

-(void)dealloc {
   [_colorSpace release];
   [_pattern release];
   NSZoneFree([self zone],_components);
   [super dealloc];
}

O2ColorRef O2ColorCreateCopy(O2ColorRef self) {
   return [self retain];
}

O2ColorRef O2ColorCreateCopyWithAlpha(O2ColorRef self,O2Float alpha) {
   int   i;
   O2Float components[self->_numberOfComponents];

   for(i=0;i<self->_numberOfComponents-1;i++)
    components[i]=self->_components[i];
   components[i]=alpha;
      
   return O2ColorInitWithColorSpace([self->isa alloc],self->_colorSpace,components);
}

O2ColorRef O2ColorRetain(O2ColorRef self) {
   return (self!=NULL)?(O2ColorRef)CFRetain(self):NULL;
}

void O2ColorRelease(O2ColorRef self) {
   if(self!=NULL)
    CFRelease(self);
}

O2ColorSpaceRef O2ColorGetColorSpace(O2ColorRef self) {
   return self->_colorSpace;
}

size_t O2ColorGetNumberOfComponents(O2ColorRef self) {
   return self->_numberOfComponents;
}

const O2Float *O2ColorGetComponents(O2ColorRef self) {
   return self->_components;
}

O2Float O2ColorGetAlpha(O2ColorRef self) {
   return self->_components[self->_numberOfComponents-1];
}

O2PatternRef O2ColorGetPattern(O2ColorRef self) {
   return self->_pattern;
}

BOOL O2ColorEqualToColor(O2ColorRef self,O2ColorRef other) {
   if(![self->_colorSpace isEqualToColorSpace:other->_colorSpace])
    return NO;

   int i;
   for(i=0;i<self->_numberOfComponents;i++)
    if(self->_components[i]!=other->_components[i])
     return NO;

   return YES;
}

int O2ColorConvertComponentsToDeviceRGB(O2ColorSpaceRef inputSpace,const O2Float *components,O2Float *rgbComponents){
   O2ColorSpaceModel model=O2ColorSpaceGetModel(inputSpace);
   
   switch(model){

    case kO2ColorSpaceModelMonochrome:
     rgbComponents[0]=components[0];
     rgbComponents[1]=components[0];
     rgbComponents[2]=components[0];
     rgbComponents[3]=components[1];
     return 1;

    case kO2ColorSpaceModelRGB:
     rgbComponents[0]=components[0];
     rgbComponents[1]=components[1];
     rgbComponents[2]=components[2];
     rgbComponents[3]=components[3];
     return 1;

    case kO2ColorSpaceModelCMYK:;
#if 1
// CMYK to CMY to RGB
     float K=components[3];
     float C=(components[0]*(1-K)+K);
     float M=(components[1]*(1-K)+K);
     float Y=(components[2]*(1-K)+K);
   
     rgbComponents[0]=(1-C);
     rgbComponents[1]=(1-M);
     rgbComponents[2]=(1-Y);
     rgbComponents[3]=components[4];
#else
     float white=1-input[3];
   
     output[0]=(input[0]>white)?0:white-input[0];
     output[1]=(input[1]>white)?0:white-input[1];
     output[2]=(input[2]>white)?0:white-input[2];
#endif
     return 1;

    case kO2ColorSpaceModelLab:
     NSLog(@"O2ColorConvertComponentsToDeviceRGB unimplemented conversion %d",model);
     rgbComponents[0]=0;
     rgbComponents[1]=0;
     rgbComponents[2]=0;
     rgbComponents[3]=1;
     return 0;

    case kO2ColorSpaceModelDeviceN:{
     O2ColorSpace_DeviceN *deviceN=(O2ColorSpace_DeviceN *)inputSpace;
     size_t                deviceNNumberOfComponents=O2ColorSpaceGetNumberOfComponents(deviceN);
     O2ColorSpaceRef          altSpace=[deviceN alternateSpace];
     O2FunctionRef            tintTransform=[deviceN tintTransform];
     size_t                   altNumberOfComponents=O2ColorSpaceGetNumberOfComponents(altSpace)+1;
     CGFloat                  altComponents[altNumberOfComponents];
     
     if(deviceNNumberOfComponents!=1){
      NSLog(@"DeviceN color has more than one component");
      return 0;
     }
     
     O2FunctionEvaluate(tintTransform,components[0],altComponents);
     
     // Tint transforms dont process alpha, so we just pass it through?
     altComponents[altNumberOfComponents-1]=components[1];
     
     return O2ColorConvertComponentsToDeviceRGB(altSpace,altComponents,rgbComponents);
     }
     break;

    case kO2ColorSpaceModelIndexed:
     NSLog(@"O2ColorConvertComponentsToDeviceRGB unimplemented conversion %d",model);
     rgbComponents[0]=0;
     rgbComponents[1]=0;
     rgbComponents[2]=0;
     rgbComponents[3]=1;
     return 0;

    case kO2ColorSpaceModelPattern:
    default:
     NSLog(@"O2ColorConvertComponentsToDeviceRGB unimplemented conversion %d",model);
     rgbComponents[0]=0;
     rgbComponents[1]=0;
     rgbComponents[2]=0;
     rgbComponents[3]=1;
     return 0;
   }
   return 0;
}

O2ColorRef O2ColorConvertToDeviceRGB(O2ColorRef self) {
   O2ColorSpaceRef colorSpace=O2ColorGetColorSpace(self);
   O2ColorSpaceModel model = O2ColorSpaceGetModel(colorSpace);
    if (model == kO2ColorSpaceModelRGB) {
        // No need for a conversion
        return O2ColorRetain(self);
    }
   const O2Float   *components=O2ColorGetComponents(self);
   O2Float         rgbComponents[4];
   
   if(!O2ColorConvertComponentsToDeviceRGB(colorSpace,components,rgbComponents))
    return NULL;
    

   O2ColorSpaceRef rgbColorSpace=O2ColorSpaceCreateDeviceRGB();
   O2ColorRef      result=O2ColorCreate(rgbColorSpace,rgbComponents);
   
   O2ColorSpaceRelease(rgbColorSpace);
   
   return result;
}

@end
