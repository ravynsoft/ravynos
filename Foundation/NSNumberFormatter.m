/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSNumberFormatter.h>
#import <Foundation/NSException.h>
#import <Foundation/NSError.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSString_unicodePtr.h>

#define NSNumberFormatterThousandSeparator 	','
#define NSNumberFormatterDecimalSeparator 	'.'
#define NSNumberFormatterPlaceholder		'#'
#define NSNumberFormatterSpace			'_'
#define NSNumberFormatterCurrency		'$'

@implementation NSNumberFormatter

static NSNumberFormatterBehavior _defaultFormatterBehavior=NSNumberFormatterBehavior10_4;

+(NSNumberFormatterBehavior)defaultFormatterBehavior {
   return _defaultFormatterBehavior;
}

+(void)setDefaultFormatterBehavior:(NSNumberFormatterBehavior)value {
   if(value==NSNumberFormatterBehaviorDefault)
    _defaultFormatterBehavior=NSNumberFormatterBehavior10_4;
   else
    _defaultFormatterBehavior=value;
}

-init {
   [super init];
   _behavior=_defaultFormatterBehavior;
   _numberStyle=NSNumberFormatterNoStyle;

    _locale= [[NSLocale currentLocale] retain];
    
    _thousandSeparator = [[_locale objectForKey:NSLocaleGroupingSeparator] retain];
    _decimalSeparator = [[_locale objectForKey:NSLocaleDecimalSeparator] retain];
   _attributedStringForNil=[[NSAttributedString allocWithZone:NULL] initWithString:@"(null)"];
   _attributedStringForNotANumber=[[NSAttributedString allocWithZone:NULL] initWithString:@"NaN"];
   _attributedStringForZero=[[NSAttributedString allocWithZone:NULL] initWithString:@"0.0"];
   _allowsFloats = YES;

   return self;
}

// FIXME: doesnt do everything

/*
 * The format string (on 10.4 - which Cocotron currently supports) follows this standard: http://unicode.org/reports/tr35/tr35-4.html#Number_Format_Patterns
 * The parser isn't handling every option in the string but at least the common things such as: [prefix] #,##0.### [suffix]
 *
 */


static void extractFormat(NSString *format,
                          NSString **prefix, NSString **suffix,
                          NSUInteger *minimumIntegerDigitsp, NSUInteger *maximumIntegerDigitsp,
                          NSUInteger *minimumFractionDigitsp, NSUInteger *maximumFractionDigitsp,
                          NSUInteger *groupingSizep, NSUInteger *secondaryGroupingSizep)
{
    NSUInteger length = [format length];
    NSUInteger prefixLength = 0;
    NSUInteger suffixLength = 0;

    NSUInteger groupingSize = 0;
    NSUInteger secondaryGroupingSize = 0;

    unichar buffer[length];
    unichar prefixBuffer[length], suffixBuffer[length];

    enum {
        STATE_PREFIX,
        STATE_INTEGER,
        STATE_FRACTION,
        STATE_SUFFIX,
    } state = STATE_PREFIX;

    NSUInteger minimumIntegerDigits, maximumIntegerDigits, minimumFractionDigits, maximumFractionDigits;
    minimumIntegerDigits = 0;
    maximumIntegerDigits = 0;
    minimumFractionDigits = 0;
    maximumFractionDigits = 0;

    BOOL foundPrimaryGrouping = NO;

    [format getCharacters:buffer];

    NSInteger i = 0;
    for (i = 0; i < length; i++) {
        unichar code = buffer[i];

        switch (state) {

            case STATE_PREFIX:
                // Looking for non-numeric chars leading off the format - stop when we find a 0 or a # or a '.'
                if (code == '.') {
                    state = STATE_FRACTION;
                } else if (code == '#' || code == '0') {// starting off with a hash or a 0
                    state = STATE_INTEGER;
                    i--; // step back so we can process these chars in the right state
                } else {
                    // Suck up chars into the prefix
                    prefixBuffer[prefixLength++] = code;
                }
                break;
            case STATE_INTEGER:
                if (code == '.') {
                    state = STATE_FRACTION;
                    // No need to step back - the '.' just marks the separation
                } else if (code == '#') {
                    if (foundPrimaryGrouping) {
                        groupingSize++;
                    }
                    maximumIntegerDigits++;
                } else if (code == '0') {
                    if (foundPrimaryGrouping) {
                        groupingSize++;
                    }
                    minimumIntegerDigits++;
                    maximumIntegerDigits++;
                } else if (code == ',') {
                    if (foundPrimaryGrouping == NO) {
                        foundPrimaryGrouping = YES;
                    } else {
                        secondaryGroupingSize = groupingSize;
                        groupingSize = 0;
                    }
                } else {
                    // Anything we don't recognize means we're into the suffix part
                    state = STATE_SUFFIX;
                    i--;
                }
                break;
            case STATE_FRACTION:
                if (code == '#') {
                   maximumFractionDigits++;
                } else if (code == '0') {
                    minimumFractionDigits++;
                    maximumFractionDigits++;
                } else {
                    state = STATE_SUFFIX;
                    i--; // and step back one to catch the contents
                }
                break;
            case STATE_SUFFIX:
                suffixBuffer[suffixLength++] = code;
                break;
        }
    }

    // Update all valud the parameters
    if (minimumIntegerDigitsp != NULL) {
        *minimumIntegerDigitsp = minimumIntegerDigits;
    }

    if (maximumIntegerDigitsp != NULL) {
        *maximumIntegerDigitsp = maximumIntegerDigits;
    }

    if (minimumFractionDigitsp != NULL) {
        *minimumFractionDigitsp = minimumFractionDigits;
    }

    if (maximumFractionDigitsp != NULL) {
        *maximumFractionDigitsp = maximumFractionDigits;
    }

    if (groupingSizep != NULL) {
        *groupingSizep = groupingSize;
    }

    if (secondaryGroupingSizep != NULL) {
        *secondaryGroupingSizep = secondaryGroupingSize;
    }

    if (prefixLength > 0) {
     *prefix = [[NSString allocWithZone: NULL] initWithCharacters: prefixBuffer length: prefixLength];
    }

    if (suffixLength > 0) {
        *suffix = [[NSString allocWithZone: NULL] initWithCharacters: suffixBuffer length: suffixLength];
    }
}


