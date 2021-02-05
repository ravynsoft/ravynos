#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSDate.h>

@class NSDictionary, NSURL, NSOrthography;

FOUNDATION_EXPORT NSString *const NSTextCheckingNameKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingJobTitleKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingOrganizationKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingStreetKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingCityKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingStateKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingZIPKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingCountryKey;
FOUNDATION_EXPORT NSString *const NSTextCheckingPhoneKey;

enum {
    NSTextCheckingTypeOrthography = 1ULL << 0,
    NSTextCheckingTypeSpelling = 1ULL << 1,
    NSTextCheckingTypeGrammar = 1ULL << 2,
    NSTextCheckingTypeDate = 1ULL << 3,
    NSTextCheckingTypeAddress = 1ULL << 4,
    NSTextCheckingTypeLink = 1ULL << 5,
    NSTextCheckingTypeQuote = 1ULL << 6,
    NSTextCheckingTypeDash = 1ULL << 7,
    NSTextCheckingTypeReplacement = 1ULL << 8,
    NSTextCheckingTypeCorrection = 1ULL << 9,
};
typedef uint64_t NSTextCheckingType;

enum {
    NSTextCheckingAllSystemTypes = 0xffffffffULL,
    NSTextCheckingAllCustomTypes = 0xffffffffULL << 32,
    NSTextCheckingAllTypes = (NSTextCheckingAllSystemTypes | NSTextCheckingAllCustomTypes),
};
typedef uint64_t NSTextCheckingTypes;

@interface NSTextCheckingResult : NSObject {
    NSTextCheckingType _resultType;
    NSRange _range;
    NSDictionary *_properties;
}

+ (NSTextCheckingResult *)addressCheckingResultWithRange:(NSRange)range components:(NSDictionary *)components;
+ (NSTextCheckingResult *)correctionCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement;
+ (NSTextCheckingResult *)dashCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement;
+ (NSTextCheckingResult *)dateCheckingResultWithRange:(NSRange)range date:(NSDate *)date;
+ (NSTextCheckingResult *)dateCheckingResultWithRange:(NSRange)range date:(NSDate *)date timeZone:(NSTimeZone *)timeZone duration:(NSTimeInterval)duration;
+ (NSTextCheckingResult *)grammarCheckingResultWithRange:(NSRange)range details:(NSArray *)details;
+ (NSTextCheckingResult *)linkCheckingResultWithRange:(NSRange)range URL:(NSURL *)url;
+ (NSTextCheckingResult *)orthographyCheckingResultWithRange:(NSRange)range orthography:(NSOrthography *)orthography;
+ (NSTextCheckingResult *)quoteCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement;
+ (NSTextCheckingResult *)replacementCheckingResultWithRange:(NSRange)range replacementString:(NSString *)replacement;
+ (NSTextCheckingResult *)spellCheckingResultWithRange:(NSRange)range;

@property(readonly) NSDictionary *addressComponents;
@property(readonly) NSDate *date;
@property(readonly) NSTimeInterval duration;
@property(readonly) NSArray *grammarDetails;
@property(readonly) NSOrthography *orthography;
@property(readonly) NSRange range;
@property(readonly) NSString *replacementString;
@property(readonly) NSTextCheckingType resultType;
@property(readonly) NSTimeZone *timeZone;
@property(readonly) NSURL *URL;

@end
