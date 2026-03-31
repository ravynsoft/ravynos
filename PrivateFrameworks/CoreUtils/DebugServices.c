/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

    To Do:

    - Use StackWalk on Windows to optionally print stack frames.
 */

#if 0
#pragma mark == Includes ==
#endif

//===========================================================================================================================
//	Includes
//===========================================================================================================================

#if ( !KERNEL )
    #include    <ctype.h>
    #include    <stdio.h>
    #include    <string.h>
#endif

#include    "CommonServices.h"

#include    "DebugServices.h"

#if ( DEBUG )

#if ( TARGET_OS_WIN32 )
    #include    <time.h>

    #if ( !TARGET_OS_WINDOWS_CE )
        #include    <fcntl.h>
        #include    <io.h>
    #endif
#endif

#if ( DEBUG_IDEBUG_ENABLED && TARGET_API_MAC_OSX_KERNEL )
    #include    <IOKit/IOLib.h>
#endif

// If MDNS_DEBUGMSGS is defined (even if defined 0), it is aware of mDNS and it is probably safe to include mDNSEmbeddedAPI.h.

#if ( defined( MDNS_DEBUGMSGS ) )
    #include    "mDNSEmbeddedAPI.h"
#endif

#if 0
#pragma mark == Macros ==
#endif

//===========================================================================================================================
//	Macros
//===========================================================================================================================

#define DebugIsPrint( C )       ( ( ( C ) >= 0x20 ) && ( ( C ) <= 0x7E ) )

#if 0
#pragma mark == Prototypes ==
#endif

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static OSStatus DebugPrint( DebugLevel inLevel, char *inData, size_t inSize );

// fprintf

#if ( DEBUG_FPRINTF_ENABLED )
static OSStatus DebugFPrintFInit( DebugOutputTypeFlags inFlags, const char *inFilename );
static void     DebugFPrintFPrint( char *inData, size_t inSize );
#endif

// iDebug (Mac OS X user and kernel)

#if ( DEBUG_IDEBUG_ENABLED )
static OSStatus DebugiDebugInit( void );
static void     DebugiDebugPrint( char *inData, size_t inSize );
#endif

// kprintf (Mac OS X Kernel)

#if ( DEBUG_KPRINTF_ENABLED )
static void DebugKPrintFPrint( char *inData, size_t inSize );
#endif

// Mac OS X IOLog (Mac OS X Kernel)

#if ( DEBUG_MAC_OS_X_IOLOG_ENABLED )
static void DebugMacOSXIOLogPrint( char *inData, size_t inSize );
#endif

// Mac OS X Log

#if ( TARGET_OS_MAC )
static OSStatus DebugMacOSXLogInit( void );
static void     DebugMacOSXLogPrint( char *inData, size_t inSize );
#endif

// Windows Debugger

#if ( TARGET_OS_WIN32 )
static void DebugWindowsDebuggerPrint( char *inData, size_t inSize );
#endif

// Windows Event Log

#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
static OSStatus DebugWindowsEventLogInit( const char *inName, HMODULE inModule );
static void DebugWindowsEventLogPrint( DebugLevel inLevel, char *inData, size_t inSize );
#endif

// DebugLib support

#if ( DEBUG_CORE_SERVICE_ASSERTS_ENABLED )
static pascal void
DebugAssertOutputHandler(
    OSType inComponentSignature,
    UInt32 inOptions,
    const char *        inAssertionString,
    const char *        inExceptionString,
    const char *        inErrorString,
    const char *        inFileName,
    long inLineNumber,
    void *              inValue,
    ConstStr255Param inOutputMsg );
#endif

// Utilities

static char *   DebugNumVersionToString( uint32_t inVersion, char *inString );

#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
static void DebugWinEnableConsole( void );
#endif

#if ( TARGET_OS_WIN32 )
static TCHAR *
DebugWinCharToTCharString(
    const char *    inCharString,
    size_t inCharCount,
    TCHAR *         outTCharString,
    size_t inTCharCountMax,
    size_t *        outTCharCount );
#endif

#if 0
#pragma mark == Globals ==
#endif

//===========================================================================================================================
//	Private Globals
//===========================================================================================================================

// General

static bool gDebugInitialized               = false;
static DebugOutputType gDebugOutputType                = kDebugOutputTypeNone;
static DebugLevel gDebugPrintLevelMin             = kDebugLevelInfo;
static DebugLevel gDebugPrintLevelMax             = kDebugLevelMax;
static DebugLevel gDebugBreakLevel                = kDebugLevelAssert;
#if ( DEBUG_CORE_SERVICE_ASSERTS_ENABLED )
static DebugAssertOutputHandlerUPP gDebugAssertOutputHandlerUPP    = NULL;
#endif

// Custom

static DebugOutputFunctionPtr gDebugCustomOutputFunction      = NULL;
static void *                               gDebugCustomOutputContext       = NULL;

// fprintf

#if ( DEBUG_FPRINTF_ENABLED )
static FILE *                           gDebugFPrintFFile               = NULL;
#endif

// MacOSXLog

#if ( TARGET_OS_MAC )
typedef int ( *DebugMacOSXLogFunctionPtr )( const char *inFormat, ... );

static DebugMacOSXLogFunctionPtr gDebugMacOSXLogFunction         = NULL;
#endif

// WindowsEventLog


#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
static HANDLE gDebugWindowsEventLogEventSource = NULL;
#endif

#if 0
#pragma mark -
#pragma mark == General ==
#endif

//===========================================================================================================================
//	DebugInitialize
//===========================================================================================================================

DEBUG_EXPORT OSStatus   DebugInitialize( DebugOutputType inType, ... )
{
    OSStatus err;
    DebugOutputType type;
    va_list args;

    va_start( args, inType );

    // Set up DebugLib stuff (if building with Debugging.h).

#if ( DEBUG_CORE_SERVICE_ASSERTS_ENABLED )
    if( !gDebugAssertOutputHandlerUPP )
    {
        gDebugAssertOutputHandlerUPP = NewDebugAssertOutputHandlerUPP( DebugAssertOutputHandler );
        check( gDebugAssertOutputHandlerUPP );
        if( gDebugAssertOutputHandlerUPP )
        {
            InstallDebugAssertOutputHandler( gDebugAssertOutputHandlerUPP );
        }
    }
#endif

    // Pre-process meta-output kind to pick an appropriate output kind for the platform.

    type = inType;
    if( type == kDebugOutputTypeMetaConsole )
    {
        #if ( TARGET_OS_MAC )
        type = kDebugOutputTypeMacOSXLog;
        #elif ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
            #if ( DEBUG_FPRINTF_ENABLED )
        type = kDebugOutputTypeFPrintF;
            #else
        type = kDebugOutputTypeWindowsDebugger;
            #endif
        #elif ( TARGET_API_MAC_OSX_KERNEL )
            #if ( DEBUG_MAC_OS_X_IOLOG_ENABLED )
        type = kDebugOutputTypeMacOSXIOLog;
            #elif ( DEBUG_IDEBUG_ENABLED )
        type = kDebugOutputTypeiDebug;
            #elif ( DEBUG_KPRINTF_ENABLED )
        type = kDebugOutputTypeKPrintF;
            #endif
        #else
            #if ( DEBUG_FPRINTF_ENABLED )
        type = kDebugOutputTypeFPrintF;
            #endif
        #endif
    }

    // Process output kind.

    gDebugOutputType = type;
    switch( type )
    {
    case kDebugOutputTypeNone:
        err = kNoErr;
        break;

    case kDebugOutputTypeCustom:
        gDebugCustomOutputFunction = va_arg( args, DebugOutputFunctionPtr );
        gDebugCustomOutputContext  = va_arg( args, void * );
        err = kNoErr;
        break;

#if ( DEBUG_FPRINTF_ENABLED )
    case kDebugOutputTypeFPrintF:
        if( inType == kDebugOutputTypeMetaConsole )
        {
            err = DebugFPrintFInit( kDebugOutputTypeFlagsStdErr, NULL );
        }
        else
        {
            DebugOutputTypeFlags flags;
            const char *                filename;

            flags = (DebugOutputTypeFlags) va_arg( args, unsigned int );
            if( ( flags & kDebugOutputTypeFlagsTypeMask ) == kDebugOutputTypeFlagsFile )
            {
                filename = va_arg( args, const char * );
            }
            else
            {
                filename = NULL;
            }
            err = DebugFPrintFInit( flags, filename );
        }
        break;
#endif

#if ( DEBUG_IDEBUG_ENABLED )
    case kDebugOutputTypeiDebug:
        err = DebugiDebugInit();
        break;
#endif

#if ( DEBUG_KPRINTF_ENABLED )
    case kDebugOutputTypeKPrintF:
        err = kNoErr;
        break;
#endif

#if ( DEBUG_MAC_OS_X_IOLOG_ENABLED )
    case kDebugOutputTypeMacOSXIOLog:
        err = kNoErr;
        break;
#endif

#if ( TARGET_OS_MAC )
    case kDebugOutputTypeMacOSXLog:
        err = DebugMacOSXLogInit();
        break;
#endif

#if ( TARGET_OS_WIN32 )
    case kDebugOutputTypeWindowsDebugger:
        err = kNoErr;
        break;
#endif

#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
    case kDebugOutputTypeWindowsEventLog:
    {
        const char *        name;
        HMODULE module;

        name   = va_arg( args, const char * );
        module = va_arg( args, HMODULE );
        err = DebugWindowsEventLogInit( name, module );
    }
    break;
#endif

    default:
        err = kParamErr;
        goto exit;
    }
    gDebugInitialized = true;

exit:
    va_end( args );
    return( err );
}

//===========================================================================================================================
//	DebugFinalize
//===========================================================================================================================

DEBUG_EXPORT void       DebugFinalize( void )
{
#if ( DEBUG_CORE_SERVICE_ASSERTS_ENABLED )
    check( gDebugAssertOutputHandlerUPP );
    if( gDebugAssertOutputHandlerUPP )
    {
        InstallDebugAssertOutputHandler( NULL );
        DisposeDebugAssertOutputHandlerUPP( gDebugAssertOutputHandlerUPP );
        gDebugAssertOutputHandlerUPP = NULL;
    }
#endif
}

//===========================================================================================================================
//	DebugGetProperty
//===========================================================================================================================

DEBUG_EXPORT OSStatus   DebugGetProperty( DebugPropertyTag inTag, ... )
{
    OSStatus err;
    va_list args;
    DebugLevel *        level;

    va_start( args, inTag );
    switch( inTag )
    {
    case kDebugPropertyTagPrintLevelMin:
        level  = va_arg( args, DebugLevel * );
        *level = gDebugPrintLevelMin;
        err = kNoErr;
        break;

    case kDebugPropertyTagPrintLevelMax:
        level  = va_arg( args, DebugLevel * );
        *level = gDebugPrintLevelMax;
        err = kNoErr;
        break;

    case kDebugPropertyTagBreakLevel:
        level  = va_arg( args, DebugLevel * );
        *level = gDebugBreakLevel;
        err = kNoErr;
        break;

    default:
        err = kUnsupportedErr;
        break;
    }
    va_end( args );
    return( err );
}

//===========================================================================================================================
//	DebugSetProperty
//===========================================================================================================================

DEBUG_EXPORT OSStatus   DebugSetProperty( DebugPropertyTag inTag, ... )
{
    OSStatus err;
    va_list args;
    DebugLevel level;

    va_start( args, inTag );
    switch( inTag )
    {
    case kDebugPropertyTagPrintLevelMin:
        level  = va_arg( args, DebugLevel );
        gDebugPrintLevelMin = level;
        err = kNoErr;
        break;

    case kDebugPropertyTagPrintLevelMax:
        level  = va_arg( args, DebugLevel );
        gDebugPrintLevelMax = level;
        err = kNoErr;
        break;

    case kDebugPropertyTagBreakLevel:
        level  = va_arg( args, DebugLevel );
        gDebugBreakLevel = level;
        err = kNoErr;
        break;

    default:
        err = kUnsupportedErr;
        break;
    }
    va_end( args );
    return( err );
}

#if 0
#pragma mark -
#pragma mark == Output ==
#endif

//===========================================================================================================================
//	DebugPrintF
//===========================================================================================================================

DEBUG_EXPORT size_t DebugPrintF( DebugLevel inLevel, const char *inFormat, ... )
{
    va_list args;
    size_t n;

    // Skip if the level is not in the enabled range..

    if( ( inLevel < gDebugPrintLevelMin ) || ( inLevel > gDebugPrintLevelMax ) )
    {
        n = 0;
        goto exit;
    }

    va_start( args, inFormat );
    n = DebugPrintFVAList( inLevel, inFormat, args );
    va_end( args );

exit:
    return( n );
}

//===========================================================================================================================
//	DebugPrintFVAList
//===========================================================================================================================

DEBUG_EXPORT size_t DebugPrintFVAList( DebugLevel inLevel, const char *inFormat, va_list inArgs )
{
    size_t n;
    char buffer[ 512 ];

    // Skip if the level is not in the enabled range..

    if( ( inLevel < gDebugPrintLevelMin ) || ( inLevel > gDebugPrintLevelMax ) )
    {
        n = 0;
        goto exit;
    }

    n = DebugSNPrintFVAList( buffer, sizeof( buffer ), inFormat, inArgs );
    DebugPrint( inLevel, buffer, (size_t) n );

exit:
    return( n );
}

//===========================================================================================================================
//	DebugPrint
//===========================================================================================================================