-(void)extractFromPositiveFormat {
   if([_positiveFormat length]){

    [_positivePrefix release];
    _positivePrefix=nil;
    [_positiveSuffix release];
    _positiveSuffix=nil;

    extractFormat(_positiveFormat, &_positivePrefix, &_positiveSuffix,
				  &_minimumIntegerDigits, &_maximumIntegerDigits,
				  &_minimumFractionDigits, &_maximumFractionDigits,
				  &_groupingSize,&_secondaryGroupingSize);
	_customMaximumFractionDigits = YES;
   }
}

-(void)extractFromNegativeFormat {
   if([_negativeFormat length]){

    [_negativePrefix release];
    _negativePrefix=nil;
    [_negativeSuffix release];
    _negativeSuffix=nil;

    extractFormat(_negativeFormat, &_negativePrefix, &_negativeSuffix, NULL, NULL, NULL, NULL, NULL, NULL);
	   if ([_negativePrefix isEqualToString: @"-"]) {
		   // That's not a very interesting prefix...
		   [_negativePrefix release];
		   _negativePrefix = nil;
	   }
   }
}

-initWithCoder:(NSCoder*)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSDictionary *attributes=[coder decodeObjectForKey:@"NS.attributes"];
    id check;

    if((check=[attributes objectForKey:@"formatterBehavior"])!=nil)
     _behavior=[check integerValue];
    if((check=[attributes objectForKey:@"numberStyle"])!=nil)
     _numberStyle=[check integerValue];
    if((check=[attributes objectForKey:@"formatWidth"])!=nil)
     _formatWidth=[check integerValue];
    _locale=[[attributes objectForKey:@"locale"] copy];
    _multiplier=[[attributes objectForKey:@"multiplier"] copy];
    if((check=[attributes objectForKey:@"allowsFloats"])!=nil)
     _allowsFloats=[check boolValue];
    if((check=[attributes objectForKey:@"alwaysShowsDecimalSeparator"])!=nil)
     _alwaysShowsDecimalSeparator=[check boolValue];
    if((check=[attributes objectForKey:@"lenient"])!=nil)
     _isLenient=[check boolValue];
    _isPartialStringValidationEnabled=NO; // not editable in IB
    if((check=[attributes objectForKey:@"generatesDecimalNumbers"])!=nil)
     _generatesDecimalNumbers=[check boolValue];
    if((check=[attributes objectForKey:@"usesGroupingSeparator"])!=nil)
     _usesGroupingSeparator=[check boolValue];
    _usesSignificantDigits=NO; // not editable in IB

    if((check=[attributes objectForKey:@"minimumIntegerDigits"])!=nil)
     _minimumIntegerDigits=[check integerValue];
    if((check=[attributes objectForKey:@"minimumFractionDigits"])!=nil)
     _minimumFractionDigits=[check integerValue];
    _minimumSignificantDigits=0;

    if((check=[attributes objectForKey:@"maximumIntegerDigits"])!=nil)
     _maximumIntegerDigits=[check integerValue];
    if((check=[attributes objectForKey:@"maximumFractionDigits"])!=nil){
     _customMaximumFractionDigits=YES;
     _maximumFractionDigits=[check integerValue];
    }
    _maximumSignificantDigits=0;

    _minimum=[[attributes objectForKey:@"minimum"] copy];
    _maximum=[[attributes objectForKey:@"maximum"] copy];

     _nilSymbol=[[attributes objectForKey:@"nilSymbol"] copy];
     _notANumberSymbol=[[attributes objectForKey:@"notANumberSymbol"] copy];
     _zeroSymbol=[[attributes objectForKey:@"zeroSymbol"] copy];
    _plusSign=[[attributes objectForKey:@"plusSign"] copy];
    _minusSign=[[attributes objectForKey:@"minusSign"] copy];

    _negativePrefix=[[attributes objectForKey:@"negativePrefix"] copy];
    _negativeSuffix=[[attributes objectForKey:@"negativeSuffix"] copy];
    _positivePrefix=[[attributes objectForKey:@"positivePrefix"] copy];
    _positiveSuffix=[[attributes objectForKey:@"positiveSuffix"] copy];
    _negativeInfinitySymbol=[[attributes objectForKey:@"negativeInfinitySymbol"] copy];
    _positiveInfinitySymbol=[[attributes objectForKey:@"positiveInfinitySymbol"] copy];

    _decimalSeparator=[[attributes objectForKey:@"decimalSeparator"] copy];
    _exponentSymbol=[[attributes objectForKey:@"exponentSymbol"] copy];
    _currencyCode=[[attributes objectForKey:@"currencyCode"] copy];
    _currencySymbol=[[attributes objectForKey:@"currencySymbol"] copy];
    _internationalCurrencySymbol=[[attributes objectForKey:@"internationalCurrencySymbol"] copy];
    _currencyDecimalSeparator=[[attributes objectForKey:@"currencyDecimalSeparator"] copy];
    _currencyGroupingSeparator=[[attributes objectForKey:@"currencyGroupingSeparator"] copy];
    _groupingSeparator=[[attributes objectForKey:@"groupingSeparator"] copy];
    _groupingSize=[[attributes objectForKey:@"groupingSize"] integerValue];
    _secondaryGroupingSize=[[attributes objectForKey:@"secondaryGroupingSize"] integerValue];
    _paddingCharacter=[[attributes objectForKey:@"paddingCharacter"] copy];
    _paddingPosition=[[attributes objectForKey:@"paddingPosition"] integerValue];

    _percentSymbol=[[attributes objectForKey:@"percentSymbol"] copy];
    _perMillSymbol=[[attributes objectForKey:@"perMillSymbol"] copy];
    _roundingIncrement=[[attributes objectForKey:@"roundingIncrement"] copy];
    _roundingMode=[[attributes objectForKey:@"roundingMode"] integerValue];

    _positiveFormat=[[attributes objectForKey:@"positiveFormat"] copy];
    _negativeFormat=[[attributes objectForKey:@"negativeFormat"] copy];

    [self extractFromPositiveFormat];
    [self extractFromNegativeFormat];

    _textAttributesForPositiveValues=nil;
    _textAttributesForNegativeValues=nil;
    _textAttributesForNegativeInfinity=nil;
    _textAttributesForNil=nil;
    _textAttributesForNotANumber=nil;
    _textAttributesForPositiveInfinity=nil;
    _textAttributesForZero=nil;

