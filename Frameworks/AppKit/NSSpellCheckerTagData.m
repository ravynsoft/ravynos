//
//  NSSpellCheckerTagData.m
//  AppKit
//
//  Created by Christopher Lloyd on 8/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "NSSpellCheckerTagData.h"
#import <Foundation/NSMutableSet.h>

@implementation NSSpellCheckerTagData

-init {
   _ignoredWords=[[NSMutableSet alloc] init];
   return self;
}

-(void)dealloc {
   [_ignoredWords release];
   [super dealloc];
}

-(void)ignoreWord:(NSString *)word {
   [_ignoredWords addObject:[[word copy] autorelease]];
}

-(NSArray *)ignoredWords {
   return [_ignoredWords allObjects];
}

-(void)setIgnoredWords:(NSArray *)words {
   [_ignoredWords setByAddingObjectsFromArray:words];
}

@end