static OSStatus DebugPrint( DebugLevel inLevel, char *inData, size_t inSize )
{
    OSStatus err;

    // Skip if the level is not in the enabled range..

    if( ( inLevel < gDebugPrintLevelMin ) || ( inLevel > gDebugPrintLevelMax ) )
    {
        err = kRangeErr;
        goto exit;
    }

    // Printing is not safe at interrupt time so check for this and warn with an interrupt safe mechanism (if available).

    if( DebugTaskLevel() & kDebugInterruptLevelMask )
    {
        err = kExecutionStateErr;
        goto exit;
    }

    // Initialize the debugging library if it hasn't already been initialized (allows for zero-config usage).

    if( !gDebugInitialized )
    {
        debug_initialize( kDebugOutputTypeMetaConsole );
    }

    // Print based on the current output type.

    switch( gDebugOutputType )
    {
    case kDebugOutputTypeNone:
        break;

    case kDebugOutputTypeCustom:
        if( gDebugCustomOutputFunction )
        {
            gDebugCustomOutputFunction( inData, inSize, gDebugCustomOutputContext );
        }
        break;

#if ( DEBUG_FPRINTF_ENABLED )
    case kDebugOutputTypeFPrintF:
        DebugFPrintFPrint( inData, inSize );
        break;
#endif

#if ( DEBUG_IDEBUG_ENABLED )
    case kDebugOutputTypeiDebug:
        DebugiDebugPrint( inData, inSize );
        break;
#endif

#if ( DEBUG_KPRINTF_ENABLED )
    case kDebugOutputTypeKPrintF:
        DebugKPrintFPrint( inData, inSize );
        break;
#endif

#if ( DEBUG_MAC_OS_X_IOLOG_ENABLED )
    case kDebugOutputTypeMacOSXIOLog:
        DebugMacOSXIOLogPrint( inData, inSize );
        break;
#endif

#if ( TARGET_OS_MAC )
    case kDebugOutputTypeMacOSXLog:
        DebugMacOSXLogPrint( inData, inSize );
        break;
#endif

#if ( TARGET_OS_WIN32 )
    case kDebugOutputTypeWindowsDebugger:
        DebugWindowsDebuggerPrint( inData, inSize );
        break;
#endif

#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
    case kDebugOutputTypeWindowsEventLog:
        DebugWindowsEventLogPrint( inLevel, inData, inSize );
        break;
#endif

    default:
        break;
    }
    err = kNoErr;

exit:
    return( err );
}

//===========================================================================================================================
//	DebugPrintAssert
//
//	Warning: This routine relies on several of the strings being string constants that will exist forever because the
//           underlying logMsg API that does the printing is asynchronous so it cannot use temporary/stack-based
//           pointer variables (e.g. local strings). The debug macros that invoke this function only use constant
//           constant strings, but if this function is invoked directly from other places, it must use constant strings.
//===========================================================================================================================

DEBUG_EXPORT void
DebugPrintAssert(
    int_least32_t inErrorCode,
    const char *    inAssertString,
    const char *    inMessage,
    const char *    inFilename,
    int_least32_t inLineNumber,
    const char *    inFunction )
{
    // Skip if the level is not in the enabled range..

    if( ( kDebugLevelAssert < gDebugPrintLevelMin ) || ( kDebugLevelAssert > gDebugPrintLevelMax ) )
    {
        return;
    }

    if( inErrorCode != 0 )
    {
        DebugPrintF(
            kDebugLevelAssert,
            "\n"
            "[ASSERT] error:  %ld (%m)\n"
            "[ASSERT] where:  \"%s\", line %ld, \"%s\"\n"
            "\n",
            inErrorCode, inErrorCode,
            inFilename ? inFilename : "",
            inLineNumber,
            inFunction ? inFunction : "" );
    }
    else
    {
        DebugPrintF(
            kDebugLevelAssert,
            "\n"
            "[ASSERT] assert: \"%s\" %s\n"
            "[ASSERT] where:  \"%s\", line %ld, \"%s\"\n"
            "\n",
            inAssertString ? inAssertString : "",
            inMessage ? inMessage : "",
            inFilename ? inFilename : "",
            inLineNumber,
            inFunction ? inFunction : "" );
    }

    // Break into the debugger if enabled.

    #if ( TARGET_OS_WIN32 )
    if( gDebugBreakLevel <= kDebugLevelAssert )
    {
        if( IsDebuggerPresent() )
        {
            DebugBreak();
        }
    }
    #endif
}

#if 0
#pragma mark -
#endif

#if ( DEBUG_FPRINTF_ENABLED )
//===========================================================================================================================
//	DebugFPrintFInit
//===========================================================================================================================

static OSStatus DebugFPrintFInit( DebugOutputTypeFlags inFlags, const char *inFilename )
{
    OSStatus err;
    DebugOutputTypeFlags typeFlags;

    typeFlags = inFlags & kDebugOutputTypeFlagsTypeMask;
    if( typeFlags == kDebugOutputTypeFlagsStdOut )
    {
        #if ( TARGET_OS_WIN32 )
        DebugWinEnableConsole();
        #endif

        gDebugFPrintFFile = stdout;
    }
    else if( typeFlags == kDebugOutputTypeFlagsStdErr )
    {
        #if ( TARGET_OS_WIN32 )
        DebugWinEnableConsole();
        #endif

        gDebugFPrintFFile = stdout;
    }
    else if( typeFlags == kDebugOutputTypeFlagsFile )
    {
        require_action_quiet( inFilename && ( *inFilename != '\0' ), exit, err = kOpenErr );

        gDebugFPrintFFile = fopen( inFilename, "a" );
        require_action_quiet( gDebugFPrintFFile, exit, err = kOpenErr );
    }
    else
    {
        err = kParamErr;
        goto exit;
    }
    err = kNoErr;

exit:
    return( err );
}

//===========================================================================================================================
//	DebugFPrintFPrint
//===========================================================================================================================

static void DebugFPrintFPrint( char *inData, size_t inSize )
{
    char *      p;
    char *      q;

    // Convert \r to \n. fprintf will interpret \n and convert to whatever is appropriate for the platform.

    p = inData;
    q = p + inSize;
    while( p < q )
    {
        if( *p == '\r' )
        {
            *p = '\n';
        }
        ++p;
    }

    // Write the data and flush.

    if( gDebugFPrintFFile )
    {
        fprintf( gDebugFPrintFFile, "%.*s", (int) inSize, inData );
        fflush( gDebugFPrintFFile );
    }
}
#endif  // DEBUG_FPRINTF_ENABLED

#if ( DEBUG_IDEBUG_ENABLED )
//===========================================================================================================================
//	DebugiDebugInit
//===========================================================================================================================

static OSStatus DebugiDebugInit( void )
{
    OSStatus err;

    #if ( TARGET_API_MAC_OSX_KERNEL )

    extern uint32_t *       _giDebugReserved1;

    // Emulate the iDebugSetOutputType macro in iDebugServices.h.
    // Note: This is not thread safe, but neither is iDebugServices.h nor iDebugKext.

    if( !_giDebugReserved1 )
    {
        _giDebugReserved1 = (uint32_t *) IOMalloc( sizeof( uint32_t ) );
        require_action_quiet( _giDebugReserved1, exit, err = kNoMemoryErr );
    }
    *_giDebugReserved1 = 0x00010000U;
    err = kNoErr;
exit:
    #else

    __private_extern__ void iDebugSetOutputTypeInternal( uint32_t inType );

    iDebugSetOutputTypeInternal( 0x00010000U );
    err = kNoErr;

    #endif

    return( err );
}

//===========================================================================================================================
//	DebugiDebugPrint
//===========================================================================================================================

static void DebugiDebugPrint( char *inData, size_t inSize )
{
    #if ( TARGET_API_MAC_OSX_KERNEL )

    // Locally declared here so we do not need to include iDebugKext.h.
    // Note: IOKit uses a global namespace for all code and only a partial link occurs at build time. When the
    // KEXT is loaded, the runtime linker will link in this extern'd symbol (assuming iDebug is present).
    // _giDebugLogInternal is actually part of IOKit proper so this should link even if iDebug is not present.

    typedef void ( *iDebugLogFunctionPtr )( uint32_t inLevel, uint32_t inTag, const char *inFormat, ... );

    extern iDebugLogFunctionPtr _giDebugLogInternal;

    if( _giDebugLogInternal )
    {
        _giDebugLogInternal( 0, 0, "%.*s", (int) inSize, inData );
    }

    #else

    __private_extern__ void iDebugLogInternal( uint32_t inLevel, uint32_t inTag, const char *inFormat, ... );

    iDebugLogInternal( 0, 0, "%.*s", (int) inSize, inData );

    #endif
}
#endif

#if ( DEBUG_KPRINTF_ENABLED )
//===========================================================================================================================
//	DebugKPrintFPrint
//===========================================================================================================================

static void DebugKPrintFPrint( char *inData, size_t inSize )
{
    extern void kprintf( const char *inFormat, ... );

    kprintf( "%.*s", (int) inSize, inData );
}
#endif

#if ( DEBUG_MAC_OS_X_IOLOG_ENABLED )
//===========================================================================================================================
//	DebugMacOSXIOLogPrint
//===========================================================================================================================

static void DebugMacOSXIOLogPrint( char *inData, size_t inSize )
{
    extern void IOLog( const char *inFormat, ... );

    IOLog( "%.*s", (int) inSize, inData );
}
#endif

#if ( TARGET_OS_MAC )
//===========================================================================================================================
//	DebugMacOSXLogInit
//===========================================================================================================================

static OSStatus DebugMacOSXLogInit( void )
{
    OSStatus err;
    CFStringRef path;
    CFURLRef url;
    CFBundleRef bundle;
    CFStringRef functionName;
    void *          functionPtr;

    bundle = NULL;

    // Create a bundle reference for System.framework.

    path = CFSTR( "/System/Library/Frameworks/System.framework" );
    url = CFURLCreateWithFileSystemPath( NULL, path, kCFURLPOSIXPathStyle, true );
    require_action_quiet( url, exit, err = memFullErr );

    bundle = CFBundleCreate( NULL, url );
    CFRelease( url );
    require_action_quiet( bundle, exit, err = memFullErr );

    // Get a ptr to the system's "printf" function from System.framework.

    functionName = CFSTR( "printf" );
    functionPtr = CFBundleGetFunctionPointerForName( bundle, functionName );
    require_action_quiet( functionPtr, exit, err = memFullErr );

    // Success! Note: The bundle cannot be released because it would invalidate the function ptr.

    gDebugMacOSXLogFunction = (DebugMacOSXLogFunctionPtr) functionPtr;
    bundle = NULL;
    err = noErr;

exit:
    if( bundle )
    {
        CFRelease( bundle );
    }
    return( err );
}

//===========================================================================================================================
//	DebugMacOSXLogPrint
//===========================================================================================================================

static void DebugMacOSXLogPrint( char *inData, size_t inSize )
{
    if( gDebugMacOSXLogFunction )
    {
        gDebugMacOSXLogFunction( "%.*s", (int) inSize, inData );
    }
}
#endif

#if ( TARGET_OS_WIN32 )
//===========================================================================================================================
//	DebugWindowsDebuggerPrint
//===========================================================================================================================

void    DebugWindowsDebuggerPrint( char *inData, size_t inSize )
{
    TCHAR buffer[ 512 ];
    const char *        src;
    const char *        end;
    TCHAR *             dst;
    char c;

    // Copy locally and null terminate the string. This also converts from char to TCHAR in case we are
    // building with UNICODE enabled since the input is always char. Also convert \r to \n in the process.

    src = inData;
    if( inSize >= sizeof_array( buffer ) )
    {
        inSize = sizeof_array( buffer ) - 1;
    }
    end = src + inSize;
    dst = buffer;
    while( src < end )
    {
        c = *src++;
        if( c == '\r' )
        {
            c = '\n';
        }
        *dst++ = (TCHAR) c;
    }
    *dst = 0;

    // Print out the string to the debugger.

    OutputDebugString( buffer );
}
#endif

#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
//===========================================================================================================================
//	DebugWindowsEventLogInit
//===========================================================================================================================

static OSStatus DebugWindowsEventLogInit( const char *inName, HMODULE inModule )
{
    OSStatus err;
    HKEY key;
    TCHAR name[ 128 ];
    const char *        src;
    TCHAR path[ MAX_PATH ];
    size_t size;
    DWORD typesSupported;
    DWORD n;

    key = NULL;

    // Use a default name if needed then convert the name to TCHARs so it works on ANSI or Unicode builds.

    if( !inName || ( *inName == '\0' ) )
    {
        inName = "DefaultApp";
    }
    DebugWinCharToTCharString( inName, kSizeCString, name, sizeof( name ), NULL );

    // Build the path string using the fixed registry path and app name.

    src = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    DebugWinCharToTCharString( src, kSizeCString, path, sizeof_array( path ), &size );
    DebugWinCharToTCharString( inName, kSizeCString, path + size, sizeof_array( path ) - size, NULL );

    // Add/Open the source name as a sub-key under the Application key in the EventLog registry key.

    err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL );
    require_noerr_quiet( err, exit );

    // Set the path in the EventMessageFile subkey. Add 1 to the TCHAR count to include the null terminator.

    n = GetModuleFileName( inModule, path, sizeof_array( path ) );
    err = translate_errno( n > 0, (OSStatus) GetLastError(), kParamErr );
    require_noerr_quiet( err, exit );
    n += 1;
    n *= sizeof( TCHAR );

    err = RegSetValueEx( key, TEXT( "EventMessageFile" ), 0, REG_EXPAND_SZ, (const LPBYTE) path, n );
    require_noerr_quiet( err, exit );

    // Set the supported event types in the TypesSupported subkey.

    typesSupported = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE |
                     EVENTLOG_AUDIT_SUCCESS | EVENTLOG_AUDIT_FAILURE;
    err = RegSetValueEx( key, TEXT( "TypesSupported" ), 0, REG_DWORD, (const LPBYTE) &typesSupported, sizeof( DWORD ) );
    require_noerr_quiet( err, exit );

    // Set up the event source.

    gDebugWindowsEventLogEventSource = RegisterEventSource( NULL, name );
    err = translate_errno( gDebugWindowsEventLogEventSource, (OSStatus) GetLastError(), kParamErr );
    require_noerr_quiet( err, exit );

exit:
    if( key )
    {
        RegCloseKey( key );
    }
    return( err );
}

//===========================================================================================================================
//	DebugWindowsEventLogPrint
//===========================================================================================================================

