/* Implementation of class NSTextCheckingController
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 02-08-2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "AppKit/NSTextCheckingController.h"
#import "AppKit/NSSpellChecker.h"

/*
@interface NSSpellChecker (Private)
- (void) _findNext: (id)sender;
- (void) _ignore: (id)sender;
- (void) _guess: (id)sender;
- (void) _correct: (id)sender;
- (void) _switchDictionary: (id)sender;
- (void) _highlightGuess: (id)sender;
@end
*/

@implementation NSTextCheckingController

// initializer
- (instancetype) initWithClient: (id<NSTextCheckingClient>)client
{
  self = [super init];
  if (self != nil)
    {
      _client = client;
      _spellCheckerDocumentTag = 0;
    }
  return self;
}

// properties...
- (id<NSTextCheckingClient>) client
{
  return _client;
}

- (NSInteger) spellCheckerDocumentTag
{
  return _spellCheckerDocumentTag;
}

- (void) setSpellCheckerDocumentTag: (NSInteger)tag
{
  _spellCheckerDocumentTag = tag;
}

// instance methods...
- (void) changeSpelling: (id)sender
{
  
}

- (void) checkSpelling: (id)sender
{
  /*
  int wordCount = 0;
  [[NSSpellChecker sharedSpellChecker] checkSpellingOfString: [_client string]
                                                  startingAt: 0
                                                    language: nil
                                                        wrap: NO
                                      inSpellDocumentWithTag: _spellCheckerDocumentTag
                                      wordCount: &wordCount];*/
}

- (void) checkTextInRange: (NSRange)range 
                    types: (NSTextCheckingTypes)checkingTypes 
                  options: (NSDictionary *)options
{
}

- (void) checkTextInSelection: (id)sender
{
}

- (void) checkTextInDocument: (id)sender
{
}

- (void) didChangeTextInRange: (NSRange)range
{
}

- (void) considerTextCheckingForRange: (NSRange)range
{
}

- (void) didChangeSelectedRange
{
}

- (void) ignoreSpelling: (id)sender
{
}

- (void) insertedTextInRange: (NSRange)range
{
}

- (void) invalidate
{
}

- (NSMenu *) menuAtIndex: (NSUInteger)location
      clickedOnSelection: (BOOL)clickedOnSelection 
          effectiveRange: (NSRangePointer)effectiveRange
{
  return nil;
}

- (void) orderFrontSubstitutionsPanel: (id)sender
{
}

- (void) showGuessPanel: (id)sender
{
}

- (void) updateCandidates
{
}

- (NSArray *) validAnnotations
{
  return nil;
}

@end

