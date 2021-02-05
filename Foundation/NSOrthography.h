#import <Foundation/NSObject.h>

@class NSArray, NSDictionary;

@interface NSOrthography : NSObject {
    NSString *_dominantScript;
    NSDictionary *_languageMap;
}

+ orthographyWithDominantScript:(NSString *)script languageMap:(NSDictionary *)languageMap;

- initWithDominantScript:(NSString *)script languageMap:(NSDictionary *)languageMap;

@property(readonly) NSDictionary *languageMap;
@property(readonly) NSArray *allLanguages;
@property(readonly) NSArray *allScripts;
@property(readonly) NSString *dominantLanguage;
@property(readonly) NSString *dominantScript;

- (NSString *)dominantLanguageForScript:(NSString *)script;
- (NSArray *)languagesForScript:(NSString *)script;

@end