/*	CFURLComponents_URIParser.c
	Copyright (c) 2015-2019, Apple Inc. All rights reserved.
	Responsibility: Jim Luther/Chris Linn
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include "CFOverflow.h"
#include "CFURLComponents_Internal.h"
#include "CFInternal.h"

typedef CF_ENUM(CFIndex, URLPredefinedCharacterSet) {
    kURLUserAllowedCharacterSet     = 0,
    kURLPasswordAllowedCharacterSet = 1,
    kURLHostAllowedCharacterSet     = 2,
    kURLPathAllowedCharacterSet     = 3,
    kURLQueryAllowedCharacterSet    = 4,
    kURLFragmentAllowedCharacterSet = 5,
    kURLAllowedCharacterSetIllegal  = 6
};

// IMPORTANT: the kURLxxxxAllowedCharacters definitions MUST match sURLAllowedCharacters (except for the '[', ':' and ']' characters in kURLHostAllowedCharacters are not kURLHostAllowed, and ';' is not in kURLPathAllowedCharacters, but is special cased in kURLPathAllowed)
#define kURLUserAllowedCharacters       "!$&'()*+,-.0123456789;=ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~"
#define kURLPasswordAllowedCharacters   "!$&'()*+,-.0123456789;=ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~"
#define kURLHostAllowedCharacters       "!$&'()*+,-.0123456789:;=ABCDEFGHIJKLMNOPQRSTUVWXYZ[]_abcdefghijklmnopqrstuvwxyz~"
#define kURLPathAllowedCharacters       "!$&'()*+,-./0123456789:=@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~" // ";" is disallowed in paths for compatibility with API which uses rfc1808 parsing where ";" was the separator between path and param. ":" is allowed only after a "/" (it cannot be in the first segment in some cases)
#define kURLQueryAllowedCharacters      "!$&'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~"
#define kURLFragmentAllowedCharacters   "!$&'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~"

// IMPORTANT: the kURLxxxxAllowedCharacters definitions MUST match sURLAllowedCharacters (except for the '[', ':' and ']' characters in kURLHostAllowedCharacters are not kURLHostAllowed)
static const unsigned short sURLAllowedCharacters[128] = {
    /* nul */ 0,
    /* soh */ 0,
    /* stx */ 0,
    /* etx */ 0,
    /* eot */ 0,
    /* enq */ 0,
    /* ack */ 0,
    /* bel */ 0,
    /* bs  */ 0,
    /* ht  */ 0,
    /* nl  */ 0,
    /* vt  */ 0,
    /* np  */ 0,
    /* cr  */ 0,
    /* so  */ 0,
    /* si  */ 0,
    /* dle */ 0,
    /* dc1 */ 0,
    /* dc2 */ 0,
    /* dc3 */ 0,
    /* dc4 */ 0,
    /* nak */ 0,
    /* syn */ 0,
    /* etb */ 0,
    /* can */ 0,
    /* em  */ 0,
    /* sub */ 0,
    /* esc */ 0,
    /* fs  */ 0,
    /* gs  */ 0,
    /* rs  */ 0,
    /* us  */ 0,
    /* sp  */ 0,
    /* '!' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '"' */ 0,
    /* '#' */ 0,
    /* '$' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '%' */ 0,
    /* '&' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed,
    /* ''' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '(' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* ')' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '*' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '+' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* ',' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '-' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '.' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '/' */                                                                                                 kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '0' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '1' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '2' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '3' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '4' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '5' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '6' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '7' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '8' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* '9' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed | kURLPortAllowed | kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed |                    kURLQueryItemNameAllowed,
    /* ':' */                                                                                                 kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* ';' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '<' */ 0,
    /* '=' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed ,
    /* '>' */ 0,
    /* '?' */                                                                                                                   kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '@' */                                                                                                 kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* 'A' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'B' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'C' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'D' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'E' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'F' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'G' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'H' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'I' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'J' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'K' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'L' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'M' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'N' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'O' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'P' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'Q' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'R' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'S' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'T' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'U' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'V' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'W' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'X' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'Y' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'Z' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* '[' */ 0,
    /* '\' */ 0,
    /* ']' */ 0,
    /* '^' */ 0,
    /* '_' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* '`' */ 0,
    /* 'a' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'b' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'c' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'd' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'e' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'f' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed | kURLHexDigAllowed | kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'g' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'h' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'i' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'j' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'k' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'l' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'm' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'n' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'o' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'p' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'q' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'r' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 's' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 't' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'u' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'v' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'w' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'x' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'y' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* 'z' */ kURLSchemeAllowed | kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                     kURLAlphaAllowed | kURLQueryItemNameAllowed,
    /* '{' */ 0,
    /* '|' */ 0,
    /* '}' */ 0,
    /* '~' */                     kURLUserAllowed | kURLPasswordAllowed | kURLHostAllowed |                   kURLPathAllowed | kURLQueryAllowed | kURLFragmentAllowed |                                        kURLQueryItemNameAllowed,
    /* del */ 0,
};

// an array of CFCharacterSetRef for each of the predefined allowed character sets
static CFCharacterSetRef *sURLAllowedCharacterSets = NULL;

/* Create the sURLAllowedCharacterSets array.
 */
