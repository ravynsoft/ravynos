#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include <stuff/bool.h>
#include <stuff/allocate.h>
#include <stuff/bytesex.h>
#include <stuff/port.h> /* cctools-port: reallocf() */
#include "dyld_bind_info.h"
#include "fixup-chains.h"

/* cctools-port start */
#undef swap16
#undef swap32
#undef swap64
/* cctools-port end */

/*****************************************************************************
 *
 *   LC_DYLD_INFO, LC_DYLD_INFO_ONLY
 *
 */

/* The entries of the ordinalTable for ThreadedRebaseBind. */
struct ThreadedBindData {
    const char* symbolName;
    int64_t addend;
    int libraryOrdinal;
    uint8_t flags;
    uint8_t type;
};

const char *
bindTypeName(
uint8_t type)
{
    switch(type){
        case BIND_TYPE_POINTER:
            return("pointer");
        case BIND_TYPE_TEXT_ABSOLUTE32:
            return("text abs32");
        case BIND_TYPE_TEXT_PCREL32:
            return("text rel32");
    }
    return("!!Unknown!!");
}
/*
static
const char *
ordinalName(
int libraryOrdinal,
const char **dylibs,
uint32_t ndylibs,
enum bool *libraryOrdinalSet)
{
    *libraryOrdinalSet = TRUE;
    switch(libraryOrdinal){
        case BIND_SPECIAL_DYLIB_SELF:
            return("this-image");
        case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
            return("main-executable");
        case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
            return("flat-namespace");
        case BIND_SPECIAL_DYLIB_WEAK_LOOKUP:
            return("weak");
    }
    if(libraryOrdinal < BIND_SPECIAL_DYLIB_WEAK_LOOKUP){
        *libraryOrdinalSet = FALSE;
        return("Unknown special ordinal");
    }
    if(libraryOrdinal > ndylibs){
        *libraryOrdinalSet = FALSE;
        return("LibraryOrdinal out of range");
    }
    return(dylibs[libraryOrdinal-1]);
}
*/
/*
 * validateOrdinal validates a dylibOrdinal value as read from the LC_DYLD_INFO
 * opcodes. If dylibOrdinal is valid, validateOrdinal will return TRUE and set
 * dylibName to either the name of a dylib or to a human-readable special name.
 * If dylibOrdinal is not valid, it will return FALSE and set error to a
 * human-readable error string. error may be NULL.
 */
static
enum bool
validateOrdinal(
int dylibOrdinal,
const char **dylibs,
uint32_t ndylibs,
const char** dylibName,
const char** error)
{
    *dylibName = NULL;
    if (error)
        *error = NULL;

    if (dylibOrdinal < BIND_SPECIAL_DYLIB_WEAK_LOOKUP){
        if (error)
            *error = "Unknown special ordinal";
        return FALSE;
    }
    if (dylibOrdinal > 0 && dylibOrdinal > ndylibs){
        if (error)
            *error = "dylibOrdinal out of range";
        return FALSE;
    }

    switch(dylibOrdinal){
        case BIND_SPECIAL_DYLIB_SELF:
            *dylibName = "this-image";
            break;
        case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
            *dylibName = "main-executable";
            break;
        case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
            *dylibName = "flat-namespace";
            break;
        case BIND_SPECIAL_DYLIB_WEAK_LOOKUP:
            *dylibName = "weak";
            break;
        default:
            *dylibName = dylibs[dylibOrdinal-1];
    }

    return TRUE;
}

/*
 * validateSegIndex validates a segIndex value as read from the LC_DYLD_INFO
 * opcodes. If segIndex is in range of the segs64 or segs arrays,
 * validateSegIndex will return TRUE. Otherwise, it will return FALSE and set
 * error to a human-readable error string.
 */
static
enum bool
validateSegIndex(
uint8_t segIndex,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64,
const char** error)
{
    *error = NULL;
    if(segs64 != NULL && segIndex >= nsegs64) {
        *error = "segment index out of range";
        return FALSE;
    }
    if(segs != NULL && segIndex >= nsegs) {
        *error = "segment index out of range";
        return FALSE;
    }
    return TRUE;
}

uint64_t
segStartAddress(
uint8_t segIndex,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64)
{
    if(segs != NULL){
        if(segIndex >= nsegs)
            return(0); /* throw "segment index out of range"; */
        return(segs[segIndex]->vmaddr);
    }
    else if(segs64 != NULL){
        if(segIndex >= nsegs64)
            return(0); /* throw "segment index out of range"; */
        return(segs64[segIndex]->vmaddr);
    }
    return(0);
}

const char *
segmentName(
uint8_t segIndex,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64)
{
    if(segs != NULL){
        if(segIndex >= nsegs)
            return("??"); /* throw "segment index out of range"; */
        return(segs[segIndex]->segname);
    }
    else if(segs64 != NULL){
        if(segIndex >= nsegs64)
            return("??"); /* throw "segment index out of range"; */
        return(segs64[segIndex]->segname);
    }
    return("??");
}

const char *
sectionName(
uint8_t segIndex,
uint64_t address,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64)
{
    struct section *s;
    struct section_64 *s64;
    uint32_t i;
    
    if(segs != NULL){
        if(segIndex >= nsegs)
            return("??"); /* throw "segment index out of range"; */
        
        s = (struct section *)((char *)segs[segIndex] +
                               sizeof(struct segment_command));
        for(i = 0; i < segs[segIndex]->nsects; i++){
            if(s->addr <= address && address < s->addr + s->size)
                return(s->sectname);
            s++;
        }
    }
    else if(segs64 != NULL){
        if(segIndex >= nsegs64)
            return("??"); /* throw "segment index out of range"; */
        
        s64 = (struct section_64 *)((char *)segs64[segIndex] +
                                    sizeof(struct segment_command_64));
        for(i = 0; i < segs64[segIndex]->nsects; i++){
            if(s64->addr <= address && address < s64->addr + s64->size)
                return(s64->sectname);
            s64++;
        }
    }
    return("??");
}

const char *
checkSegAndOffset(
int segIndex,
uint64_t segOffset,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64,
enum bool endInvalid)
{
    uint64_t address;
    
    if(segIndex == -1)
        return("missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB");
    if(segs != NULL){
        if(segIndex >= nsegs)
            return("bad segIndex (too large)");
        address = segs[segIndex]->vmaddr + segOffset;
        if(address > segs[segIndex]->vmaddr + segs[segIndex]->vmsize)
            return("bad segOffset, too large");
        if(endInvalid == TRUE &&
           address == segs[segIndex]->vmaddr + segs[segIndex]->vmsize)
            return("bad segOffset, too large");
    }
    else if(segs64 != NULL){
        if(segIndex >= nsegs64)
            return("bad segIndex (too large)");
        address = segs64[segIndex]->vmaddr + segOffset;
        if(address > segs64[segIndex]->vmaddr + segs64[segIndex]->vmsize)
            return("bad segOffset, too large");
        if(endInvalid == TRUE &&
           address == segs64[segIndex]->vmaddr + segs64[segIndex]->vmsize)
            return("bad segOffset, too large");
    }
    return(NULL);
}

const char *
checkCountAndSkip(
uint32_t *count,
uint64_t skip,
int segIndex,
uint64_t segOffset,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64)
{
    uint64_t address, i;
    
    i = 0;
    if(segs != NULL){
        if(segIndex >= nsegs){
            *count = 1;
            return("bad segIndex (too large)");
        }
        if(*count > 1)
            i = (skip + 4) * (*count - 1);
        address = segs[segIndex]->vmaddr + segOffset;
        if(address >= segs[segIndex]->vmaddr + segs[segIndex]->vmsize){
            *count = 1;
            return("bad segOffset, too large");
        }
        if(address + i >= segs[segIndex]->vmaddr + segs[segIndex]->vmsize){
            *count = 1;
            return("bad count and skip, too large");
        }
    }
    else if(segs64 != NULL){
        if(segIndex >= nsegs64){
            *count = 1;
            return("bad segIndex (too large)");
        }
        if(*count > 1)
            i = (skip + 8) * (*count - 1);
        address = segs64[segIndex]->vmaddr + segOffset;
        if(address >= segs64[segIndex]->vmaddr + segs64[segIndex]->vmsize){
            *count = 1;
            return("bad segOffset, too large");
        }
        if(address + i >=
           segs64[segIndex]->vmaddr + segs64[segIndex]->vmsize){
            *count = 1;
            return("bad count and skip, too large");
        }
    }
    return(NULL);
}

static
uint64_t
read_uleb128(
const uint8_t **pp,
const uint8_t* end,
const char **error)
{
    const uint8_t *p = *pp;
    uint64_t result = 0;
    int bit = 0;
    
    *error = NULL;
    do{
        if(p >= end){
            *pp = p;
            *error = "malformed uleb128, extends past opcode bytes";
            return(0);
        }
        
        uint64_t slice = *p & 0x7f;
        
        if(bit >= 64 || slice << bit >> bit != slice){
            *pp = p;
            *error = "uleb128 too big for uint64";
            return(0);
        }
        else {
            result |= (slice << bit);
            bit += 7;
        }
    }while(*p++ & 0x80);
    *pp = p;
    return(result);
}

static
int64_t
read_sleb128(
const uint8_t **pp,
const uint8_t* end,
const char **error)
{
    const uint8_t *p = *pp;
    int64_t result = 0;
    int bit = 0;
    uint8_t byte;
    
    *error = NULL;
    do{
        if(p == end){
            *pp = p;
            *error = "malformed sleb128, extends past opcode bytes";
            return(0);
        }
        byte = *p++;
        result |= (((int64_t)(byte & 0x7f)) << bit);
        bit += 7;
    }while (byte & 0x80);
    // sign extend negative numbers
    if((byte & 0x40) != 0)
        result |= (-1LL) << bit;
    *pp = p;
    return(result);
}

static inline
void
printErrorBind(
int pass,
enum bool verbose,
const char* opName,
unsigned long location,
const char* error,
uint32_t *errorCount)
{
    if (1 == pass && verbose && error) {
        printf("bad bind info (for %s opcode at 0x%lx) %s\n",
               opName, location, error);
    }
    errorCount++;
}

static inline
void
printErrorOrdinal(
int pass,
enum bool verbose,
const char* opName,
unsigned long location,
int32_t ordinal,
uint32_t ndylibs,
const char* error,
uint32_t *errorCount)
{
    if (1 == pass && verbose && error) {
        printf("bad bind info (for %s opcode at 0x%lx) bad library ordinal "
               "%u (max %u): %s\n",
               opName, location, ordinal, ndylibs, error);
    }
    errorCount +=1;
}

