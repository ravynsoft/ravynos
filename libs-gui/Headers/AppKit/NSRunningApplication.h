/*
   NSRunningApplication.m

   Provide information about a single instance of an app.

   Copyright (C) 2017 Free Software Foundation, Inc.

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017

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

#ifndef _GNUstep_H_NSRunningApplication
#define _GNUstep_H_NSRunningApplication

#import <Foundation/Foundation.h>
#import <AppKit/NSWorkspace.h>
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

typedef NSInteger NSApplicationActivationOptions;
enum {
  NSApplicationActivateAllWindows = 1 << 0,
  NSApplicationActivateIgnoringOtherApps = 1 << 1
};

typedef NSInteger NSApplicationActivationPolicy;
enum {
  NSApplicationActivationPolicyRegular, 
  NSApplicationActivationPolicyAccessory,
  NSApplicationActivationPolicyProhibited
};

@interface NSRunningApplication : NSObject
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, getter=isTerminated) BOOL terminated;
@property (readonly, getter=isFinishedLaunching) BOOL finishedLaunching;
@property (readonly, getter=isHidden) BOOL hidden;
@property (readonly, getter=isActive) BOOL active;
@property (readonly) NSApplicationActivationPolicy activationPolicy;
@property (readonly, copy) NSString *localizedName;
@property (readonly, copy) NSString *bundleIdentifier;
@property (readonly, copy) NSURL *bundleURL;
@property (readonly, copy) NSURL *executableURL;
@property (readonly) pid_t processIdentifier;
@property (readonly) NSInteger executableArchitecture;
@property (readonly, copy) NSDate *launchDate;
@property (readonly, retain) NSImage *icon;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
@property (readonly) BOOL ownsMenuBar;
#endif
#else
- (BOOL)isTerminated;
- (BOOL)isFinishedLaunching;
- (BOOL)isHidden;
- (BOOL)isActive;

- (NSApplicationActivationPolicy)activationPolicy;
- (NSString *)localizedName;
- (NSString *)bundleIdentifier;
- (NSURL *)bundleURL;
- (NSURL *)executableURL;

- (pid_t)processIdentifier;
- (NSInteger)executableArchitecture;

- (NSDate *)launchDate;
- (NSImage *)icon;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
- (BOOL)ownsMenuBar;
#endif
#endif

- (BOOL)hide;
- (BOOL)unhide;
- (BOOL)activateWithOptions:(NSApplicationActivationOptions)options;
- (BOOL)terminate;
- (BOOL)forceTerminate;

+ (NSArray *)runningApplicationsWithBundleIdentifier:(NSString *)bundleIdentifier;
+ (id)runningApplicationWithProcessIdentifier:(pid_t)pid;
+ (id)currentApplication;
+ (void)terminateAutomaticallyTerminableApplications;
@end

@interface NSWorkspace (NSWorkspaceRunningApplications)
#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, copy) NSArray *runningApplications;
#else
- (NSArray *)runningApplications;
#endif
@end

#endif

#endif