static void InitializeURLAllowedCharacterSets(void)
{
    static dispatch_once_t static_init = 0;
    dispatch_once(&static_init, ^{
        sURLAllowedCharacterSets = (CFCharacterSetRef *)CFAllocatorAllocate(kCFAllocatorDefault, sizeof(CFCharacterSetRef) * kURLAllowedCharacterSetIllegal, 0);
        sURLAllowedCharacterSets[kURLUserAllowedCharacterSet] = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(kURLUserAllowedCharacters));
        sURLAllowedCharacterSets[kURLPasswordAllowedCharacterSet] = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(kURLPasswordAllowedCharacters));
        sURLAllowedCharacterSets[kURLHostAllowedCharacterSet] = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(kURLHostAllowedCharacters));
        sURLAllowedCharacterSets[kURLPathAllowedCharacterSet] = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(kURLPathAllowedCharacters));
        sURLAllowedCharacterSets[kURLQueryAllowedCharacterSet] = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(kURLQueryAllowedCharacters));
        sURLAllowedCharacterSets[kURLFragmentAllowedCharacterSet] = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(kURLFragmentAllowedCharacters));
    });
}

/* Returns the set from sURLAllowedCharacterSets for the given URLPredefinedCharacterSet.
 */
static CFCharacterSetRef GetURLAllowedCharacterSet(URLPredefinedCharacterSet allowedSet)
{
    CFCharacterSetRef result;
    
    // make sure sURLAllowedCharacterSets is initialized
    InitializeURLAllowedCharacterSets();
    // return the URLPredefinedCharacterSet requested
    if ( (allowedSet >= 0) && (allowedSet < kURLAllowedCharacterSetIllegal) ) {
        result = sURLAllowedCharacterSets[allowedSet];
    }
    else {
        result = nil;
    }
    return ( result );
}

/* Returns the URLPredefinedCharacterSet that matches the set, or kURLAllowedCharacterSetIllegal if there is no match.
 */
static URLPredefinedCharacterSet GetURLPredefinedCharacterSet(CFCharacterSetRef characterSet)
{
    URLPredefinedCharacterSet result;
    
    // make sure sURLAllowedCharacterSets is initialized
    InitializeURLAllowedCharacterSets();
    // see if characterSet is one of the URLPredefinedCharacterSets
    if ( characterSet ) {
        for ( result = 0; result < kURLAllowedCharacterSetIllegal; ++result ) {
            // yes, I really want a pointer comparison because some of the sURLAllowedCharacterSets have the same bitmaps
            if ( characterSet == sURLAllowedCharacterSets[result] ) {
                break;
            }
        }
    }
    else {
        result = kURLAllowedCharacterSetIllegal;
    }
    
    return ( result );
}


//******************************************************************************

CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLUserAllowedCharacterSet() {
    return ( GetURLAllowedCharacterSet(kURLUserAllowedCharacterSet) );
}

CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLPasswordAllowedCharacterSet() {
    return ( GetURLAllowedCharacterSet(kURLPasswordAllowedCharacterSet) );
}

CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLHostAllowedCharacterSet() {
    return ( GetURLAllowedCharacterSet(kURLHostAllowedCharacterSet) );
}

CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLPathAllowedCharacterSet() {
    return ( GetURLAllowedCharacterSet(kURLPathAllowedCharacterSet) );
}

CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLQueryAllowedCharacterSet() {
    return ( GetURLAllowedCharacterSet(kURLQueryAllowedCharacterSet) );
}

CF_EXPORT CFCharacterSetRef _CFURLComponentsGetURLFragmentAllowedCharacterSet() {
    return ( GetURLAllowedCharacterSet(kURLFragmentAllowedCharacterSet) );
}


#if 0
#pragma mark -
#pragma mark CFString extensions
#endif