static inline
void
printErrorOrdAddr(
                  int pass,
                  enum bool verbose,
                  const char* opName,
                  unsigned long location,
                  int32_t ordinal,
                  uint64_t ordAddr,
                  uint32_t *errorCount)
{
    if (1 == pass && verbose) {
        printf("bad bind info (for %s opcode at 0x%lx) bad ordinal: %u in "
               "pointer at address 0x%llx\n",
               opName, location, ordinal, ordAddr);
    }
    errorCount +=1;
}

static inline
void
printErrorBindType(
int pass,
enum bool verbose,
const char* opName,
unsigned long location,
uint8_t bindType,
uint32_t *errorCount)
{
    if (1 == pass && verbose) {
        printf("bad bind info (for %s opcode at 0x%lx) "
               "bad bind type: %u\n",
               opName, location, bindType);
    }
    errorCount +=1;
}

static inline
void
printErrorOffset(
int pass,
enum bool verbose,
const char* opName,
unsigned long location,
uint64_t address,
int reason,
uint32_t *errorCount)
{
    if (1 == pass && verbose) {
        const char* error = "invalid";
        if (0 == reason)
            error = "past end of file";
        else if (1 == reason)
            error = "past end of the same page";
        printf("bad bind info (for %s opcode at: 0x%lx) offset to next pointer "
               "in the chain after one at address 0x%llx is %s\n",
               opName, location, address, error);
    }
    errorCount +=1;
}

static inline
void
printErrorOpcode(
int pass,
enum bool verbose,
const char* opName,
unsigned long location,
uint8_t opcode,
uint32_t *errorCount)
{
    if (1 == pass && verbose) {
        if (opName)
            printf("bad bind info (for %s opcode at 0x%lx) bad sub-opcode "
                   "value 0x%x\n",
                   opName, location, opcode);
        else
            printf("bad bind info (for opcode at 0x%lx) bad opcode "
                   "value 0x%x\n",
                   location, opcode);
    }
    errorCount++;
}
/*
 * get_dyld_bind_info() unpacks the dyld bind info from the data from an
 * LC_BIND_INFO load command pointed to by start through end into the internal
 * dyld_bind_info structs returned through dbi, and its count ndbi.  The array
 * of dylib names and their count are passed in in dylibs and ndylibs.  The
 * array of segments (either 32-bit with segs & nsegs or 64-bit segs64 & nsegs64)
 * are used to determine which sections the pointers are in.
 *
 * A word about libraryOrdinal. The libraryOrdinal is a combination of array
 * ordinals and special magic values:
 *
 *       -3     BIND_SPECIAL_DYLIB_WEAK_LOOKUP
 *       -2     BIND_SPECIAL_DYLIB_FLAT_LOOKUP
 *       -1     BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE
 *        0     BIND_SPECIAL_DYLIB_SELF
 *   [1..n]     library ordinal
 *
 * The libraryOrdinal is commonly used to index into a C-array (that is, in
 * the range [0, n-1]) of dylibs. Ordinarily, once libraryOrdinal is assigned
 * one should call ordinalName to validate the ordinal. ordinalName will either
 * set libraryOrdinalSet to TRUE and return a human-readable library or special
 * name, or it will set libraryOrdinalSet to FALSE and return a human-readable
 * error message. While all this sounds perfectly reasonable, there is a
 * potential for mischief here ...
 */
void
get_dyld_bind_info(
const uint8_t* start, /* inputs */
const uint8_t* end,
const char **dylibs,
uint32_t ndylibs,
struct segment_command **segs,
uint32_t nsegs,
struct segment_command_64 **segs64,
uint32_t nsegs64,
enum bool swapped,
char *object_addr,
uint64_t object_size,
struct dyld_bind_info **dbi, /* output */
uint64_t *ndbi,
enum chain_format_t *chain_format,
enum bool print_errors)
{
    const uint8_t *p, *opcode_start;
    uint8_t type;
    int segIndex;
    uint64_t segOffset;
    const char* symbolName;
    const char* fromDylib;
    enum bool libraryOrdinalSet;
    int libraryOrdinal;
    int64_t addend;
    int64_t flags;
    uint32_t count;
    uint32_t skip;
    uint64_t segStartAddr;
    const char* segName;
    const char* weak_import;
    enum bool done = FALSE;
    const char *sectName, *error;
#define MAXERRORCOUNT 20
    uint32_t pass, errorCount;
    uint64_t n;
    uint32_t sizeof_pointer;
    char *pointerLocation;
    uint64_t offset, ordinalTableCount, ordinalTableIndex, delta;
    struct ThreadedBindData *ordinalTable;
    uint16_t ordinal;
    uint64_t pointerAddress, pointerPageStart;
    const char* opName;
    
    *chain_format = CHAIN_FORMAT_NONE;
    *dbi = NULL;
    *ndbi = 0;

    if (start == NULL) {
        printf("missing dyld bind info.\n");
        return;
    }
    
