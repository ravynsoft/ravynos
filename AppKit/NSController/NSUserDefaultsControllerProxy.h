//
//  NSUserDefaultsControllerProxy.h
//  AppKit
//
//  Created by Johannes Fortmann on 26.09.08.
//  Copyright 2008 -. All rights reserved.
//

#import <Foundation/NSObject.h>
@class NSMutableDictionary, NSUserDefaultsController;

@interface NSUserDefaultsControllerProxy : NSObject {
    NSUserDefaultsController *_controller;
    NSMutableDictionary *_cachedValues;
}
- (id)initWithController:(NSUserDefaultsController *)controller;
- (void)save;
- (void)revert;
- (void)revertToInitialValues;
- (BOOL)hasUnappliedChanges;
@end