static void DebugWindowsEventLogPrint( DebugLevel inLevel, char *inData, size_t inSize )
{
    WORD type;
    TCHAR buffer[ 512 ];
    const char *        src;
    const char *        end;
    TCHAR *             dst;
    char c;
    const TCHAR *       array[ 1 ];

    // Map the debug level to a Windows EventLog type.

    if( inLevel <= kDebugLevelNotice )
    {
        type = EVENTLOG_INFORMATION_TYPE;
    }
    else if( inLevel <= kDebugLevelWarning )
    {
        type = EVENTLOG_WARNING_TYPE;
    }
    else
    {
        type = EVENTLOG_ERROR_TYPE;
    }

    // Copy locally and null terminate the string. This also converts from char to TCHAR in case we are
    // building with UNICODE enabled since the input is always char. Also convert \r to \n in the process.

    src = inData;
    if( inSize >= sizeof_array( buffer ) )
    {
        inSize = sizeof_array( buffer ) - 1;
    }
    end = src + inSize;
    dst = buffer;
    while( src < end )
    {
        c = *src++;
        if( c == '\r' )
        {
            c = '\n';
        }
        *dst++ = (TCHAR) c;
    }
    *dst = 0;

    // Add the the string to the event log.

    array[ 0 ] = buffer;
    if( gDebugWindowsEventLogEventSource )
    {
        ReportEvent( gDebugWindowsEventLogEventSource, type, 0, 0x20000001L, NULL, 1, 0, array, NULL );
    }
}
#endif  // TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE

#if ( DEBUG_CORE_SERVICE_ASSERTS_ENABLED )
//===========================================================================================================================
//	DebugAssertOutputHandler
//===========================================================================================================================

static pascal void
DebugAssertOutputHandler(
    OSType inComponentSignature,
    UInt32 inOptions,
    const char *        inAssertString,
    const char *        inExceptionString,
    const char *        inErrorString,
    const char *        inFileName,
    long inLineNumber,
    void *              inValue,
    ConstStr255Param inOutputMsg )
{
    DEBUG_UNUSED( inComponentSignature );
    DEBUG_UNUSED( inOptions );
    DEBUG_UNUSED( inExceptionString );
    DEBUG_UNUSED( inValue );
    DEBUG_UNUSED( inOutputMsg );

    DebugPrintAssert( 0, inAssertString, inErrorString, inFileName, (int_least32_t) inLineNumber, "" );
}
#endif

#if 0
#pragma mark -
#pragma mark == Utilities ==
#endif

//===========================================================================================================================
//	DebugSNPrintF
//
//	Stolen from mDNS.c's mDNS_snprintf/mDNS_vsnprintf with the following changes:
//
//	Changed names to avoid name collisions with the mDNS versions.
//	Changed types to standard C types since mDNSEmbeddedAPI.h may not be available.
//	Conditionalized mDNS stuff so it can be used with or with mDNSEmbeddedAPI.h.
//	Added 64-bit support for %d (%lld), %i (%lli), %u (%llu), %o (%llo), %x (%llx), and %b (%llb).
//	Added %@   - Cocoa/CoreFoundation object. Param is the object. Strings are used directly. Others use CFCopyDescription.
//	Added %.8a - FIbre Channel address. Arg=ptr to address.
//	Added %##a - IPv4 (if AF_INET defined) or IPv6 (if AF_INET6 defined) sockaddr. Arg=ptr to sockaddr.
//	Added %b   - Binary representation of integer (e.g. 01101011). Modifiers and arg=the same as %d, %x, etc.
//	Added %C   - Mac-style FourCharCode (e.g. 'APPL'). Arg=32-bit value to print as a Mac-style FourCharCode.
//	Added %H   - Hex Dump (e.g. "\x6b\xa7" -> "6B A7"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
//	Added %#H  - Hex Dump & ASCII (e.g. "\x41\x62" -> "6B A7 'Ab'"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
//	Added %m   - Error Message (e.g. 0 -> "kNoErr"). Modifiers and error code args are the same as %d, %x, etc.
//	Added %S   - UTF-16 string. Host order if no BOM. Precision is UTF-16 char count. BOM counts in any precision. Arg=ptr.
//	Added %#S  - Big Endian UTF-16 string (unless BOM overrides). Otherwise the same as %S.
//	Added %##S - Little Endian UTF-16 string (unless BOM overrides). Otherwise the same as %S.
//	Added %U   - Universally Unique Identifier (UUID) (e.g. 6ba7b810-9dad-11d1-80b4-00c04fd430c8). Arg=ptr to 16-byte UUID.
//===========================================================================================================================

DEBUG_EXPORT size_t DebugSNPrintF(char *sbuffer, size_t buflen, const char *fmt, ...)
{
    size_t length;

    va_list ptr;
    va_start(ptr,fmt);
    length = DebugSNPrintFVAList(sbuffer, buflen, fmt, ptr);
    va_end(ptr);

    return(length);
}

//===========================================================================================================================
//	DebugSNPrintFVAList	- va_list version of DebugSNPrintF. See DebugSNPrintF for more info.
//===========================================================================================================================

DEBUG_EXPORT size_t DebugSNPrintFVAList(char *sbuffer, size_t buflen, const char *fmt, va_list arg)
{
    static const struct DebugSNPrintF_format
    {
        unsigned leftJustify : 1;
        unsigned forceSign : 1;
        unsigned zeroPad : 1;
        unsigned havePrecision : 1;
        unsigned hSize : 1;
        char lSize;
        char altForm;
        char sign;              // +, - or space
        unsigned int fieldWidth;
        unsigned int precision;
    } DebugSNPrintF_format_default = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    size_t nwritten = 0;
    int c;
    if (buflen == 0) return(0);
    buflen--;       // Pre-reserve one space in the buffer for the terminating nul
    if (buflen == 0) goto exit;

    for (c = *fmt; c != 0; c = *++fmt)
    {
        if (c != '%')
        {
            *sbuffer++ = (char)c;
            if (++nwritten >= buflen) goto exit;
        }
        else
        {
            size_t i=0, j;
            // The mDNS Vsprintf Argument Conversion Buffer is used as a temporary holding area for
            // generating decimal numbers, hexdecimal numbers, IP addresses, domain name strings, etc.
            // The size needs to be enough for a 256-byte domain name plus some error text.
            #define mDNS_VACB_Size 300
            char mDNS_VACB[mDNS_VACB_Size];
            #define mDNS_VACB_Lim (&mDNS_VACB[mDNS_VACB_Size])
            #define mDNS_VACB_Remain(s) ((size_t)(mDNS_VACB_Lim - s))
            char *s = mDNS_VACB_Lim;
            const char *digits = "0123456789ABCDEF";
            struct DebugSNPrintF_format F = DebugSNPrintF_format_default;

            for(;;) //  decode flags
            {
                c = *++fmt;
                if      (c == '-') F.leftJustify = 1;
                else if (c == '+') F.forceSign = 1;
                else if (c == ' ') F.sign = ' ';
                else if (c == '#') F.altForm++;
                else if (c == '0') F.zeroPad = 1;
                else break;
            }

            if (c == '*')   //  decode field width
            {
                int f = va_arg(arg, int);
                if (f < 0) { f = -f; F.leftJustify = 1; }
                F.fieldWidth = (unsigned int)f;
                c = *++fmt;
            }
            else
            {
                for (; c >= '0' && c <= '9'; c = *++fmt)
                    F.fieldWidth = (10 * F.fieldWidth) + (c - '0');
            }

            if (c == '.')   //  decode precision
            {
                if ((c = *++fmt) == '*')
                { F.precision = va_arg(arg, unsigned int); c = *++fmt; }
                else for (; c >= '0' && c <= '9'; c = *++fmt)
                        F.precision = (10 * F.precision) + (c - '0');
                F.havePrecision = 1;
            }

            if (F.leftJustify) F.zeroPad = 0;

conv:
            switch (c)  //  perform appropriate conversion
            {
                #if TYPE_LONGLONG_NATIVE
                unsigned_long_long_compat n;
                unsigned_long_long_compat base;
                #else
                unsigned long n;
                unsigned long base;
                #endif
            case 'h':  F.hSize = 1; c = *++fmt; goto conv;
            case 'l':       // fall through
            case 'L':  F.lSize++; c = *++fmt; goto conv;
            case 'd':
            case 'i':  base = 10;
                goto canBeSigned;
            case 'u':  base = 10;
                goto notSigned;
            case 'o':  base = 8;
                goto notSigned;
            case 'b':  base = 2;
                goto notSigned;
            case 'p':  n = va_arg(arg, uintptr_t);
                F.havePrecision = 1;
                F.precision = (sizeof(uintptr_t) == 4) ? 8 : 16;
                F.sign = 0;
                base = 16;
                c = 'x';
                goto number;
            case 'x':  digits = "0123456789abcdef";
            case 'X':  base = 16;
                goto notSigned;
canBeSigned:
                            #if TYPE_LONGLONG_NATIVE
                if (F.lSize == 1) n = (unsigned_long_long_compat)va_arg(arg, long);
                else if (F.lSize == 2) n = (unsigned_long_long_compat)va_arg(arg, long_long_compat);
                else n = (unsigned_long_long_compat)va_arg(arg, int);
                            #else
                if (F.lSize == 1) n = (unsigned long)va_arg(arg, long);
                else if (F.lSize == 2) goto exit;
                else n = (unsigned long)va_arg(arg, int);
                            #endif
                if (F.hSize) n = (short) n;
                            #if TYPE_LONGLONG_NATIVE
                if ((long_long_compat) n < 0) { n = (unsigned_long_long_compat)-(long_long_compat)n; F.sign = '-'; }
                            #else
                if ((long) n < 0) { n = (unsigned long)-(long)n; F.sign = '-'; }
                            #endif
                else if (F.forceSign) F.sign = '+';
                goto number;

notSigned:  if (F.lSize == 1) n = va_arg(arg, unsigned long);
                else if (F.lSize == 2)
                {
                                #if TYPE_LONGLONG_NATIVE
                    n = va_arg(arg, unsigned_long_long_compat);
                                #else
                    goto exit;
                                #endif
                }
                else n = va_arg(arg, unsigned int);
                if (F.hSize) n = (unsigned short) n;
                F.sign = 0;
                goto number;

number:     if (!F.havePrecision)
                {
                    if (F.zeroPad)
                    {
                        F.precision = F.fieldWidth;
                        if (F.altForm) F.precision -= 2;
                        if (F.sign) --F.precision;
                    }
                    if (F.precision < 1) F.precision = 1;
                }
                if (F.precision > mDNS_VACB_Size - 1)
                    F.precision = mDNS_VACB_Size - 1;
                for (i = 0; n; n /= base, i++) *--s = (char)(digits[n % base]);
                for (; i < F.precision; i++) *--s = '0';
                if (F.altForm) { *--s = (char)c; *--s = '0'; i += 2; }
                if (F.sign) { *--s = F.sign; i++; }
                break;

            case 'a':  {
                unsigned char *a = va_arg(arg, unsigned char *);
                char pre[4] = "";
                char post[32] = "";
                if (!a) { static char emsg[] = "<<NULL>>"; s = emsg; i = sizeof(emsg)-1; }
                else
                {
                    s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                    if (F.altForm == 1)
                    {
                                    #if (defined(MDNS_DEBUGMSGS))
                        mDNSAddr *ip = (mDNSAddr*)a;
                        switch (ip->type)
                        {
                        case mDNSAddrType_IPv4: F.precision =  4; a = (unsigned char *)&ip->ip.v4; break;
                        case mDNSAddrType_IPv6: F.precision = 16; a = (unsigned char *)&ip->ip.v6; break;
                        default:                F.precision =  0; break;
                        }
                                    #else
                        F.precision = 0;                    // mDNSEmbeddedAPI.h not included so no mDNSAddr support
                                    #endif
                    }
                    else if (F.altForm == 2)
                    {
                                    #ifdef AF_INET
                        const struct sockaddr *sa;
                        unsigned char *port;
                        sa = (const struct sockaddr*)a;
                        switch (sa->sa_family)
                        {
                        case AF_INET:  F.precision =  4; a = (unsigned char*)&((const struct sockaddr_in *)a)->sin_addr;
                            port = (unsigned char*)&((const struct sockaddr_in *)sa)->sin_port;
                            DebugSNPrintF(post, sizeof(post), ":%d", (port[0] << 8) | port[1]); break;
                                            #ifdef AF_INET6
                        case AF_INET6: F.precision = 16; a = (unsigned char*)&((const struct sockaddr_in6 *)a)->sin6_addr;
                            pre[0] = '['; pre[1] = '\0';
                            port = (unsigned char*)&((const struct sockaddr_in6 *)sa)->sin6_port;
                            DebugSNPrintF(post, sizeof(post), "%%%d]:%d",
                                          (int)((const struct sockaddr_in6 *)sa)->sin6_scope_id,
                                          (port[0] << 8) | port[1]); break;
                                            #endif
                        default:       F.precision =  0; break;
                        }
                                    #else
                        F.precision = 0;                    // socket interfaces not included so no sockaddr support
                                    #endif
                    }
                    switch (F.precision)
                    {
                    case  4: i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "%d.%d.%d.%d%s",
                                               a[0], a[1], a[2], a[3], post); break;
                    case  6: i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "%02X:%02X:%02X:%02X:%02X:%02X",
                                               a[0], a[1], a[2], a[3], a[4], a[5]); break;
                    case  8: i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                                               a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]); break;
                    case 16: i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB),
                                               "%s%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X%s",
                                               pre, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8],
                                               a[9], a[10], a[11], a[12], a[13], a[14], a[15], post); break;
                    default: i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "%s", "<< ERROR: Must specify address size "
                                               "(i.e. %.4a=IPv4, %.6a=Ethernet, %.8a=Fibre Channel %.16a=IPv6) >>"); break;
                    }
                }
            }
            break;

            case 'U':  {
                unsigned char *a = va_arg(arg, unsigned char *);
                if (!a) { static char emsg[] = "<<NULL>>"; s = emsg; i = sizeof(emsg)-1; }
                else
                {
                    s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                    i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                                      *((uint32_t*) &a[0]), *((uint16_t*) &a[4]), *((uint16_t*) &a[6]),
                                      a[8], a[9], a[10], a[11], a[12], a[13], a[14], a[15]); break;
                }
            }
            break;

            case 'c':  *--s = (char)va_arg(arg, int); i = 1; break;

            case 'C':  if (F.lSize) n = va_arg(arg, unsigned long);
                else n = va_arg(arg, unsigned int);
                if (F.hSize) n = (unsigned short) n;
                c = (int)( n        & 0xFF); *--s = (char)(DebugIsPrint(c) ? c : '^');
                c = (int)((n >>  8) & 0xFF); *--s = (char)(DebugIsPrint(c) ? c : '^');
                c = (int)((n >> 16) & 0xFF); *--s = (char)(DebugIsPrint(c) ? c : '^');
                c = (int)((n >> 24) & 0xFF); *--s = (char)(DebugIsPrint(c) ? c : '^');
                i = 4;
                break;

            case 's':  s = va_arg(arg, char *);
                if (!s) { static char emsg[] = "<<NULL>>"; s = emsg; i = sizeof(emsg)-1; }
                else switch (F.altForm)
                    {
                    case 0: i=0;
                        if (F.havePrecision)                                // C string
                        {
                            while((i < F.precision) && s[i]) i++;
                            // Make sure we don't truncate in the middle of a UTF-8 character.
                            // If the last character is part of a multi-byte UTF-8 character, back up to the start of it.
                            j=0;
                            while((i > 0) && ((c = s[i-1]) & 0x80)) { j++; i--; if((c & 0xC0) != 0x80) break;}
                            // If the actual count of UTF-8 characters matches the encoded UTF-8 count, add it back.
                            if((j > 1) && (j <= 6))
                            {
                                int test = (0xFF << (8-j)) & 0xFF;
                                int mask = test | (1 << ((8-j)-1));
                                if((c & mask) == test) i += j;
                            }
                        }
                        else
                            while(s[i]) i++;
                        break;
                    case 1: i = (unsigned char) *s++; break;                // Pascal string
                    case 2: {                                               // DNS label-sequence name
                        unsigned char *a = (unsigned char *)s;
                        s = mDNS_VACB;                  // Adjust s to point to the start of the buffer, not the end
                        if (*a == 0) *s++ = '.';                    // Special case for root DNS name
                        while (*a)
                        {
                            if (*a > 63) { s += DebugSNPrintF(s, mDNS_VACB_Remain(s), "<<INVALID LABEL LENGTH %u>>", *a); break; }
                            if (s + *a >= &mDNS_VACB[254]) { s += DebugSNPrintF(s, mDNS_VACB_Remain(s), "<<NAME TOO LONG>>"); break; }
                            s += DebugSNPrintF(s, mDNS_VACB_Remain(s), "%#s.", a);
                            a += 1 + *a;
                        }
                        i = (size_t)(s - mDNS_VACB);
                        s = mDNS_VACB;                  // Reset s back to the start of the buffer
                        break;
                    }
                    }
                if (F.havePrecision && i > F.precision)                 // Make sure we don't truncate in the middle of a UTF-8 character
                { i = F.precision; while (i>0 && (s[i] & 0xC0) == 0x80) i--;}
                break;

            case 'S':   {       // UTF-16 string
                unsigned char *a = va_arg(arg, unsigned char *);
                uint16_t      *u = (uint16_t*)a;
                if (!u) { static char emsg[] = "<<NULL>>"; s = emsg; i = sizeof(emsg)-1; }
                if ((!F.havePrecision || F.precision))
                {
                    if      ((a[0] == 0xFE) && (a[1] == 0xFF)) { F.altForm = 1; u += 1; a += 2; F.precision--; }                // Big Endian
                    else if ((a[0] == 0xFF) && (a[1] == 0xFE)) { F.altForm = 2; u += 1; a += 2; F.precision--; }                // Little Endian
                }
                s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                switch (F.altForm)
                {
                case 0: while ((!F.havePrecision || (i < F.precision)) && u[i] && mDNS_VACB_Remain(s))                  // Host Endian
                    { c = u[i]; *s++ = (char)(DebugIsPrint(c) ? c : '^'); i++; }
                    break;
                case 1: while ((!F.havePrecision || (i < F.precision)) && u[i] && mDNS_VACB_Remain(s))                  // Big Endian
                    { c = ((a[0] << 8) | a[1]) & 0xFF; *s++ = (char)(DebugIsPrint(c) ? c : '^'); i++; a += 2; }
                    break;
                case 2: while ((!F.havePrecision || (i < F.precision)) && u[i] && mDNS_VACB_Remain(s))                  // Little Endian
                    { c = ((a[1] << 8) | a[0]) & 0xFF; *s++ = (char)(DebugIsPrint(c) ? c : '^'); i++; a += 2; }
                    break;
                }
            }
                s = mDNS_VACB;              // Reset s back to the start of the buffer
                break;

            #if TARGET_OS_MAC
            case '@':   {       // Cocoa/CoreFoundation object
                CFTypeRef cfObj;
                CFStringRef cfStr;
                cfObj = (CFTypeRef) va_arg(arg, void *);
                cfStr = (CFGetTypeID(cfObj) == CFStringGetTypeID()) ? (CFStringRef)CFRetain(cfObj) : CFCopyDescription(cfObj);
                s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                if (cfStr)
                {
                    CFRange range;
                    CFIndex m;
                    range = CFRangeMake(0, CFStringGetLength(cfStr));
                    m = 0;
                    CFStringGetBytes(cfStr, range, kCFStringEncodingUTF8, '^', false, (UInt8*)mDNS_VACB, (CFIndex)sizeof(mDNS_VACB), &m);
                    CFRelease(cfStr);
                    i = (size_t) m;
                }
                else
                {
                    i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "%s", "ERROR: <invalid CF object>" );
                }
            }
                if (F.havePrecision && i > F.precision)                 // Make sure we don't truncate in the middle of a UTF-8 character
                { i = F.precision; while (i>0 && (s[i] & 0xC0) == 0x80) i--;}
                break;
            #endif

            case 'm':  {        // Error Message
                long err;
                if (F.lSize) err = va_arg(arg, long);
                else err = va_arg(arg, int);
                if (F.hSize) err = (short)err;
                DebugGetErrorString(err, mDNS_VACB, sizeof(mDNS_VACB));
                s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                for(i=0; s[i]; i++) {}
            }
            break;

            case 'H':  {        // Hex Dump
                void *a = va_arg(arg, void *);
                size_t size = (size_t)va_arg(arg, int);
                size_t max = (size_t)va_arg(arg, int);
                DebugFlags flags =
                    kDebugFlagsNoAddress | kDebugFlagsNoOffset | kDebugFlagsNoNewLine |
                    kDebugFlags8BitSeparator | kDebugFlagsNo32BitSeparator |
                    kDebugFlagsNo16ByteHexPad | kDebugFlagsNoByteCount;
                if (F.altForm == 0) flags |= kDebugFlagsNoASCII;
                size = (max < size) ? max : size;
                s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                i = DebugHexDump(kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, a, a, size, flags, mDNS_VACB, sizeof(mDNS_VACB));
            }
            break;

            case 'v':  {        // Version
                uint32_t version;
                version = va_arg(arg, unsigned int);
                DebugNumVersionToString(version, mDNS_VACB);
                s = mDNS_VACB;              // Adjust s to point to the start of the buffer, not the end
                for(i=0; s[i]; i++) {}
            }
            break;

            case 'n':  s = va_arg(arg, char *);
                if      (F.hSize) *(short *) s = (short)nwritten;
                else if (F.lSize) *(long  *) s = (long)nwritten;
                else *(int   *) s = (int)nwritten;
                continue;

            default:    s = mDNS_VACB;
                i = DebugSNPrintF(mDNS_VACB, sizeof(mDNS_VACB), "<<UNKNOWN FORMAT CONVERSION CODE %%%c>>", c);

            case '%':  *sbuffer++ = (char)c;
                if (++nwritten >= buflen) goto exit;
                break;
            }

            if (i < F.fieldWidth && !F.leftJustify)         // Pad on the left
                do  {
                    *sbuffer++ = ' ';
                    if (++nwritten >= buflen) goto exit;
                } while (i < --F.fieldWidth);

            if (i > buflen - nwritten)  // Make sure we don't truncate in the middle of a UTF-8 character
            { i = buflen - nwritten; while (i>0 && (s[i] & 0xC0) == 0x80) i--;}
            for (j=0; j<i; j++) *sbuffer++ = *s++;          // Write the converted result
            nwritten += i;
            if (nwritten >= buflen) goto exit;

            for (; i < F.fieldWidth; i++)                   // Pad on the right
            {
                *sbuffer++ = ' ';
                if (++nwritten >= buflen) goto exit;
            }
        }
    }