CF_EXPORT CFStringRef _CFStringCreateByAddingPercentEncodingWithAllowedCharacters(CFAllocatorRef alloc, CFStringRef string, CFCharacterSetRef allowedCharacters) {
    CFStringRef result = NULL;
    CFIndex inLength = CFStringGetLength((CFStringRef)string);
    
    if ( inLength == 0 ) {
        // No characters? Nothing to percent-encode.
        result = CFStringCreateCopy(alloc, string);
    }
    else {
        static const UInt8 hexchars[] = "0123456789ABCDEF";
        CFIndex maxBufferSize = CFStringGetMaximumSizeForEncoding(inLength, kCFStringEncodingUTF8);
        // CFStringGetMaximumSizeForEncoding returns kCFNotFound if the result would be too big
        if ( maxBufferSize != kCFNotFound ) {
            enum {
                kStackBufferSize = 4096,
                kInStackBufferSize = kStackBufferSize / 4,
                kOutStackBufferSize = kInStackBufferSize * 3,
            };
            STACK_BUFFER_DECL(UInt8, stackBuffer, kStackBufferSize);
            UInt8 *inBuf = NULL;
            UInt8 *outBuf;
            // choose a buffer to put the input bytes AND output bytes into
            if ( maxBufferSize <= kInStackBufferSize ) {
                inBuf = &stackBuffer[0];
            }
            else {
                // not big enough? malloc it.
                CFIndex mallocSize;
                if ( _CFMultiplyBufferSizeWithoutOverflow(maxBufferSize, 4, &mallocSize) ) {
                    if (mallocSize >= 0) {
                        inBuf = (UInt8 *)malloc(mallocSize);
                    }
                }
            }
            if ( inBuf ) {
                CFIndex charsConverted;
                CFIndex inLen;
                // use the other 3/4 of the buffer for the percent-encoded bytes
                outBuf = &inBuf[maxBufferSize];
                
                charsConverted = CFStringGetBytes(string, CFRangeMake(0, inLength), kCFStringEncodingUTF8, 0, false, inBuf, maxBufferSize, &inLen);
                if ( charsConverted ) {
                    UInt8 *inBytePtr = inBuf;
                    UInt8 *outBytePtr = outBuf;
                    CFIndex idx;
                    
                    // there are two very similar loops below -- they aren't combined because I didn't want an extra comparison per character to determine which code was going to set the allowed variable.
                    
                    URLPredefinedCharacterSet allowedSet = GetURLPredefinedCharacterSet(allowedCharacters);
                    if ( allowedSet != kURLAllowedCharacterSetIllegal ) {
                        // fastest -- allowedCharacters is one of the predefined sets so use sURLAllowedCharacters to determine what characters are allowed
                        unsigned char allowedMask;
                        Boolean isIPLiteral = false;
                        
                        // determine the allowedMask
                        switch (allowedSet) {
                            case kURLUserAllowedCharacterSet:
                                allowedMask = kURLUserAllowed;
                                break;
                            case kURLPasswordAllowedCharacterSet:
                                allowedMask = kURLPasswordAllowed;
                                break;
                            case kURLHostAllowedCharacterSet:
                                allowedMask = kURLHostAllowed;
                                // if the host is an IP-Literal, percent-encode everything within the brackets but not the brackets
                                if ( (inLen >= 2)  && (*inBytePtr == '[') && (inBytePtr[inLen - 1] == ']') ) {
                                    isIPLiteral = true;
                                    ++inBytePtr;
                                    // copy the open bracket
                                    *outBytePtr++ = '[';
                                    inLen -= 2;
                                }
                                break;
                            case kURLPathAllowedCharacterSet:
                                allowedMask = kURLPathAllowed;
                                break;
                            case kURLQueryAllowedCharacterSet:
                                allowedMask = kURLQueryAllowed;
                                break;
                            case kURLFragmentAllowedCharacterSet:
                                allowedMask = kURLFragmentAllowed;
                                break;
                            default:
                                // GetURLPredefinedCharacterSet will return one of the above or kURLAllowedCharacterSetIllegal so this will never be hit
                                allowedMask = 0;
                                break;
                        }
                        if ( allowedSet == kURLPathAllowedCharacterSet ) {
                            Boolean pastSlash = false;
                            for ( idx = 0; idx < inLen; ++idx ) {
                                UInt8 ch = *inBytePtr++;
                                if ( pastSlash ) {
                                    // !!!: percent encode ';' for backwards compatibility with API which uses rfc1808 parsing
                                    Boolean allowed = (ch <= 127) && (ch != ';') && ((sURLAllowedCharacters[ch] & allowedMask) != 0);
                                    if ( allowed ) {
                                        *outBytePtr++ = ch;
                                    }
                                    else {
                                        *outBytePtr++ = '%';
                                        *outBytePtr++ = hexchars[ch >> 4];
                                        *outBytePtr++ = hexchars[ch & 0x0f];
                                    }
                                }
                                else {
                                    if ( ch == '/' ) {
                                        pastSlash = true;
                                    }
                                    // !!!: percent encode ';' for backwards compatibility with API which uses rfc1808 parsing
                                    Boolean allowed = (ch <= 127) && (ch != ';') && (ch != ':') && ((sURLAllowedCharacters[ch] & allowedMask) != 0);
                                    if ( allowed ) {
                                        *outBytePtr++ = ch;
                                    }
                                    else {
                                        *outBytePtr++ = '%';
                                        *outBytePtr++ = hexchars[ch >> 4];
                                        *outBytePtr++ = hexchars[ch & 0x0f];
                                    }
                                }
                            }
                        }
                        else if ( allowedSet == kURLHostAllowedCharacterSet ) {
                            for ( idx = 0; idx < inLen; ++idx ) {
                                UInt8 ch = *inBytePtr++;
                                Boolean allowed = (ch <= 127) && ((sURLAllowedCharacters[ch] & allowedMask) != 0);
                                if ( allowed || (isIPLiteral && ch == ':') ) {  // the colon is allowed in IP-Literal
                                    *outBytePtr++ = ch;
                                }
                                else {
                                    *outBytePtr++ = '%';
                                    *outBytePtr++ = hexchars[ch >> 4];
                                    *outBytePtr++ = hexchars[ch & 0x0f];
                                }
                            }
                            if ( isIPLiteral ) {
                                // copy the close bracket
                                *outBytePtr++ = ']';
                            }
                        }
                        else {
                            for ( idx = 0; idx < inLen; ++idx ) {
                                UInt8 ch = *inBytePtr++;
                                Boolean allowed = (ch <= 127) && ((sURLAllowedCharacters[ch] & allowedMask) != 0);
                                if ( allowed ) {
                                    *outBytePtr++ = ch;
                                }
                                else {
                                    *outBytePtr++ = '%';
                                    *outBytePtr++ = hexchars[ch >> 4];
                                    *outBytePtr++ = hexchars[ch & 0x0f];
                                }
                            }
                        }
                    }
                    else {
                        // use the allowedCharacters NSCharacterSet to determine what characters are allowed
                        // non-ASCII characters are ignored
                        for ( idx = 0; idx < inLen; ++idx ) {
                            UInt8 ch = *inBytePtr++;
                            // CFCharacterSet
                            Boolean allowed = (ch <= 127) && CFCharacterSetIsCharacterMember((CFCharacterSetRef)allowedCharacters, ch);
                            if ( allowed ) {
                                *outBytePtr++ = ch;
                            }
                            else {
                                *outBytePtr++ = '%';
                                *outBytePtr++ = hexchars[ch >> 4];
                                *outBytePtr++ = hexchars[ch & 0x0f];
                            }
                        }
                    }
                    
                    result = CFStringCreateWithBytes(kCFAllocatorDefault, outBuf, outBytePtr - outBuf, kCFStringEncodingUTF8, false);
                }
                
                if ( inBuf != stackBuffer ) {
                    free(inBuf);
                }
            }
        }
    }
    return ( result );
}