    ordinalTable = NULL;
    ordinalTableCount = 0;
    sizeof_pointer = segs ? 4 : 8;
    errorCount = 0;
    n = 0;
    for(pass = 1; pass <= 2; pass++){
        p = start;
        type = 0;
        segIndex = -1;
        segOffset = 0;
        symbolName = NULL;
        fromDylib = NULL;
        libraryOrdinalSet = FALSE;
        libraryOrdinal = 0;
        addend = 0;
        segStartAddr = 0;
        segName = NULL;
        sectName = NULL;
        weak_import = "";
        done = FALSE;
        ordinalTableIndex = (uint64_t)-1;
        flags = 0;
        if(errorCount >= MAXERRORCOUNT){
            if(print_errors)
                printf("too many bind info errors, giving up.\n");
            *dbi = NULL;
            *ndbi = 0;
            if(ordinalTable != NULL)
                free(ordinalTable);
            return;
        }
        if(pass == 2){
            *dbi = (struct dyld_bind_info *)
            allocate(n * sizeof(struct dyld_bind_info));
            memset(*dbi, 0, n * sizeof(struct dyld_bind_info));
            *ndbi = n;
            n = 0;
        }
        while(!done && (p < end) && errorCount < MAXERRORCOUNT){
            error = NULL;
            opcode_start = p;
            uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
            uint8_t opcode = *p & BIND_OPCODE_MASK;
            opcode = *p & BIND_OPCODE_MASK;
            ++p;
            switch(opcode){
                case BIND_OPCODE_DONE:
                    done = TRUE;
                    break;
                case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                    /*
                     * consume: immediate
                     * produce: libraryOrdinal, libraryOrdinalSet, fromDylib
                     */
                    opName = "BIND_OPCODE_SET_DYLIB_ORDINAL_IMM";
                    libraryOrdinal = immediate;
                    libraryOrdinalSet = validateOrdinal(libraryOrdinal,
                                                        dylibs, ndylibs,
                                                        &fromDylib, &error);
                    if (FALSE == libraryOrdinalSet)
                        printErrorOrdinal(pass, print_errors, opName,
                                          opcode_start - start,
                                          libraryOrdinal, ndylibs,
                                          error, &errorCount);
                    break;
                case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                    /*
                     * consume: p, end
                     * produce: libraryOrdinal, libraryOrdinalSet, fromDylib,
                     *          error
                     */
                    opName = "BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB";
                    libraryOrdinal = (int)read_uleb128(&p, end, &error);
                    if (error) {
                        libraryOrdinalSet = FALSE;
                    }
                    else {
                        libraryOrdinalSet = validateOrdinal(libraryOrdinal,
                                                            dylibs, ndylibs,
                                                            &fromDylib, &error);
                    }
                    if (FALSE == libraryOrdinalSet)
                        printErrorOrdinal(pass, print_errors, opName,
                                          opcode_start - start,
                                          libraryOrdinal, ndylibs,
                                          error, &errorCount);
                    break;
                case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                    /*
                     * consume: immediate
                     * produce: libraryOrdinal, libraryOrdinalSet, fromDylib
                     *
                     * the special ordinals are in the range of [-3, 0]
                     */
                    opName = "BIND_OPCODE_SET_DYLIB_SPECIAL_IMM";
                    if (immediate == 0) {
                        libraryOrdinal = 0;
                    }
                    else {
                        int8_t signExtended = BIND_OPCODE_MASK | immediate;
                        libraryOrdinal = signExtended;
                    }
                    libraryOrdinalSet = validateOrdinal(libraryOrdinal,
                                                        dylibs, ndylibs,
                                                        &fromDylib, &error);
                    if (FALSE == libraryOrdinalSet)
                        printErrorOrdinal(pass, print_errors, opName,
                                          opcode_start - start,
                                          libraryOrdinal, ndylibs,
                                          error, &errorCount);
                    break;
                case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                    /*
                     * consume: p, end, immediate
                     * produce: p, symbolName, flags, weak_import
                     */
                    opName = "BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM";
                    weak_import = "";
                    symbolName = (char*)p;
                    while(*p != '\0' && (p < end))
                        ++p;
                    if(p == end){
                        error = "symbol name extends past opcodes";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        /*
                         * Even though the name does not end with a '\0' it will
                         * not be used as it is past the opcodes so there can't
                         * be a BIND opcode that follows that will use it.
                         */
                    }
                    else {
                        /* string terminator */
                        ++p;
                    }
                    flags = immediate;
                    if((flags & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0)
                        weak_import = " (weak import)";
                    break;
                case BIND_OPCODE_SET_TYPE_IMM:
                    /*
                     * consume: immediate
                     * produce: type
                     */
                    opName = "BIND_OPCODE_SET_TYPE_IMM";
                    if (immediate == 0 || immediate > BIND_TYPE_TEXT_PCREL32) {
                        printErrorBindType(pass, print_errors, opName,
                                           opcode_start - start,
                                           immediate, &errorCount);
                        type = 0;
                    }
                    else {
                        type = immediate;
                    }
                    break;
                case BIND_OPCODE_SET_ADDEND_SLEB:
                    /*
                     * consume: p, end
                     * produce: addend, error
                     */
                    opName = "BIND_OPCODE_SET_ADDEND_SLEB";
                    addend = read_sleb128(&p, end, &error);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        addend = 0;
                    }
                    break;
                case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                    /*
                     * consume: immediate
                     * produce: segIndex, segStartAddr, segName, segOffset,
                     *          error
                     */
                    opName = "BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB";
                    if (validateSegIndex(immediate, segs, nsegs,
                                         segs64, nsegs64, &error))
                    {
                        segOffset = read_uleb128(&p, end, &error);
                    }
                    if (!error) {
                        segIndex = immediate;
                        segStartAddr = segStartAddress(segIndex, segs, nsegs,
                                                       segs64, nsegs64);
                        segName = segmentName(segIndex, segs, nsegs,
                                              segs64, nsegs64);
                        error = checkSegAndOffset(segIndex, segOffset,
                                                  segs, nsegs,
                                                  segs64, nsegs64, TRUE);
                    }
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segStartAddr = 0;
                        segName = NULL;
                        segOffset = 0;
                        error = NULL;
                    }
                    break;
                case BIND_OPCODE_ADD_ADDR_ULEB:
                    /*
                     * consume: p, end, segOffset
                     * produce: segOffset, error
                     */
                    opName = "BIND_OPCODE_ADD_ADDR_ULEB";
                    error = NULL;
                    if (-1 == segIndex) {
                        error = "segment index is invalid";
                    }
                    if (!error) {
                        segOffset += read_uleb128(&p, end, &error);
                    }
                    if (!error) {
                        error = checkSegAndOffset(segIndex, segOffset,
                                                  segs, nsegs,
                                                  segs64, nsegs64, TRUE);
                    }
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        error = NULL;
                    }
                    break;
                case BIND_OPCODE_DO_BIND:
                    /*
                     * consume: segIndex, segOffset, symbolName,
                     *          libraryOrdinalSet, ordinalTableIndex,
                     *          ordinalTableCount, ordinalTable, libraryOrdinal,
                     *          addend, flags, type, segName, segStartAddr,
                     *          fromDylib, weak_import
                     * produce: sectName, segOffset, dbi
                     */
                    opName = "BIND_OPCODE_DO_BIND";
                    /* start by doing all of the validations */
                    if(CHAIN_FORMAT_NONE == *chain_format)
                    {
                        error = checkSegAndOffset(segIndex, segOffset, segs,
                                                  nsegs, segs64, nsegs64, TRUE);
                        if (error) {
                            printErrorBind(pass, print_errors, opName,
                                           opcode_start-start, error,
                                           &errorCount);
                            segIndex = -1;
                            segName = NULL;
                            segStartAddr = 0;
                            segOffset = 0;
                            sectName = NULL;
                        }
                        else {
                            sectName = sectionName(segIndex,
                                                   segStartAddr + segOffset,
                                                   segs, nsegs,
                                                   segs64, nsegs64);
                        }
                    }
                    if (NULL == symbolName) {
                        error = "missing preceding BIND_OPCODE_SET_SYMBOL_"
                            "TRAILING_FLAGS_IMM opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if (FALSE == libraryOrdinalSet){
                        error = "missing preceding BIND_OPCODE_SET_DYLIB_"
                            "ORDINAL_* opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if(CHAIN_FORMAT_ARM64E == *chain_format){
                        /*
                         * At this point ordinalTableIndex should not equal
                         * ordinalTableCount or we have seen too many
                         * BIND_OPCODE_DO_BIND opcodes and that does not match
                         * the ordinalTableCount.
                         */
                        if(ordinalTableIndex >= ordinalTableCount){
                            error = "incorrect ordinal table size: number of "
                            "BIND_OPCODE_DO_BIND opcodes exceed the count in "
                            "previous BIND_SUBOPCODE_THREADED_SET_BIND_"
                            "ORDINAL_TABLE_SIZE_ULEB opcode";
                            printErrorBind(pass, print_errors, opName,
                                           opcode_start-start, error,
                                           &errorCount);
                        }
                        
                        /*
                         * The ordinalTable needs to be built regardless of
                         * success or failure.
                         */
                        const uint64_t oti = ordinalTableIndex++;
                        ordinalTable[oti].symbolName = symbolName;
                        ordinalTable[oti].addend = addend;
                        ordinalTable[oti].libraryOrdinal
                        = libraryOrdinalSet ? libraryOrdinal : 0;
                        ordinalTable[oti].flags = flags;
                        ordinalTable[oti].type = type;
                    }
                    else
                    {
                        if(pass == 2){
                            char* segname = strdup(segName ? segName : "??");
                            char* sectname = strdup(sectName ? sectName : "??");
                            if (strlen(segname)>16)
                                segname[16] = 0;
                            if (strlen(sectname)>16)
                                sectname[16] = 0;
                            (*dbi)[n].segname = segname;
                            (*dbi)[n].sectname = sectname;
                            (*dbi)[n].address = segStartAddr + segOffset;
                            (*dbi)[n].bind_type = type;
                            (*dbi)[n].addend = addend;
                            (*dbi)[n].dylibname = fromDylib ? fromDylib : "??";
                            (*dbi)[n].symbolname = (symbolName ? symbolName :
                                                    "Symbol name not set");
                            (*dbi)[n].weak_import = *weak_import != '\0';
                            (*dbi)[n].pointer_value = 0;
                        }
                        n++;
                        segOffset += sizeof_pointer;
                    }
                    break;
                case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                    /*
                     * consume: segIndex, segOffset, symbolName,
                     *          libraryOrdinalSet, ordinalTableIndex,
                     *          ordinalTableCount, ordinalTable, libraryOrdinal,
                     *          addend, flags, type, segName, segStartAddr,
                     *          fromDylib, weak_import
                     * produce: sectName, segOffset, dbi
                     */
                    opName = "BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB";
                    error = checkSegAndOffset(segIndex, segOffset, segs, nsegs,
                                              segs64, nsegs64, TRUE);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    else {
                        sectName = sectionName(segIndex, segStartAddr+segOffset,
                                               segs, nsegs, segs64, nsegs64);
                    }
                    if (NULL == symbolName) {
                        error = "missing preceding BIND_OPCODE_SET_SYMBOL_"
                        "TRAILING_FLAGS_IMM opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if (FALSE == libraryOrdinalSet){
                        error = "missing preceding BIND_OPCODE_SET_DYLIB_"
                        "ORDINAL_* opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if(2 == pass){
                        char* segname = strdup(segName ? segName : "??");
                        char* sectname = strdup(sectName ? sectName : "??");
                        if (strlen(segname)>16)
                            segname[16] = 0;
                        if (strlen(sectname)>16)
                            sectname[16] = 0;
                        (*dbi)[n].segname = segname;
                        (*dbi)[n].sectname = sectname;
                        (*dbi)[n].address = segStartAddr+segOffset;
                        (*dbi)[n].bind_type = type;
                        (*dbi)[n].addend = addend;
                        (*dbi)[n].dylibname = fromDylib ? fromDylib : "??";
                        (*dbi)[n].symbolname = (symbolName ? symbolName :
                                                "Symbol name not set");
                        (*dbi)[n].weak_import = *weak_import != '\0';
                        (*dbi)[n].pointer_value = 0;
                    }
                    n++;
                    segOffset += read_uleb128(&p, end, &error) + sizeof_pointer;
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    /*
                     * Note, this is not really an error until the next bind
                     * but make so sense for a BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB
                     * to not be followed by another bind operation.
                     */
                    error = checkSegAndOffset(segIndex, segOffset, segs, nsegs,
                                              segs64, nsegs64, FALSE);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    break;
                case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                    /*
                     * consume: segIndex, segOffset, symbolName,
                     *          libraryOrdinalSet, ordinalTableIndex,
                     *          ordinalTableCount, ordinalTable, libraryOrdinal,
                     *          addend, flags, type, segName, segStartAddr,
                     *          fromDylib, weak_import
                     * produce: sectName, segOffset, dbi
                     */
                    opName = "BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED";
                    error = checkSegAndOffset(segIndex, segOffset, segs, nsegs,
                                              segs64, nsegs64, TRUE);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    else {
                        sectName = sectionName(segIndex, segStartAddr+segOffset,
                                               segs, nsegs, segs64, nsegs64);
                    }
                    if (NULL == symbolName) {
                        error = "missing preceding BIND_OPCODE_SET_SYMBOL_"
                        "TRAILING_FLAGS_IMM opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if (FALSE == libraryOrdinalSet){
                        error = "missing preceding BIND_OPCODE_SET_DYLIB_"
                        "ORDINAL_* opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if(2 == pass){
                        char* segname = strdup(segName ? segName : "??");
                        char* sectname = strdup(sectName ? sectName : "??");
                        if (strlen(segname)>16)
                            segname[16] = 0;
                        if (strlen(sectname)>16)
                            sectname[16] = 0;
                        (*dbi)[n].segname = segname;
                        (*dbi)[n].sectname = sectname;
                        (*dbi)[n].address = segStartAddr+segOffset;
                        (*dbi)[n].bind_type = type;
                        (*dbi)[n].addend = addend;
                        (*dbi)[n].dylibname = fromDylib ? fromDylib : "??";
                        (*dbi)[n].symbolname = (symbolName ? symbolName :
                                                "Symbol name not set");
                        (*dbi)[n].weak_import = *weak_import != '\0';
                        (*dbi)[n].pointer_value = 0;
                    }
                    n++;
                    segOffset += immediate * sizeof_pointer + sizeof_pointer;
                    /*
                     * Note, this is not really an error until the next bind
                     * but make so sense for a BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB
                     * to not be followed by another bind operation.
                     */
                    error = checkSegAndOffset(segIndex, segOffset, segs, nsegs,
                                              segs64, nsegs64, FALSE);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    break;
                case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                    /*
                     * consume: p, end, segIndex, segOffset,symbolName,
                     *          libraryOrdinalSet, fromDylib,
                     * produce: count, skip, sectName
                     */
                    opName = "BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB";
                    count = (uint32_t)read_uleb128(&p, end, &error);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        count = 0;
                    }
                    skip = (uint32_t)read_uleb128(&p, end, &error);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        skip = 0;
                    }
                    error = checkSegAndOffset(segIndex, segOffset, segs, nsegs,
                                              segs64, nsegs64, TRUE);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    else {
                        sectName = sectionName(segIndex, segStartAddr + segOffset,
                                               segs, nsegs, segs64, nsegs64);
                    }
                    if (NULL == symbolName) {
                        error = "missing preceding BIND_OPCODE_SET_SYMBOL_"
                        "TRAILING_FLAGS_IMM opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    if (FALSE == libraryOrdinalSet){
                        error = "missing preceding BIND_OPCODE_SET_DYLIB_"
                        "ORDINAL_* opcode";
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                    }
                    error = checkCountAndSkip(&count, skip, segIndex,
                                              segOffset, segs, nsegs,
                                              segs64, nsegs64);
                    if (error) {
                        printErrorBind(pass, print_errors, opName,
                                       opcode_start-start, error, &errorCount);
                        segIndex = -1;
                        segName = NULL;
                        segStartAddr = 0;
                        segOffset = 0;
                        sectName = NULL;
                    }
                    for (uint32_t i=0; i < count; ++i) {
                        if(2 == pass){
                            char* segname = strdup(segName ? segName : "??");
                            char* sectname = strdup(sectName ? sectName : "??");
                            if (strlen(segname)>16)
                                segname[16] = 0;
                            if (strlen(sectname)>16)
                                sectname[16] = 0;
                            (*dbi)[n].segname = segname;
                            (*dbi)[n].sectname = sectname;
                            (*dbi)[n].address = segStartAddr+segOffset;
                            (*dbi)[n].bind_type = type;
                            (*dbi)[n].addend = addend;
                            (*dbi)[n].dylibname = fromDylib ? fromDylib : "??";
                            (*dbi)[n].symbolname = (symbolName ? symbolName :
                                                    "Symbol name not set");
                            (*dbi)[n].weak_import = *weak_import != '\0';
                            (*dbi)[n].pointer_value = 0;
                        }
                        n++;
                        if (-1 != segIndex)
                            segOffset += skip + sizeof_pointer;
                    }
                    break;
                case BIND_OPCODE_THREADED:
                    /* Note the immediate is a sub opcode */
                    /*
                     * [MDT: Giving up on 80 column formatting here... ]
                     */
                    switch(immediate){
                        case BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
                            /*
                             * consume: p, end
                             * produce: ordinalTableCount, ordinalTableIndex,
                             *          ordinalTable, chain_format
                             */
                            opName = "BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB";
                            ordinalTableCount = read_uleb128(&p, end, &error);
                            if (error) {
                                printErrorBind(pass, print_errors, opName, opcode_start - start, error, &errorCount);
                                ordinalTableCount = 0;
                                ordinalTableIndex = (uint64_t)-1;
                            }
                            else {
                                ordinalTableIndex = 0;
                            }
                            ordinalTable = reallocate(ordinalTable, sizeof(struct ThreadedBindData) * ordinalTableCount);
                            *chain_format = CHAIN_FORMAT_ARM64E;
                            break;
                        case BIND_SUBOPCODE_THREADED_APPLY:
                            /*
                             * consume: ordinalTableIndex, ordinalTableCount,
                             *          ordinalTable, segIndex, segOffset,
                             *          segAddr,
                             * produce: sectName, offset, pointerAddress,
                             *          pointerPageStart, pointerLocation,
                             *          delta, ordinal,
                             */
                            opName = "BIND_SUBOPCODE_THREADED_APPLY";
                            /*
                             * At this point ordinalTableIndex should equal
                             * ordinalTableCount or we have a mismatch between
                             * BIND_OPCODE_DO_BIND and ordinalTableCount.
                             */
                            if(ordinalTableIndex != ordinalTableCount){
                                error = ("incorrect ordinal table size: count of previous "
                                         "BIND_OPCODE_DO_BIND opcodes don't match count in previous "
                                         "BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB opcodes");
                                printErrorBind(pass, print_errors, opName, opcode_start - start, error, &errorCount);
                            }
                            /*
                             * We check for segOffset + 8 as we need to read a
                             * 64-bit pointer.
                             */
                            error = checkSegAndOffset(segIndex, segOffset, segs, nsegs, segs64, nsegs64, FALSE);
                            if (error) {
                                printErrorBind(pass, print_errors, opName, opcode_start-start, error, &errorCount);
                                segIndex = -1;
                                segName = NULL;
                                segStartAddr = 0;
                                segOffset = 0;
                                sectName = NULL;
                            }
                            else {
                                sectName = sectionName(segIndex, segStartAddr + segOffset, segs, nsegs, segs64, nsegs64);
                            }
                            /*
                             * Check segStartAddr + segOffset is 8-byte aligned.
                             */
                            if(((segStartAddr + segOffset) & 0x3) != 0){
                                error = "bad segOffset, not 8-byte aligned";
                                printErrorBind(pass, print_errors, opName, opcode_start - start, error, &errorCount);
                            }
                            /*
                             * This is a start a new thread of Rebase/Bind
                             * pointer chain from the previously set segIndex
                             * and segOffset. Record these locations even if
                             * misaligned. Do not record these locations if
                             * segIndex is invalid.
                             */
                            if (-1 != segIndex) {
                                offset = segs64[segIndex]->fileoff + segOffset;
                                pointerAddress = segs64[segIndex]->vmaddr + segOffset;
                                pointerPageStart = pointerAddress & ~0x3fff;
                                delta = 0;
                                do{
                                    uint64_t value;
                                    enum bool isRebase;
                                    pointerLocation = object_addr + offset;
                                    value = *(uint64_t *)pointerLocation;
                                    if(swapped)
                                        value = SWAP_LONG_LONG(value);
                                    isRebase = (value & (1ULL << 62)) == 0;
                                    if(isRebase){
                                        /* not doing anything with Rebase,
                                         only bind so no code here. */
                                        ;
                                    } else {
                                        /* the ordinal is bits are [0..15] */
                                        ordinal = value & 0xFFFF;
                                        if(ordinal >= ordinalTableCount){
                                            printErrorOrdAddr(pass, print_errors, opName, opcode_start - start,
                                                              ordinal, pointerAddress, &errorCount);
                                            break;
                                        }
                                        libraryOrdinal = ordinalTable[ordinal].libraryOrdinal;
                                        if (!validateOrdinal(libraryOrdinal, dylibs, ndylibs, &fromDylib, &error)) {
                                            printErrorOrdinal(pass, print_errors, opName, opcode_start - start, libraryOrdinal, ndylibs, error, &errorCount);
                                            break;
                                        }
                                        flags = ordinalTable[ordinal].flags;
                                        if((flags & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0)
                                            weak_import = " (weak import)";
                                        else
                                            weak_import = "";
                                        if(pass == 2){
                                            char* segname = strdup(segName ? segName : "??");
                                            char* sectname = strdup(sectName ? sectName : "??");
                                            if (strlen(segname)>16)
                                                segname[16] = 0;
                                            if (strlen(sectname)>16)
                                                sectname[16] = 0;
                                            (*dbi)[n].segname = segname;
                                            (*dbi)[n].sectname = sectname;
                                            (*dbi)[n].address = segStartAddr+segOffset;
                                            (*dbi)[n].bind_type = ordinalTable[ordinal].type;
                                            (*dbi)[n].addend = ordinalTable[ordinal].addend;
                                            (*dbi)[n].dylibname = fromDylib;
                                            (*dbi)[n].symbolname = ordinalTable[ordinal].symbolName;
                                            (*dbi)[n].weak_import = *weak_import != '\0';
                                            (*dbi)[n].pointer_value = value;
                                        }
                                        n++;
                                    }
                                    
                                    /*
                                     * Now on to the next pointer in the chain if there
                                     * is one.
                                     */
                                    /* The delta is bits [51..61] */
                                    /* And bit 62 is to tell us if we are a rebase (0)
                                     or bind (1) */
                                    value &= ~(1ULL << 62);
                                    delta = (value & 0x3FF8000000000000) >> 51;
                                    /*
                                     * If the delta is zero there is no next pointer so
                                     * don't check the offset to the next pointer.
                                     */
                                    if(delta == 0)
                                        break;
                                    segOffset += delta * 8; /* sizeof(pint_t); */
                                    /*
                                     * Want to check that the segOffset plus 8 is not
                                     * past the end of this file and on the same page
                                     * in this segment so we can get the next pointer
                                     * in this thread.
                                     */
                                    offset = segs64[segIndex]->fileoff + segOffset;
                                    pointerAddress = segs64[segIndex]->vmaddr +
                                    segOffset;
                                    if(offset + 8 > object_size){
                                        printErrorOffset(pass, print_errors, opName, opcode_start - start,
                                                         pointerAddress, 0, &errorCount);
                                        break;
                                    }
                                    if(pointerPageStart != (pointerAddress & ~0x3fff)){
                                        printErrorOffset(pass, print_errors, opName, opcode_start - start,
                                                         pointerAddress, 1, &errorCount);
                                        break;
                                    }
                                } while(delta != 0);
                            }
                            break;
                        default:
                            printErrorOpcode(pass, print_errors, opName,
                                             opcode_start - start, immediate,
                                             &errorCount);
                            done = TRUE;
                            break;
                    }
                    break;
                default:
                    printErrorOpcode(pass, print_errors, NULL,
                                     opcode_start - start, opcode, &errorCount);
                    done = TRUE;
                    break;
            }
        }
    }
    if(ordinalTable != NULL)
        free(ordinalTable);
}