exit:
    *sbuffer++ = 0;
    return(nwritten);
}

//===========================================================================================================================
//	DebugGetErrorString
//===========================================================================================================================

DEBUG_EXPORT const char *   DebugGetErrorString( int_least32_t inErrorCode, char *inBuffer, size_t inBufferSize )
{
    const char *        s;
    char *              dst;
    char *              end;
#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
    char buffer[ 256 ];
#endif

    switch( inErrorCode )
    {
        #define CaseErrorString( X, STR )                   case X: s = STR; break
        #define CaseErrorStringify( X )                     case X: s = # X; break
        #define CaseErrorStringifyHardCode( VALUE, X )      case VALUE: s = # X; break

        // General Errors

        CaseErrorString( 0,  "no error" );
        CaseErrorString( 1,  "in-progress/waiting" );
        CaseErrorString( -1, "catch-all unknown error" );

        // ACP Errors

        CaseErrorStringifyHardCode( -2,  kACPBadRequestErr );
        CaseErrorStringifyHardCode( -3,  kACPNoMemoryErr );
        CaseErrorStringifyHardCode( -4,  kACPBadParamErr );
        CaseErrorStringifyHardCode( -5,  kACPNotFoundErr );
        CaseErrorStringifyHardCode( -6,  kACPBadChecksumErr );
        CaseErrorStringifyHardCode( -7,  kACPCommandNotHandledErr );
        CaseErrorStringifyHardCode( -8,  kACPNetworkErr );
        CaseErrorStringifyHardCode( -9,  kACPDuplicateCommandHandlerErr );
        CaseErrorStringifyHardCode( -10, kACPUnknownPropertyErr );
        CaseErrorStringifyHardCode( -11, kACPImmutablePropertyErr );
        CaseErrorStringifyHardCode( -12, kACPBadPropertyValueErr );
        CaseErrorStringifyHardCode( -13, kACPNoResourcesErr );
        CaseErrorStringifyHardCode( -14, kACPBadOptionErr );
        CaseErrorStringifyHardCode( -15, kACPBadSizeErr );
        CaseErrorStringifyHardCode( -16, kACPBadPasswordErr );
        CaseErrorStringifyHardCode( -17, kACPNotInitializedErr );
        CaseErrorStringifyHardCode( -18, kACPNonReadablePropertyErr );
        CaseErrorStringifyHardCode( -19, kACPBadVersionErr );
        CaseErrorStringifyHardCode( -20, kACPBadSignatureErr );
        CaseErrorStringifyHardCode( -21, kACPBadIndexErr );
        CaseErrorStringifyHardCode( -22, kACPUnsupportedErr );
        CaseErrorStringifyHardCode( -23, kACPInUseErr );
        CaseErrorStringifyHardCode( -24, kACPParamCountErr );
        CaseErrorStringifyHardCode( -25, kACPIDErr );
        CaseErrorStringifyHardCode( -26, kACPFormatErr );
        CaseErrorStringifyHardCode( -27, kACPUnknownUserErr );
        CaseErrorStringifyHardCode( -28, kACPAccessDeniedErr );
        CaseErrorStringifyHardCode( -29, kACPIncorrectFWErr );

        // Common Services Errors

        CaseErrorStringify( kUnknownErr );
        CaseErrorStringify( kOptionErr );
        CaseErrorStringify( kSelectorErr );
        CaseErrorStringify( kExecutionStateErr );
        CaseErrorStringify( kPathErr );
        CaseErrorStringify( kParamErr );
        CaseErrorStringify( kParamCountErr );
        CaseErrorStringify( kCommandErr );
        CaseErrorStringify( kIDErr );
        CaseErrorStringify( kStateErr );
        CaseErrorStringify( kRangeErr );
        CaseErrorStringify( kRequestErr );
        CaseErrorStringify( kResponseErr );
        CaseErrorStringify( kChecksumErr );
        CaseErrorStringify( kNotHandledErr );
        CaseErrorStringify( kVersionErr );
        CaseErrorStringify( kSignatureErr );
        CaseErrorStringify( kFormatErr );
        CaseErrorStringify( kNotInitializedErr );
        CaseErrorStringify( kAlreadyInitializedErr );
        CaseErrorStringify( kNotInUseErr );
        CaseErrorStringify( kInUseErr );
        CaseErrorStringify( kTimeoutErr );
        CaseErrorStringify( kCanceledErr );
        CaseErrorStringify( kAlreadyCanceledErr );
        CaseErrorStringify( kCannotCancelErr );
        CaseErrorStringify( kDeletedErr );
        CaseErrorStringify( kNotFoundErr );
        CaseErrorStringify( kNoMemoryErr );
        CaseErrorStringify( kNoResourcesErr );
        CaseErrorStringify( kDuplicateErr );
        CaseErrorStringify( kImmutableErr );
        CaseErrorStringify( kUnsupportedDataErr );
        CaseErrorStringify( kIntegrityErr );
        CaseErrorStringify( kIncompatibleErr );
        CaseErrorStringify( kUnsupportedErr );
        CaseErrorStringify( kUnexpectedErr );
        CaseErrorStringify( kValueErr );
        CaseErrorStringify( kNotReadableErr );
        CaseErrorStringify( kNotWritableErr );
        CaseErrorStringify( kBadReferenceErr );
        CaseErrorStringify( kFlagErr );
        CaseErrorStringify( kMalformedErr );
        CaseErrorStringify( kSizeErr );
        CaseErrorStringify( kNameErr );
        CaseErrorStringify( kNotReadyErr );
        CaseErrorStringify( kReadErr );
        CaseErrorStringify( kWriteErr );
        CaseErrorStringify( kMismatchErr );
        CaseErrorStringify( kDateErr );
        CaseErrorStringify( kUnderrunErr );
        CaseErrorStringify( kOverrunErr );
        CaseErrorStringify( kEndingErr );
        CaseErrorStringify( kConnectionErr );
        CaseErrorStringify( kAuthenticationErr );
        CaseErrorStringify( kOpenErr );
        CaseErrorStringify( kTypeErr );
        CaseErrorStringify( kSkipErr );
        CaseErrorStringify( kNoAckErr );
        CaseErrorStringify( kCollisionErr );
        CaseErrorStringify( kBackoffErr );
        CaseErrorStringify( kNoAddressAckErr );
        CaseErrorStringify( kBusyErr );
        CaseErrorStringify( kNoSpaceErr );

        // mDNS/DNS-SD Errors

        CaseErrorStringifyHardCode( -65537, mStatus_UnknownErr );
        CaseErrorStringifyHardCode( -65538, mStatus_NoSuchNameErr );
        CaseErrorStringifyHardCode( -65539, mStatus_NoMemoryErr );
        CaseErrorStringifyHardCode( -65540, mStatus_BadParamErr );
        CaseErrorStringifyHardCode( -65541, mStatus_BadReferenceErr );
        CaseErrorStringifyHardCode( -65542, mStatus_BadStateErr );
        CaseErrorStringifyHardCode( -65543, mStatus_BadFlagsErr );
        CaseErrorStringifyHardCode( -65544, mStatus_UnsupportedErr );
        CaseErrorStringifyHardCode( -65545, mStatus_NotInitializedErr );
        CaseErrorStringifyHardCode( -65546, mStatus_NoCache );
        CaseErrorStringifyHardCode( -65547, mStatus_AlreadyRegistered );
        CaseErrorStringifyHardCode( -65548, mStatus_NameConflict );
        CaseErrorStringifyHardCode( -65549, mStatus_Invalid );
        CaseErrorStringifyHardCode( -65550, mStatus_GrowCache );
        CaseErrorStringifyHardCode( -65551, mStatus_Incompatible );
        CaseErrorStringifyHardCode( -65552, mStatus_BadInterfaceErr );
        CaseErrorStringifyHardCode( -65791, mStatus_ConfigChanged );
        CaseErrorStringifyHardCode( -65792, mStatus_MemFree );

        // RSP Errors

        CaseErrorStringifyHardCode( -400000, kRSPUnknownErr );
        CaseErrorStringifyHardCode( -400050, kRSPParamErr );
        CaseErrorStringifyHardCode( -400108, kRSPNoMemoryErr );
        CaseErrorStringifyHardCode( -405246, kRSPRangeErr );
        CaseErrorStringifyHardCode( -409057, kRSPSizeErr );
        CaseErrorStringifyHardCode( -400200, kRSPHardwareErr );
        CaseErrorStringifyHardCode( -401712, kRSPTimeoutErr );
        CaseErrorStringifyHardCode( -402053, kRSPUnsupportedErr );
        CaseErrorStringifyHardCode( -402419, kRSPIDErr );
        CaseErrorStringifyHardCode( -403165, kRSPFlagErr );
        CaseErrorString(            -200000, "kRSPControllerStatusBase - 0x50" );
        CaseErrorString(            -200080, "kRSPCommandSucceededErr - 0x50" );
        CaseErrorString(            -200001, "kRSPCommandFailedErr - 0x01" );
        CaseErrorString(            -200051, "kRSPChecksumErr - 0x33" );
        CaseErrorString(            -200132, "kRSPCommandTimeoutErr - 0x84" );
        CaseErrorString(            -200034, "kRSPPasswordRequiredErr - 0x22 OBSOLETE" );
        CaseErrorString(            -200128, "kRSPCanceledErr - 0x02 Async" );

        // XML Errors

        CaseErrorStringifyHardCode( -100043, kXMLNotFoundErr );
        CaseErrorStringifyHardCode( -100050, kXMLParamErr );
        CaseErrorStringifyHardCode( -100108, kXMLNoMemoryErr );
        CaseErrorStringifyHardCode( -100206, kXMLFormatErr );
        CaseErrorStringifyHardCode( -100586, kXMLNoRootElementErr );
        CaseErrorStringifyHardCode( -101703, kXMLWrongDataTypeErr );
        CaseErrorStringifyHardCode( -101726, kXMLKeyErr );
        CaseErrorStringifyHardCode( -102053, kXMLUnsupportedErr );
        CaseErrorStringifyHardCode( -102063, kXMLMissingElementErr );
        CaseErrorStringifyHardCode( -103026, kXMLParseErr );
        CaseErrorStringifyHardCode( -103159, kXMLBadDataErr );
        CaseErrorStringifyHardCode( -103170, kXMLBadNameErr );
        CaseErrorStringifyHardCode( -105246, kXMLRangeErr );
        CaseErrorStringifyHardCode( -105251, kXMLUnknownElementErr );
        CaseErrorStringifyHardCode( -108739, kXMLMalformedInputErr );
        CaseErrorStringifyHardCode( -109057, kXMLBadSizeErr );
        CaseErrorStringifyHardCode( -101730, kXMLMissingChildElementErr );
        CaseErrorStringifyHardCode( -102107, kXMLMissingParentElementErr );
        CaseErrorStringifyHardCode( -130587, kXMLNonRootElementErr );
        CaseErrorStringifyHardCode( -102015, kXMLDateErr );

    #if ( __MACH__ )

        // Mach Errors

        CaseErrorStringifyHardCode( 0x00002000, MACH_MSG_IPC_SPACE );
        CaseErrorStringifyHardCode( 0x00001000, MACH_MSG_VM_SPACE );
        CaseErrorStringifyHardCode( 0x00000800, MACH_MSG_IPC_KERNEL );
        CaseErrorStringifyHardCode( 0x00000400, MACH_MSG_VM_KERNEL );
        CaseErrorStringifyHardCode( 0x10000001, MACH_SEND_IN_PROGRESS );
        CaseErrorStringifyHardCode( 0x10000002, MACH_SEND_INVALID_DATA );
        CaseErrorStringifyHardCode( 0x10000003, MACH_SEND_INVALID_DEST );
        CaseErrorStringifyHardCode( 0x10000004, MACH_SEND_TIMED_OUT );
        CaseErrorStringifyHardCode( 0x10000007, MACH_SEND_INTERRUPTED );
        CaseErrorStringifyHardCode( 0x10000008, MACH_SEND_MSG_TOO_SMALL );
        CaseErrorStringifyHardCode( 0x10000009, MACH_SEND_INVALID_REPLY );
        CaseErrorStringifyHardCode( 0x1000000A, MACH_SEND_INVALID_RIGHT );
        CaseErrorStringifyHardCode( 0x1000000B, MACH_SEND_INVALID_NOTIFY );
        CaseErrorStringifyHardCode( 0x1000000C, MACH_SEND_INVALID_MEMORY );
        CaseErrorStringifyHardCode( 0x1000000D, MACH_SEND_NO_BUFFER );
        CaseErrorStringifyHardCode( 0x1000000E, MACH_SEND_TOO_LARGE );
        CaseErrorStringifyHardCode( 0x1000000F, MACH_SEND_INVALID_TYPE );
        CaseErrorStringifyHardCode( 0x10000010, MACH_SEND_INVALID_HEADER );
        CaseErrorStringifyHardCode( 0x10000011, MACH_SEND_INVALID_TRAILER );
        CaseErrorStringifyHardCode( 0x10000015, MACH_SEND_INVALID_RT_OOL_SIZE );
        CaseErrorStringifyHardCode( 0x10004001, MACH_RCV_IN_PROGRESS );
        CaseErrorStringifyHardCode( 0x10004002, MACH_RCV_INVALID_NAME );
        CaseErrorStringifyHardCode( 0x10004003, MACH_RCV_TIMED_OUT );
        CaseErrorStringifyHardCode( 0x10004004, MACH_RCV_TOO_LARGE );
        CaseErrorStringifyHardCode( 0x10004005, MACH_RCV_INTERRUPTED );
        CaseErrorStringifyHardCode( 0x10004006, MACH_RCV_PORT_CHANGED );
        CaseErrorStringifyHardCode( 0x10004007, MACH_RCV_INVALID_NOTIFY );
        CaseErrorStringifyHardCode( 0x10004008, MACH_RCV_INVALID_DATA );
        CaseErrorStringifyHardCode( 0x10004009, MACH_RCV_PORT_DIED );
        CaseErrorStringifyHardCode( 0x1000400A, MACH_RCV_IN_SET );
        CaseErrorStringifyHardCode( 0x1000400B, MACH_RCV_HEADER_ERROR );
        CaseErrorStringifyHardCode( 0x1000400C, MACH_RCV_BODY_ERROR );
        CaseErrorStringifyHardCode( 0x1000400D, MACH_RCV_INVALID_TYPE );
        CaseErrorStringifyHardCode( 0x1000400E, MACH_RCV_SCATTER_SMALL );
        CaseErrorStringifyHardCode( 0x1000400F, MACH_RCV_INVALID_TRAILER );
        CaseErrorStringifyHardCode( 0x10004011, MACH_RCV_IN_PROGRESS_TIMED );

        // Mach OSReturn Errors

        CaseErrorStringifyHardCode( 0xDC000001, kOSReturnError );
        CaseErrorStringifyHardCode( 0xDC004001, kOSMetaClassInternal );
        CaseErrorStringifyHardCode( 0xDC004002, kOSMetaClassHasInstances );
        CaseErrorStringifyHardCode( 0xDC004003, kOSMetaClassNoInit );
        CaseErrorStringifyHardCode( 0xDC004004, kOSMetaClassNoTempData );
        CaseErrorStringifyHardCode( 0xDC004005, kOSMetaClassNoDicts );
        CaseErrorStringifyHardCode( 0xDC004006, kOSMetaClassNoKModSet );
        CaseErrorStringifyHardCode( 0xDC004007, kOSMetaClassNoInsKModSet );
        CaseErrorStringifyHardCode( 0xDC004008, kOSMetaClassNoSuper );
        CaseErrorStringifyHardCode( 0xDC004009, kOSMetaClassInstNoSuper );
        CaseErrorStringifyHardCode( 0xDC00400A, kOSMetaClassDuplicateClass );

        // IOKit Errors

        CaseErrorStringifyHardCode( 0xE00002BC, kIOReturnError );
        CaseErrorStringifyHardCode( 0xE00002BD, kIOReturnNoMemory );
        CaseErrorStringifyHardCode( 0xE00002BE, kIOReturnNoResources );
        CaseErrorStringifyHardCode( 0xE00002BF, kIOReturnIPCError );
        CaseErrorStringifyHardCode( 0xE00002C0, kIOReturnNoDevice );
        CaseErrorStringifyHardCode( 0xE00002C1, kIOReturnNotPrivileged );
        CaseErrorStringifyHardCode( 0xE00002C2, kIOReturnBadArgument );
        CaseErrorStringifyHardCode( 0xE00002C3, kIOReturnLockedRead );
        CaseErrorStringifyHardCode( 0xE00002C4, kIOReturnLockedWrite );
        CaseErrorStringifyHardCode( 0xE00002C5, kIOReturnExclusiveAccess );
        CaseErrorStringifyHardCode( 0xE00002C6, kIOReturnBadMessageID );
        CaseErrorStringifyHardCode( 0xE00002C7, kIOReturnUnsupported );
        CaseErrorStringifyHardCode( 0xE00002C8, kIOReturnVMError );
        CaseErrorStringifyHardCode( 0xE00002C9, kIOReturnInternalError );
        CaseErrorStringifyHardCode( 0xE00002CA, kIOReturnIOError );
        CaseErrorStringifyHardCode( 0xE00002CC, kIOReturnCannotLock );
        CaseErrorStringifyHardCode( 0xE00002CD, kIOReturnNotOpen );
        CaseErrorStringifyHardCode( 0xE00002CE, kIOReturnNotReadable );
        CaseErrorStringifyHardCode( 0xE00002CF, kIOReturnNotWritable );
        CaseErrorStringifyHardCode( 0xE00002D0, kIOReturnNotAligned );
        CaseErrorStringifyHardCode( 0xE00002D1, kIOReturnBadMedia );
        CaseErrorStringifyHardCode( 0xE00002D2, kIOReturnStillOpen );
        CaseErrorStringifyHardCode( 0xE00002D3, kIOReturnRLDError );
        CaseErrorStringifyHardCode( 0xE00002D4, kIOReturnDMAError );
        CaseErrorStringifyHardCode( 0xE00002D5, kIOReturnBusy );
        CaseErrorStringifyHardCode( 0xE00002D6, kIOReturnTimeout );
        CaseErrorStringifyHardCode( 0xE00002D7, kIOReturnOffline );
        CaseErrorStringifyHardCode( 0xE00002D8, kIOReturnNotReady );
        CaseErrorStringifyHardCode( 0xE00002D9, kIOReturnNotAttached );
        CaseErrorStringifyHardCode( 0xE00002DA, kIOReturnNoChannels );
        CaseErrorStringifyHardCode( 0xE00002DB, kIOReturnNoSpace );
        CaseErrorStringifyHardCode( 0xE00002DD, kIOReturnPortExists );
        CaseErrorStringifyHardCode( 0xE00002DE, kIOReturnCannotWire );
        CaseErrorStringifyHardCode( 0xE00002DF, kIOReturnNoInterrupt );
        CaseErrorStringifyHardCode( 0xE00002E0, kIOReturnNoFrames );
        CaseErrorStringifyHardCode( 0xE00002E1, kIOReturnMessageTooLarge );
        CaseErrorStringifyHardCode( 0xE00002E2, kIOReturnNotPermitted );
        CaseErrorStringifyHardCode( 0xE00002E3, kIOReturnNoPower );
        CaseErrorStringifyHardCode( 0xE00002E4, kIOReturnNoMedia );
        CaseErrorStringifyHardCode( 0xE00002E5, kIOReturnUnformattedMedia );
        CaseErrorStringifyHardCode( 0xE00002E6, kIOReturnUnsupportedMode );
        CaseErrorStringifyHardCode( 0xE00002E7, kIOReturnUnderrun );
        CaseErrorStringifyHardCode( 0xE00002E8, kIOReturnOverrun );
        CaseErrorStringifyHardCode( 0xE00002E9, kIOReturnDeviceError     );
        CaseErrorStringifyHardCode( 0xE00002EA, kIOReturnNoCompletion    );
        CaseErrorStringifyHardCode( 0xE00002EB, kIOReturnAborted     );
        CaseErrorStringifyHardCode( 0xE00002EC, kIOReturnNoBandwidth     );
        CaseErrorStringifyHardCode( 0xE00002ED, kIOReturnNotResponding   );
        CaseErrorStringifyHardCode( 0xE00002EE, kIOReturnIsoTooOld   );
        CaseErrorStringifyHardCode( 0xE00002EF, kIOReturnIsoTooNew   );
        CaseErrorStringifyHardCode( 0xE00002F0, kIOReturnNotFound );
        CaseErrorStringifyHardCode( 0xE0000001, kIOReturnInvalid );

        // IOKit FireWire Errors

        CaseErrorStringifyHardCode( 0xE0008010, kIOFireWireResponseBase );
        CaseErrorStringifyHardCode( 0xE0008020, kIOFireWireBusReset );
        CaseErrorStringifyHardCode( 0xE0008001, kIOConfigNoEntry );
        CaseErrorStringifyHardCode( 0xE0008002, kIOFireWirePending );
        CaseErrorStringifyHardCode( 0xE0008003, kIOFireWireLastDCLToken );
        CaseErrorStringifyHardCode( 0xE0008004, kIOFireWireConfigROMInvalid );
        CaseErrorStringifyHardCode( 0xE0008005, kIOFireWireAlreadyRegistered );
        CaseErrorStringifyHardCode( 0xE0008006, kIOFireWireMultipleTalkers );
        CaseErrorStringifyHardCode( 0xE0008007, kIOFireWireChannelActive );
        CaseErrorStringifyHardCode( 0xE0008008, kIOFireWireNoListenerOrTalker );
        CaseErrorStringifyHardCode( 0xE0008009, kIOFireWireNoChannels );
        CaseErrorStringifyHardCode( 0xE000800A, kIOFireWireChannelNotAvailable );
        CaseErrorStringifyHardCode( 0xE000800B, kIOFireWireSeparateBus );
        CaseErrorStringifyHardCode( 0xE000800C, kIOFireWireBadSelfIDs );
        CaseErrorStringifyHardCode( 0xE000800D, kIOFireWireLowCableVoltage );
        CaseErrorStringifyHardCode( 0xE000800E, kIOFireWireInsufficientPower );
        CaseErrorStringifyHardCode( 0xE000800F, kIOFireWireOutOfTLabels );
        CaseErrorStringifyHardCode( 0xE0008101, kIOFireWireBogusDCLProgram );
        CaseErrorStringifyHardCode( 0xE0008102, kIOFireWireTalkingAndListening );
        CaseErrorStringifyHardCode( 0xE0008103, kIOFireWireHardwareSlept );
        CaseErrorStringifyHardCode( 0xE00087D0, kIOFWMessageServiceIsRequestingClose );
        CaseErrorStringifyHardCode( 0xE00087D1, kIOFWMessagePowerStateChanged );
        CaseErrorStringifyHardCode( 0xE00087D2, kIOFWMessageTopologyChanged );

        // IOKit USB Errors

        CaseErrorStringifyHardCode( 0xE0004061, kIOUSBUnknownPipeErr );
        CaseErrorStringifyHardCode( 0xE0004060, kIOUSBTooManyPipesErr );
        CaseErrorStringifyHardCode( 0xE000405F, kIOUSBNoAsyncPortErr );
        CaseErrorStringifyHardCode( 0xE000405E, kIOUSBNotEnoughPipesErr );
        CaseErrorStringifyHardCode( 0xE000405D, kIOUSBNotEnoughPowerErr );
        CaseErrorStringifyHardCode( 0xE0004057, kIOUSBEndpointNotFound );
        CaseErrorStringifyHardCode( 0xE0004056, kIOUSBConfigNotFound );
        CaseErrorStringifyHardCode( 0xE0004051, kIOUSBTransactionTimeout );
        CaseErrorStringifyHardCode( 0xE0004050, kIOUSBTransactionReturned );
        CaseErrorStringifyHardCode( 0xE000404F, kIOUSBPipeStalled );
        CaseErrorStringifyHardCode( 0xE000404E, kIOUSBInterfaceNotFound );
        CaseErrorStringifyHardCode( 0xE000404D, kIOUSBLowLatencyBufferNotPreviouslyAllocated );
        CaseErrorStringifyHardCode( 0xE000404C, kIOUSBLowLatencyFrameListNotPreviouslyAllocated );
        CaseErrorStringifyHardCode( 0xE000404B, kIOUSBHighSpeedSplitError );
        CaseErrorStringifyHardCode( 0xE0004010, kIOUSBLinkErr );
        CaseErrorStringifyHardCode( 0xE000400F, kIOUSBNotSent2Err );
        CaseErrorStringifyHardCode( 0xE000400E, kIOUSBNotSent1Err );
        CaseErrorStringifyHardCode( 0xE000400D, kIOUSBBufferUnderrunErr );
        CaseErrorStringifyHardCode( 0xE000400C, kIOUSBBufferOverrunErr );
        CaseErrorStringifyHardCode( 0xE000400B, kIOUSBReserved2Err );
        CaseErrorStringifyHardCode( 0xE000400A, kIOUSBReserved1Err );
        CaseErrorStringifyHardCode( 0xE0004007, kIOUSBWrongPIDErr );
        CaseErrorStringifyHardCode( 0xE0004006, kIOUSBPIDCheckErr );
        CaseErrorStringifyHardCode( 0xE0004003, kIOUSBDataToggleErr );
        CaseErrorStringifyHardCode( 0xE0004002, kIOUSBBitstufErr );
        CaseErrorStringifyHardCode( 0xE0004001, kIOUSBCRCErr );

    #endif  // __MACH__

    // Other Errors

    default:
        s = NULL;
            #if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
        if( inBuffer && ( inBufferSize > 0 ) )
        {
            DWORD n;

            n = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, (DWORD) inErrorCode,
                                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buffer, sizeof( buffer ), NULL );
            if( n > 0 )
            {
                // Remove any trailing CR's or LF's since some messages have them.

                while( ( n > 0 ) && isspace( ( (unsigned char *) buffer )[ n - 1 ] ) )
                {
                    buffer[ --n ] = '\0';
                }
                s = buffer;
            }
        }
            #endif

        if( !s )
        {
                #if ( !TARGET_API_MAC_OSX_KERNEL && !TARGET_OS_WINDOWS_CE )
            s = strerror( inErrorCode );
                #endif
            if( !s )
            {
                s = "<unknown error code>";
            }
        }
        break;
    }

    // Copy the string to the output buffer. If no buffer is supplied or it is empty, return an empty string.

    if( inBuffer && ( inBufferSize > 0 ) )
    {
        dst = inBuffer;
        end = dst + ( inBufferSize - 1 );
        while( ( ( end - dst ) > 0 ) && ( *s != '\0' ) )
        {
            *dst++ = *s++;
        }
        *dst = '\0';
        s = inBuffer;
    }
    return( s );
}