// 10.0, these need to be stored separately
#if 0
    _nilSymbol=[[coder decodeObjectForKey:@"NS.nil"] copy];
    _zeroSymbol=[[coder decodeObjectForKey:@"NS.zero"] copy];

    _positiveFormat=[[coder decodeObjectForKey:@"NS.positiveformat"] copy];
    _negativeFormat=[[coder decodeObjectForKey:@"NS.negativeformat"] copy];
    _textAttributesForPositiveValues=[[coder decodeObjectForKey:@"NS.positiveattrs"] copy]
    _textAttributesForNegativeValues=[[coder decodeObjectForKey:@"NS.negativeattrs"] copy]

    _decimalSeparator=[[coder decodeObjectForKey:@"NS.decimal"] copy];
    _thousandSeparator=[[coder decodeObjectForKey:@"NS.thousand"] copy];
    _hasThousandSeparators=[coder decodeBoolForKey:@"NS.hasthousands"];
    _allowsFloats=[coder decodeBoolForKey:@"NS.allowsfloats"];
    _localizesFormat=[coder decodeBoolForKey:@"NS.localized"];
#endif
   }

	return self;
}

-(void)dealloc {
    
    [_locale release];
    [_multiplier release];
    
    [_minimum release];
    [_maximum release];
    
    [_nilSymbol release];
    [_notANumberSymbol release];
    [_zeroSymbol release];
    [_plusSign release];
    [_minusSign release];
    [_negativePrefix release];
    [_negativeSuffix release];
    [_positivePrefix release];
    [_positiveSuffix release];
    [_negativeInfinitySymbol release];
    [_positiveInfinitySymbol release];
    
    [_decimalSeparator release];
    [_exponentSymbol release];
    [_currencyCode release];
    [_currencySymbol release];
    [_internationalCurrencySymbol release];
    [_currencyDecimalSeparator release];
    [_currencyGroupingSeparator release];
    [_groupingSeparator release];
    [_paddingCharacter release];
    [_percentSymbol release];
    [_perMillSymbol release];
    [_roundingIncrement release];
    [_positiveFormat release];
    [_negativeFormat release];
    [_textAttributesForPositiveValues release];
    [_textAttributesForNegativeValues release];
    [_textAttributesForNegativeInfinity release];
    [_textAttributesForNil release];
    [_textAttributesForNotANumber release];
    [_textAttributesForPositiveInfinity release];
    [_textAttributesForZero release];
    
    [_attributedStringForNil release];
    [_attributedStringForNotANumber release];
    [_attributedStringForZero release];
    [_roundingBehavior release];
    [_thousandSeparator release];
    
   [super dealloc];
}

-(NSNumberFormatterBehavior)formatterBehavior {
   return _behavior;
}

-(NSNumberFormatterStyle)numberStyle {
   return _numberStyle;
}

-(NSString *)format {
   return [NSString stringWithFormat:@"%@;%@;%@", _positiveFormat, _attributedStringForZero, _negativeFormat];
}

-(NSUInteger)formatWidth {
   return _formatWidth;
}

-(NSLocale *)locale {
   return _locale;
}

-(NSNumber *)multiplier {
   if(_multiplier==nil){

    if(_numberStyle==NSNumberFormatterPercentStyle)
     return [NSNumber numberWithInt:100];
}

   return _multiplier;
}

-(BOOL)allowsFloats {
   return _allowsFloats;
}

-(BOOL)localizesFormat {
   return _localizesFormat;
}

-(BOOL)hasThousandSeparators {
   return _hasThousandSeparators;
}

-(BOOL)alwaysShowsDecimalSeparator {
   return _alwaysShowsDecimalSeparator;
}

-(BOOL)isLenient {
   return _isLenient;
}

-(BOOL)isPartialStringValidationEnabled {
   return _isPartialStringValidationEnabled;
}

-(BOOL)generatesDecimalNumbers {
   return _generatesDecimalNumbers;
}

-(BOOL)usesGroupingSeparator {
   return _usesGroupingSeparator;
}

-(BOOL)usesSignificantDigits {
   return _usesSignificantDigits;
}

-(NSUInteger)minimumIntegerDigits {
   return _minimumIntegerDigits;
}

-(NSUInteger)minimumFractionDigits {
   return _minimumFractionDigits;
}

-(NSUInteger)minimumSignificantDigits {
   return _minimumSignificantDigits;
}

-(NSUInteger)maximumIntegerDigits {
   return _maximumIntegerDigits;
}

-(NSUInteger)maximumFractionDigits {
   if(_customMaximumFractionDigits)
    return _maximumFractionDigits;

   if(_numberStyle==NSNumberFormatterDecimalStyle)
    return 3;

   return 0;
}

-(NSUInteger)maximumSignificantDigits {
   return _maximumSignificantDigits;
}

-(NSNumber *)minimum {
   return _minimum;
}

-(NSNumber *)maximum {
   return _maximum;
}

-(NSString *)nilSymbol {
   return _nilSymbol;
}

-(NSString *)notANumberSymbol {
   if(_notANumberSymbol==nil)
    return @"NaN";

   return _notANumberSymbol;
}

-(NSString *)zeroSymbol {
   return _zeroSymbol;
}

-(NSString *)plusSign {
   return _plusSign;
}

-(NSString *)minusSign {
   return _minusSign;
}

-(NSString *)negativePrefix {
   if(_negativePrefix==nil)
    return @"";

   return _negativePrefix;
}

-(NSString *)negativeSuffix {
// Suffixes return the percent symbol if specified

   if(_negativeSuffix==nil){

    if(_numberStyle==NSNumberFormatterPercentStyle)
     return [self percentSymbol];

    return @"";
}

   return _negativeSuffix;
}

-(NSString *)positivePrefix {
   if(_positivePrefix==nil)
    return @"";

   return _positivePrefix;
}

-(NSString *)positiveSuffix {
// Suffixes return the percent symbol if specified

   if(_positiveSuffix==nil){

    if(_numberStyle==NSNumberFormatterPercentStyle)
     return [self percentSymbol];

    return @"";
}

   return _positiveSuffix;
}

-(NSString *)negativeInfinitySymbol {
   return _negativeInfinitySymbol;
}

-(NSString *)positiveInfinitySymbol {
   return _positiveInfinitySymbol;
}

