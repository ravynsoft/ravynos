#import <QuartzCore/CIFilter.h>
#import <QuartzCore/CIImage.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSDictionary.h>

@interface CIImage(private)
-(void)setFilter:(CIFilter *)filter;
@end

@implementation CIFilter

-initWithName:(NSString *)name {
   _keyValues=[NSMutableDictionary new];
   [_keyValues setObject:name forKey:@"kCIAttributeFilterName"];
   return self;
}

-(void)dealloc {
   [_keyValues release];
   [super dealloc];
}

+(CIFilter *)filterWithName:(NSString *)name {
   return [[[self alloc] initWithName:name] autorelease];
}

-(void)setDefaults {
 //  NSUnimplementedMethod();
}

-(void)setValue:value forKey:(NSString *)key {
   if(value)
    [_keyValues setObject:value forKey:key];
   else
    [_keyValues removeObjectForKey:key];
}

-valueForKey:(NSString *)key {
   if([key isEqual:@"outputImage"]){
    CIImage *image=[CIImage emptyImage];
    
    [image setFilter:self];
    return image;
   }
   
   return [_keyValues objectForKey:key];
}

@end
