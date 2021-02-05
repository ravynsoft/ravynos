/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSPredicate.h>

@class NSExpression;

typedef enum {
    NSDirectPredicateModifier,
    NSAllPredicateModifier,
    NSAnyPredicateModifier
} NSComparisonPredicateModifier;

typedef enum {
    NSLessThanPredicateOperatorType,
    NSLessThanOrEqualToPredicateOperatorType,
    NSGreaterThanPredicateOperatorType,
    NSGreaterThanOrEqualToPredicateOperatorType,
    NSEqualToPredicateOperatorType,
    NSNotEqualToPredicateOperatorType,
    NSMatchesPredicateOperatorType,
    NSLikePredicateOperatorType,
    NSBeginsWithPredicateOperatorType,
    NSEndsWithPredicateOperatorType,
    NSInPredicateOperatorType,
    NSCustomSelectorPredicateOperatorType
} NSPredicateOperatorType;

enum {
    NSCaseInsensitivePredicateOption = 0x01,
    NSDiacriticInsensitivePredicateOption = 0x02
};

@interface NSComparisonPredicate : NSPredicate <NSCoding, NSCopying> {
    NSExpression *_left;
    NSExpression *_right;

    NSComparisonPredicateModifier _modifier;
    NSPredicateOperatorType _type;
    NSUInteger _options;
    SEL _customSelector;
}

- initWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right modifier:(NSComparisonPredicateModifier)modifier type:(NSPredicateOperatorType)type options:(NSUInteger)options;
- initWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right customSelector:(SEL)selector;

+ (NSPredicate *)predicateWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right modifier:(NSComparisonPredicateModifier)modifier type:(NSPredicateOperatorType)type options:(NSUInteger)options;
+ (NSPredicate *)predicateWithLeftExpression:(NSExpression *)left rightExpression:(NSExpression *)right customSelector:(SEL)selector;

- (NSExpression *)leftExpression;
- (NSExpression *)rightExpression;
- (NSPredicateOperatorType)predicateOperatorType;

- (NSComparisonPredicateModifier)comparisonPredicateModifier;
- (NSUInteger)options;

- (SEL)customSelector;

@end