CF_EXPORT CFStringRef _CFStringCreateByRemovingPercentEncoding(CFAllocatorRef alloc, CFStringRef string) {
    CFStringRef result = NULL;
    CFIndex strLength = CFStringGetLength(string);
    if ( strLength ) {
        CFIndex maxBufferSize = CFStringGetMaximumSizeForEncoding(strLength, kCFStringEncodingUTF8);
        // CFStringGetMaximumSizeForEncoding returns kCFNotFound if the result would be too big
        if ( maxBufferSize != kCFNotFound ) {
            enum {
                kStackBufferSize = 4096,
                kHalfStackBufferSize = kStackBufferSize / 2,
                
            };
            STACK_BUFFER_DECL(UInt8, stackBuffer, kStackBufferSize);
            UInt8 *encodedBuf = NULL;
            UInt8 *decodedBuf;
            // choose a buffer to put the percent-encoded bytes AND to percent-decode into
            if ( maxBufferSize <= kHalfStackBufferSize ) {
                encodedBuf = &stackBuffer[0];
            }
            else {
                // not big enough? malloc it.
                CFIndex mallocSize;
                if ( _CFMultiplyBufferSizeWithoutOverflow(maxBufferSize, 2, &mallocSize) ) {
                    if (mallocSize >= 0) {
                        encodedBuf = (UInt8 *)malloc(mallocSize);
                    }
                }
            }
            if ( encodedBuf ) {
                CFIndex charsConverted;
                CFIndex usedBufLen;
                // use the other half of the buffer for the percent-decoded bytes
                decodedBuf = &encodedBuf[maxBufferSize];
                charsConverted = CFStringGetBytes(string, CFRangeMake(0, strLength), kCFStringEncodingUTF8, 0, false, encodedBuf, maxBufferSize, &usedBufLen);
                if ( charsConverted ) {
                    // 0x80 marks invalid hex digits so this table can validate the digits while getting the values
                    static const UInt8 hexvalues[] = {
                        /* 00 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 08 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 10 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 18 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 20 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 28 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 30 */  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        /* 38 */  0x08, 0x09, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 40 */  0x80, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x80,
                        /* 48 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 50 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 58 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 60 */  0x80, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x80,
                        /* 68 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 70 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 78 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        
                        /* 80 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 88 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 90 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* 98 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* A0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* A8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* B0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* B8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* C0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* C8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* D0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* D8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* E0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* E8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* F0 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                        /* F8 */  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                    };
                    UInt8 *bufStartPtr;
                    UInt8 *bufPtr;
                    const UInt8 *bytePtr = encodedBuf;
                    CFIndex idx;
                    
                    bufPtr = bufStartPtr = decodedBuf;
                    Boolean conversionOK = TRUE;
                    
                    for ( idx = 0; (idx < usedBufLen) && conversionOK; ++idx ) {
                        switch ( *bytePtr ) {
                            case '%':
                                idx += 2;
                                if ( idx < usedBufLen ) {
                                    UInt8 hex1, hex2;
                                    // skip over %
                                    bytePtr++;
                                    // get the hex digits
                                    hex1 = hexvalues[*bytePtr++];
                                    hex2 = hexvalues[*bytePtr++];
                                    // validate them
                                    if ( ((hex1 | hex2) & 0x80) == 0 ) {
                                        // convert hex digits
                                        *bufPtr = (hex1 << 4) + hex2;
                                    }
                                    else {
                                        conversionOK = FALSE;
                                    }
                                }
                                else {
                                    conversionOK = FALSE;
                                }
                                break;
                            default:
                                // copy everything else
                                *bufPtr = *bytePtr++;
                                break;
                        }
                        ++bufPtr;
                    }
                    if ( conversionOK ) {
                        result = CFStringCreateWithBytes(kCFAllocatorDefault, decodedBuf, bufPtr - bufStartPtr, kCFStringEncodingUTF8, false);
                    }
                }
                
                // free the buffer if we malloc'd it
                if ( encodedBuf != &stackBuffer[0] ) {
                    free(encodedBuf);
                }
            }
        }
    }
    else {
        result = CFStringCreateCopy(alloc, string);
    }
    return ( result );
}


#if 0
#pragma mark -
#pragma mark URI Parser
#endif

/*
 *	Called by ParseURIReference to parse the authority component to find the
 *	userInfo, host and port. The results of the parse are returned in the fields
 *	of parseInfo.
 */