/*
 * print_dyld_rebase_opcodes() prints the raw contents of the DYLD_INFO rebase
 * data. Its purpose is to "disassemble" the contents of the opcode stream,
 * not to interpret the data. It is resilient to errors in the data, making it
 * useful for debugging malformed binaries.
 *
 * opcodes are printed in a concise table of data:
 *
 *   index, file offset, raw byte, symbolic opcode and opcode-specific data
 *
 * this format differs from dyldinfo(1) slightly, in that the offset is
 * relative to the start of mach_header, rather than to the start of the
 * opcode stream. Meaning, this output can be compared directly with commands
 * such as hexdump(1).
 */
extern
void
print_dyld_rebase_opcodes(
const uint8_t* object_addr,
uint64_t object_size,
uint32_t offset,
uint32_t size)
{
    if (offset > object_size) {
        printf("rebase offset %u beyond the size of file %llu\n",
               offset, object_size);
        return;
    }
    if (offset + size > object_size) {
        printf("rebase offset + size %u beyond the size of file %llu\n",
               offset + size, object_size);
        size = (uint32_t)(offset - object_size);
    }

    const uint8_t* p = object_addr + offset;
    const uint8_t* end = object_addr + offset + size;
    enum bool done = FALSE;
    uint32_t idx = 0;

    printf("rebase opcodes:\n");
    printf("  %4s %-10s %4s %s\n", "idx", "offset", "byte", "opcode");

    while (!done)
    {
        uint32_t poffset = (uint32_t)(p - object_addr);
        idx++;

        if (poffset >= object_size) {
            printf("rebase opcode #%u offset %u beyond the size of file %llu\n",
                   idx, poffset, object_size);
            break;
        }
        if (p >= end) {
            printf("%s opcode #%u offset %u beyond the size of opcodes %u\n",
                   "rebase", idx, poffset, size);
            break;
        }

        uint8_t byte = *p;
        uint8_t immediate = byte & REBASE_IMMEDIATE_MASK;
        uint8_t opcode = byte & REBASE_OPCODE_MASK;
        ++p;

        const char* name = NULL;
        const char* error = NULL;
        uint64_t uleb = 0;

        switch (opcode) {
                /*
                 * codes with IMM
                 */
            case REBASE_OPCODE_SET_TYPE_IMM:
                if (!name)
                    name = "REBASE_OPCODE_SET_TYPE_IMM";
                /* FALLTHROUGH */
            case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                if (!name)
                    name = "REBASE_OPCODE_ADD_ADDR_IMM_SCALED";
                /* FALLTHROUGH */
            case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                if (!name)
                    name = "REBASE_OPCODE_DO_REBASE_IMM_TIMES";
                printf("  %4u 0x%08x 0x%02x %s(%d)\n",
                       idx, poffset, byte, name, immediate);
                break;
                /*
                 * codes with ULEB
                 */
            case REBASE_OPCODE_ADD_ADDR_ULEB:
                if (!name)
                    name = "REBASE_OPCODE_ADD_ADDR_ULEB";
                /* FALLTHROUGH */
            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                if (!name)
                    name = "REBASE_OPCODE_DO_REBASE_ULEB_TIMES";
                /* FALLTHROUGH */
            case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                if (!name)
                    name = "REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB";
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("  %4u 0x%08x 0x%02x %s(error: %s)\n",
                           idx, poffset, byte, name, error);
                else
                    printf("  %4u 0x%08x 0x%02x %s(0x%llx)\n",
                           idx, poffset, byte, name, uleb);
                break;
                /*
                 * codes with IMM and ULEB
                 */
            case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                name = "REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB";
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("  %4u 0x%08x 0x%02x %s(%d, error: %s)\n",
                           idx, poffset, byte, name, immediate, error);
                else
                    printf("  %4u 0x%08x 0x%02x %s(%d, 0x%llx)\n",
                           idx, poffset, byte, name, immediate, uleb);
                break;
                /*
                 * codes with ULEB and ULEB
                 */
            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                name = "REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB";
                printf("  %4u 0x%08x 0x%02x %s(", idx, poffset, byte, name);
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("error: %s, ", error);
                else
                    printf("0x%llx, ", uleb);
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("error: %s)\n", error);
                else
                    printf("0x%llx)\n", uleb);
                break;
                /*
                 * codes with no arguments
                 */
            case REBASE_OPCODE_DONE:
                name = "REBASE_OPCODE_DONE";
                done = TRUE;
                /* FALLTHROUGH */
            default:
                if (!name)
                    name = "UNKNOWN OPCODE";
                printf("  %4u 0x%08x 0x%02x %s\n", idx, poffset, byte, name);
        }
    }
}