//===========================================================================================================================
//	DebugHexDump
//===========================================================================================================================

DEBUG_EXPORT size_t
DebugHexDump(
    DebugLevel inLevel,
    int inIndent,
    const char *    inLabel,
    size_t inLabelSize,
    int inLabelMinWidth,
    const char *    inType,
    size_t inTypeSize,
    const void *    inDataStart,
    const void *    inData,
    size_t inDataSize,
    DebugFlags inFlags,
    char *          outBuffer,
    size_t inBufferSize )
{
    static const char kHexChars[] = "0123456789ABCDEF";
    const uint8_t *         start;
    const uint8_t *         src;
    char *                  dst;
    char *                  end;
    size_t n;
    int offset;
    int width;
    const char *            newline;
    char separator[ 8 ];
    char *                  s;

    DEBUG_UNUSED( inType );
    DEBUG_UNUSED( inTypeSize );

    // Set up the function-wide variables.

    if( inLabelSize == kSizeCString )
    {
        inLabelSize = strlen( inLabel );
    }
    start   = (const uint8_t *) inData;
    src     = start;
    dst     = outBuffer;
    end     = dst + inBufferSize;
    offset  = (int)( (intptr_t) inData - (intptr_t) inDataStart );
    width   = ( (int) inLabelSize > inLabelMinWidth ) ? (int) inLabelSize : inLabelMinWidth;
    newline = ( inFlags & kDebugFlagsNoNewLine ) ? "" : "\n";

    // Set up the separator string. This is used to insert spaces on subsequent "lines" when not using newlines.

    s = separator;
    if( inFlags & kDebugFlagsNoNewLine )
    {
        if( inFlags & kDebugFlags8BitSeparator )
        {
            *s++ = ' ';
        }
        if( inFlags & kDebugFlags16BitSeparator )
        {
            *s++ = ' ';
        }
        if( !( inFlags & kDebugFlagsNo32BitSeparator ) )
        {
            *s++ = ' ';
        }
        check( ( (size_t)( s - separator ) ) < sizeof( separator ) );
    }
    *s = '\0';

    for( ;; )
    {
        char prefixString[ 32 ];
        char hexString[ 64 ];
        char asciiString[ 32 ];
        char byteCountString[ 32 ];
        int c;
        size_t chunkSize;
        size_t i;

        // If this is a label-only item (i.e. no data), print the label (accounting for prefix string spacing) and exit.

        if( inDataSize == 0 )
        {
            if( inLabel && ( inLabelSize > 0 ) )
            {
                width = 0;
                if( !( inFlags & kDebugFlagsNoAddress ) )
                {
                    width += 8;         // "00000000"
                    if( !( inFlags & kDebugFlagsNoOffset ) )
                    {
                        width += 1;     // "+"
                    }
                }
                if( inFlags & kDebugFlags32BitOffset )
                {
                    width += 8;         // "00000000"
                }
                else if( !( inFlags & kDebugFlagsNoOffset ) )
                {
                    width += 4;         // "0000"
                }

                if( outBuffer )
                {
                    dst += DebugSNPrintF( dst, (size_t)( end - dst ), "%*s" "%-*.*s" "%.*s" "%s",
                                          width, "",
                                          ( width > 0 ) ? ": " : "",
                                          width, (int) inLabelSize, inLabel,
                                          newline );
                }
                else
                {
                    dst += DebugPrintF( inLevel, "%*s" "%-*.*s" "%.*s" "%s",
                                        width, "",
                                        ( width > 0 ) ? ": " : "",
                                        width, (int) inLabelSize, inLabel,
                                        newline );
                }
            }
            break;
        }

        // Build the prefix string. It will be in one of the following formats:
        //
        // 1) "00000000+0000[0000]"	(address and offset)
        // 2) "00000000"			(address only)
        // 3) "0000[0000]"			(offset only)
        // 4) ""					(no address or offset)
        //
        // Note: If we're printing multiple "lines", but not printing newlines, a space is used to separate.

        s = prefixString;
        if( !( inFlags & kDebugFlagsNoAddress ) )
        {
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >> 28 ) & 0xF ];
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >> 24 ) & 0xF ];
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >> 20 ) & 0xF ];
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >> 16 ) & 0xF ];
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >> 12 ) & 0xF ];
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >>  8 ) & 0xF ];
            *s++ = kHexChars[ ( ( (uintptr_t) src ) >>  4 ) & 0xF ];
            *s++ = kHexChars[   ( (uintptr_t) src )         & 0xF ];

            if( !( inFlags & kDebugFlagsNoOffset ) )
            {
                *s++ = '+';
            }
        }
        if( !( inFlags & kDebugFlagsNoOffset ) )
        {
            if( inFlags & kDebugFlags32BitOffset )
            {
                *s++ = kHexChars[ ( offset >> 28 ) & 0xF ];
                *s++ = kHexChars[ ( offset >> 24 ) & 0xF ];
                *s++ = kHexChars[ ( offset >> 20 ) & 0xF ];
                *s++ = kHexChars[ ( offset >> 16 ) & 0xF ];
            }
            *s++ = kHexChars[ ( offset >> 12 ) & 0xF ];
            *s++ = kHexChars[ ( offset >>  8 ) & 0xF ];
            *s++ = kHexChars[ ( offset >>  4 ) & 0xF ];
            *s++ = kHexChars[   offset         & 0xF ];
        }
        if( s != prefixString )
        {
            *s++ = ':';
            *s++ = ' ';
        }
        check( ( (size_t)( s - prefixString ) ) < sizeof( prefixString ) );
        *s = '\0';

        // Build a hex string with a optional spaces after every 1, 2, and/or 4 bytes to make it easier to read.
        // Optionally pads the hex string with space to fill the full 16 byte range (so it lines up).

        s = hexString;
        chunkSize = ( inDataSize < 16 ) ? inDataSize : 16;
        n = ( inFlags & kDebugFlagsNo16ByteHexPad ) ? chunkSize : 16;
        for( i = 0; i < n; ++i )
        {
            if( ( inFlags & kDebugFlags8BitSeparator ) && ( i > 0 ) )
            {
                *s++ = ' ';
            }
            if( ( inFlags & kDebugFlags16BitSeparator ) && ( i > 0 ) && ( ( i % 2 ) == 0 ) )
            {
                *s++ = ' ';
            }
            if( !( inFlags & kDebugFlagsNo32BitSeparator ) && ( i > 0 ) && ( ( i % 4 ) == 0 ) )
            {
                *s++ = ' ';
            }
            if( i < chunkSize )
            {
                *s++ = kHexChars[ src[ i ] >> 4   ];
                *s++ = kHexChars[ src[ i ] &  0xF ];
            }
            else
            {
                *s++ = ' ';
                *s++ = ' ';
            }
        }
        check( ( (size_t)( s - hexString ) ) < sizeof( hexString ) );
        *s = '\0';

        // Build a string with the ASCII version of the data (replaces non-printable characters with '^').
        // Optionally pads the string with '`' to fill the full 16 byte range (so it lines up).

        s = asciiString;
        if( !( inFlags & kDebugFlagsNoASCII ) )
        {
            *s++ = ' ';
            *s++ = '|';
            for( i = 0; i < n; ++i )
            {
                if( i < chunkSize )
                {
                    c = src[ i ];
                    if( !DebugIsPrint( c ) )
                    {
                        c = '^';
                    }
                }
                else
                {
                    c = '`';
                }
                *s++ = (char) c;
            }
            *s++ = '|';
            check( ( (size_t)( s - asciiString ) ) < sizeof( asciiString ) );
        }
        *s = '\0';

        // Build a string indicating how bytes are in the hex dump. Only printed on the first line.

        s = byteCountString;
        if( !( inFlags & kDebugFlagsNoByteCount ) )
        {
            if( src == start )
            {
                s += DebugSNPrintF( s, sizeof( byteCountString ), " (%d bytes)", (int) inDataSize );
            }
        }
        check( ( (size_t)( s - byteCountString ) ) < sizeof( byteCountString ) );
        *s = '\0';

        // Build the entire line from all the pieces we've previously built.

        if( outBuffer )
        {
            if( src == start )
            {
                dst += DebugSNPrintF( dst, (size_t)( end - dst ),
                                      "%*s" // Indention
                                      "%s" // Separator (only if needed)
                                      "%s" // Prefix
                                      "%-*.*s" // Label
                                      "%s" // Separator
                                      "%s" // Hex
                                      "%s" // ASCII
                                      "%s" // Byte Count
                                      "%s", // Newline
                                      inIndent, "",
                                      ( src != start ) ? separator : "",
                                      prefixString,
                                      width, (int) inLabelSize, inLabel ? inLabel : "",
                                      ( width > 0 ) ? " " : "",
                                      hexString,
                                      asciiString,
                                      byteCountString,
                                      newline );
            }
            else
            {
                dst += DebugSNPrintF( dst, (size_t)( end - dst ),
                                      "%*s" // Indention
                                      "%s" // Separator (only if needed)
                                      "%s" // Prefix
                                      "%*s" // Label Spacing
                                      "%s" // Separator
                                      "%s" // Hex
                                      "%s" // ASCII
                                      "%s" // Byte Count
                                      "%s", // Newline
                                      inIndent, "",
                                      ( src != start ) ? separator : "",
                                      prefixString,
                                      width, "",
                                      ( width > 0 ) ? " " : "",
                                      hexString,
                                      asciiString,
                                      byteCountString,
                                      newline );
            }
        }
        else
        {
            if( src == start )
            {
                dst += DebugPrintF( inLevel,
                                    "%*s" // Indention
                                    "%s" // Separator (only if needed)
                                    "%s" // Prefix
                                    "%-*.*s" // Label
                                    "%s" // Separator
                                    "%s" // Hex
                                    "%s" // ASCII
                                    "%s" // Byte Count
                                    "%s", // Newline
                                    inIndent, "",
                                    ( src != start ) ? separator : "",
                                    prefixString,
                                    width, (int) inLabelSize, inLabel,
                                    ( width > 0 ) ? " " : "",
                                    hexString,
                                    asciiString,
                                    byteCountString,
                                    newline );
            }
            else
            {
                dst += DebugPrintF( inLevel,
                                    "%*s" // Indention
                                    "%s" // Separator (only if needed)
                                    "%s" // Prefix
                                    "%*s" // Label Spacing
                                    "%s" // Separator
                                    "%s" // Hex
                                    "%s" // ASCII
                                    "%s" // Byte Count
                                    "%s", // Newline
                                    inIndent, "",
                                    ( src != start ) ? separator : "",
                                    prefixString,
                                    width, "",
                                    ( width > 0 ) ? " " : "",
                                    hexString,
                                    asciiString,
                                    byteCountString,
                                    newline );
            }
        }

        // Move to the next chunk. Exit if there is no more data.

        offset      += (int) chunkSize;
        src         += chunkSize;
        inDataSize  -= chunkSize;
        if( inDataSize == 0 )
        {
            break;
        }
    }

    // Note: The "dst - outBuffer" size calculation works even if "outBuffer" is NULL because it's all relative.

    return( (size_t)( dst - outBuffer ) );
}

