/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * FILE: bootfiles.h
 * AUTH: Soren Spies (sspies)
 * DATE: 22 March 2006 (Copyright Apple Computer, Inc)
 * DESC: constants for boot caches
 */

#ifndef __BOOTFILES_H__
#define __BOOTFILES_H__

#include <paths.h>

/* Boot != Root directories in Apple_Boot */
#define kBootDirR "com.apple.boot.R"
#define kBootDirP "com.apple.boot.P"
#define kBootDirS "com.apple.boot.S"

/* Boot != Root key for firmware's /chosen  */
#define kBootRootActiveKey      "bootroot-active"

/* Recovery OS directory in Apple_Boot */
#define kRecoveryBootDir        "com.apple.recovery.boot"

/* Recovery boot hints */

/* With minor exceptions for things like reanimation, efiboot will
   bypass primary OS booting in favor of executing the booter from
   com.apple.recovery.boot any time the NVRAM variable recovery-boot-mode
   is set to any value.  The Recovery OS is generally responsible for
   interpreting these values and unsetting the variable as appropriate.
   See comments for each value as to which components set which values.
*/
#define kRecoveryBootVar        "recovery-boot-mode"  // how to boot Recovery

/* Recovery-based Guest Mode is used on FDE systems.
   Set by: any login panel (EFI, OS, or screen lock) when "guest" is selected
   Cleared by: Recovery OS (which component?), presumably on entry

   Guest mode can be enabled in Users & Groups, but is also implicitly
   enabled by Find My Mac (FMM).  FMM needs guest mode so that people who
   find FDE machines can get them online to receive "good samaritan,"
   "lock," and "wipe" messages.  Guest mode is restrictive and generally
   only allows connecting to networks and running Safari.
*/
#define kRecoveryBootModeGuest   "guest"             // guest boot (usu. once)

/* Locked mode is used by Find My Mac to restrict access to a machine.
   Set by: Find My Mac
   Cleared by: Find My Mac only with the locking user's authorization.

   When Find My Mac receives a remote message to lock the computer, FMM
   sets this variable and reboots.  The Recovery OS enters a highly
   restrictive mode which only displays a prompt for a PIN code.  This
   variable is only unset when the original user unlocks FMM.
*/
#define kRecoveryBootModeLocked     "locked"        // system is FMM-locked

/* FDE password reset mode helps users reset their passwords.
   Set by: efiboot
   Cleared by: Recovery OS on entry

   If a user has trouble remembering their password, they can click on
   "reset my password using iCloud" at EFI Login.  efiboot then sets this
   variable and boots the Recovery OS, which takes the user directly to
   a "reset password" panel.
*/
#define kRecoveryBootModeFDEPasswordReset   "fde-password-reset"  // forgot

/* FDE Recovery mode helps solve EFI Login issues
   Set by: efiboot
   Cleared by: Recovery OS on entry

   Users having trouble entering passwords at EFI Login can force power
   off while an "if you are having trouble" message is showing.  efiboot
   will notice that the message was visible when power was forced off and
   the next power-on will go to the Recovery OS with the variable set to
   this value.  The Recovery OS then guides the user towards a password
   change or disabling FDE.  The latter accomodates those with with
   hardware or software issues that prevent them from using FDE.
*/
#define kRecoveryBootModeFDERecovery        "fde-recovery"      // help!

#define kRecoveryBootModeActivationLock        "activation-lock"

/* A generic mode is defined, but has not yet been meaningfully used. */
// #define kRecoveryBootModeGeneric     "generic"
#define kRcevoeryBootModeRecovery       "unused"


/* The kernel */
#define kDefaultKernelPath  "/System/Library/Kernels/kernel"
//#define kDefaultKernel      "/mach_kernel"
#define kKernelSymfile        (_PATH_VARRUN "mach.sym")
// kKernelSymfile obsolete, remove when load.c deleted

/* The system extensions folder */
#define kSystemExtensionsDir  "/System/Library/Extensions"
/* The library extensions folder */
#define kLibraryExtensionsDir  "/Library/Extensions"

/* Prelinked kernel */
#define kPrelinkedKernelPath "/System/Library/PrelinkedKernels/prelinkedkernel"


/* The booter configuration file */
#define kBootConfig           "/Library/Preferences/SystemConfiguration/com.apple.Boot.plist"
#define kKernelFlagsKey       "Kernel Flags"
#define kMKextCacheKey        "MKext Cache"
#define kKernelNameKey        "Kernel"
#define kKernelCacheKey       "Kernel Cache"
#define kRootUUIDKey          "Root UUID"
#define kRootMatchKey         "Root Match"

#endif /* __BOOTFILES_H__ */