-(NSString *)thousandSeparator {
   return _thousandSeparator;
}

-(NSString *)decimalSeparator {
   return _decimalSeparator;
}

-(NSString *)exponentSymbol {
   return _exponentSymbol;
}

-(NSString *)currencyCode {
   return _currencyCode;
}

-(NSString *)currencySymbol {
   return _currencySymbol;
}

-(NSString *)internationalCurrencySymbol {
   return _internationalCurrencySymbol;
}

-(NSString *)currencyDecimalSeparator {
   return _currencyDecimalSeparator;
}

-(NSString *)currencyGroupingSeparator {
   return _currencyGroupingSeparator;
}

-(NSString *)groupingSeparator {
   if(_groupingSeparator==nil){
    NSString *check=[_locale objectForKey:NSLocaleGroupingSeparator];

    if(check!=nil)
     return check;

    return @",";
}

   return _groupingSeparator;
}

-(NSUInteger)groupingSize {
   return _groupingSize;
}

-(NSUInteger)secondaryGroupingSize {
   return _secondaryGroupingSize;
}

-(NSString *)paddingCharacter {
   return _paddingCharacter;
}

-(NSNumberFormatterPadPosition)paddingPosition {
   return _paddingPosition;
}

-(NSString *)percentSymbol {
   if(_percentSymbol==nil)
    return @"%";

   return _percentSymbol;
}

-(NSString *)perMillSymbol {
   return _perMillSymbol;
}

-(NSDecimalNumberHandler *)roundingBehavior {
   return _roundingBehavior;
}

-(NSNumber *)roundingIncrement {
   return _roundingIncrement;
}

-(NSNumberFormatterRoundingMode)roundingMode {
   return _roundingMode;
}

-(NSString *)positiveFormat {
   return _positiveFormat;
}

-(NSString *)negativeFormat {
   return _negativeFormat;
}

-(NSDictionary *)textAttributesForPositiveValues {
   return _textAttributesForPositiveValues;
}

-(NSDictionary *)textAttributesForNegativeValues {
   return _textAttributesForNegativeValues;
}

-(NSAttributedString *)attributedStringForNil {
   return _attributedStringForNil;
}

-(NSAttributedString *)attributedStringForNotANumber {
   return _attributedStringForNotANumber;
}

-(NSAttributedString *)attributedStringForZero {
   return _attributedStringForZero;
}

-(NSDictionary *)textAttributesForNegativeInfinity {
   return _textAttributesForNegativeInfinity;
}

-(NSDictionary *)textAttributesForNil {
   return _textAttributesForNil;
}

-(NSDictionary *)textAttributesForNotANumber {
   return _textAttributesForNotANumber;
}

-(NSDictionary *)textAttributesForPositiveInfinity {
   return _textAttributesForPositiveInfinity;
}

-(NSDictionary *)textAttributesForZero {
   return _textAttributesForZero;
}

-(void)setFormat:(NSString *)format {
// This is 10.0 behavior only, probably broken anyway
   NSArray *formatStrings = [format componentsSeparatedByString:@";"];

   _positiveFormat = [[formatStrings objectAtIndex:0] retain];

   if ([formatStrings count] == 3) {
      _negativeFormat = [[formatStrings objectAtIndex:2] retain];
      _attributedStringForZero = [[NSAttributedString allocWithZone:NULL] initWithString:[formatStrings objectAtIndex:1]
         attributes:[NSDictionary dictionary]];
   }
   else if ([formatStrings count] == 2)
      _negativeFormat = [[formatStrings objectAtIndex:1] retain];
   else
      _negativeFormat = [NSString stringWithFormat:@"-%@", _positiveFormat];

   if ([format rangeOfString:@","].location != NSNotFound ||
      [format rangeOfString:_thousandSeparator].location != NSNotFound)
      [self setHasThousandSeparators:YES];
}

-(void)setAllowsFloats:(BOOL)flag {
   _allowsFloats = flag;
}

-(void)setLocalizesFormat:(BOOL)flag {
   _localizesFormat = flag;
}

-(void)setCurrencyCode:(NSString *)value {
   value=[value copy];
   [_currencyCode release];
   _currencyCode=value;
}

-(void)setCurrencyDecimalSeparator:(NSString *)value {
   value=[value copy];
   [_currencyDecimalSeparator release];
   _currencyDecimalSeparator=value;
}

-(void)setCurrencyGroupingSeparator:(NSString *)value {
   value=[value copy];
   [_currencyGroupingSeparator release];
   _currencyGroupingSeparator=value;
}

-(void)setCurrencySymbol:(NSString *)value {
   value=[value copy];
   [_currencySymbol release];
   _currencySymbol=value;
}

-(void)setDecimalSeparator:(NSString *)value {
   value=[value copy];
   [_decimalSeparator release];
   _decimalSeparator=value;
}

-(void)setExponentSymbol:(NSString *)value {
   value=[value copy];
   [_exponentSymbol release];
   _exponentSymbol=value;
}

-(void)setFormatterBehavior:(NSNumberFormatterBehavior)value {
   _behavior=value;
}

-(void)setFormatWidth:(NSUInteger)value {
   _formatWidth=value;
}

-(void)setGeneratesDecimalNumbers:(BOOL)value {
   _generatesDecimalNumbers=value;
}

-(void)setGroupingSeparator:(NSString *)value {
   value=[value copy];
   [_groupingSeparator release];
   _groupingSeparator=value;
}

-(void)setGroupingSize:(NSUInteger)value {
   _groupingSize=value;
}

-(void)setInternationalCurrencySymbol:(NSString *)value {
   value=[value copy];
   [_internationalCurrencySymbol release];
   _internationalCurrencySymbol=value;
}

-(void)setLenient:(BOOL)value {
   _isLenient=value;
}

-(void)setLocale:(NSLocale *)value {
   value=[value copy];
   [_locale release];
   _locale=value;
}

-(void)setMaximumFractionDigits:(NSUInteger)value {
   _customMaximumFractionDigits=YES;
   _maximumFractionDigits=value;
}

-(void)setMaximumIntegerDigits:(NSUInteger)value {
   _maximumIntegerDigits=value;
}

-(void)setMaximumSignificantDigits:(NSUInteger)value {
   _maximumSignificantDigits=value;
}

-(void)setMinimum:(NSNumber *)value {
   value=[value copy];
   [_minimum release];
   _minimum=value;
}

