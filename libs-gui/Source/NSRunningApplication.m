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

/*
 * FIXME: This whole class is a stub!
 */

#import <AppKit/NSRunningApplication.h>

@implementation NSRunningApplication
- (BOOL)isTerminated
{
  return NO;
}

- (BOOL)isFinishedLaunching
{
  return YES;
}

- (BOOL)isHidden
{
  return NO;
}

- (BOOL)isActive
{
  return YES;
}

- (NSApplicationActivationPolicy)activationPolicy
{
  return 0;
}

- (NSString *)localizedName
{
  return nil; 
}

- (NSString *)bundleIdentifier
{
  return nil;
}

- (NSURL *)bundleURL
{
  return nil;
}

- (NSURL *)executableURL
{
  return nil;
}

- (pid_t)processIdentifier
{
  return 0;
}

- (NSInteger)executableArchitecture
{
  return 0;
}

- (NSDate *)launchDate
{
  return nil;
}

- (NSImage *)icon
{
  return nil;
}

- (BOOL)ownsMenuBar
{
  return NO;
}

- (BOOL)hide
{
  return YES;
}

- (BOOL)unhide
{
  return YES;
}

- (BOOL)activateWithOptions:(NSApplicationActivationOptions)options
{
  return YES;
}

- (BOOL)terminate
{
  return YES;
}

- (BOOL)forceTerminate
{
  return YES;
}

+ (NSArray *)runningApplicationsWithBundleIdentifier:(NSString *)bundleIdentifier
{
  return nil;
}

+ (id)runningApplicationWithProcessIdentifier:(pid_t)pid
{
  return nil;
}

+ (id)currentApplication
{
  return nil;
}

+ (void)terminateAutomaticallyTerminableApplications
{
  return;
}
@end

@implementation NSWorkspace (NSWorkspaceRunningApplications)
- (NSArray *)runningApplications
{
  return nil;
}
@end