static inline void _ParseAuthority(CFStringInlineBuffer *buf, unsigned long authorityLocation, unsigned long authorityLength, struct _URIParseInfo *parseInfo)
{
    Boolean doneParsingComponent;
    Boolean isIPLiteral;
    unsigned long currentCharIndex;
    unsigned long endCharIndex;
    UniChar currentUniChar;
    unsigned long userinfoCharIndex;
    
    currentCharIndex = authorityLocation;
    endCharIndex = authorityLocation + authorityLength;
    
    currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
    
    //
    // find userinfo
    //
    userinfoCharIndex = currentCharIndex;
    doneParsingComponent = false;
    while ( !doneParsingComponent ) {
        if ( currentCharIndex != endCharIndex ) {
            if ( currentUniChar == '@' ) {
                // parse the userinfo
                unsigned long userinfoEndCharIndex = currentCharIndex;
                Boolean doneParsingUserInfo;
                UniChar userinfoUniChar;
                
                userinfoUniChar = CFStringGetCharacterFromInlineBuffer(buf, userinfoCharIndex);
                
                // there's always a userinfoName if there's userinfo... it just might be zero length
                parseInfo->userinfoNameExists = true;
                parseInfo->userinfoNameOffset = userinfoCharIndex;
                
                // find end of userinfoName and determine if there's a userinfoPassword
                doneParsingUserInfo = false;
                while ( !doneParsingUserInfo ) {
                    if ( userinfoCharIndex != userinfoEndCharIndex ) {
                        if ( userinfoUniChar == ':' ) {
                            parseInfo->userinfoPasswordExists = true;
                            parseInfo->userinfoPasswordOffset = userinfoCharIndex + 1;
                            doneParsingUserInfo = true;
                        }
                        else {
                            ++userinfoCharIndex;
                            userinfoUniChar = CFStringGetCharacterFromInlineBuffer(buf, userinfoCharIndex);
                        }
                    }
                    else {
                        doneParsingUserInfo = true;
                    }
                }
                
                // next character
                ++currentCharIndex;
                currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
                doneParsingComponent = true;
            }
            else {
                ++currentCharIndex;
                currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
            }
        }
        else {
            // there was no userinfo -- reset currentChar and result
            currentCharIndex = userinfoCharIndex;
            currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
            doneParsingComponent = true;
        }
    }
    
    //
    // find host
    //
    // there's always a host if there's an authority... it just might be zero length
    parseInfo->hostExists = true;
    parseInfo->hostOffset = currentCharIndex;
    
    isIPLiteral = ( currentUniChar == '[' );
    doneParsingComponent = false;
    while ( !doneParsingComponent ) {
        if ( currentCharIndex != endCharIndex ) {
            if ( isIPLiteral ) {
                if ( currentUniChar == ']' ) {
                    ++currentCharIndex;
                    currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
                    doneParsingComponent = true;
                }
                else {
                    ++currentCharIndex;
                    currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
                }
            }
            else {
                if ( currentUniChar == ':' ) {
                    doneParsingComponent = true;
                }
                else {
                    ++currentCharIndex;
                    currentUniChar = CFStringGetCharacterFromInlineBuffer(buf, currentCharIndex);
                }
            }
        }
        else {
            doneParsingComponent = true;
        }
    }
    
    //
    // find port
    //
    if ( (currentCharIndex != endCharIndex) && (currentUniChar == ':') ) {
        parseInfo->portExists = true;
        parseInfo->portOffset = currentCharIndex + 1;
        
        // no need to find the end of the port unless we're validating legal characters
    }
    // else no port
}


/*
 *	Parse the uriReference find the scheme, authority, path, query and fragment
 *	components and their subcomponents. The results of the parse are returned in
 *	the fields of parseInfo.
 */
