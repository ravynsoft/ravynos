#import <Foundation/NSTextCheckingResult.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDictionary.h>

@implementation NSTextCheckingResult

-initWithResultType:(NSTextCheckingType)resultType range:(NSRange)range properties:(NSDictionary *)properties {
   _resultType=resultType;
   _range=range;
   _properties=[properties copy];
   return self;
}

-initWithResultType:(NSTextCheckingType)resultType range:(NSRange)range property:property name:(NSString *)name {
   NSDictionary *properties=[NSDictionary dictionaryWithObject:property forKey:name];

   return [self initWithResultType:resultType range:range properties:properties];
}

-(void)dealloc {
   [_properties release];
   [super dealloc];
}

+(NSTextCheckingResult *)addressCheckingResultWithRange:(NSRange)range components:(NSDictionary *)components {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)correctionCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)dashCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)dateCheckingResultWithRange:(NSRange)range date:(NSDate *)date {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)dateCheckingResultWithRange:(NSRange)range date:(NSDate *)date timeZone:(NSTimeZone *)timeZone duration:(NSTimeInterval)duration {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)grammarCheckingResultWithRange:(NSRange)range details:(NSArray *)details {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)linkCheckingResultWithRange:(NSRange)range URL:(NSURL *)url {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)orthographyCheckingResultWithRange:(NSRange)range orthography:(NSOrthography *)orthography {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)quoteCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)replacementCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement {
    NSUnimplementedMethod();
    return nil;
}

+(NSTextCheckingResult *)spellCheckingResultWithRange:(NSRange)range {
   return [[[self alloc] initWithResultType:NSTextCheckingTypeSpelling range:range properties:nil] autorelease];
}

-(NSDictionary *)addressComponents {
    NSUnimplementedMethod();
    return nil;
}

-(NSDate *)date {
    NSUnimplementedMethod();
    return nil;
}

-(NSTimeInterval)duration {
    NSUnimplementedMethod();
    return 0;
}

-(NSArray *)grammarDetails {
    NSUnimplementedMethod();
    return nil;
}

-(NSOrthography *)orthography {
    NSUnimplementedMethod();
    return nil;
}

-(NSRange)range {
    return _range;
}

-(NSString *)replacementString {
    NSUnimplementedMethod();
    return nil;
}

-(NSTextCheckingType)resultType {
    return _resultType;
}

-(NSTimeZone *)timeZone {
    NSUnimplementedMethod();
    return nil;
}

-(NSURL *)URL {
    NSUnimplementedMethod();
    return nil;
}

@end
