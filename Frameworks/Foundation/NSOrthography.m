#import <Foundation/NSOrthography.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>

@implementation NSOrthography

+orthographyWithDominantScript:(NSString *)script languageMap:(NSDictionary *)languageMap {
    return [[[self alloc] initWithDominantScript:script languageMap:languageMap] autorelease];
}

-initWithDominantScript:(NSString *)script languageMap:(NSDictionary *)languageMap {
    _dominantScript=[script copy];
    _languageMap=[languageMap copy];
    return self;
}

-(void)dealloc {
    [_dominantScript release];
    [_languageMap release];
    [super dealloc];
}

-(NSDictionary *)languageMap {
    return _languageMap;
}

-(NSArray *)allLanguages {
   NSMutableArray *result=[NSMutableArray array];
   
   for(NSArray *entry in [_languageMap allValues])
    [result addObjectsFromArray: entry];
    
   return result;
}

-(NSArray *)allScripts {
   return [_languageMap allKeys];
}

-(NSString *)dominantLanguage {
   NSArray *languages=[_languageMap objectForKey:[self dominantScript]];
   
   return [languages count]?[languages objectAtIndex:0]:nil;
}

-(NSString *)dominantScript {
    return _dominantScript;
}

-(NSString *)dominantLanguageForScript:(NSString *)script {
   NSArray *languages=[_languageMap objectForKey:[self dominantScript]];
   
   return [languages count]?[languages objectAtIndex:0]:nil;
}

-(NSArray *)languagesForScript:(NSString *)script {
   return [_languageMap objectForKey:script];
}

@end