//===========================================================================================================================
//	DebugNumVersionToString
//===========================================================================================================================

static char *   DebugNumVersionToString( uint32_t inVersion, char *inString )
{
    char *      s;
    uint8_t majorRev;
    uint8_t minor;
    uint8_t bugFix;
    uint8_t stage;
    uint8_t revision;

    check( inString );

    majorRev    = (uint8_t)( ( inVersion >> 24 ) & 0xFF );
    minor       = (uint8_t)( ( inVersion >> 20 ) & 0x0F );
    bugFix      = (uint8_t)( ( inVersion >> 16 ) & 0x0F );
    stage       = (uint8_t)( ( inVersion >>  8 ) & 0xFF );
    revision    = (uint8_t)(   inVersion         & 0xFF );

    // Convert the major, minor, and bugfix numbers.

    s  = inString;
    s += sprintf( s, "%u", majorRev );
    s += sprintf( s, ".%u", minor );
    if( bugFix != 0 )
    {
        s += sprintf( s, ".%u", bugFix );
    }

    // Convert the version stage and non-release revision number.

    switch( stage )
    {
    case kVersionStageDevelopment:
        s += sprintf( s, "d%u", revision );
        break;

    case kVersionStageAlpha:
        s += sprintf( s, "a%u", revision );
        break;

    case kVersionStageBeta:
        s += sprintf( s, "b%u", revision );
        break;

    case kVersionStageFinal:

        // A non-release revision of zero is a special case indicating the software is GM (at the golden master
        // stage) and therefore, the non-release revision should not be added to the string.

        if( revision != 0 )
        {
            s += sprintf( s, "f%u", revision );
        }
        break;

    default:
        dlog( kDebugLevelError, "invalid NumVersion stage (0x%02X)\n", stage );
        break;
    }
    return( inString );
}