/*
 * print_dyld_bind_opcodes() prints the raw contents of DYLD_INFO bind data.
 * Its purpose is to "disassemble" the contents of the opcode stream, not to
 * interpret the data. It is resilient to errors in the data, making it useful
 * for debugging malformed binaries.
 *
 * opcodes are printed in a concise table of data:
 *
 *   index, file offset, raw byte, symbolic opcode and opcode-specific data
 *
 * this format differs from dyldinfo(1) slightly, in that the offset is
 * relative to the start of mach_header, rather than to the start of the
 * opcode stream. Meaning, this output can be compared directly with commands
 * such as hexdump(1).
 */
extern
void
print_dyld_bind_opcodes(
const uint8_t* object_addr,
uint64_t object_size,
const char* type,
uint32_t offset,
uint32_t size)
{
    if (offset > object_size) {
        printf("%s offset %u beyond the size of file %llu\n",
               type, offset, object_size);
        return;
    }
    if (offset + size > object_size) {
        printf("%s offset + size %u beyond the size of file %llu\n",
               type, offset + size, object_size);
        size = (uint32_t)(offset - object_size);
    }

    const uint8_t* p = object_addr + offset;
    const uint8_t* end = object_addr + offset + size;
    enum bool done = FALSE;
    uint32_t idx = 0;

    printf("%s opcodes:\n", type);
    printf("  %4s %-10s %4s %s\n", "idx", "offset", "byte", "opcode");

    while (!done)
    {
        uint32_t poffset = (uint32_t)(p - object_addr);
        idx++;

        if (poffset >= object_size) {
            printf("%s opcode #%u offset %u beyond the size of file %llu\n",
                   type, idx, poffset, object_size);
            break;
        }
        if (p >= end) {
            printf("%s opcode #%u offset %u beyond the size of opcodes %u\n",
                   type, idx, poffset, size);
            break;
        }

        uint8_t byte = *p;
        uint8_t immediate = byte & REBASE_IMMEDIATE_MASK;
        uint8_t opcode = byte & REBASE_OPCODE_MASK;
        ++p;

        const char* name = NULL;
        const char* error = NULL;
        const char* symbol = NULL;
        uint64_t uleb = 0;
        int64_t sleb = 0;

        switch (opcode) {
                /*
                 * codes with IMM
                 */
            case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                if (!name)
                    name = "BIND_OPCODE_SET_DYLIB_ORDINAL_IMM";
                /* FALLTHROUGH */
            case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                if (!name)
                    name = "BIND_OPCODE_SET_DYLIB_SPECIAL_IMM";
                /* FALLTHROUGH */
            case BIND_OPCODE_SET_TYPE_IMM:
                if (!name)
                    name = "BIND_OPCODE_SET_TYPE_IMM";
                /* FALLTHROUGH */
            case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                if (!name)
                    name = "BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED";
                printf("  %4u 0x%08x 0x%02x %s(%d)\n",
                       idx, poffset, byte, name, immediate);
                break;
                /*
                 * codes with IMM and symbol name
                 */
            case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                name = "BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM";
                printf("  %4u 0x%08x 0x%02x %s(%d, ",
                       idx, poffset, byte, name, immediate);
                symbol = (const char*)p;
                while(p < end && *p != '\0')
                    ++p;
                if (p < end && *p == '\0') {
                    if (p == (uint8_t*)symbol)
                        printf("(empty string))\n");
                    else
                        printf("%s)\n", symbol);
                    ++p;
                }
                else {
                    size_t len = strlen(symbol);
                    char* str = malloc(len + 1);
                    if (str) {
                        memcpy(str, symbol, len);
                        str[len] = '\0';
                        printf("%s (unterminated string)", str);
                        free(str);
                    }
                    printf(")\n");
                }
                break;
                /*
                 * codes with SLEB
                 */
            case BIND_OPCODE_SET_ADDEND_SLEB:
                name = "BIND_OPCODE_SET_ADDEND_SLEB";
                sleb = read_sleb128(&p, end, &error);
                if (error)
                    printf("  %4u 0x%08x 0x%02x %s(error: %s)\n",
                           idx, poffset, byte, name, error);
                else
                    printf("  %4u 0x%08x 0x%02x %s(%lld)\n",
                           idx, poffset, byte, name, sleb);
                break;
                /*
                 * codes with ULEB
                 */
            case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                if (!name)
                    name = "REBASE_OPCODE_ADD_ADDR_ULEB";
                /* FALLTHROUGH */
            case BIND_OPCODE_ADD_ADDR_ULEB:
                if (!name)
                    name = "BIND_OPCODE_ADD_ADDR_ULEB";
                /* FALLTHROUGH */
            case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                if (!name)
                    name = "BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB";
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("  %4u 0x%08x 0x%02x %s(error: %s)\n",
                           idx, poffset, byte, name, error);
                else
                    printf("  %4u 0x%08x 0x%02x %s(0x%llx)\n",
                           idx, poffset, byte, name, uleb);
                break;
                /*
                 * codes with IMM and ULEB
                 */
            case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                name = "BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB";
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("  %4u 0x%08x 0x%02x %s(%d, error: %s)\n",
                           idx, poffset, byte, name, immediate, error);
                else
                    printf("  %4u 0x%08x 0x%02x %s(%d, 0x%llx)\n",
                           idx, poffset, byte, name, immediate, uleb);
                break;
                /*
                 * codes with ULEB and ULEB
                 */
            case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                name = "BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB";
                printf("  %4u 0x%08x 0x%02x %s(", idx, poffset, byte, name);
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("error: %s, ", error);
                else
                    printf("0x%llx, ", uleb);
                uleb = read_uleb128(&p, end, &error);
                if (error)
                    printf("error: %s)\n", error);
                else
                    printf("0x%llx)\n", uleb);
                break;
                /*
                 * codes with IMM opcodes
                 */
            case BIND_OPCODE_THREADED:
                switch (immediate) {
                    case BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
                        name = ("BIND_OPCODE_THREADED "
                                "BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_"
                                "TABLE_SIZE_ULEB");
                        uleb = read_uleb128(&p, end, &error);
                        if (error)
                            printf("  %4u 0x%08x 0x%02x %s(error: %s)\n",
                                   idx, poffset, byte, name, error);
                        else
                            printf("  %4u 0x%08x 0x%02x %s(0x%llx)\n",
                                   idx, poffset, byte, name, uleb);
                        break;
                    case BIND_SUBOPCODE_THREADED_APPLY:
                        name = ("BIND_OPCODE_THREADED "
                                "BIND_SUBOPCODE_THREADED_APPLY");
                        printf("  %4u 0x%08x 0x%02x %s\n",
                               idx, poffset, byte, name);
                        break;
                }
                break;
                /*
                 * codes with no arguments
                 */
            case BIND_OPCODE_DONE:
                name = "BIND_OPCODE_DONE";
                done = TRUE;
                /* FALLTHROUGH */
            case BIND_OPCODE_DO_BIND:
                if (!name)
                    name = "BIND_OPCODE_DO_BIND";
                /* FALLTHROUGH */
            default:
                if (!name)
                    name = "UNKNOWN OPCODE";
                printf("  %4u 0x%08x 0x%02x %s\n", idx, poffset, byte, name);
        }
    }
}

/*
 * print_dyld_bind_info() prints the internal expanded dyld bind information in
 * the same format as dyldinfo(1)'s -bind option.
 */
