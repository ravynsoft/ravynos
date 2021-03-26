//
//  NSSpellCheckerTagData.h
//  AppKit
//
//  Created by Christopher Lloyd on 8/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSObject.h>

@class NSMutableSet, NSArray;

@interface NSSpellCheckerTagData : NSObject {
    NSMutableSet *_ignoredWords;
}

- (void)ignoreWord:(NSString *)word;
- (NSArray *)ignoredWords;
- (void)setIgnoredWords:(NSArray *)words;

@end
