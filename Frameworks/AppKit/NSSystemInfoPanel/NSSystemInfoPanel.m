/* Copyright (c) 2007 Dr. Rolf Jansen - original code
   Copyright (c) 2010 Ivan Vucica - showInfoPanel:withOptions: and corrections for correctness

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDictionary.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSSystemInfoPanel.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSText.h>

@implementation NSSystemInfoPanel

static NSSystemInfoPanel *_sharedInfoPanel = nil;

+ (NSSystemInfoPanel *)standardAboutPanel
{
   if(_sharedInfoPanel == nil)
   {
      _sharedInfoPanel = [NSSystemInfoPanel alloc];
      if(![NSBundle loadNibNamed:@"NSSystemInfoPanel" owner:_sharedInfoPanel])
         NSLog(@"Cannot load NSSystemInfoPanel.nib");
   }
   return _sharedInfoPanel;
}

- (void)awakeFromNib
{
   [self _resetInfoPanel];
}

- (void)_resetInfoPanel
{
   NSImage *icon = [NSImage imageNamed:[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIconFile"]];
   if (icon != nil)
      [appIconView setImage:icon];
   
   [appNameField setStringValue:[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"]];
   
   NSString *bundleVersion=[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
   NSString *bundleShortVersion=[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
   NSString *versionString=bundleVersion;
   
   if(bundleShortVersion!=nil)
    versionString=[NSString stringWithFormat:@"%@ (%@)",bundleShortVersion,bundleVersion];

   if(versionString!=nil)
    [versionField setStringValue:versionString];


   NSRect frame = [infoPanel frame];
   static float resetFrameSize = 0;
   if(resetFrameSize == 0)
      resetFrameSize = frame.size.height;
   frame.size.height = resetFrameSize;
   NSString *resourceFileName = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
   if (resourceFileName != nil)
   {
      frame.size.height += 170;
      [creditView readRTFDFromFile:resourceFileName];
   }
   else
      [creditScrollView setFrame:NSMakeRect(0, 0, 0, 0)];

   [legalTextField setStringValue:[[NSBundle mainBundle] localizedStringForKey:@"NSHumanReadableCopyright" value:@" " table:@"InfoPlist"]];

   frame.origin.y = [[NSScreen mainScreen] frame].size.height - 150 - frame.size.height;
   frame.origin.x = ([[NSScreen mainScreen] frame].size.width - frame.size.width)/2.0;
   [infoPanel setFrame:frame display:YES];
  // [[infoPanel contentView] setNeedsDisplay:YES];
}

- (IBAction)showInfoPanel:(id)sender
{
   [self _resetInfoPanel]; 
   [infoPanel makeKeyAndOrderFront:sender];
}
- (void)showInfoPanel:(id)sender withOptions:(NSDictionary*)options
{
   if(!options) 
   {
      [self showInfoPanel:sender];
      return;
   }


   [self _resetInfoPanel];
   
   // read user prefs (for easier use below)
   NSImage  *ApplicationIcon = [options objectForKey:@"ApplicationIcon"];
   NSString *ApplicationName = [options objectForKey:@"ApplicationName"];
   NSString *Version = [options objectForKey:@"Version"];
   NSString *ApplicationVersion = [options objectForKey:@"ApplicationVersion"];
   NSString *Copyright = [options objectForKey:@"Copyright"];
   
   // read plist contents
   NSString *CFBundleVersion=[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
   NSString *CFBundleShortVersionString=[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];

   // now begin setup

   // set up icon using NSImage
   if(ApplicationIcon) 
      [appIconView setImage:ApplicationIcon];
   
   // set up app name
   if(ApplicationName) 
      [appNameField setStringValue:ApplicationName];

   // construct version string according to docs
   NSString *VerUsed = Version ? Version : CFBundleVersion;
   NSString *AppVerUsed = ApplicationVersion ? ApplicationVersion : [NSString stringWithFormat:@"Version %@", CFBundleShortVersionString];
   if(!AppVerUsed)
   {
      if(VerUsed)
          [versionField setStringValue:[NSString stringWithFormat: @"Version %@", VerUsed]];
      else
          [versionField setStringValue:@""];
   }
   else
   {
      if(VerUsed)
          [versionField setStringValue:[NSString stringWithFormat:@"%@ (%@)", AppVerUsed, VerUsed]];
      else
          [versionField setStringValue:AppVerUsed];
   }

   // set up copyright
   [legalTextField setStringValue:Copyright];

   // it may be a good idea to inform developer about not supporting Credits being set up here
   if([options objectForKey:@"Credits"])
   {
	   // TODO warn somehow or implement setting NSAttributedString into the text field!
   }

   [infoPanel makeKeyAndOrderFront:sender];   
}
@end
