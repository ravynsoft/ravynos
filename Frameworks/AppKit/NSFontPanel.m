/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSFontPanel.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSFontPanelCell.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSButton.h>
#import <AppKit/NSRaise.h>

#import <AppKit/NSFontFamily.h>
#import <AppKit/NSFontTypeface.h>

@implementation NSFontPanel

-(NSArray *)availableFontFamilies {
   return [[NSFontManager sharedFontManager] availableFontFamilies];
}

-(NSArray *)availableTraitsInFamily:(NSString *)familyName {
   NSMutableArray *result=[NSMutableArray array];
   NSArray        *members=[[NSFontManager sharedFontManager] availableMembersOfFontFamily:familyName];
   int             i,count=[members count];

// (fullName,traitName,size,traits) 
   for(i=0;i<count;i++)
    [result addObject:[[members objectAtIndex:i] objectAtIndex:1]];

   return result;
}

-(NSArray *)availablePointSizes {
   return [NSArray arrayWithObjects:@"8",@"9",@"10",@"11",@"12",
    @"14",@"16",@"18",@"24",@"36",@"48",@"64",@"72",nil];
}

-(void)buildFamilyMatrix {
   NSArray *names=[self availableFontFamilies];
   int      i,count=[names count];

   [_familyMatrix renewRows:count columns:1];
   for(i=0;i<count;i++){
    NSFontPanelCell *cell=[_familyMatrix cellAtRow:i column:0];
    [cell setStringValue:[names objectAtIndex:i]];
   }
   [_familyMatrix sizeToFit];
   [_familyMatrix setNeedsDisplay:YES];
}

-(void)awakeFromNib {
   [_familyMatrix setPrototype:[[NSFontPanelCell new] autorelease]];
   [_familyMatrix renewRows:0 columns:1];
   [_familyMatrix setDoubleAction:@selector(set:)];
   [_typefaceMatrix setPrototype:[[NSFontPanelCell new] autorelease]];
   [_typefaceMatrix renewRows:0 columns:1];
   [_typefaceMatrix setDoubleAction:@selector(set:)];
   [_sizeMatrix setPrototype:[[NSFontPanelCell new] autorelease]];
   [_sizeMatrix renewRows:0 columns:1];
   [_sizeMatrix setDoubleAction:@selector(set:)];
   [self buildFamilyMatrix];
   [[self fieldEditor: YES forObject: self] setUsesFontPanel: NO];
}

+(BOOL)sharedFontPanelExists {
   return ([[NSFontManager sharedFontManager] fontPanel:NO]!=nil)?YES:NO;
}

+(NSFontPanel *)sharedFontPanel {
   return [[NSFontManager sharedFontManager] fontPanel:YES];
}

-(BOOL)isEnabled {
   return [_setButton isEnabled];
}

-(NSView *)accessoryView {
   return _accessoryView;
}

-(void)setEnabled:(BOOL)value {
   [_setButton setEnabled:value];
}

-(void)setAccessoryView:(NSView *)view {
   view=[view retain];
   [_accessoryView release];
   _accessoryView=view;
   NSUnimplementedMethod();
}

-(BOOL)worksWhenModal {
   return YES;
}

-(void)reloadDefaultFontFamilies {
   NSUnimplementedMethod();
}

-(NSString *)selectedFamilyName {
   NSArray *names=[[NSFontManager sharedFontManager] availableFontFamilies];
   int      row=[_familyMatrix selectedRow];

   if(row<0)
    row=0;

   return [names objectAtIndex:row];
}

-(NSString *)selectedFontName {
   NSArray *members=[[NSFontManager sharedFontManager] availableMembersOfFontFamily:[self selectedFamilyName]];
   int      row=[_typefaceMatrix selectedRow];

   if(row<0)
    row=0;

   return [[members objectAtIndex:row] objectAtIndex:0];
}

-(float)selectedPointSize {
   float result=[_sizeTextField floatValue];

   if(result==0)
    return 12.0;

   return result;
}

-(NSFont *)selectedFont {
   return [NSFont fontWithName:[self selectedFontName] size:[self selectedPointSize]]; 
}


