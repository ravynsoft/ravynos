/*
    File: DNSServiceDiscoveryPref.h

    Abstract: System Preference Pane for Dynamic DNS and Wide-Area DNS Service Discovery

    Copyright: (c) Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <PreferencePanes/PreferencePanes.h>
#import <SecurityInterface/SFAuthorizationView.h>
#import <dns_sd.h>

@class CNBonjourDomainView;
@class CNDomainBrowserView;

@interface DNSServiceDiscoveryPref : NSPreferencePane
{
    IBOutlet NSTextField          *hostName;
    IBOutlet NSTextField          *sharedSecretName;
    IBOutlet NSSecureTextField    *sharedSecretValue;
    IBOutlet NSTextField          *browseDomainTextField;
	IBOutlet NSTextField          *regDomainTextField;
	IBOutlet CNBonjourDomainView  *regDomainView;
    IBOutlet NSButton             *wideAreaCheckBox;
    IBOutlet NSButton             *hostNameSharedSecretButton;
	IBOutlet NSButton             *registrationSelectButton;
	IBOutlet NSButton             *registrationSharedSecretButton;
    IBOutlet NSButton             *applyButton;
    IBOutlet NSButton             *revertButton;
    IBOutlet NSWindow             *sharedSecretWindow;
	IBOutlet NSWindow             *addBrowseDomainWindow;
	IBOutlet NSWindow             *addBrowseDomainManualWindow;
	IBOutlet NSWindow             *selectRegistrationDomainWindow;
	IBOutlet NSWindow             *selectRegistrationDomainManualWindow;
    IBOutlet NSButton             *addBrowseDomainButton;
    IBOutlet NSButton             *removeBrowseDomainButton;
    IBOutlet NSButton             *secretOKButton;
    IBOutlet NSButton             *secretCancelButton;
    IBOutlet NSImageView          *statusImageView;
    IBOutlet NSTabView            *tabView;
	IBOutlet NSTableView          *browseDomainList;
	IBOutlet CNDomainBrowserView  *bonjourBrowserView;
	IBOutlet CNDomainBrowserView  *registrationBrowserView;
    IBOutlet SFAuthorizationView  *comboAuthButton;

    NSWindow            *mainWindow;
    NSString            *currentHostName;
    NSString            *currentRegDomain;
    NSArray             *currentBrowseDomainsArray;
    NSMutableArray      *browseDomainsArray;
    NSString            *defaultRegDomain;

    NSString            *hostNameSharedSecretName;
    NSString            *hostNameSharedSecretValue;
    NSString            *regSharedSecretName;
    NSString            *regSharedSecretValue;
    BOOL                currentWideAreaState;
    BOOL                prefsNeedUpdating;
    BOOL                browseDomainListEnabled;
    NSImage             *successImage;
    NSImage             *inprogressImage;
    NSImage             *failureImage;

    NSMutableArray      *registrationDataSource;
}

-(IBAction)applyClicked : (id)sender;
-(IBAction)enableBrowseDomainClicked : (id)sender;
-(IBAction)addBrowseDomainClicked : (id)sender;
-(IBAction)removeBrowseDomainClicked : (id)sender;
-(IBAction)revertClicked : (id)sender;
-(IBAction)changeButtonPressed : (id)sender;
-(IBAction)closeMyCustomSheet : (id)sender;
-(IBAction)wideAreaCheckBoxChanged : (id)sender;


-(NSMutableArray *)registrationDataSource;
-(NSString *)currentRegDomain;
-(NSArray *)currentBrowseDomainsArray;
-(NSString *)currentHostName;
-(NSString *)defaultRegDomain;
-(void)setDefaultRegDomain : (NSString *)domain;


-(void)enableApplyButton;
-(void)disableApplyButton;
-(void)applyCurrentState;
-(void)setupInitialValues;
-(void)toggleWideAreaBonjour : (BOOL)state;
-(void)updateApplyButtonState;
-(void)enableControls;
-(void)disableControls;
-(void)validateTextFields;
-(void)readPreferences;
-(void)savePreferences;
-(void)restorePreferences;
-(void)watchForPreferenceChanges;
-(void)updateStatusImageView;


-(NSString *)sharedSecretKeyName : (NSString * )domain;
-(NSString *)domainForHostName : (NSString *)hostNameString;
-(int)statusForHostName : (NSString * )domain;
-(NSData *)dataForDomainArray : (NSArray *)domainArray;
-(NSData *)dataForDomain : (NSString *)domainName isEnabled : (BOOL)enabled;
-(NSDictionary *)dictionaryForSharedSecret : (NSString *)secret domain : (NSString *)domainName key : (NSString *)keyName;
-(BOOL)domainAlreadyInList : (NSString *)domainString;
-(NSString *)trimCharactersFromDomain : (NSString *)domain;


// Delegate methods
-(void)authorizationViewDidAuthorize : (SFAuthorizationView *)view;
-(void)authorizationViewDidDeauthorize : (SFAuthorizationView *)view;
-(void)mainViewDidLoad;
-(void)controlTextDidChange : (NSNotification *) notification;

@end