CF_PRIVATE Boolean _CFURIParserParseURIReference(CFStringRef urlString, struct _URIParseInfo *parseInfo)
{
    Boolean doneParsingComponent;
    
    CFStringInlineBuffer buf;
    unsigned long currentCharIndex;
    unsigned long urlStringLength = CFStringGetLength(urlString);
    UniChar currentUniChar;
    
    // clear the parseInfo
    bzero(parseInfo, sizeof(*parseInfo));
    
    // Make sure the URL string isn't too long. We're limiting it to 2GB for backwards compatibility with 32-bit excutables using NS/CFURL
    if ( (urlStringLength > 0) && (urlStringLength <= INT_MAX) )
    {
        CFStringInitInlineBuffer(urlString, &buf, CFRangeMake(0, urlStringLength));
        
        parseInfo->endOffset = urlStringLength;
        
        //
        // find scheme
        //
        
        currentCharIndex = 0;
        currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
        
        // The first character of scheme has to be ALPHA so this is a quick outside the while loop check for that. In the switch statement below, the default case makes sure all characters are valid in a scheme, so only the lower bounds ('A') check is needed here to make sure the character is not a DIGIT.
        if ( currentUniChar >= 'A' ) {
            doneParsingComponent = false;
            while ( !doneParsingComponent ) {
                if ( currentUniChar == 0 ) {
                    doneParsingComponent = true;
                    // there was no scheme so this is a relative-ref -- reset currentChar and we're done looking for a scheme
                    currentCharIndex = 0;
                    currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                }
                else {
                    switch ( currentUniChar ) {
                        case ':':
                            // !!!: This checks to make sure the scheme is at least 1 character. However, it makes this parser completely different than CFURL's parser when the string starts with a ":" character.
                            if ( currentCharIndex > 0 ) {
                                parseInfo->schemeExists = true;
                                // the scheme's offset is always 0
                                ++currentCharIndex;
                                currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                                doneParsingComponent = true;
                            }
                            else {
                                // there were no valid scheme characters before the ':' -- reset currentChar and we're done looking for a scheme
                                currentCharIndex = 0;
                                currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                                doneParsingComponent = true;
                            }
                            break;
                            // !!!: These cases are commented out because default handles them. The scheme is validated as the URI string is parsed (unlike CFURL's parser).
                            //                    case '/':
                            //                    case '?':
                            //                    case '#':
                            //                        // there was no scheme so this is a relative-ref -- reset currentChar and we're done looking for a scheme
                            //                        currentCharIndex = 0;
                            //                        currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                            //                        doneParsingComponent = true;
                            //                        break;
                        default:
                            if ( (currentUniChar <= 127) && ((sURLAllowedCharacters[currentUniChar] & kURLSchemeAllowed) != 0) ) {
                                ++currentCharIndex;
                                currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                            }
                            else {
                                // invalid scheme characters  -- reset currentChar and we're done looking for a scheme
                                currentCharIndex = 0;
                                currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                                doneParsingComponent = true;
                            }
                            break;
                    }
                }
            }
        }
        
        //
        // find authority
        //
        if ( (currentUniChar == '/') && (CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex + 1) == '/') ) {
            unsigned long firstComponentCharIndex;
            unsigned long componentLength;
            
            currentCharIndex += 2;
            currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
            
            parseInfo->authorityExists = true;
            
            firstComponentCharIndex = currentCharIndex;
            
            doneParsingComponent = false;
            while ( !doneParsingComponent ) {
                if ( currentUniChar == 0 ) {
                    componentLength = currentCharIndex - firstComponentCharIndex;
                    doneParsingComponent = true;
                }
                else {
                    switch ( currentUniChar ) {
                        case '/':
                        case '?':
                        case '#':
                            componentLength = currentCharIndex - firstComponentCharIndex;
                            doneParsingComponent = true;
                            break;
                        default:
                            ++currentCharIndex;
                            currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                            break;
                    }
                }
            }
            _ParseAuthority(&buf, firstComponentCharIndex, componentLength, parseInfo);
        }
        // else no authority
        
        //
        // find path
        //
        
        // there's always a path... it just might be zero length
        parseInfo->pathOffset = currentCharIndex;
        
        doneParsingComponent = false;
        while ( !doneParsingComponent ) {
            if ( currentUniChar == 0 ) {
                doneParsingComponent = true;
            }
            else {
                switch ( currentUniChar ) {
                    case '?':
                    case '#':
                        doneParsingComponent = true;
                        break;
                    case ';':
                        // keep track of the obsolete param subcomponent
                        parseInfo->semicolonInPathExists = true;
                        // fall through to get next character
                    default:
                        ++currentCharIndex;
                        currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                        break;
                }
            }
        }
        
        //
        // find query
        //
        if ( currentUniChar == '?' ) {
            ++currentCharIndex;
            currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
            
            parseInfo->queryExists = true;
            parseInfo->queryOffset = currentCharIndex;
            
            doneParsingComponent = false;
            while ( !doneParsingComponent ) {
                if ( currentUniChar == 0 ) {
                    doneParsingComponent = true;
                }
                else {
                    switch ( currentUniChar ) {
                        case '#':
                            doneParsingComponent = true;
                            break;
                        default:
                            ++currentCharIndex;
                            currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                            break;
                    }
                }
            }
        }
        // else no query
        
        //
        // find fragment
        //
        if ( currentUniChar == '#' ) {
            ++currentCharIndex;
            currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
            
            parseInfo->fragmentExists = true;
            parseInfo->fragmentOffset = currentCharIndex;
            
            doneParsingComponent = false;
            while ( !doneParsingComponent ) {
                if ( currentUniChar == 0 ) {
                    doneParsingComponent = true;
                }
                else {
                    ++currentCharIndex;
                    currentUniChar = CFStringGetCharacterFromInlineBuffer(&buf, currentCharIndex);
                }
            }
        }
        // else no fragment
        
        return  ( true );
    }
    else {
        return  ( false );
    }
}


/*
 *	Returns the range of the scheme component.
 *
 *	If includeSeparators is true, the characters that separate the scheme
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetSchemeRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->schemeExists ) {
        if ( parseInfo->userinfoNameExists ) {
            // end is userinfoNameOffset minus the '://'
            result = CFRangeMake(0, parseInfo->userinfoNameOffset - (includeSeparators ? 0 : 3));
        }
        else if ( parseInfo->userinfoPasswordExists ) {
            // end is userinfoPasswordOffset minus the '://:'
            result = CFRangeMake(0, parseInfo->userinfoPasswordOffset - (includeSeparators ? 1 : 4));
        }
        else if ( parseInfo->hostExists ) {
            // end is hostOffset minus the '://'
            result = CFRangeMake(0, parseInfo->hostOffset - (includeSeparators ? 0 : 3));
        }
        else if ( parseInfo->portExists ) {
            // end is portOffset minus the '://:'
            result = CFRangeMake(0, parseInfo->portOffset - (includeSeparators ? 1 : 4));
        }
        else {
            if ( parseInfo->authorityExists ) {
                // end is pathOffset minus the '://'
                result = CFRangeMake(0, parseInfo->pathOffset - (includeSeparators ? 0 : 3));
            }
            else {
                // end is pathOffset minus the ':'
                result = CFRangeMake(0, parseInfo->pathOffset - (includeSeparators ? 0 : 1));
            }
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}


#if 0 // unused but might be needed in the future

/*
 *	Returns the range of the authority component.
 *
 *	If includeSeparators is true, the characters that separate the authority
 *	from other components/subcomponents are included.
 */
static CFRange _CFURIParserGetAuthorityRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->authorityExists ) {
        if ( parseInfo->userinfoNameExists ) {
            // authority begins at userinfoNameOffset
            // end is pathOffset
            result = CFRangeMake(parseInfo->userinfoNameOffset, parseInfo->pathOffset - parseInfo->userinfoNameOffset);
        }
        else {
            // authority begins at hostOffset
            // end is pathOffset
            result = CFRangeMake(parseInfo->hostOffset, parseInfo->pathOffset - parseInfo->hostOffset);
        }
        
        if ( includeSeparators ) {
            result.location -= 3;
            result.length += 3;
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}

#endif // unused but might be needed in the future


/*
 *	Returns the range of the userinfoName part of the userinfo sub-component of
 *	the authority component.
 *
 *	If includeSeparators is true, the characters that separate the userinfoName
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetUserinfoNameRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->userinfoNameExists ) {
        if ( parseInfo->userinfoPasswordExists ) {
            // end is userinfoPasswordOffset minus the ':'
            result = CFRangeMake(parseInfo->userinfoNameOffset, parseInfo->userinfoPasswordOffset - parseInfo->userinfoNameOffset - 1);
        }
        else if ( parseInfo->hostExists ) {
            // end is hostOffset minus the '@'
            result = CFRangeMake(parseInfo->userinfoNameOffset, parseInfo->hostOffset - parseInfo->userinfoNameOffset - 1);
        }
        else if ( parseInfo->portExists ) {
            // end is portOffset minus the '@:'
            result = CFRangeMake(parseInfo->userinfoNameOffset, parseInfo->portOffset - parseInfo->userinfoNameOffset - 2);
        }
        else {
            // end is pathOffset minus the '@'
            result = CFRangeMake(parseInfo->userinfoNameOffset, parseInfo->pathOffset - parseInfo->userinfoNameOffset - 1);
        }
        
        if ( includeSeparators ) {
            result.location -= 3;
            result.length += 4; // either ends at ':' password delimiter, or at '@' host delimiter
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}


/*
 *	Returns the range of the userinfoPassword part of the userinfo sub-component
 *	of the authority component.
 *
 *	If includeSeparators is true, the characters that separate the userinfoPassword
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetUserinfoPasswordRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->userinfoPasswordExists ) {
        if ( parseInfo->hostExists ) {
            // end is hostOffset minus the '@'
            result = CFRangeMake(parseInfo->userinfoPasswordOffset, parseInfo->hostOffset - parseInfo->userinfoPasswordOffset - 1);
        }
        else if ( parseInfo->portExists ) {
            // end is portOffset minus the '@:'
            result = CFRangeMake(parseInfo->userinfoPasswordOffset, parseInfo->portOffset - parseInfo->userinfoPasswordOffset - 2);
        }
        else {
            // end is pathOffset minus the '@'
            result = CFRangeMake(parseInfo->userinfoPasswordOffset, parseInfo->pathOffset - parseInfo->userinfoPasswordOffset - 1);
        }
        
        if ( includeSeparators ) {
            result.location--;
            result.length += 2;
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}


/*
 *	Returns the range of the host sub-component of the authority component.
 *
 *	If includeSeparators is true, the characters that separate the host
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetHostRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->hostExists ) {
        if ( parseInfo->portExists ) {
            // end is portOffset minus the ':'
            result = CFRangeMake(parseInfo->hostOffset, parseInfo->portOffset - parseInfo->hostOffset - 1);
        }
        else {
            // end is pathOffset
            result = CFRangeMake(parseInfo->hostOffset, parseInfo->pathOffset - parseInfo->hostOffset);
        }
        
        if ( includeSeparators ) {
            if ( parseInfo->userinfoNameExists ) {
                result.location--;
                result.length++;
            }
            else {
                result.location -= 3;
                result.length += 3;
            }
            if ( parseInfo->portExists ) {
                result.length++;
            }
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}


/*
 *	Returns the range of the port sub-component of the authority component.
 *
 *	If includeSeparators is true, the characters that separate the port
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetPortRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->portExists ) {
        // end is pathOffset
        result = CFRangeMake(parseInfo->portOffset, parseInfo->pathOffset - parseInfo->portOffset);
        if ( includeSeparators ) {
            result.location--;
            result.length++;
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}


/*
 *	Returns the range of the path component.
 *
 *	If includeSeparators is true, the characters that separate the path
 *	from other components/subcomponents are included.
 *
 *	If minusParam is false, the path component is the rfc3986 path. If minusParam
 *	is true, the path component ends at the first ';' character and the rest of
 *	the rfc3986 path after ';' is considered the obsolete rfc1808 param component.
 */
CF_PRIVATE CFRange _CFURIParserGetPathRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->queryExists ) {
        // end is queryOffset minus the '?'
        result = CFRangeMake(parseInfo->pathOffset, parseInfo->queryOffset - parseInfo->pathOffset - (includeSeparators ? 0 : 1));
    }
    else if ( parseInfo->fragmentExists ) {
        // end fragmentOffset is minus the '#'
        result = CFRangeMake(parseInfo->pathOffset, parseInfo->fragmentOffset - parseInfo->pathOffset - (includeSeparators ? 0 : 1));
    }
    else {
        // end is endOffset
        result = CFRangeMake(parseInfo->pathOffset, parseInfo->endOffset - parseInfo->pathOffset);
    }
    return ( result );
}


#if 0 // unused but might be needed in the future

/*
 *	Returns the range of the obsolete resource specifier component.
 *
 *	If includeSeparators is true, the characters that separate the resource specifier
 *	from other components/subcomponents are included.
 */
static CFRange _CFURIParserGetResourceSpecifierRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->queryExists ) {
        // start is queryOffset; end is endOffset
        result = CFRangeMake(parseInfo->queryOffset, parseInfo->endOffset - parseInfo->queryOffset);
    }
    else if ( parseInfo->fragmentExists ) {
        // start is fragmentOffset; end is endOffset
        result = CFRangeMake(parseInfo->fragmentOffset, parseInfo->endOffset - parseInfo->fragmentOffset);
    }
    else if ( !parseInfo->authorityExists ) {
        // start is pathOffset; end is endOffset
        result = CFRangeMake(parseInfo->pathOffset, parseInfo->endOffset - parseInfo->pathOffset);
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    
    if ( includeSeparators && (result.location != kCFNotFound) ) {
        result.location--;
        result.length++;
    }
    return ( result );
}

