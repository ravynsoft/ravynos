/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSValueTransformer.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import "NSValueTransformer_IsNil.h"
#import "NSValueTransformer_IsNotNil.h"
#import "NSValueTransformer_NegateBoolean.h"
#import "NSValueTransformer_UnarchiveFromData.h"
#import "NSValueTransformer_KeyedUnarchiveFromData.h"

// Do not change these values
NSString * const NSIsNilTransformerName=@"NSIsNil";
NSString * const NSIsNotNilTransformerName=@"NSIsNotNil";
NSString * const NSNegateBooleanTransformerName=@"NSNegateBoolean";
NSString * const NSUnarchiveFromDataTransformerName=@"NSUnarchiveFromData";
NSString * const NSKeyedUnarchiveFromDataTransformerName=@"NSKeyedUnarchiveFromData";

@implementation NSValueTransformer

static NSMapTable *_nameToTransformer=NULL;

+(void)initialize {
   if(self==[NSValueTransformer class]){
    _nameToTransformer=NSCreateMapTable(NSObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0);
    NSMapInsert(_nameToTransformer,NSIsNilTransformerName,[[[NSValueTransformer_IsNil alloc] init] autorelease]);
    NSMapInsert(_nameToTransformer,NSIsNotNilTransformerName,[[[NSValueTransformer_IsNotNil alloc] init] autorelease]);
    NSMapInsert(_nameToTransformer,NSNegateBooleanTransformerName,[[[NSValueTransformer_NegateBoolean alloc] init] autorelease]);
       NSMapInsert(_nameToTransformer,NSUnarchiveFromDataTransformerName,[[[NSValueTransformer_UnarchiveFromData alloc] init] autorelease]);
       NSMapInsert(_nameToTransformer,NSKeyedUnarchiveFromDataTransformerName,[[[NSValueTransformer_KeyedUnarchiveFromData alloc] init] autorelease]);
   }
}

+(NSArray *)valueTransformerNames {
   return NSAllMapTableKeys(_nameToTransformer);
}

+(NSValueTransformer *)valueTransformerForName:(NSString *)name {
   return NSMapGet(_nameToTransformer,name);
}

+(void)setValueTransformer:(NSValueTransformer *)transformer forName:(NSString *)name {
   NSMapInsert(_nameToTransformer,name,transformer);
}


+(Class)transformedValueClass {
   return Nil;
}

+(BOOL)allowsReverseTransformation {
   return NO;
}

-transformedValue:value {
   return value;
}

-reverseTransformedValue:value {
   if(![[self class] allowsReverseTransformation])
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] on value transformer which returns NO allowsReverseTransformation",isa,sel_getName(_cmd)];
   
   return [self transformedValue:value];
}

@end