-(void)buildTypefaceMatrix {
   NSArray  *traits=[self availableTraitsInFamily:[self selectedFamilyName]];
   int       i,count=[traits count];
   int       selectRow=0;
   NSString *oldTrait=[[[[_typefaceMatrix selectedCell] stringValue] retain] autorelease];

   [_typefaceMatrix renewRows:count columns:1];
   for(i=0;i<count;i++){
    NSFontPanelCell *cell=[_typefaceMatrix cellAtRow:i column:0];
    NSString        *trait=[traits objectAtIndex:i];

    if([trait isEqualToString:oldTrait])
     selectRow=i;

    [cell setStringValue:trait];
   }
   [_typefaceMatrix selectCellAtRow:selectRow column:0];
   [_typefaceMatrix sizeToFit];
   [_typefaceMatrix setNeedsDisplay:YES];
}

-(void)buildSizeMatrix {
   NSArray *sizes=[self availablePointSizes];
   int      i,count=[sizes count];

   [_sizeMatrix renewRows:count columns:1];
   for(i=0;i<count;i++){
    NSFontPanelCell *cell=[_sizeMatrix cellAtRow:i column:0];
    [cell setStringValue:[sizes objectAtIndex:i]];
   }
   [_sizeMatrix sizeToFit];
   [_sizeMatrix setNeedsDisplay:YES];
}

-(void)buildSampleTextField {
   NSFont *font=[self selectedFont];
   float     pointSize=[self selectedPointSize];

   [_sampleTextField setStringValue:[[font displayName] stringByAppendingFormat:@" %g pt",pointSize]];
   [_sampleTextField setFont:font];
}

-(void)setPanelFont:(NSFont *)font isMultiple:(BOOL)isMultiple {
   NSFontFamily   *family=[NSFontFamily fontFamilyWithTypefaceName:[font fontName]];
   NSFontTypeface *typeface=[family typefaceWithName:[font fontName]];

   {
    NSArray  *families=[self availableFontFamilies];
    NSString *familyName=[family name];
    unsigned  familyIndex=[families indexOfObject:familyName];
    NSArray  *traits=[self availableTraitsInFamily:familyName];
    unsigned  traitIndex=[traits indexOfObject:[typeface traitName]];
    NSArray  *sizes=[self availablePointSizes];
    unsigned  sizeIndex=[sizes indexOfObject:[NSString stringWithFormat:@"%g",[font pointSize]]];

    [self buildFamilyMatrix];
    [_familyMatrix selectCellAtRow:familyIndex column:0];
    [self buildTypefaceMatrix];
    [_typefaceMatrix selectCellAtRow:traitIndex column:0];
    [self buildSizeMatrix];
    [_sizeMatrix selectCellAtRow:sizeIndex column:0];
    [_sizeTextField setFloatValue:[font pointSize]];
    [self buildSampleTextField];
   }
}

-(NSFont *)panelConvertFont:(NSFont *)font {
   return [self selectedFont];
}

-(void)set:sender {
    [[NSFontManager sharedFontManager] modifyFontViaPanel:self];
}

-(void)revert:sender {
   NSFontManager *manager=[NSFontManager sharedFontManager];

   [self setPanelFont:[manager selectedFont] isMultiple:[manager isMultiple]];
}

-(void)clickFamilyMatrix:sender {
   [self buildTypefaceMatrix];
   [self buildSizeMatrix];
   [self buildSampleTextField];
}

-(void)clickTypefaceMatrix:sender {
   [self buildSizeMatrix];
   [self buildSampleTextField];
}

-(void)clickSizeText:sender {
   [self buildSampleTextField];
}

-(void)clickSizeMatrix:sender {
   NSArray *sizes=[self availablePointSizes];
   int      row=[_sizeMatrix selectedRow];

   if(row>=0){
    [_sizeTextField setStringValue:[NSString stringWithFormat:@"%g",[[sizes objectAtIndex:row] floatValue]]];
   }

   [self buildSampleTextField];
}




@end