-(void)setMaximum:(NSNumber *)value {
   value=[value copy];
   [_maximum release];
   _maximum=value;
}

-(void)setMinimumFractionDigits:(NSUInteger)value {
   _minimumFractionDigits=value;
}

-(void)setMinimumIntegerDigits:(NSUInteger)value {
   _minimumIntegerDigits=value;
}

-(void)setMinimumSignificantDigits:(NSUInteger)value {
   _minimumSignificantDigits=value;
}

-(void)setMinusSign:(NSString *)value {
   value=[value copy];
   [_minusSign release];
   _minusSign=value;
}

-(void)setMultiplier:(NSNumber *)value {
   value=[value copy];
   [_multiplier release];
   _multiplier=value;
}

-(void)setNegativeInfinitySymbol:(NSString *)value {
   value=[value copy];
   [_negativeInfinitySymbol release];
   _negativeInfinitySymbol=value;
}

-(void)setNegativePrefix:(NSString *)value {
   value=[value copy];
   [_negativePrefix release];
   _negativePrefix=value;
}

-(void)setNegativeSuffix:(NSString *)value {
   value=[value copy];
   [_negativeSuffix release];
   _negativeSuffix=value;
}

-(void)setNilSymbol:(NSString *)value {
   value=[value copy];
   [_nilSymbol release];
   _nilSymbol=value;
}

-(void)setNotANumberSymbol:(NSString *)value {
   value=[value copy];
   [_notANumberSymbol release];
   _notANumberSymbol=value;
}

-(void)setNumberStyle:(NSNumberFormatterStyle)value {
   _numberStyle=value;
}

-(void)setPaddingCharacter:(NSString *)value {
   value=[value copy];
   [_paddingCharacter release];
   _paddingCharacter=value;
}

-(void)setPaddingPosition:(NSNumberFormatterPadPosition)value {
   _paddingPosition=value;
}

-(void)setPartialStringValidationEnabled:(BOOL)value {
   _isPartialStringValidationEnabled=value;
}

-(void)setPercentSymbol:(NSString *)value {
   value=[value copy];
   [_percentSymbol release];
   _percentSymbol=value;
}

-(void)setPerMillSymbol:(NSString *)value {
   value=[value copy];
   [_perMillSymbol release];
   _perMillSymbol=value;
}

-(void)setPlusSign:(NSString *)value {
   value=[value copy];
   [_plusSign release];
   _plusSign=value;
}

-(void)setPositiveInfinitySymbol:(NSString *)value {
   value=[value copy];
   [_positiveInfinitySymbol release];
   _positiveInfinitySymbol=value;
}

-(void)setPositivePrefix:(NSString *)value {
   value=[value copy];
   [_positivePrefix release];
   _positivePrefix=value;
}

-(void)setPositiveSuffix:(NSString *)value {
   value=[value copy];
   [_positiveSuffix release];
   _positiveSuffix=value;
}

-(void)setRoundingBehavior:(NSDecimalNumberHandler *)value {
   value=[value retain];
   [_roundingBehavior release];
   _roundingBehavior=value;
}

-(void)setRoundingIncrement:(NSNumber *)value {
   value=[value copy];
   [_roundingIncrement release];
   _roundingIncrement=value;
}

-(void)setRoundingMode:(NSNumberFormatterRoundingMode)value {
   _roundingMode=value;
}

-(void)setSecondaryGroupingSize:(NSUInteger)value {
   _secondaryGroupingSize=value;
}

-(void)setTextAttributesForNegativeInfinity:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForNegativeInfinity release];
   _textAttributesForNegativeInfinity=value;
}

-(void)setTextAttributesForNil:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForNil release];
   _textAttributesForNil=value;
}

-(void)setTextAttributesForNotANumber:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForNotANumber release];
   _textAttributesForNotANumber=value;
}

-(void)setTextAttributesForPositiveInfinity:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForPositiveInfinity release];
   _textAttributesForPositiveInfinity=value;
}

-(void)setTextAttributesForPositiveValues:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForPositiveValues release];
   _textAttributesForPositiveValues=value;
}

-(void)setTextAttributesForZero:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForZero release];
   _textAttributesForZero=value;
}

-(void)setThousandSeparator:(NSString *)value {
   value=[value copy];
   [_thousandSeparator release];
   _thousandSeparator=value;
   [self setHasThousandSeparators:YES];
}

-(void)setUsesGroupingSeparator:(BOOL)value {
   _usesGroupingSeparator=value;
}

-(void)setUsesSignificantDigits:(BOOL)value {
   _usesSignificantDigits=value;
}

-(void)setZeroSymbol:(NSString *)value {
   value=[value copy];
   [_zeroSymbol release];
   _zeroSymbol=value;
}

-(void)setHasThousandSeparators:(BOOL)value {
   _hasThousandSeparators = value;
}

-(void)setAlwaysShowsDecimalSeparator:(BOOL)value {
   _alwaysShowsDecimalSeparator=value;
}

-(void)setPositiveFormat:(NSString *)value {
   value=[value copy];
   [_positiveFormat release];
   _positiveFormat=value;
// FIXME: generate formatting values from string
}

-(void)setNegativeFormat:(NSString *)value {
   value=[value copy];
   [_negativeFormat release];
   _negativeFormat=value;
// FIXME: generate formatting values from string
}

-(void)setTextAttributesForNegativeValues:(NSDictionary *)value {
   value=[value copy];
   [_textAttributesForNegativeValues release];
   _textAttributesForNegativeValues=value;
}

-(void)setAttributedStringForNil:(NSAttributedString *)value {
   value=[value copy];
   [_attributedStringForNil release];
   _attributedStringForNil=value;
}

-(void)setAttributedStringForNotANumber:(NSAttributedString *)value {
   value=[value copy];
   [_attributedStringForNotANumber release];
   _attributedStringForNotANumber=value;
}

-(void)setAttributedStringForZero:(NSAttributedString *)value {
   value=[value copy];
   [_attributedStringForZero release];
   _attributedStringForZero=value;
}

-(NSString *)stringFromNumber10_0:(NSNumber *)number {
   NSUnimplementedMethod();
   return [number description];
}

