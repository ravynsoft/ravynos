/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#ifndef _MACH_O_DYLD_H_
#define _MACH_O_DYLD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif

#include <mach-o/loader.h>
#include <AvailabilityMacros.h>
#ifndef AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER
#define AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER
#endif

#ifndef ENUM_DYLD_BOOL
#define ENUM_DYLD_BOOL
#undef FALSE
#undef TRUE
enum DYLD_BOOL {
    FALSE,
    TRUE
};
#endif /* ENUM_DYLD_BOOL */

/*
 * The high level NS... API.
 */

/* Object file image API */
typedef enum {
    NSObjectFileImageFailure, /* for this a message is printed on stderr */
    NSObjectFileImageSuccess,
    NSObjectFileImageInappropriateFile,
    NSObjectFileImageArch,
    NSObjectFileImageFormat, /* for this a message is printed on stderr */
    NSObjectFileImageAccess
} NSObjectFileImageReturnCode;

typedef void * NSObjectFileImage;

/* limited implementation, only MH_BUNDLE files can be used */
extern NSObjectFileImageReturnCode NSCreateObjectFileImageFromFile(
    const char *pathName,
    NSObjectFileImage *objectFileImage);
extern NSObjectFileImageReturnCode NSCreateCoreFileImageFromFile(
    const char *pathName,
    NSObjectFileImage *objectFileImage);
/* not yet implemented */
extern NSObjectFileImageReturnCode NSCreateObjectFileImageFromMemory(
    void *address,
    unsigned long size, 
    NSObjectFileImage *objectFileImage);
extern enum DYLD_BOOL NSDestroyObjectFileImage(
    NSObjectFileImage objectFileImage);
/*
 * API on NSObjectFileImage's for:
 *   "for Each Symbol Definition In Object File Image" (for Dynamic Bundles)
 *   and the same thing for references
 */
extern unsigned long NSSymbolDefinitionCountInObjectFileImage(
    NSObjectFileImage objectFileImage);
extern const char * NSSymbolDefinitionNameInObjectFileImage(
    NSObjectFileImage objectFileImage,
    unsigned long ordinal);
extern unsigned long NSSymbolReferenceCountInObjectFileImage(
    NSObjectFileImage objectFileImage);
extern const char * NSSymbolReferenceNameInObjectFileImage(
    NSObjectFileImage objectFileImage,
    unsigned long ordinal,
    enum DYLD_BOOL *tentative_definition); /* can be NULL */
/*
 * API on NSObjectFileImage:
 *   "does Object File Image define symbol name X" (using sorted symbol table)
 *   and a way to get the named objective-C section
 */
extern enum DYLD_BOOL NSIsSymbolDefinedInObjectFileImage(
    NSObjectFileImage objectFileImage,
    const char *symbolName);
extern void * NSGetSectionDataInObjectFileImage(
    NSObjectFileImage objectFileImage,
    const char *segmentName,
    const char *sectionName,
    unsigned long *size); /* can be NULL */
/* SPI first appeared in Mac OS X 10.3 */
extern enum DYLD_BOOL NSHasModInitObjectFileImage(
    NSObjectFileImage objectFileImage)
    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

/* module API */
typedef void * NSModule;
extern const char * NSNameOfModule(
    NSModule m); 
extern const char * NSLibraryNameForModule(
    NSModule m);

/* limited implementation, only MH_BUNDLE files can be linked */
extern NSModule NSLinkModule(
    NSObjectFileImage objectFileImage, 
    const char *moduleName,
    unsigned long options);
#define NSLINKMODULE_OPTION_NONE		0x0
#define NSLINKMODULE_OPTION_BINDNOW		0x1
#define NSLINKMODULE_OPTION_PRIVATE		0x2
#define NSLINKMODULE_OPTION_RETURN_ON_ERROR	0x4
#define NSLINKMODULE_OPTION_DONT_CALL_MOD_INIT_ROUTINES 0x8
#define NSLINKMODULE_OPTION_TRAILING_PHYS_NAME	0x10

/* limited implementation, only modules loaded with NSLinkModule() can be
   unlinked */
extern enum DYLD_BOOL NSUnLinkModule(
    NSModule module, 
    unsigned long options);
#define NSUNLINKMODULE_OPTION_NONE			0x0
#define NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED	0x1
#define NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES	0x2

/* not yet implemented */
extern NSModule NSReplaceModule(
    NSModule moduleToReplace,
    NSObjectFileImage newObjectFileImage, 
    unsigned long options);

/* symbol API */
typedef void * NSSymbol;
extern enum DYLD_BOOL NSIsSymbolNameDefined(
    const char *symbolName);
extern enum DYLD_BOOL NSIsSymbolNameDefinedWithHint(
    const char *symbolName,
    const char *libraryNameHint);
extern enum DYLD_BOOL NSIsSymbolNameDefinedInImage(
    const struct mach_header *image,
    const char *symbolName);
extern NSSymbol NSLookupAndBindSymbol(
    const char *symbolName);
extern NSSymbol NSLookupAndBindSymbolWithHint(
    const char *symbolName,
    const char *libraryNameHint);
extern NSSymbol NSLookupSymbolInModule(
    NSModule module,
    const char *symbolName);
extern NSSymbol NSLookupSymbolInImage(
    const struct mach_header *image,
    const char *symbolName,
    unsigned long options);