//===========================================================================================================================
//	DebugTaskLevel
//===========================================================================================================================

DEBUG_EXPORT uint32_t   DebugTaskLevel( void )
{
    uint32_t level;

    level = 0;

    return( level );
}

#if ( TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE )
//===========================================================================================================================
//	DebugWinEnableConsole
//===========================================================================================================================

#pragma warning( disable:4311 )

static void DebugWinEnableConsole( void )
{
    static bool sConsoleEnabled = false;
    BOOL result;
    int fileHandle;
    FILE *          file;
    int err;

    if( sConsoleEnabled )
    {
        goto exit;
    }

    // Create console window.

    result = AllocConsole();
    require_quiet( result, exit );

    // Redirect stdin to the console stdin.

    fileHandle = _open_osfhandle( (long) GetStdHandle( STD_INPUT_HANDLE ), _O_TEXT );

    #if ( defined( __MWERKS__ ) )
    file = __handle_reopen( (unsigned long) fileHandle, "r", stdin );
    require_quiet( file, exit );
    #else
    file = _fdopen( fileHandle, "r" );
    require_quiet( file, exit );

    *stdin = *file;
    #endif

    err = setvbuf( stdin, NULL, _IONBF, 0 );
    require_noerr_quiet( err, exit );

    // Redirect stdout to the console stdout.

    fileHandle = _open_osfhandle( (long) GetStdHandle( STD_OUTPUT_HANDLE ), _O_TEXT );

    #if ( defined( __MWERKS__ ) )
    file = __handle_reopen( (unsigned long) fileHandle, "w", stdout );
    require_quiet( file, exit );
    #else
    file = _fdopen( fileHandle, "w" );
    require_quiet( file, exit );

    *stdout = *file;
    #endif

    err = setvbuf( stdout, NULL, _IONBF, 0 );
    require_noerr_quiet( err, exit );

    // Redirect stderr to the console stdout.

    fileHandle = _open_osfhandle( (long) GetStdHandle( STD_OUTPUT_HANDLE ), _O_TEXT );

    #if ( defined( __MWERKS__ ) )
    file = __handle_reopen( (unsigned long) fileHandle, "w", stderr );
    require_quiet( file, exit );
    #else
    file = _fdopen( fileHandle, "w" );
    require_quiet( file, exit );

    *stderr = *file;
    #endif

    err = setvbuf( stderr, NULL, _IONBF, 0 );
    require_noerr_quiet( err, exit );

    sConsoleEnabled = true;

exit:
    return;
}

#pragma warning( default:4311 )

#endif  // TARGET_OS_WIN32 && !TARGET_OS_WINDOWS_CE

#if ( TARGET_OS_WIN32 )
//===========================================================================================================================
//	DebugWinCharToTCharString
//===========================================================================================================================

static TCHAR *
DebugWinCharToTCharString(
    const char *    inCharString,
    size_t inCharCount,
    TCHAR *         outTCharString,
    size_t inTCharCountMax,
    size_t *        outTCharCount )
{
    const char *        src;
    TCHAR *             dst;
    TCHAR *             end;

    if( inCharCount == kSizeCString )
    {
        inCharCount = strlen( inCharString );
    }
    src = inCharString;
    dst = outTCharString;
    if( inTCharCountMax > 0 )
    {
        inTCharCountMax -= 1;
        if( inTCharCountMax > inCharCount )
        {
            inTCharCountMax = inCharCount;
        }

        end = dst + inTCharCountMax;
        while( dst < end )
        {
            *dst++ = (TCHAR) *src++;
        }
        *dst = 0;
    }
    if( outTCharCount )
    {
        *outTCharCount = (size_t)( dst - outTCharString );
    }
    return( outTCharString );
}
#endif

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

//===========================================================================================================================
//	DebugServicesTest
//===========================================================================================================================