static NSString *stringWithFormatGrouping(NSString *format,id locale,NSString *groupingSeparator,NSInteger groupingSize,...){
   NSUInteger length=0;
   va_list arguments;

   va_start(arguments,groupingSize);

   unichar *unicode=NSCharactersNewWithFormatAndGrouping(format,locale,arguments,&length,NULL,groupingSeparator,groupingSize);

   return [NSString_unicodePtrNewNoCopy(NULL,unicode,length,YES) autorelease];
}

-(NSString *)_stringFromNumber:(NSNumber *)number  {
   NSString *string=nil;

   if(number==nil)
    string=[self nilSymbol];
   else if(number==(NSNumber *)kCFNumberNaN)
    string=[self notANumberSymbol];
   else if(number==(NSNumber *)kCFNumberPositiveInfinity){
    NSString *check=[self positiveInfinitySymbol];

    if(check==nil){
     unichar code=0x221E; // unicode infinity

     check=[NSString stringWithCharacters:&code length:1];
    }

    string=check;
   }
   else if(number==(NSNumber *)kCFNumberNegativeInfinity){
    NSString *check=[self negativeInfinitySymbol];

    if(check==nil){
     unichar codes[2]={ '-', 0x221E }; // unicode infinity

     check=[NSString stringWithCharacters:codes length:2];
    }

    string=check;
   }

   const char *objcType=[number objCType];

   if(objcType==NULL || strlen(objcType)!=1)
    objcType="?";

   switch(*objcType){
    case _C_CHR:
    case _C_SHT:
    case _C_INT:
#ifndef __LP64__
    case _C_LNG:
#endif
     string=stringWithFormatGrouping(@"%i",_locale,[self groupingSeparator],[self groupingSize],[number intValue]);
     break;


    case _C_UCHR:
    case _C_USHT:
    case _C_UINT:
#ifndef __LP64__
    case _C_ULNG:
#endif
     string=stringWithFormatGrouping(@"%u",_locale,[self groupingSeparator],[self groupingSize],[number unsignedIntValue]);
     break;

#ifdef __LP64__
    case _C_LNG:
#endif
    case _C_LNG_LNG:
     string=stringWithFormatGrouping(@"%qd",_locale,[self groupingSeparator],[self groupingSize],[number longLongValue]);
     break;
     break;

#ifdef __LP64__
    case _C_ULNG:
#endif
    case _C_ULNG_LNG:
     string=stringWithFormatGrouping(@"%qu",_locale,[self groupingSeparator],[self groupingSize],[number unsignedLongLongValue]);
     break;

    case _C_FLT:
    case _C_DBL:;
     NSString *format;

     format=[NSString stringWithFormat:@"%%.%df",[self minimumFractionDigits]];

     string=stringWithFormatGrouping(format,_locale,[self groupingSeparator],[self groupingSize],[number doubleValue]);
     break;

    default:
     string=[number description];
     break;
   }

   return string;
}

static NSNumber *multipliedNumber(NSNumber *number,NSNumber *multiplier){
   if(multiplier==nil)
    return number;

   return [NSNumber numberWithDouble:[number doubleValue]*[multiplier doubleValue]];
}

static BOOL numberIsNegative(NSNumber *number){
   if(number==nil)
    return NO;

   return (copysign(1.0,[number doubleValue])<0)?YES:NO;
}

static BOOL numberIsPositive(NSNumber *number){
   if(number==nil)
    return YES; // ?

   return (copysign(1.0,[number doubleValue])>0)?YES:NO;
}

-(NSString *)stringFromNumberNoStyle:(NSNumber *)number {
   number=multipliedNumber(number,[self multiplier]);

   NSString *prefix;
   NSString *suffix;
   //unused
   //NSString *format;

   if(numberIsNegative(number)){
    prefix=[self negativePrefix];
    suffix=[self negativeSuffix];
    //format=[self negativeFormat];
   }
   else {
    prefix=[self positivePrefix];
    suffix=[self positiveSuffix];
    //format=[self positiveFormat];
   }

   NSString *result;

   result=prefix;
   result=[result stringByAppendingString:[self _stringFromNumber:number]];
   result=[result stringByAppendingString:suffix];

   return result;
}

-(NSString *)stringFromNumberPercentStyle:(NSNumber *)number {
   NSUnimplementedMethod();
   return [[self stringFromNumberNoStyle:number] stringByAppendingString:[self percentSymbol]];
}

-(NSString *)stringFromNumber10_4:(NSNumber *)number {
   switch(_numberStyle){

    case NSNumberFormatterNoStyle:
     return [self stringFromNumberNoStyle:number];

    case NSNumberFormatterDecimalStyle:
     return [self stringFromNumberNoStyle:number];

    case NSNumberFormatterCurrencyStyle:
     NSUnimplementedMethod();
     break;

    case NSNumberFormatterPercentStyle:
     return [self stringFromNumberNoStyle:number];

    case NSNumberFormatterScientificStyle:
     NSUnimplementedMethod();
     break;

    case NSNumberFormatterSpellOutStyle:
     NSUnimplementedMethod();
     break;
   }
   return [number description];
}

-(NSString *)stringFromNumber:(NSNumber *)number {
   NSNumberFormatterBehavior check=_behavior;

   if(check==NSNumberFormatterBehaviorDefault)
    check=NSNumberFormatterBehavior10_4;

   if(check==NSNumberFormatterBehavior10_0)
    return [self stringFromNumber10_0:number];
   else
    return [self stringFromNumber10_4:number];
}