#define NSLOOKUPSYMBOLINIMAGE_OPTION_BIND            0x0
#define NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_NOW        0x1
#define NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_FULLY      0x2
#define NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR 0x4
extern const char * NSNameOfSymbol(
    NSSymbol symbol);
extern void * NSAddressOfSymbol(
    NSSymbol symbol);
extern NSModule NSModuleForSymbol(
    NSSymbol symbol);

/* error handling API */
typedef enum {
    NSLinkEditFileAccessError,
    NSLinkEditFileFormatError,
    NSLinkEditMachResourceError,
    NSLinkEditUnixResourceError,
    NSLinkEditOtherError,
    NSLinkEditWarningError,
    NSLinkEditMultiplyDefinedError,
    NSLinkEditUndefinedError
} NSLinkEditErrors;

/*
 * For the NSLinkEditErrors value NSLinkEditOtherError these are the values
 * passed to the link edit error handler as the errorNumber (what would be an
 * errno value for NSLinkEditUnixResourceError or a kern_return_t value for
 * NSLinkEditMachResourceError).
 */
typedef enum {
    NSOtherErrorRelocation, 
    NSOtherErrorLazyBind,
    NSOtherErrorIndrLoop,
    NSOtherErrorLazyInit,
    NSOtherErrorInvalidArgs
} NSOtherErrorNumbers;

extern void NSLinkEditError(
    NSLinkEditErrors *c,
    int *errorNumber, 
    const char **fileName,
    const char **errorString);

typedef struct {
     void     (*undefined)(const char *symbolName);
     NSModule (*multiple)(NSSymbol s, NSModule oldModule, NSModule newModule); 
     void     (*linkEdit)(NSLinkEditErrors errorClass, int errorNumber,
                          const char *fileName, const char *errorString);
} NSLinkEditErrorHandlers;

extern void NSInstallLinkEditErrorHandlers(
    NSLinkEditErrorHandlers *handlers);

/* other API */
extern enum DYLD_BOOL NSAddLibrary(
    const char *pathName);
extern enum DYLD_BOOL NSAddLibraryWithSearching(
    const char *pathName);
extern const struct mach_header * NSAddImage(
    const char *image_name,
    unsigned long options);
#define NSADDIMAGE_OPTION_NONE                  	0x0
#define NSADDIMAGE_OPTION_RETURN_ON_ERROR       	0x1
#define NSADDIMAGE_OPTION_WITH_SEARCHING        	0x2
#define NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED 	0x4
#define NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME	0x8
extern long NSVersionOfRunTimeLibrary(
    const char *libraryName);
extern long NSVersionOfLinkTimeLibrary(
    const char *libraryName);
extern int _NSGetExecutablePath( /* SPI first appeared in Mac OS X 10.2 */
    char *buf,
    uint32_t *bufsize);

/*
 * The low level _dyld_... API.
 * (used by the objective-C runtime primarily)
 */
extern unsigned long _dyld_present(
    void);

extern unsigned long _dyld_image_count(
    void);
#ifdef __LP64__
extern struct mach_header_64 * _dyld_get_image_header(
    uint32_t image_index);
#else /* !defined(__LP64__) */
extern struct mach_header * _dyld_get_image_header(
    unsigned long image_index);
#endif /* !defined(__LP64__) */
extern unsigned long _dyld_get_image_vmaddr_slide(
    unsigned long image_index);
extern char * _dyld_get_image_name(
    unsigned long image_index);

extern void _dyld_register_func_for_add_image(
    void (*func)(struct mach_header *mh, unsigned long vmaddr_slide));
extern void _dyld_register_func_for_remove_image(
    void (*func)(struct mach_header *mh, unsigned long vmaddr_slide));
extern void _dyld_register_func_for_link_module(
    void (*func)(NSModule module));
/* not yet implemented */
extern void _dyld_register_func_for_unlink_module(
    void (*func)(NSModule module));
/* not yet implemented */
extern void _dyld_register_func_for_replace_module(
    void (*func)(NSModule oldmodule, NSModule newmodule));
extern void _dyld_get_objc_module_sect_for_module(
    NSModule module,
    void **objc_module,
    unsigned long *size);
extern void _dyld_bind_objc_module(
    void *objc_module);
extern enum DYLD_BOOL _dyld_bind_fully_image_containing_address(
    unsigned long *address);
extern enum DYLD_BOOL _dyld_image_containing_address(
    unsigned long address);
/* SPI first appeared in Mac OS X 10.3 */
extern struct mach_header * _dyld_get_image_header_containing_address(
    unsigned long address)
    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

extern void _dyld_moninit(
    void (*monaddition)(char *lowpc, char *highpc));
extern enum DYLD_BOOL _dyld_launched_prebound(
    void);
/* SPI first appeared in Mac OS X 10.3 */
extern enum DYLD_BOOL _dyld_all_twolevel_modules_prebound(
    void)
    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

extern void _dyld_lookup_and_bind(
    const char *symbol_name,
    unsigned long *address,
    void **module);
extern void _dyld_lookup_and_bind_with_hint(
    const char *symbol_name,
    const char *library_name_hint,
    unsigned long *address,
    void **module);
extern void _dyld_lookup_and_bind_objc(
    const char *symbol_name,
    unsigned long *address,
    void **module);
extern void _dyld_lookup_and_bind_fully(
    const char *symbol_name,
    unsigned long *address,
    void **module);

__private_extern__ int _dyld_func_lookup(
    const char *dyld_func_name,
    unsigned long *address);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MACH_O_DYLD_H_ */
