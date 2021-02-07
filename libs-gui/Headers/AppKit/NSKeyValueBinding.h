/** <title>NSKeyValueBinding</title>

   <abstract>Interface declaration for key value binding</abstract>

   Copyright <copy>(C) 2006 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: June 2006

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSKeyValueBinding
#define _GNUstep_H_NSKeyValueBinding

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

@class NSString;
@class NSArray;
@class NSDictionary;


@interface NSObject (NSKeyValueBindingCreation)

+ (void) exposeBinding: (NSString *)key;

- (NSArray *) exposedBindings;
- (Class) valueClassForBinding: (NSString *)binding;
- (void) bind: (NSString *)binding 
     toObject: (id)controller 
  withKeyPath: (NSString *)keyPath 
      options: (NSDictionary *)options;
- (void) unbind: (NSString *)binding;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSDictionary *) infoForBinding: (NSString *)binding;
#endif
@end

@interface NSObject (NSPlaceholder)

+ (id) defaultPlaceholderForMarker: (id)marker 
                       withBinding: (NSString *)binding;
+ (void) setDefaultPlaceholder: (id)placeholder 
                     forMarker: (id)marker 
                   withBinding: (NSString *)binding;

@end


@interface NSObject (NSEditor)

- (BOOL) commitEditing;
- (void) discardEditing;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) commitEditingWithDelegate: (id)delegate
                 didCommitSelector: (SEL)didCommitSelector 
                       contextInfo: (void *)contextInfo;
#endif

@end

@interface NSObject (NSEditorRegistration)

- (void) objectDidBeginEditing: (id)editor;
- (void) objectDidEndEditing: (id)editor;

@end

// Keys in options dictionary

// binding values

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
APPKIT_EXPORT BOOL NSIsControllerMarker(id object);

// Keys in dictionary returned by infoForBinding
APPKIT_EXPORT NSString *NSObservedObjectKey;
APPKIT_EXPORT NSString *NSObservedKeyPathKey;
APPKIT_EXPORT NSString *NSOptionsKey;

// special markers
APPKIT_EXPORT id NSMultipleValuesMarker;
APPKIT_EXPORT id NSNoSelectionMarker;
APPKIT_EXPORT id NSNotApplicableMarker;

// Binding name constants
APPKIT_EXPORT NSString *NSAlignmentBinding;
APPKIT_EXPORT NSString *NSContentArrayBinding;
APPKIT_EXPORT NSString *NSContentBinding;
APPKIT_EXPORT NSString *NSContentObjectBinding;
APPKIT_EXPORT NSString *NSContentValuesBinding;
APPKIT_EXPORT NSString *NSEditableBinding;
APPKIT_EXPORT NSString *NSEnabledBinding;
APPKIT_EXPORT NSString *NSFontBinding;
APPKIT_EXPORT NSString *NSFontNameBinding;
APPKIT_EXPORT NSString *NSFontSizeBinding;
APPKIT_EXPORT NSString *NSHiddenBinding;
APPKIT_EXPORT NSString *NSSelectedIndexBinding;
APPKIT_EXPORT NSString *NSSelectedObjectBinding;
APPKIT_EXPORT NSString *NSSelectedTagBinding;
APPKIT_EXPORT NSString *NSSelectedValueBinding;
APPKIT_EXPORT NSString *NSSelectionIndexesBinding;
APPKIT_EXPORT NSString *NSSortDescriptorsBinding;
APPKIT_EXPORT NSString *NSTextColorBinding;
APPKIT_EXPORT NSString *NSTitleBinding;
APPKIT_EXPORT NSString *NSToolTipBinding;
APPKIT_EXPORT NSString *NSValueBinding;

//Binding options constants
APPKIT_EXPORT NSString *NSAllowsEditingMultipleValuesSelectionBindingOption;
APPKIT_EXPORT NSString *NSAllowsNullArgumentBindingOption;
APPKIT_EXPORT NSString *NSConditionallySetsEditableBindingOption;
APPKIT_EXPORT NSString *NSConditionallySetsEnabledBindingOption;
APPKIT_EXPORT NSString *NSConditionallySetsHiddenBindingOption;
APPKIT_EXPORT NSString *NSContinuouslyUpdatesValueBindingOption;
APPKIT_EXPORT NSString *NSCreatesSortDescriptorBindingOption;
APPKIT_EXPORT NSString *NSDeletesObjectsOnRemoveBindingsOption;
APPKIT_EXPORT NSString *NSDisplayNameBindingOption;
APPKIT_EXPORT NSString *NSDisplayPatternBindingOption;
APPKIT_EXPORT NSString *NSHandlesContentAsCompoundValueBindingOption;
APPKIT_EXPORT NSString *NSInsertsNullPlaceholderBindingOption;
APPKIT_EXPORT NSString *NSInvokesSeparatelyWithArrayObjectsBindingOption;
APPKIT_EXPORT NSString *NSMultipleValuesPlaceholderBindingOption;
APPKIT_EXPORT NSString *NSNoSelectionPlaceholderBindingOption;
APPKIT_EXPORT NSString *NSNotApplicablePlaceholderBindingOption;
APPKIT_EXPORT NSString *NSNullPlaceholderBindingOption;
APPKIT_EXPORT NSString *NSPredicateFormatBindingOption;
APPKIT_EXPORT NSString *NSRaisesForNotApplicableKeysBindingOption;
APPKIT_EXPORT NSString *NSSelectorNameBindingOption;
APPKIT_EXPORT NSString *NSSelectsAllWhenSettingContentBindingOption;
APPKIT_EXPORT NSString *NSValidatesImmediatelyBindingOption;
APPKIT_EXPORT NSString *NSValueTransformerNameBindingOption;
APPKIT_EXPORT NSString *NSValueTransformerBindingOption;
#endif

#endif // OS_API_VERSION

#endif // _GNUstep_H_NSKeyValueBinding