-(NSNumber *)_numberFromString:(NSString *)string error:(NSString **)error
{
    // Note: this method is still quite incomplete compared to the thousand of formatting combinations you can set on a number
    // formatter...
    
    NSMutableString *mutableString = [[string mutableCopy] autorelease];
    unichar thousandSeparator = [_thousandSeparator characterAtIndex:0];
    
    for (NSUInteger i = 0; i < [mutableString length]; ++i) {
        // take out the thousand separator
        if (_hasThousandSeparators && [mutableString characterAtIndex:i] == thousandSeparator) {
            [mutableString deleteCharactersInRange:NSMakeRange(i, 1)];
        }
    }
    // Locale to use to parse the string
    NSLocale *locale = _locale;
    if (locale == nil) {
        locale = [NSLocale currentLocale];
    }
    if (_decimalSeparator) {
        // Replace the decimal separator to the scanner locale one so the scanner can properly parse the string in case the formatter custom
        // separator doesn't match the locale one
        NSString *localeDecimalSeparator = [locale objectForKey:NSLocaleDecimalSeparator];
        if (localeDecimalSeparator) {
            [mutableString replaceOccurrencesOfString:_decimalSeparator withString:localeDecimalSeparator options:0 range:NSMakeRange(0, [mutableString length])];
        }
    }

    NSScanner *scanner = [NSScanner scannerWithString: mutableString];
    [scanner setLocale: (id)locale];

    float value;
    NSNumber *number = nil;
    BOOL isValid = YES;
    if ([scanner scanFloat:&value] == NO) {
        isValid = NO;
    } else {
        if (![scanner isAtEnd]) {
            // The number is valid only if the remaining string is the suffix
            NSString *suffix = value>=0?[self positiveSuffix]:[self negativeSuffix];
            if ([suffix length]) {
                NSString *remainingString = [[scanner string] substringFromIndex:[scanner scanLocation]];
                if (![remainingString isEqualToString:suffix]) {
                    isValid = NO;
                }
            } else {
                isValid = NO;
            }
        }
        if (isValid) {
            if ([self multiplier]) {
                value /= [[self multiplier] floatValue];
            }
            number = [NSNumber numberWithFloat:value];
        }
    }
    if (isValid == NO) {
        if (error != NULL) {
            *error = NSLocalizedStringFromTableInBundle(@"Invalid number", nil, [NSBundle bundleForClass: [NSNumberFormatter class]], @"");
        }
        number = nil;
    }
    return number;
}

-(NSNumber *)numberFromString:(NSString *)string {
    return [self _numberFromString:string error:NULL];
}

// BROKEN
#if 0
-(NSString *)_objectValue:(id)object withNumberFormat:(NSString *)format {
   NSString *stringValue = [[NSNumber numberWithDouble:[object doubleValue]] stringValue];
   //NSAllocateMemoryPages wtf??
   unichar *valueBuffer = NSAllocateMemoryPages([stringValue length]+1);
   unichar *formatBuffer = NSAllocateMemoryPages([format length]+1);
   unichar *outputBuffer = NSAllocateMemoryPages([format length]+64);
   BOOL isNegative = [stringValue hasPrefix:@"-"];
   BOOL done = NO;
   NSUInteger formatIndex, valueIndex = 0, outputIndex = 0;
   NSUInteger prePoint, postPoint;
   NSInteger thousandSepCounter;

   // remove -
   if (isNegative)
      stringValue = [stringValue substringWithRange:NSMakeRange(1, [stringValue length]-1)];

   prePoint = [stringValue rangeOfString:@"."].location;
   postPoint = [stringValue length] - prePoint - 1;

   // decremented in main loop, when zero, time for a separator
   if (_hasThousandSeparators)
      thousandSepCounter = (prePoint % 3) ? (prePoint % 3) : 3;
   else
      thousandSepCounter = -1;		   // never

   NSLog(@"%@: pre %d post %d sep %d", stringValue, prePoint, postPoint, thousandSepCounter);

   [format getCharacters:formatBuffer];
   [stringValue getCharacters:valueBuffer];

   while (!done) {
      switch(formatBuffer[formatIndex]) {
         case NSNumberFormatterThousandSeparator:
            [self setHasThousandSeparators:YES];
            [self setThousandSeparator:[NSString stringWithCharacters:formatBuffer+formatIndex length:1]];
            break;

         case NSNumberFormatterDecimalSeparator:
            [self setDecimalSeparator:[NSString stringWithCharacters:formatBuffer+formatIndex length:1]];
            break;

         case NSNumberFormatterPlaceholder:
         case NSNumberFormatterSpace:
            outputBuffer[outputIndex++] = valueBuffer[valueIndex++];

            if (valueIndex < prePoint) {
               thousandSepCounter--;
               if (thousandSepCounter == 0) {
                  outputBuffer[outputIndex++] = [_thousandSeparator characterAtIndex:0];
                  thousandSepCounter = 3;
               }
            }
            else if (valueIndex == prePoint)
               outputBuffer[outputIndex++] = [_decimalSeparator characterAtIndex:0];
            else {
               postPoint--;
               if (postPoint == 0)
                  done = YES;
            }

            break;

         case NSNumberFormatterCurrency:
            // localize

         default:
            outputBuffer[outputIndex++] = formatBuffer[formatIndex];
            break;
      }

      formatIndex++;
   }

   NSLog(@"stringValue %@ format %@", stringValue, format);
   return [NSString stringWithCharacters:outputBuffer length:outputIndex];
}
#endif

// this section works, but it's pretty lame...
// it doesn't round, it truncates; integers in the format specifier are ignored...
-(NSString *)_separatedStringIfNeededWithString:(NSString *)string {
   NSUInteger thousandSepCounter, i, j = 0;
   unichar buffer[256];

   if (!_hasThousandSeparators)
      return string;

   if ([string length] < 4)
      return string;

   thousandSepCounter = ([string length] % 3) ? ([string length] % 3) : 3;
   for (i = 0; i < [string length]; ++i) {
      buffer[j++] = [string characterAtIndex:i];
      thousandSepCounter--;
      if (thousandSepCounter == 0) {
         buffer[j++] = [_thousandSeparator characterAtIndex:0];
         thousandSepCounter = 3;
      }
   }
   buffer[--j] = (unichar)0;

   return [NSString stringWithCharacters:buffer length:j];
}