void
print_dyld_bind_info(
struct dyld_bind_info *dbi,
uint64_t ndbi)
{
    uint64_t n;
    uint64_t value;
    uint16_t diversity;
    enum bool hasAddressDiversity;
    uint8_t key;
    enum bool isAuthenticated;
    static const char *keyNames[] = { "IA", "IB", "DA", "DB" };
    const char* bind_name;
    const char* symbolName;
    const char* dylibName;
    char valuestr[20];
    
    if (0 == ndbi)
        return;
    
    enum bool is32 = (DYLD_CHAINED_PTR_32 == dbi[0].pointer_format ||
                      DYLD_CHAINED_PTR_32_CACHE == dbi[0].pointer_format ||
                      DYLD_CHAINED_PTR_32_FIRMWARE == dbi[0].pointer_format);
    
    size_t maxsegname = strlen("segment");
    size_t maxsectname = strlen("section");
    size_t maxdylibname = strlen("dylib");
    size_t maxaddr = 0;
    size_t maxvalue = is32 ? 10 : 18;
    
    for (n = 0; n < ndbi; n++) {
        const char* name = dbi[n].segname ? dbi[n].segname : "";
        size_t len = strlen(name);
        if (len > maxsegname)
            maxsegname = len;
        name = dbi[n].sectname ? dbi[n].sectname : "";
        len = strlen(name);
        if (len > maxsectname)
            maxsectname = len;
        name = dbi[n].dylibname ? dbi[n].dylibname : "";
        len = strlen(name);
        if (len > maxdylibname)
            maxdylibname = len;
        len = snprintf(NULL, 0, "%llX", dbi[n].address);
        if (len > maxaddr)
            maxaddr = len;
    }

    printf("bind information:\n");
    printf("%*s %*s %*s %*s %-10s %-8s %*s %s\n",
           -(int)maxsegname, "segment",
           -(int)maxsectname, "section",
           -(int)(maxaddr+2), "address",
           -(int)maxvalue, "value",
           "type",
           "addend",
           -(int)maxdylibname, "dylib",
           "symbol");
    for(n = 0; n < ndbi; n++){
        bind_name = (dbi[n].bind_name ? dbi[n].bind_name :
                     bindTypeName(dbi[n].bind_type));
        dylibName = dbi[n].dylibname ? dbi[n].dylibname : "";
        symbolName = dbi[n].symbolname ? dbi[n].symbolname : "";
        if (dbi[n].pointer_value) {
            if (is32) {
                snprintf(valuestr, sizeof(valuestr), "0x%08llX",
                         dbi[n].pointer_value);
            } else {
                snprintf(valuestr, sizeof(valuestr), "0x%016llX",
                         dbi[n].pointer_value);
            }
        }
        else {
            snprintf(valuestr, sizeof(valuestr), "");
        }

        printf("%*s %*s 0x%*llX %*s %-10s 0x%-6llX %*s %s%s",
               -(int)maxsegname, dbi[n].segname,
               -(int)maxsectname, dbi[n].sectname,
               -(int)maxaddr, dbi[n].address,
               -(int)maxvalue, valuestr,
               bind_name,
               dbi[n].addend,
               -(int)maxdylibname, dylibName,
               symbolName,
               dbi[n].weak_import ? " (weak import)" : "");

        value = dbi[n].pointer_value;
        diversity = (uint16_t)(value >> 32);
        hasAddressDiversity = (value & (1ULL << 48)) != 0;
        key = (value >> 49) & 0x3;
        isAuthenticated = (value & (1ULL << 63)) != 0;
        if(isAuthenticated){
            printf(" (JOP: diversity %d, address %s, %s)", diversity,
                   hasAddressDiversity ? "true" : "false", keyNames[key]);
        }
        printf("\n");
    }
}

/*
 * get_dyld_bind_info_symbolname() is passed an address and the internal
 * expanded dyld bind information.  If the address is found its binding symbol
 * name is returned.  If not NULL.
 */
const char *
get_dyld_bind_info_symbolname(
uint64_t address,
struct dyld_bind_info *dbi,
uint64_t ndbi,
enum chain_format_t chain_format,
int64_t *addend)
{
    uint64_t n;
    
    for(n = 0; n < ndbi; n++){
        if(dbi[n].address == address){
            if(chain_format && addend != NULL)
                *addend = dbi[n].addend;
            return(dbi[n].symbolname);
        }
    }
    return(NULL);
}

/*****************************************************************************
 *
 *   Chained Fixups
 *
 */

/*
 * swap16, swap32, and swap64 are little inline functions for abstracting away
 * endian swapping.
 */
static inline uint16_t swap16(uint16_t x, enum bool swapped)
{
    return swapped == TRUE ? OSSwapInt16(x): x;
}
static inline uint32_t swap32(uint32_t x, enum bool swapped)
{
    return swapped == TRUE ? OSSwapInt32(x): x;
}

static inline uint64_t swap64(uint64_t x, enum bool swapped)
{
    return swapped == TRUE ? OSSwapInt64(x): x;
}

/*
 * get_base_addr computes the base vmaddress from a list of segments. by
 * convention, this is the vmaddr of the __TEXT segment. It will return
 * (uint64_t)-1 on error;
 */
static uint64_t
get_base_addr(struct segment_command **segs,
              uint32_t nsegs,
              struct segment_command_64 **segs64,
              uint32_t nsegs64)
{
    for (uint32_t i = 0; i < nsegs64; ++i) {
        if (0 == strcmp("__TEXT", segs64[i]->segname))
            return segs64[i]->vmaddr;
    }
    for (uint32_t i = 0; i < nsegs; ++i) {
        if (0 == strcmp("__TEXT", segs[i]->segname))
            return segs[i]->vmaddr;
    }
    
    return ((uint64_t)-1);
}

/*
 * segmentIndexFromAddr returns the index of a segment that contains the
 * specified input address. The address is a vmaddr, not a fileoff. Canonical
 * Mach-O files will include either 32-bit or 64-bit segments. If for some
 * reason both 32- and 64-bit segments are passed in here, 64-bit segments will
 * be consulted first; but really, you'll have other issues ...
 *
 * Earlier Mach-O structures encoded segment indicies directly. dyld3's
 * chained fixups format saves space by ommitting this information, knowing
 * that with a quick O(N) algorithm one can simply compute this information
 * from a vmaddr.
 */
static uint32_t
segmentIndexFromAddr(uint64_t addr,
                     struct segment_command **segs,
                     uint32_t nsegs,
                     struct segment_command_64 **segs64,
                     uint32_t nsegs64)
{
    for (uint32_t i = 0; i < nsegs64; ++i) {
        if (segs64[i]->vmaddr <= addr &&
            segs64[i]->vmaddr + segs64[i]->vmsize > addr)
            return i;
    }
    for (uint32_t i = 0; i < nsegs; ++i) {
        if (segs[i]->vmaddr <= addr &&
            segs[i]->vmaddr + segs[i]->vmsize > addr)
            return i;
    }
    
    return ((uint32_t)-1);
}

/*
 * fixup_target comes from dyld3. it represents a fixup target, which is most
 * likely a symbol. this structure captures values from a variety of different
 * fixup formats.
 */
struct fixup_target
{
    uint64_t    value;
    const char* dylib;
    const char* symbolName;
    uint64_t    addend;
    enum bool   weakImport;
};

/*
 * signExtendedAddend64 comes from dyld3. It computes the sign extended
 * addend from a 64-bit pointer.
 */
static uint64_t signExtendedAddend64(uint64_t addend27)
{
    uint64_t top8Bits     = addend27 & 0x00007F80000ULL;
    uint64_t bottom19Bits = addend27 & 0x0000007FFFFULL;
    uint64_t newValue     = (top8Bits << 13) |
                            (((uint64_t)(bottom19Bits << 37) >> 37) &
                             0x00FFFFFFFFFFFFFF);
    return newValue;
}

/*
 * signExtendedAddend64e comes from dyld3. It computes the sign extended
 * addend from a 64-bit arm64e pointer (no auth).
 */
static uint64_t signExtendedAddend64e(uint64_t addend19)
{
    if ( addend19 & 0x40000 )
        return addend19 | 0xFFFFFFFFFFFC0000ULL;
    else
        return addend19;
}

/*
 * libOrdinalFromUInt8() returns a signed libOrdinal (aka dylibOrdinal) from
 * a uint8_t as found in the chained import data.
 */
static int libOrdinalFromUInt8(uint8_t value)
{
    if (((uint8_t)-1) == value)
        return -1;
    else if (((uint8_t)-2) == value)
        return -2;
    else if (((uint8_t)-3) == value)
        return -3;
    return value;
}

/*
 * libOrdinalFromUInt16() returns a signed libOrdinal (aka dylibOrdinal) from
 * a uint16_t as found in the chained import data.
 */
static int libOrdinalFromUInt16(uint16_t value)
{
    if (((uint16_t)-1) == value)
        return -1;
    else if (((uint16_t)-2) == value)
        return -2;
    else if (((uint16_t)-3) == value)
        return -3;
    return value;
}

/*
 * get_fixup_targets builds a list of targets from a dyld_chained_fixups_header
 * and a list of dylibs ordered by ordinal. targets and ntarget may be
 * uninitialized prior to call.
 *
 * This function will print errors as they are encountered, some of which
 * will halt processing early and some of which will not.
 */