DEBUG_EXPORT OSStatus   DebugServicesTest( void )
{
    OSStatus err;
    char s[ 512 ];
    uint8_t *       p;
    uint8_t data[] =
    {
        0x11, 0x22, 0x33, 0x44,
        0x55, 0x66,
        0x77, 0x88, 0x99, 0xAA,
        0xBB, 0xCC, 0xDD,
        0xEE,
        0xFF,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
        0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0,
        0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x81, 0x91, 0xA1
    };

    debug_initialize( kDebugOutputTypeMetaConsole );

    // check's

    check( 0 && "SHOULD SEE: check" );
    check( 1 && "SHOULD *NOT* SEE: check (valid)" );
    check_string( 0, "SHOULD SEE: check_string" );
    check_string( 1, "SHOULD *NOT* SEE: check_string (valid)" );
    check_noerr( -123 );
    check_noerr( 10038 );
    check_noerr( 22 );
    check_noerr( 0 );
    check_noerr_string( -6712, "SHOULD SEE: check_noerr_string" );
    check_noerr_string( 0, "SHOULD *NOT* SEE: check_noerr_string (valid)" );
    check_translated_errno( 0 >= 0 && "SHOULD *NOT* SEE", -384, -999 );
    check_translated_errno( -1 >= 0 && "SHOULD SEE", -384, -999 );
    check_translated_errno( -1 >= 0 && "SHOULD SEE", 0, -999 );
    check_ptr_overlap( "SHOULD *NOT* SEE" ? 10 : 0, 10, 22, 10 );
    check_ptr_overlap( "SHOULD SEE" ? 10 : 0, 10,  5, 10 );
    check_ptr_overlap( "SHOULD SEE" ? 10 : 0, 10, 12,  6 );
    check_ptr_overlap( "SHOULD SEE" ? 12 : 0,  6, 10, 10 );
    check_ptr_overlap( "SHOULD SEE" ? 12 : 0, 10, 10, 10 );
    check_ptr_overlap( "SHOULD *NOT* SEE" ? 22 : 0, 10, 10, 10 );
    check_ptr_overlap( "SHOULD *NOT* SEE" ? 10 : 0, 10, 20, 10 );
    check_ptr_overlap( "SHOULD *NOT* SEE" ? 20 : 0, 10, 10, 10 );

    // require's

    require( 0 && "SHOULD SEE", require1 );
    { err = kResponseErr; goto exit; }
require1:
    require( 1 && "SHOULD *NOT* SEE", require2 );
    goto require2Good;
require2:
    { err = kResponseErr; goto exit; }
require2Good:
    require_string( 0 && "SHOULD SEE", require3, "SHOULD SEE: require_string" );
    { err = kResponseErr; goto exit; }
require3:
    require_string( 1 && "SHOULD *NOT* SEE", require4, "SHOULD *NOT* SEE: require_string (valid)" );
    goto require4Good;
require4:
    { err = kResponseErr; goto exit; }
require4Good:
    require_quiet( 0 && "SHOULD SEE", require5 );
    { err = kResponseErr; goto exit; }
require5:
    require_quiet( 1 && "SHOULD *NOT* SEE", require6 );
    goto require6Good;
require6:
    { err = kResponseErr; goto exit; }
require6Good:
    require_noerr( -1, require7 );
    { err = kResponseErr; goto exit; }
require7:
    require_noerr( 0, require8 );
    goto require8Good;
require8:
    { err = kResponseErr; goto exit; }
require8Good:
    require_noerr_string( -2, require9, "SHOULD SEE: require_noerr_string");
    { err = kResponseErr; goto exit; }
require9:
    require_noerr_string( 0, require10, "SHOULD *NOT* SEE: require_noerr_string (valid)" );
    goto require10Good;
require10:
    { err = kResponseErr; goto exit; }
require10Good:
    require_noerr_action_string( -3, require11, dlog( kDebugLevelMax, "action 1 (expected)\n" ), "require_noerr_action_string" );
    { err = kResponseErr; goto exit; }
require11:
    require_noerr_action_string( 0, require12, dlog( kDebugLevelMax, "action 2\n" ), "require_noerr_action_string (valid)" );
    goto require12Good;
require12:
    { err = kResponseErr; goto exit; }
require12Good:
    require_noerr_quiet( -4, require13 );
    { err = kResponseErr; goto exit; }
require13:
    require_noerr_quiet( 0, require14 );
    goto require14Good;
require14:
    { err = kResponseErr; goto exit; }
require14Good:
    require_noerr_action( -5, require15, dlog( kDebugLevelMax, "SHOULD SEE: action 3 (expected)\n" ) );
    { err = kResponseErr; goto exit; }
require15:
    require_noerr_action( 0, require16, dlog( kDebugLevelMax, "SHOULD *NOT* SEE: action 4\n" ) );
    goto require16Good;
require16:
    { err = kResponseErr; goto exit; }
require16Good:
    require_noerr_action_quiet( -4, require17, dlog( kDebugLevelMax, "SHOULD SEE: action 5 (expected)\n" ) );
    { err = kResponseErr; goto exit; }
require17:
    require_noerr_action_quiet( 0, require18, dlog( kDebugLevelMax, "SHOULD *NOT* SEE: action 6\n" ) );
    goto require18Good;
require18:
    { err = kResponseErr; goto exit; }
require18Good:
    require_action( 0 && "SHOULD SEE", require19, dlog( kDebugLevelMax, "SHOULD SEE: action 7 (expected)\n" ) );
    { err = kResponseErr; goto exit; }
require19:
    require_action( 1 && "SHOULD *NOT* SEE", require20, dlog( kDebugLevelMax, "SHOULD *NOT* SEE: action 8\n" ) );
    goto require20Good;
require20:
    { err = kResponseErr; goto exit; }
require20Good:
    require_action_quiet( 0, require21, dlog( kDebugLevelMax, "SHOULD SEE: action 9 (expected)\n" ) );
    { err = kResponseErr; goto exit; }
require21:
    require_action_quiet( 1, require22, dlog( kDebugLevelMax, "SHOULD *NOT* SEE: action 10\n" ) );
    goto require22Good;
require22:
    { err = kResponseErr; goto exit; }
require22Good:
    require_action_string( 0, require23, dlog( kDebugLevelMax, "SHOULD SEE: action 11 (expected)\n" ), "SHOULD SEE: require_action_string" );
    { err = kResponseErr; goto exit; }
require23:
    require_action_string( 1, require24, dlog( kDebugLevelMax, "SHOULD *NOT* SEE: action 12\n" ), "SHOULD *NOT* SEE: require_action_string" );
    goto require24Good;
require24:
    { err = kResponseErr; goto exit; }
require24Good:

#if ( defined( __MWERKS__ )  )
    #if ( defined( __cplusplus ) && __option( exceptions ) )
        #define COMPILER_HAS_EXCEPTIONS     1
    #else
        #define COMPILER_HAS_EXCEPTIONS     0
    #endif
#else
    #if ( defined( __cplusplus ) )
        #define COMPILER_HAS_EXCEPTIONS     1
    #else
        #define COMPILER_HAS_EXCEPTIONS     0
    #endif
#endif

#if ( COMPILER_HAS_EXCEPTIONS )
    try
    {
        require_throw( 1 && "SHOULD *NOT* SEE" );
        require_throw( 0 && "SHOULD SEE" );
    }
    catch(... )
    {
        goto require26Good;
    }
    { err = kResponseErr; goto exit; }
require26Good:
#endif

    // translate_errno

    err = translate_errno( 1 != -1, -123, -567 );
    require( ( err == 0 ) && "SHOULD *NOT* SEE", exit );

    err = translate_errno( -1 != -1, -123, -567 );
    require( ( err == -123 ) && "SHOULD *NOT* SEE", exit );

    err = translate_errno( -1 != -1, 0, -567 );
    require( ( err == -567 ) && "SHOULD *NOT* SEE", exit );

    // debug_string

    debug_string( "debug_string" );

    // DebugSNPrintF

    DebugSNPrintF( s, sizeof( s ), "%d", 1234 );
    require_action( strcmp( s, "1234" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%X", 0x2345 );
    require_action( strcmp( s, "2345" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%#s", "\05test" );
    require_action( strcmp( s, "test" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%##s", "\03www\05apple\03com" );
    require_action( strcmp( s, "www.apple.com." ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%ld", (long) INT32_C( 2147483647 ) );
    require_action( strcmp( s, "2147483647" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%lu", (unsigned long) UINT32_C( 4294967295 ) );
    require_action( strcmp( s, "4294967295" ) == 0, exit, err = -1 );

    #if ( TYPE_LONGLONG_NATIVE )
    DebugSNPrintF( s, sizeof( s ), "%lld", (long_long_compat) INT64_C( 9223372036854775807 ) );
    require_action( strcmp( s, "9223372036854775807" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%lld", (long_long_compat) INT64_C( -9223372036854775807 ) );
    require_action( strcmp( s, "-9223372036854775807" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%llu", (unsigned_long_long_compat) UINT64_C( 18446744073709551615 ) );
    require_action( strcmp( s, "18446744073709551615" ) == 0, exit, err = -1 );
    #endif

    DebugSNPrintF( s, sizeof( s ), "%lb", (unsigned long) binary_32( 01111011, 01111011, 01111011, 01111011 ) );
    require_action( strcmp( s, "1111011011110110111101101111011" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%C", 0x41624364 );  // 'AbCd'
    require_action( strcmp( s, "AbCd" ) == 0, exit, err = -1 );

    #if ( defined( MDNS_DEBUGMSGS ) )
    {
        mDNSAddr maddr;

        memset( &maddr, 0, sizeof( maddr ) );
        maddr.type = mDNSAddrType_IPv4;
        maddr.ip.v4.b[ 0 ] = 127;
        maddr.ip.v4.b[ 1 ] = 0;
        maddr.ip.v4.b[ 2 ] = 0;
        maddr.ip.v4.b[ 3 ] = 1;
        DebugSNPrintF( s, sizeof( s ), "%#a", &maddr );
        require_action( strcmp( s, "127.0.0.1" ) == 0, exit, err = -1 );

        memset( &maddr, 0, sizeof( maddr ) );
        maddr.type = mDNSAddrType_IPv6;
        maddr.ip.v6.b[  0 ] = 0xFE;
        maddr.ip.v6.b[  1 ] = 0x80;
        maddr.ip.v6.b[ 15 ] = 0x01;
        DebugSNPrintF( s, sizeof( s ), "%#a", &maddr );
        require_action( strcmp( s, "FE80:0000:0000:0000:0000:0000:0000:0001" ) == 0, exit, err = -1 );
    }
    #endif

    #if ( AF_INET )
    {
        struct sockaddr_in sa4;

        memset( &sa4, 0, sizeof( sa4 ) );
        sa4.sin_family      = AF_INET;
        p                   = (uint8_t *) &sa4.sin_port;
        p[ 0 ]              = (uint8_t)( ( 80 >> 8 ) & 0xFF );
        p[ 1 ]              = (uint8_t)(   80        & 0xFF );
        p                   = (uint8_t *) &sa4.sin_addr.s_addr;
        p[ 0 ]              = (uint8_t)( ( INADDR_LOOPBACK >> 24 ) & 0xFF );
        p[ 1 ]              = (uint8_t)( ( INADDR_LOOPBACK >> 16 ) & 0xFF );
        p[ 2 ]              = (uint8_t)( ( INADDR_LOOPBACK >>  8 ) & 0xFF );
        p[ 3 ]              = (uint8_t)(   INADDR_LOOPBACK         & 0xFF );
        DebugSNPrintF( s, sizeof( s ), "%##a", &sa4 );
        require_action( strcmp( s, "127.0.0.1:80" ) == 0, exit, err = -1 );
    }
    #endif

    #if ( AF_INET6 )
    {
        struct sockaddr_in6 sa6;

        memset( &sa6, 0, sizeof( sa6 ) );
        sa6.sin6_family             = AF_INET6;
        p                           = (uint8_t *) &sa6.sin6_port;
        p[ 0 ]                      = (uint8_t)( ( 80 >> 8 ) & 0xFF );
        p[ 1 ]                      = (uint8_t)(   80        & 0xFF );
        sa6.sin6_addr.s6_addr[  0 ] = 0xFE;
        sa6.sin6_addr.s6_addr[  1 ] = 0x80;
        sa6.sin6_addr.s6_addr[ 15 ] = 0x01;
        sa6.sin6_scope_id           = 2;
        DebugSNPrintF( s, sizeof( s ), "%##a", &sa6 );
        require_action( strcmp( s, "[FE80:0000:0000:0000:0000:0000:0000:0001%2]:80" ) == 0, exit, err = -1 );
    }
    #endif

    // Unicode

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "tes" );
    require_action( strcmp( s, "tes" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "test" );
    require_action( strcmp( s, "test" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "testing" );
    require_action( strcmp( s, "test" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "te\xC3\xA9" );
    require_action( strcmp( s, "te\xC3\xA9" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "te\xC3\xA9ing" );
    require_action( strcmp( s, "te\xC3\xA9" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "tes\xC3\xA9ing" );
    require_action( strcmp( s, "tes" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "t\xed\x9f\xbf" );
    require_action( strcmp( s, "t\xed\x9f\xbf" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "t\xed\x9f\xbfing" );
    require_action( strcmp( s, "t\xed\x9f\xbf" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "te\xed\x9f\xbf" );
    require_action( strcmp( s, "te" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 4, "te\xed\x9f\xbfing" );
    require_action( strcmp( s, "te" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 7, "te\xC3\xA9\xed\x9f\xbfing" );
    require_action( strcmp( s, "te\xC3\xA9\xed\x9f\xbf" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 6, "te\xC3\xA9\xed\x9f\xbfing" );
    require_action( strcmp( s, "te\xC3\xA9" ) == 0, exit, err = kResponseErr );

    DebugSNPrintF(s, sizeof(s), "%.*s", 5, "te\xC3\xA9\xed\x9f\xbfing" );
    require_action( strcmp( s, "te\xC3\xA9" ) == 0, exit, err = kResponseErr );

    #if ( TARGET_RT_BIG_ENDIAN )
    DebugSNPrintF( s, sizeof( s ), "%S", "\x00" "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" "\x00" );
    require_action( strcmp( s, "abcd" ) == 0, exit, err = -1 );
    #else
    DebugSNPrintF( s, sizeof( s ), "%S", "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" "\x00" "\x00" );
    require_action( strcmp( s, "abcd" ) == 0, exit, err = -1 );
    #endif

    DebugSNPrintF( s, sizeof( s ), "%S",
                   "\xFE\xFF" "\x00" "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" "\x00" ); // Big Endian BOM
    require_action( strcmp( s, "abcd" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%S",
                   "\xFF\xFE" "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" "\x00" "\x00" ); // Little Endian BOM
    require_action( strcmp( s, "abcd" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%#S", "\x00" "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" "\x00" );  // Big Endian
    require_action( strcmp( s, "abcd" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%##S", "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" "\x00" "\x00" ); // Little Endian
    require_action( strcmp( s, "abcd" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%.*S",
                   4, "\xFE\xFF" "\x00" "a" "\x00" "b" "\x00" "c" "\x00" "d" ); // Big Endian BOM
    require_action( strcmp( s, "abc" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%.*S",
                   4, "\xFF\xFE" "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" ); // Little Endian BOM
    require_action( strcmp( s, "abc" ) == 0, exit, err = -1 );

    #if ( TARGET_RT_BIG_ENDIAN )
    DebugSNPrintF( s, sizeof( s ), "%.*S", 3, "\x00" "a" "\x00" "b" "\x00" "c" "\x00" "d" );
    require_action( strcmp( s, "abc" ) == 0, exit, err = -1 );
    #else
    DebugSNPrintF( s, sizeof( s ), "%.*S", 3, "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" );
    require_action( strcmp( s, "abc" ) == 0, exit, err = -1 );
    #endif

    DebugSNPrintF( s, sizeof( s ), "%#.*S", 3, "\x00" "a" "\x00" "b" "\x00" "c" "\x00" "d" );   // Big Endian
    require_action( strcmp( s, "abc" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%##.*S", 3, "a" "\x00" "b" "\x00" "c" "\x00" "d" "\x00" );  // Little Endian
    require_action( strcmp( s, "abc" ) == 0, exit, err = -1 );

    // Misc

    DebugSNPrintF( s, sizeof( s ), "%U", "\x10\xb8\xa7\x6b" "\xad\x9d" "\xd1\x11" "\x80\xb4" "\x00\xc0\x4f\xd4\x30\xc8" );
    require_action( strcmp( s, "6ba7b810-9dad-11d1-80b4-00c04fd430c8" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%m", 0 );
    require_action( strcmp( s, "no error" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "%lm", (long) 0 );
    require_action( strcmp( s, "no error" ) == 0, exit, err = -1 );

    DebugSNPrintF( s, sizeof( s ), "\"%H\"", "\x6b\xa7\xb8\x10\x9d\xad\x11\xd1\x80\xb4\x00\xc0\x4f\xd4\x30\xc8", 16, 16 );
    DebugPrintF( kDebugLevelMax, "%s\n\n", s );

    DebugSNPrintF( s, sizeof( s ), "\"%H\"",
                   "\x6b\xa7\xb8\x10\x9d\xad\x11\xd1\x80\xb4\x00\xc0\x4f\xd4\x30\xc8"
                   "\x6b\xa7\xb8\x10\x9d\xad\x11\xd1\x80\xb4\x00\xc0\x4f\xd4\x30\xc8",
                   32, 32 );
    DebugPrintF( kDebugLevelMax, "%s\n\n", s );

    DebugSNPrintF( s, sizeof( s ), "\"%H\"", "\x6b\xa7", 2, 2 );
    DebugPrintF( kDebugLevelMax, "%s\n\n", s );

    // Hex Dumps

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, "My Label", kSizeCString, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNone, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoAddress | kDebugFlagsNoOffset, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, "My Label", kSizeCString, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoAddress | kDebugFlagsNoOffset, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, "My Label", kSizeCString, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoAddress, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoOffset, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoAddress, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoOffset, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoByteCount, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, "\x41\x62\x43\x64", "\x41\x62\x43\x64", 4,    // 'AbCd'
                  kDebugFlagsNoAddress | kDebugFlagsNoOffset | kDebugFlagsNoNewLine |
                  kDebugFlagsNo32BitSeparator | kDebugFlagsNo16ByteHexPad | kDebugFlagsNoByteCount,
                  s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 0, NULL, 0, 0, NULL, 0, data, data, sizeof( data ),
                  kDebugFlagsNoAddress | kDebugFlagsNoOffset | kDebugFlagsNoASCII | kDebugFlagsNoNewLine |
                  kDebugFlags16BitSeparator | kDebugFlagsNo32BitSeparator |
                  kDebugFlagsNo16ByteHexPad | kDebugFlagsNoByteCount, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    s[ 0 ] = '\0';
    DebugHexDump( kDebugLevelMax, 8, NULL, 0, 0, NULL, 0, data, data, sizeof( data ), kDebugFlagsNone, s, sizeof( s ) );
    DebugPrintF( kDebugLevelMax, "%s\n", s );

    // dlog's

    dlog( kDebugLevelNotice, "dlog\n" );
    dlog( kDebugLevelNotice, "dlog integer: %d\n", 123 );
    dlog( kDebugLevelNotice, "dlog string:  \"%s\"\n", "test string" );
    dlogmem( kDebugLevelNotice, data, sizeof( data ) );

    // Done

    DebugPrintF( kDebugLevelMax, "\n\nALL TESTS DONE\n\n" );
    err = kNoErr;

exit:
    if( err )
    {
        DebugPrintF( kDebugLevelMax, "\n\n### TEST FAILED ###\n\n" );
    }
    return( err );
}

#endif  // DEBUG