#endif // unused but might be needed in the future


/*
 *	Returns the range of the query component.
 *
 *	If includeSeparators is true, the characters that separate the query
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetQueryRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->queryExists ) {
        if ( parseInfo->fragmentExists ) {
            // end is fragmentOffset minus the '#'
            result = CFRangeMake(parseInfo->queryOffset, parseInfo->fragmentOffset - parseInfo->queryOffset - 1);
        }
        else {
            // end is endOffset
            result = CFRangeMake(parseInfo->queryOffset, parseInfo->endOffset - parseInfo->queryOffset);
        }
        
        if ( includeSeparators ) {
            result.location--;
            result.length += parseInfo->fragmentExists ? 2 : 1;
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}


/*
 *	Returns the range of the fragment component.
 *
 *	If includeSeparators is true, the characters that separate the fragment
 *	from other components/subcomponents are included.
 */
CF_PRIVATE CFRange _CFURIParserGetFragmentRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators)
{
    CFRange result;
    
    if ( parseInfo->fragmentExists ) {
        // end is endOffset
        result = CFRangeMake(parseInfo->fragmentOffset, parseInfo->endOffset - parseInfo->fragmentOffset);
        
        if ( includeSeparators ) {
            result.location--;
            result.length++;
        }
    }
    else {
        result = CFRangeMake(kCFNotFound, 0);
    }
    return ( result );
}

CF_PRIVATE Boolean _CFURIParserAlphaAllowed(UniChar ch) {
    return ((sURLAllowedCharacters[ch] & kURLAlphaAllowed) != 0);
}

CF_PRIVATE Boolean _CFURIParserValidateComponent(CFStringRef urlString, CFRange componentRange, _CFURIParserURLAllowedCharacter allowedMask, Boolean pctEncodedAllowed)
{
    Boolean result = true;
    CFStringInlineBuffer buf;
    unsigned long checkHexDigit = 0;
    if ( (componentRange.location != kCFNotFound) && (componentRange.length > 0) ) {
        // we've already checked componentRange.location for kCFNotFound and componentRange.length is always less than 2GB so this cast to a CFRange is safe
        CFStringInitInlineBuffer(urlString, &buf, *(CFRange*)&componentRange);
        for (CFIndex idx = 0; idx < componentRange.length; ++idx ) {
            UniChar ch = __CFStringGetCharacterFromInlineBufferQuick(&buf, idx);
            if ( !checkHexDigit ) {
                if ( pctEncodedAllowed && (ch == '%') ) {
                    // percent encoded? make sure there at least 2 characters left to check
                    if ( (idx + 2) < componentRange.length ) {
                        // the next 2 characters must be HEXDIG
                        checkHexDigit = 2;
                        continue;
                    }
                    else {
                        result = false;
                        break;
                    }
                }
                else if ( (ch > 127) || ((sURLAllowedCharacters[ch] & allowedMask) == 0) ) {
                    if ( (allowedMask == kURLHostAllowed) && (ch == ':') ) {
                        continue;
                    }
                    result = false;
                    break;
                }
            }
            else {
                if ( (ch <= 127) && ((sURLAllowedCharacters[ch] & kURLHexDigAllowed) != 0) ) {
                    --checkHexDigit;
                    continue;
                }
                else {
                    result = false;
                    break;
                }
            }
        }
    }
    return ( result );
}

CF_PRIVATE Boolean _CFURIParserURLStringIsValid(CFStringRef urlString, struct _URIParseInfo *parseInfo)
{
    Boolean result = true;
    CFRange componentRange;
    
    // scheme range is already valid from ParseURIReference()
    
    // validate the user
    componentRange = _CFURIParserGetUserinfoNameRange(parseInfo, false);
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLUserAllowed, true);
    if ( !result ) goto invalidComponent;
    
    // validate the password
    componentRange = _CFURIParserGetUserinfoPasswordRange(parseInfo, false);
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLPasswordAllowed, true);
    if ( !result ) goto invalidComponent;
    
    // validate the host
    componentRange = _CFURIParserGetHostRange(parseInfo, false);
    if ( (componentRange.location != kCFNotFound) && (componentRange.length >= 2) && (CFStringGetCharacterAtIndex(urlString, componentRange.location) == '[') && (CFStringGetCharacterAtIndex(urlString, componentRange.location + componentRange.length - 1) == ']') ) {
        // the host is an IP-Literal -- only validate the characters inside brackets
        ++componentRange.location;
        componentRange.length -= 2;
    }
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLHostAllowed, true);
    if ( !result ) goto invalidComponent;
    
    // validate the port
    componentRange = _CFURIParserGetPortRange(parseInfo, false);
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLPortAllowed, false);
    if ( !result ) goto invalidComponent;
    
    // validate the path
    componentRange = _CFURIParserGetPathRange(parseInfo, false);
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLPathAllowed, true);
    if ( !result ) goto invalidComponent;
    
    // validate the query
    componentRange = _CFURIParserGetQueryRange(parseInfo, false);
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLQueryAllowed, true);
    if ( !result ) goto invalidComponent;
    
    // validate the fragment
    componentRange = _CFURIParserGetFragmentRange(parseInfo, false);
    result = _CFURIParserValidateComponent(urlString, componentRange, kURLFragmentAllowed, true);
    
invalidComponent:
    return ( result );
}
