//
//  NSSpellEngine.m
//  Foundation
//
//  Created by Christopher Lloyd on 8/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSSpellEngine.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSArray.h>

@implementation NSSpellEngine

static NSMutableArray *_allSpellEngines=nil;

+(void)initialize {
   if(self==[NSSpellEngine class]){

    NSArray *allPaths=[[NSBundle bundleForClass:self] pathsForResourcesOfType:@"spellEngine" inDirectory:nil];
    int      i,count=[allPaths count];

    _allSpellEngines=[[NSMutableArray alloc] init];

    for(i=0;i<count;i++){
     NSString *path=[allPaths objectAtIndex:i];
     NSBundle *check=[NSBundle bundleWithPath:path];
     Class     cls=[check principalClass];
     //unused
     //NSArray  *engines=[cls spellEngines];

     [_allSpellEngines addObjectsFromArray:[cls spellEngines]];
    }
   }
}

+(NSArray *)allSpellEngines {
   return _allSpellEngines;
}

+(NSArray *)spellEngines {
   return nil;
}

-(NSString *)vendor {
   return nil;
}

-(NSArray *)languages {
   return nil;
}

-(NSRange)checkGrammarInString:(NSString *)string language:(NSString *)language details:(NSArray **)outDetails {
   NSUnimplementedMethod();
   return NSMakeRange(0,0);
}

-(NSArray *)checkString:(NSString *)stringToCheck offset:(NSUInteger)offset types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger *)wordCount {
   NSUnimplementedMethod();
   return nil;
}

-(void)didForgetWord:(NSString *)word inLanguage:(NSString *)language {
   NSUnimplementedMethod();
}

-(void)didLearnWord:(NSString *)word inLanguage:(NSString *)language {
   NSUnimplementedMethod();
}

-(NSRange)findMisspelledWordInString:(NSString *)stringToCheck language:(NSString *)language wordCount:(NSInteger *)wordCount countOnly:(BOOL)countOnly {
   NSUnimplementedMethod();
   return NSMakeRange(0,0);
}

-(NSArray *)suggestCompletionsForPartialWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language {
   NSUnimplementedMethod();
   return nil;
}

-(NSArray *)suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language {
   NSUnimplementedMethod();
   return nil;
}

-(NSRange)spellServer:(NSSpellServer *)sender checkGrammarInString:(NSString *)string language:(NSString *)language details:(NSArray **)outDetails {
   return [self checkGrammarInString:string language:language details:outDetails];
}

-(NSArray *)spellServer:(NSSpellServer *)sender checkString:(NSString *)stringToCheck offset:(NSUInteger)offset types:(NSTextCheckingTypes)checkingTypes options:(NSDictionary *)options orthography:(NSOrthography *)orthography wordCount:(NSInteger *)wordCount {
   return [self checkString:stringToCheck offset:offset types:checkingTypes options:options orthography:orthography wordCount:wordCount];
}

-(void)spellServer:(NSSpellServer *)sender didForgetWord:(NSString *)word inLanguage:(NSString *)language {
   return [self didForgetWord:word inLanguage:language];
}

-(void)spellServer:(NSSpellServer *)sender didLearnWord:(NSString *)word inLanguage:(NSString *)language {
   return [self didLearnWord:word inLanguage:language];
}

-(NSRange)spellServer:(NSSpellServer *)sender findMisspelledWordInString:(NSString *)stringToCheck language:(NSString *)language wordCount:(NSInteger *)wordCount countOnly:(BOOL)countOnly {
   return [self findMisspelledWordInString:stringToCheck language:language wordCount:wordCount countOnly:countOnly];
}

-(NSArray *)spellServer:(NSSpellServer *)sender suggestCompletionsForPartialWordRange:(NSRange)range inString:(NSString *)string language:(NSString *)language {
   return [self suggestCompletionsForPartialWordRange:range inString:string language:language];
}

-(NSArray *)spellServer:(NSSpellServer *)sender suggestGuessesForWord:(NSString *)word inLanguage:(NSString *)language {
   return [self suggestGuessesForWord:word inLanguage:language];
}

@end