static void get_fixup_targets(/* inputs */
                              uint32_t datasize,
                              struct dyld_chained_fixups_header* header,
                              enum bool swapped,
                              const char **dylibs,
                              uint32_t ndylibs,
                              /* outputs */
                              struct fixup_target **targets,
                              uint32_t *ntarget
                              )
{
    /* build the list of fixup targets, one per symbol */
    uint32_t imports_count = swap32(header->imports_count, swapped);
    uint32_t imports_format = swap32(header->imports_format, swapped);
    uint32_t symbols_offset = swap32(header->symbols_offset, swapped);
    uint32_t max_symbols_offset = datasize - symbols_offset;
    const char* symbolsPool = (char*)header + symbols_offset;
    const char* dylibName;
    
    *ntarget = imports_count;
    *targets = reallocf(*targets, sizeof(struct fixup_target) * imports_count);
    memset(*targets, 0, sizeof(struct fixup_target) * imports_count);
    
    if (imports_format < 1 || imports_format > 3) {
        printf("unknown chained-fixups import format: %d\n", imports_format);
        return;
    }
    
    for (uint32_t i=0; i < imports_count; ++i) {
        int lib_ordinal = 0;

        if (imports_format == DYLD_CHAINED_IMPORT) {
            /* get a swapped dyld_chained_import struct */
            uint32_t values[1];
            memcpy(values, ((uint8_t*)header + header->imports_offset +
                            i * sizeof(struct dyld_chained_import)),
                   sizeof(values));
            values[0] = swap32(values[0], swapped);
            struct dyld_chained_import imports;
            memcpy(&imports, &values, sizeof(imports));

            /* look for overflow */
            if (imports.name_offset > max_symbols_offset) {
                printf("chained-fixups import #%u symbol offset extends "
                       "past end: %u (max %u)\n", i, imports.name_offset,
                       max_symbols_offset);
                imports.name_offset = 0;
            }

            /* record the information */
            (*targets)[i].symbolName = &symbolsPool[imports.name_offset];
            (*targets)[i].weakImport = imports.weak_import;
            lib_ordinal = libOrdinalFromUInt8(imports.lib_ordinal);
        }
        else if (imports_format == DYLD_CHAINED_IMPORT_ADDEND) {
            /* get a swapped dyld_chained_import_addend struct */
            uint32_t values[2];
            memcpy(values, ((uint8_t*)header + header->imports_offset +
                            i * sizeof(struct dyld_chained_import_addend)),
                   sizeof(values));
            values[0] = swap32(values[0], swapped);
            values[1] = swap32(values[1], swapped);
            struct dyld_chained_import_addend imports;
            memcpy(&imports, &values, sizeof(imports));
            
            /* look for overflow */
            if (imports.name_offset > max_symbols_offset) {
                printf("chained-fixups import #%u symbol offset extends "
                       "past end: %u (max %u)\n", i, imports.name_offset,
                       max_symbols_offset);
                imports.name_offset = 0;
            }
            
            /* record the information */
            (*targets)[i].symbolName = &symbolsPool[imports.name_offset];
            (*targets)[i].weakImport = imports.weak_import;
            (*targets)[i].addend = imports.addend;
            lib_ordinal = libOrdinalFromUInt8(imports.lib_ordinal);
        }
        else if (imports_format == DYLD_CHAINED_IMPORT_ADDEND64) {
            /* get a swapped dyld_chained_import_addend64 struct */
            uint64_t values[2];
            memcpy(values, ((uint8_t*)header + header->imports_offset +
                            i * sizeof(struct dyld_chained_import_addend64)),
                   sizeof(values));
            values[0] = swap64(values[0], swapped);
            values[1] = swap64(values[1], swapped);
            struct dyld_chained_import_addend64 imports;
            memcpy(&imports, &values, sizeof(imports));
            
            /* look for overflow */
            if (imports.name_offset > max_symbols_offset) {
                printf("chained-fixups import #%u symbol offset extends "
                       "past end: %u (max %u)\n", i, imports.name_offset,
                       max_symbols_offset);
                imports.name_offset = 0;
            }
            
            /* record the information */
            (*targets)[i].symbolName = &symbolsPool[imports.name_offset];
            (*targets)[i].weakImport = imports.weak_import;
            (*targets)[i].addend = imports.addend;
            lib_ordinal = libOrdinalFromUInt16(imports.lib_ordinal);
        }
        
        if (!validateOrdinal(lib_ordinal, dylibs, ndylibs, &dylibName, NULL)) {
            if (lib_ordinal < 0)
                printf("chained-fixups import #%u bad special library ordinal: "
                       "%d\n", i, lib_ordinal);
            else
                printf("chained-fixups import #%u bad library ordinal: %d "
                       "(max %u)\n", i, lib_ordinal, ndylibs);
            continue;
        }
        
        (*targets)[i].dylib = dylibName;
    }
}

/*
 * walk_chain walks a chain of binds and rebases, starting at the pointer
 * location indicated by the dyld_chained_starts_in_segment struct, the
 * pageIndex, and the offsetInPage values. The chain will be followed until
 * it ends.
 *
 * For each pointer found in the chain, walk_chain will build a dyld_bind_info
 * structure using information found in the segment lists and the list of fixup
 * targets (e.g., symbols). The dyld_bind_info is not quiiiiiite what we want
 * for this: it does not properly support BIND types vs REBASE types. But the
 * format used by the original arm64e format and the new chained binds format
 * are so semantically equivalent, it does a reasonable job.
 *
 * The output values dbi and ndbi must be initialized to NULL / 0 before
 * first calling walk_chain; values modified by walk_chain may be passed
 * back into walk_chain safely.
 */
static void walk_chain(/* inputs */
                       char *object_addr,
                       uint64_t object_size,
                       struct segment_command **segs,
                       uint32_t nsegs,
                       struct segment_command_64 **segs64,
                       uint32_t nsegs64,
                       struct fixup_target *targets,
                       uint32_t ntarget,
                       enum bool swapped,
                       struct dyld_chained_starts_in_segment* segInfo,
                       uint16_t pageIndex,
                       uint16_t offsetInPage,
                       /* output */
                       struct dyld_bind_info **dbi,
                       uint64_t *ndbi)
{
    uint64_t segment_offset = swap64(segInfo->segment_offset, swapped);
    uint16_t page_size = swap16(segInfo->page_size, swapped);
    uint16_t pointer_format = swap16(segInfo->pointer_format, swapped);
    uint64_t vmaddr = segment_offset + pageIndex * page_size;
    uint64_t baseaddr = get_base_addr(segs, nsegs, segs64, nsegs64);
    uint8_t* pageContentStart = (uint8_t*)(object_addr + vmaddr);
    void* chain = (pageContentStart+offsetInPage);
    uint64_t chain_addr = baseaddr + vmaddr + offsetInPage;
    enum bool done = FALSE;
    
    /* loop over all of the fixups in the chain */
    while (!done) {
        uint64_t dbii = *ndbi;
        *ndbi += 1;
        *dbi = reallocf(*dbi, sizeof(**dbi) * (*ndbi));
        memset(&(*dbi)[dbii], 0, sizeof(**dbi));
        
        uint64_t addend = 0;
        uint32_t segIndex = segmentIndexFromAddr(chain_addr, segs, nsegs,
                                                 segs64, nsegs64);
        const char* symbolName = NULL;
        const char* dylibName = NULL;
        const char* bindName = NULL;
        enum bool weakImport = FALSE;
        
        switch (pointer_format) {
            case DYLD_CHAINED_PTR_ARM64E:
            {
                uint64_t chain_value = swap64(*(uint64_t*)chain, swapped);
                uint64_t address = chain_addr;
                uint64_t pointer_value = chain_value;
                
                struct dyld_chained_ptr_arm64e_auth_rebase auth_rebase;
                memcpy(&auth_rebase, &chain_value, sizeof(auth_rebase));
                
                if (auth_rebase.auth) {
                    struct dyld_chained_ptr_arm64e_auth_bind auth_bind;
                    memcpy(&auth_bind, &chain_value, sizeof(auth_bind));
                    if (auth_bind.bind) {
                        struct fixup_target* target =
                        &targets[auth_bind.ordinal];
                        if (target->addend) {
                            addend = target->addend;
                        }
                        symbolName = target->symbolName;
                        dylibName = target->dylib;
                        weakImport = target->weakImport;
                        bindName = "bind aptr";
                    }
                    else {
                        uint64_t targetOffset = auth_rebase.target;
                        address = targetOffset + baseaddr;
                        bindName = "rebase aptr";
                        pointer_value = targetOffset + baseaddr;
                    }
                }
                else {
                    struct dyld_chained_ptr_arm64e_bind bind;
                    memcpy(&bind, &chain_value, sizeof(bind));
                    if (bind.bind) {
                        struct fixup_target* target = &targets[bind.ordinal];
                        addend = target->addend +
                            signExtendedAddend64e(bind.addend);
                        symbolName = target->symbolName;
                        dylibName = target->dylib;
                        weakImport = target->weakImport;
                        bindName = "bind ptr";
                        pointer_value = 0;
                    }
                    else {
                        struct dyld_chained_ptr_arm64e_rebase rebase;
                        memcpy(&rebase, &chain_value, sizeof(rebase));
                        bindName = "rebase ptr";
                        pointer_value = ((uint64_t)(rebase.high8) << 56) |
                        rebase.target;
                    }
                }
                
                const char* csegname = segmentName(segIndex, segs, nsegs,
                                                   segs64, nsegs64);
                const char* csectname = sectionName(segIndex, chain_addr, segs,
                                                    nsegs, segs64, nsegs64);
                char* segname = strdup(csegname);
                char* sectname = strdup(csectname);
                if (strlen(segname)>16)
                    segname[16] = 0;
                if (strlen(sectname)>16)
                    sectname[16] = 0;
                
                (*dbi)[dbii].segname = segname;
                (*dbi)[dbii].sectname = sectname;
                (*dbi)[dbii].address = address;
                (*dbi)[dbii].bind_name = bindName;
                (*dbi)[dbii].addend = addend;
                (*dbi)[dbii].dylibname = dylibName;
                (*dbi)[dbii].symbolname = symbolName;
                (*dbi)[dbii].weak_import = weakImport;
                (*dbi)[dbii].pointer_value = pointer_value;
                
                if (auth_rebase.next == 0) {
                    done = TRUE;
                }
                else {
                    chain += auth_rebase.next * 8;
                    chain_addr += auth_rebase.next * 8;
                }
            }
                break;
            case DYLD_CHAINED_PTR_64:
            {
                uint64_t chain_value = swap64(*(uint64_t*)chain, swapped);
                uint64_t pointer_value = chain_value;
                
                struct dyld_chained_ptr_64_bind bind;
                memcpy(&bind, &chain_value, sizeof(bind));
                
                if (bind.bind) {
                    struct fixup_target* target = &targets[bind.ordinal];
                    addend = target->addend +
                        signExtendedAddend64(bind.addend);
                    symbolName = target->symbolName;
                    dylibName = target->dylib;
                    weakImport = target->weakImport;
                    bindName = "bind ptr";
                    pointer_value = 0;
                }
                else {
                    struct dyld_chained_ptr_64_rebase rebase;
                    memcpy(&rebase, &chain_value, sizeof(rebase));
                    bindName = "rebase ptr";
                    pointer_value = ((uint64_t)(rebase.high8) << 56) |
                    rebase.target;
                }
                
                const char* csegname = segmentName(segIndex, segs, nsegs,
                                                   segs64, nsegs64);
                const char* csectname = sectionName(segIndex, chain_addr, segs,
                                                    nsegs, segs64, nsegs64);
                char* segname = strdup(csegname);
                char* sectname = strdup(csectname);
                if (strlen(segname)>16)
                    segname[16] = 0;
                if (strlen(sectname)>16)
                    sectname[16] = 0;
                
                (*dbi)[dbii].segname = segname;
                (*dbi)[dbii].sectname = sectname;
                (*dbi)[dbii].address = chain_addr;
                (*dbi)[dbii].bind_name = bindName;
                (*dbi)[dbii].addend = addend;
                (*dbi)[dbii].dylibname = dylibName;
                (*dbi)[dbii].symbolname = symbolName;
                (*dbi)[dbii].weak_import = weakImport;
                (*dbi)[dbii].pointer_value = pointer_value;
                
                if (bind.next == 0) {
                    done = TRUE;
                }
                else {
                    chain += bind.next * 4;
                    chain_addr += bind.next * 4;
                }
            }
                break;
            case DYLD_CHAINED_PTR_32:
            {
                uint32_t chain_value = swap32(*(uint32_t*)chain, swapped);
                uint32_t pointer_value = chain_value;
                
                struct dyld_chained_ptr_32_bind bind;
                memcpy(&bind, &chain_value, sizeof(bind));
                
                if (bind.bind) {
                    struct fixup_target* target = &targets[bind.ordinal];
                    addend = target->addend + bind.addend;
                    symbolName = target->symbolName;
                    dylibName = target->dylib;
                    weakImport = target->weakImport;
                    bindName = "bind ptr";
                    pointer_value = 0;
                }
                else {
                    struct dyld_chained_ptr_32_rebase rebase;
                    memcpy(&rebase, &chain_value, sizeof(rebase));
                    bindName = "rebase ptr";
                    pointer_value = rebase.target;
                }
                
                const char* csegname = segmentName(segIndex, segs, nsegs,
                                                   segs64, nsegs64);
                const char* csectname = sectionName(segIndex, chain_addr, segs,
                                                    nsegs, segs64, nsegs64);
                char* segname = strdup(csegname);
                char* sectname = strdup(csectname);
                if (strlen(segname)>16)
                    segname[16] = 0;
                if (strlen(sectname)>16)
                    sectname[16] = 0;
                
                (*dbi)[dbii].segname = segname;
                (*dbi)[dbii].sectname = sectname;
                (*dbi)[dbii].address = chain_addr;
                (*dbi)[dbii].bind_name = bindName;
                (*dbi)[dbii].addend = addend;
                (*dbi)[dbii].dylibname = dylibName;
                (*dbi)[dbii].symbolname = symbolName;
                (*dbi)[dbii].weak_import = weakImport;
                (*dbi)[dbii].pointer_value = pointer_value;
                (*dbi)[dbii].pointer_format = pointer_format;
                
                if (bind.next == 0) {
                    done = TRUE;
                }
                else {
                    struct dyld_chained_ptr_32_rebase rebase;
                    memcpy(&rebase, &chain_value, sizeof(rebase));
                    chain += rebase.next * 4;
                    chain_addr += rebase.next * 4;
                    
                    /* Skip over non-pointers */
                    chain_value = swap32(*(uint32_t*)chain, swapped);
                    memcpy(&rebase, &chain_value, sizeof(rebase));
                    while ((rebase.bind == 0) &&
                           (rebase.target > segInfo->max_valid_pointer))
                    {
                        chain += rebase.next * 4;
                        chain_addr += rebase.next * 4;
                        chain_value = swap32(*(uint32_t*)chain, swapped);
                        memcpy(&rebase, &chain_value, sizeof(rebase));
                    }
                }
            }
                break;
            case DYLD_CHAINED_PTR_32_CACHE:
            case DYLD_CHAINED_PTR_32_FIRMWARE:
                /* otool does not support printing these formats */
                done = TRUE;
                break;
        }
    }
}