-(NSString *)_stringValue:(NSString *)stringValue withNumberFormat:(NSString *)format {
   NSString *rightSide = nil, *leftSide = nil;
   NSMutableString *result = [NSMutableString string];
   NSRange r;
   NSUInteger i, indexRight = 0;
   BOOL formatNoDecPoint = ([format rangeOfString:@"."].location == NSNotFound);
   BOOL havePassedDecPoint = NO;
   NSInteger lastPlaceholder = 0;

   // remove negative sign if present
   if ([stringValue hasPrefix:@"-"])
      stringValue = [stringValue substringWithRange:NSMakeRange(1, [stringValue length]-1)];

   // since we key from the decimal point... if there isn't one in the format spec
   // we have to go on the "last placeholder"; if we have neither decimal NOR
   // placeholders, well, we can't really format the number can we
   if (formatNoDecPoint) {
      lastPlaceholder = [format rangeOfString:@"#" options:NSBackwardsSearch].location;
      if (lastPlaceholder == NSNotFound)
          [NSException raise:NSInvalidArgumentException format:@"NSNumberFormatter: Invalid format string"];
   }

   // split this into left/right strings
   r = [stringValue rangeOfString:@"."];
   if (r.location != NSNotFound) {
      leftSide = [stringValue substringWithRange:NSMakeRange(0, r.location)];
      rightSide = [stringValue substringWithRange:NSMakeRange(r.location+1, [stringValue length]-r.location-1)];
   }
   else
      leftSide = stringValue;

   // do commas
   leftSide = [self _separatedStringIfNeededWithString:leftSide];

   // ugh. loop through the format string, looking for the decimal point
   // or the last placeholder. characters other than special are passed through
   for (i = 0; i < [format length]; ++i) {
      unichar ch = [format characterAtIndex:i];

      switch(ch) {
         case NSNumberFormatterPlaceholder:
            if (formatNoDecPoint && i == lastPlaceholder)
               [result appendString:leftSide];
            break;

         // ignore?
         case NSNumberFormatterSpace:
         // ignore; already handled
         case NSNumberFormatterThousandSeparator:
            break;

         case NSNumberFormatterDecimalSeparator:
            [result appendString:leftSide];
            [result appendString:_decimalSeparator];
            havePassedDecPoint = YES;
            break;

         case NSNumberFormatterCurrency:
// FIX, add localization

         default:
            if (ch >= (unichar)'0' && ch <= (unichar)'9') {
               if (havePassedDecPoint == YES) {
                  ch = [rightSide characterAtIndex:indexRight++];
                  if (ch == (unichar)0)
                     ch = (unichar)'0';
               }
               else
                  break;
            }

            [result appendString:[NSString stringWithCharacters:&ch length:1]];
            break;
      }
   }

   return result;
}

-(NSString *)stringForObjectValue:object {
   if([object isKindOfClass:[NSNumber class]])
    return [self stringFromNumber:object];
   else
    return [object description];
}

-(NSAttributedString *)attributedStringForObjectValue10_0:object withDefaultAttributes:(NSDictionary *)defaultAttributes {
   if(object==nil){
    NSAttributedString *check=[self attributedStringForNil];

    if(check!=nil)
     return check;
}
   if(object==(NSNumber *)kCFNumberNaN){
    NSAttributedString *check=[self attributedStringForNotANumber];

    if(check!=nil)
     return check;
   }

   NSString     *string=[self stringForObjectValue:object];
   NSDictionary *attributes=nil;

   if(numberIsPositive(object))
    attributes=[self textAttributesForPositiveValues];
   else if(numberIsNegative(object))
    attributes=[self textAttributesForNegativeValues];
   else
    attributes=[self textAttributesForZero];

   if(attributes==nil)
    attributes=defaultAttributes;

   return [[[NSAttributedString allocWithZone:NULL] initWithString:string attributes:attributes] autorelease];
}

-(NSAttributedString *)attributedStringForObjectValue10_4:object withDefaultAttributes:(NSDictionary *)defaultAttributes {
   NSString     *string=[self stringForObjectValue:object];
   NSDictionary *attributes=nil;

   if (object == nil)
    attributes=[self textAttributesForNil];
   else if(object==(NSNumber *)kCFNumberNaN)
    attributes=[self textAttributesForNotANumber];
   else if(object==(NSNumber *)kCFNumberPositiveInfinity)
    attributes=[self textAttributesForPositiveInfinity];
   else if(object==(NSNumber *)kCFNumberNegativeInfinity)
    attributes=[self textAttributesForNegativeInfinity];
   else if(numberIsPositive(object))
    attributes=[self textAttributesForPositiveValues];
   else if(numberIsNegative(object))
    attributes=[self textAttributesForNegativeValues];
   else
    attributes=[self textAttributesForZero];

   if(attributes==nil)
    attributes=defaultAttributes;

   return [[[NSAttributedString allocWithZone:NULL] initWithString:string attributes:attributes] autorelease];
   }

-(NSAttributedString *)attributedStringForObjectValue:object withDefaultAttributes:(NSDictionary *)attributes {
   NSNumberFormatterBehavior check=_behavior;

   if(check==NSNumberFormatterBehaviorDefault)
    check=NSNumberFormatterBehavior10_4;

   if(check==NSNumberFormatterBehavior10_0)
    return [self attributedStringForObjectValue10_0:object withDefaultAttributes:(NSDictionary *)attributes];
   else
    return [self attributedStringForObjectValue10_4:object withDefaultAttributes:(NSDictionary *)attributes];
}

-(NSString *)editingStringForObjectValue:(id)object {
   return [self stringForObjectValue:object];
}

-(BOOL)getObjectValue:(id *)valuep forString:(NSString *)string range:(NSRange *)rangep error:(NSError **)errorp {
    NSString *errorDescription = nil;
    BOOL result = [self getObjectValue:valuep forString:string errorDescription:&errorDescription];
    if (errorp) {
        if (result) {
            *errorp = nil;
        } else {
            NSDictionary *userInfo = [NSDictionary dictionaryWithObject:errorDescription forKey:NSLocalizedDescriptionKey];
            *errorp = [NSError errorWithDomain:NSCocoaErrorDomain code:2048 userInfo:userInfo];
        }
    }
   return result;
}

-(BOOL)getObjectValue:(id *)object forString:(NSString *)string errorDescription:(NSString **)error {
    if (object) {
        *object = nil;
    }
    NSNumber *number = [self _numberFromString:string error:error];
    if (number) {
        float value = [number floatValue];
        if ([self maximum] && value > [[self maximum] floatValue]) {
            if (error != NULL) {
                *error = NSLocalizedStringFromTableInBundle(@"Number too big", nil, [NSBundle bundleForClass: [NSNumberFormatter class]], @"");
            }
            number = nil;
        } else if ([self minimum] && value < [[self minimum] floatValue]) {
            if (error != NULL) {
                *error = NSLocalizedStringFromTableInBundle(@"Number too smaller", nil, [NSBundle bundleForClass: [NSNumberFormatter class]], @"");
            }
            number = nil;
        } else {
            if (object) {
                *object = number;
            }
        }
    }
    
   return number != nil;
}

-(BOOL)isPartialStringValid:(NSString *)partialString
   newEditingString:(NSString **)newString
   errorDescription:(NSString **)error {
   //
   return YES;
}

@end