/*
 * get_dyld_chained_fixups() unpacks the dyld bind info from the data pointed
 * to by an LC_DYLD_CHAINED_FIXUPS load command. Information is returned in
 * the older dyld_bind_info so that existing logic can be reused.
 */
void
get_dyld_chained_fixups(/* input */
                        const uint8_t* start,
                        const uint8_t* end,
                        const char **dylibs,
                        uint32_t ndylibs,
                        struct segment_command **segs,
                        uint32_t nsegs,
                        struct segment_command_64 **segs64,
                        uint32_t nsegs64,
                        enum bool swapped,
                        char *object_addr,
                        uint64_t object_size,
                        /* output */
                        struct dyld_bind_info **dbi,
                        uint64_t *ndbi,
                        enum chain_format_t *chain_format,
                        enum bool print_errors)
{
    uint64_t end_offset = end - start;
    struct fixup_target* targets = NULL;
    uint32_t ntarget = 0;
    
    struct dyld_chained_fixups_header* header;
    struct dyld_chained_starts_in_image* imageStarts;
    
    *chain_format = CHAIN_FORMAT_NONE;
    
    /* get the chained fixups header */
    if (sizeof(struct dyld_chained_fixups_header) + start > end) {
        printf("bad chained fixups: header size %lu extends past end %llu\n",
               sizeof(struct dyld_chained_fixups_header), end_offset);
        return;
    }
    header = (struct dyld_chained_fixups_header*)start;
    
    /* get the fixup targets */
    get_fixup_targets((uint32_t)(end - start), header, swapped, dylibs, ndylibs,
                      &targets, &ntarget);
    
    /* get the chained starts */
    uint32_t starts_offset = swap32(header->starts_offset, swapped);
    if (starts_offset < sizeof(struct dyld_chained_fixups_header)) {
        printf("bad chained fixups: image starts offset %u overlaps with "
               "fixups header size %lu\n", starts_offset,
               sizeof(struct dyld_chained_fixups_header));
        return;
    }
    if (starts_offset +
        sizeof(struct dyld_chained_starts_in_image) > end_offset) {
        printf("bad chained fixups: image starts end %lu extends past "
               "end %llu\n", starts_offset +
               sizeof(struct dyld_chained_starts_in_image), end_offset);
        return;
    }
    imageStarts = (struct dyld_chained_starts_in_image*)(start + starts_offset);
    
    /* walk the image starts */
    uint32_t nseg = swap32(imageStarts->seg_count, swapped);
    for (uint32_t iseg = 0; iseg < nseg; ++iseg)
    {
        struct dyld_chained_starts_in_segment* segInfo;
        
        uint32_t seg_info_offset = swap32(imageStarts->seg_info_offset[iseg],
                                          swapped);
        
        /* seg_info_offsets are allowed to be 0 */
        if (0 == seg_info_offset)
            continue;
        
        /*
         * dyld does not strictly check that the dyld_chained_starts_in_segment
         * struct fits in the Mach-O. As long as page_count is addressable
         * and has a 0 value, the rest of the struct can be any random data.
         */
        if (starts_offset + seg_info_offset > end_offset) {
            printf("bad chained fixups: seg_info %i offset %u starts past "
                   "end %llu\n", iseg, starts_offset + seg_info_offset,
                   end_offset);
            continue;
        }
        if (starts_offset + seg_info_offset +
            sizeof(struct dyld_chained_starts_in_segment) > end_offset) {
            printf("bad chained fixups: seg_info %i end %lu extends past "
                   "end %llu\n", iseg, starts_offset + seg_info_offset +
                   sizeof(struct dyld_chained_starts_in_segment), end_offset);
        }
        
        segInfo = ((struct dyld_chained_starts_in_segment*)
                   (start + starts_offset + seg_info_offset));
        
        uint16_t page_count = swap16(segInfo->page_count, swapped);
        for (uint16_t pageIndex = 0; pageIndex < page_count; ++pageIndex)
        {
            /* set the chain format. If already set, verify it. */
            uint16_t pointer_format = swap16(segInfo->pointer_format, swapped);
            if (CHAIN_FORMAT_NONE == *chain_format) {
                *chain_format = (enum chain_format_t)pointer_format;
            }
            else if (pointer_format != *chain_format) {
                printf("bad chained fixups: seg_info %i pointer format %d "
                       "does not match previous pointer format %d\n", iseg,
                       pointer_format, *chain_format);
            }
            
            /* walk the chains */
            uint16_t offsetInPage = swap16(segInfo->page_start[pageIndex],
                                           swapped);
            if ( offsetInPage == DYLD_CHAINED_PTR_START_NONE )
                continue;
            if ( offsetInPage & DYLD_CHAINED_PTR_START_MULTI ) {
                uint32_t overflowIndex =
                offsetInPage & ~DYLD_CHAINED_PTR_START_MULTI;
                enum bool chainEnd = FALSE;
                while (!chainEnd) {
                    uint16_t page_start =
                    swap16(segInfo->page_start[overflowIndex], swapped);
                    chainEnd = (page_start & DYLD_CHAINED_PTR_START_LAST);
                    offsetInPage = (page_start & ~DYLD_CHAINED_PTR_START_LAST);
                    walk_chain(object_addr, object_size, segs, nsegs,
                               segs64, nsegs64, targets, ntarget, swapped,
                               segInfo, pageIndex, offsetInPage, dbi, ndbi);
                    ++overflowIndex;
                }
            }
            else {
                walk_chain(object_addr, object_size, segs, nsegs,
                           segs64, nsegs64, targets, ntarget, swapped,
                           segInfo, pageIndex, offsetInPage, dbi, ndbi);
            }
        }
    }
    
    free(targets);
}

uint64_t get_chained_rebase_value(
                                  uint64_t chain_value,
                                  enum chain_format_t chain_format,
                                  enum bool *has_auth)
{
    if (has_auth)
        *has_auth = FALSE;
    
    switch (chain_format) {
        case CHAIN_FORMAT_NONE:
            break;
        case CHAIN_FORMAT_ARM64E:
        {
            struct dyld_chained_ptr_arm64e_auth_rebase* auth_rebase =
            (struct dyld_chained_ptr_arm64e_auth_rebase*)&chain_value;
            
            if (auth_rebase->auth) {
                if (!auth_rebase->bind) {
                    if (has_auth)
                        *has_auth = TRUE;
                    return auth_rebase->target;
                }
            }
            else {
                struct dyld_chained_ptr_arm64e_rebase* rebase =
                (struct dyld_chained_ptr_arm64e_rebase*)&chain_value;
                if (!rebase->bind) {
                    return rebase->target;
                }
            }
        }
            break;
        case CHAIN_FORMAT_PTR_64:
        {
            struct dyld_chained_ptr_64_rebase* rebase =
            (struct dyld_chained_ptr_64_rebase*)&chain_value;
            if (!rebase->bind) {
                return rebase->target;
            }
        }
            break;
        case CHAIN_FORMAT_PTR_32:
        {
            struct dyld_chained_ptr_32_rebase* rebase =
            (struct dyld_chained_ptr_32_rebase*)&chain_value;
            if (!rebase->bind) {
                return rebase->target;
            }
        }
            break;
        case CHAIN_FORMAT_PTR_32_CACHE:
        {
            struct dyld_chained_ptr_32_cache_rebase* rebase =
            (struct dyld_chained_ptr_32_cache_rebase*)&chain_value;
            return rebase->target;
        }
            break;
        case CHAIN_FORMAT_PTR_32_FIRMWARE:
        {
            struct dyld_chained_ptr_32_firmware_rebase* rebase =
            (struct dyld_chained_ptr_32_firmware_rebase*)&chain_value;
            return rebase->target;
        }
            break;
    }
    
    return chain_value;
}
