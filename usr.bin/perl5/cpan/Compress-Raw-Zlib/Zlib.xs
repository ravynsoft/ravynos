/* Filename: Zlib.xs
 * Author  : Paul Marquess, <pmqs@cpan.org>
 * Created : 22nd January 1996
 * Version : 2.000
 *
 *   Copyright (c) 1995-2013 Paul Marquess. All rights reserved.
 *   This program is free software; you can redistribute it and/or
 *   modify it under the same terms as Perl itself.
 *
 */

/* Parts of this code are based on the files gzio.c and gzappend.c from
 * the standard zlib source distribution. Below are the copyright statements
 * from each.
 */

/* gzio.c -- IO on .gz files
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* gzappend -- command to append to a gzip file

  Copyright (C) 2003 Mark Adler, all rights reserved
  version 1.1, 4 Nov 2003
*/


#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#if USE_ZLIB_NG
#  include "zlib-ng.h"
#else
#  include "zlib.h"
#endif


/* zlib prior to 1.06 doesn't know about z_off_t */
#ifndef z_off_t
#  define z_off_t   long
#endif

#if ! USE_ZLIB_NG && (! defined(ZLIB_VERNUM) || ZLIB_VERNUM < 0x1200)
#  define NEED_DUMMY_BYTE_AT_END
#endif

#if USE_ZLIB_NG || (defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1210)
#  define MAGIC_APPEND
#  define AT_LEAST_ZLIB_1_2_1
#endif

#if USE_ZLIB_NG || (defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1221)
#  define AT_LEAST_ZLIB_1_2_2_1
#endif

#if USE_ZLIB_NG || (defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1222)
#  define AT_LEAST_ZLIB_1_2_2_2
#endif

#if USE_ZLIB_NG || (defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1223)
#  define AT_LEAST_ZLIB_1_2_2_3
#endif

#if USE_ZLIB_NG || (defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1230)
#  define AT_LEAST_ZLIB_1_2_3
#endif

#if USE_ZLIB_NG || (defined(ZLIB_VERNUM) && ZLIB_VERNUM >= 0x1252)
/*
    Use Z_SOLO to build source means need own malloc/free
 */
#  define AT_LEAST_ZLIB_1_2_5_2
#endif


/* zlib vs zlib-ng */

#if USE_ZLIB_NG

/* zlibng native */

#  define HAVE_ZLIB_NG_NATIVE       TRUE
#  define HAVE_ZLIB_NG_COMPAT       FALSE

#  ifndef ZLIBNG_VER_STATUS
#    define ZLIBNG_VER_STATUS 0
#  endif

#  ifndef ZLIBNG_VER_MODIFIED
#    define ZLIBNG_VER_MODIFIED 0
#  endif

#  define CRZ_adlerInitial          zng_adler32(0L, Z_NULL, 0)
#  define CRZ_crcInitial            zng_crc32(0L, Z_NULL, 0)

#  define CRZ_ZSTREAM               zng_stream



#  define CRZ_adler32               zng_adler32
#  define CRZ_adler32_combine       zng_adler32_combine
#  define CRZ_crc32                 zng_crc32
#  define CRZ_crc32_combine         zng_crc32_combine
#  define CRZ_deflate               zng_deflate
#  define CRZ_deflateEnd            zng_deflateEnd
#  define CRZ_deflateInit           zng_deflateInit
#  define CRZ_deflateInit2          zng_deflateInit2
#  define CRZ_deflateParams         zng_deflateParams
#  define CRZ_deflatePrime          zng_deflatePrime
#  define CRZ_deflateReset          zng_deflateReset
#  define CRZ_deflateSetDictionary  zng_deflateSetDictionary
#  define CRZ_deflateTune           zng_deflateTune
#  define CRZ_inflate               zng_inflate
#  define CRZ_inflateEnd            zng_inflateEnd
#  define CRZ_inflateInit2          zng_inflateInit2
#  define CRZ_inflateReset          zng_inflateReset
#  define CRZ_inflateSetDictionary  zng_inflateSetDictionary
#  define CRZ_inflateSync           zng_inflateSync
#  define CRZ_zlibCompileFlags      zng_zlibCompileFlags


/* zlib  symbols & functions */

// #  define CRZ_ZLIB_VERSION          ZLIBNG_VERSION
// #  define ZLIB_VERSION              ZLIBNG_VERSION
#  define CRZ_ZLIB_VERSION          ""
#  define ZLIB_VERSION              ""

// #  define CRZ_zlibVersion           zlibng_version
// #  define CRZ_zlib_version          zlibng_version

   const char *CRZ_zlibVersion(void)  { return ""; }
   const char *CRZ_zlib_version(void) { return ""; }


#else /* zlib specific */


#  define HAVE_ZLIB_NG_NATIVE       FALSE

/* Is this real zlib or zlib-ng in compat mode */
#  ifdef ZLIBNG_VERSION
     /* zlib-ng in compat mode */
#    define HAVE_ZLIB_NG_COMPAT     TRUE

#    ifndef ZLIBNG_VER_STATUS
#      define ZLIBNG_VER_STATUS 0
#    endif

#    ifndef ZLIBNG_VER_MODIFIED
#      define ZLIBNG_VER_MODIFIED 0
#    endif

   const char *zlibng_version(void)  { return ZLIBNG_VERSION ; }


#  else
     /* zlib native mode */

#    define HAVE_ZLIB_NG_COMPAT     FALSE

     /* zlib doesn't have the ZLIBNG synbols, so create them */
#    define ZLIBNG_VERSION          ""
#    define ZLIBNG_VERNUM           0
#    define ZLIBNG_VER_MAJOR        0
#    define ZLIBNG_VER_MINOR        0
#    define ZLIBNG_VER_REVISION     0
#    define ZLIBNG_VER_STATUS       0
#    define ZLIBNG_VER_MODIFIED     0
#    define ZLIBNG_VERNUM           0

   const char *zlibng_version(void) { return ""; }

#  endif



#  define CRZ_adlerInitial          adler32(0L, Z_NULL, 0)
#  define CRZ_crcInitial            crc32(0L, Z_NULL, 0)

#  define CRZ_ZSTREAM               z_stream

#  define CRZ_adler32               adler32
#  define CRZ_adler32_combine       adler32_combine
#  define CRZ_crc32                 crc32
#  define CRZ_crc32_combine         crc32_combine
#  define CRZ_deflate               deflate
#  define CRZ_deflateEnd            deflateEnd
#  define CRZ_deflateInit           deflateInit
#  define CRZ_deflateInit2          deflateInit2
#  define CRZ_deflateParams         deflateParams
#  define CRZ_deflatePrime          deflatePrime
#  define CRZ_deflateReset          deflateReset
#  define CRZ_deflateSetDictionary  deflateSetDictionary
#  define CRZ_deflateTune           deflateTune
#  define CRZ_inflate               inflate
#  define CRZ_inflateEnd            inflateEnd
#  define CRZ_inflateInit2          inflateInit2
#  define CRZ_inflateReset          inflateReset
#  define CRZ_inflateSetDictionary  inflateSetDictionary
#  define CRZ_inflateSync           inflateSync
#  define CRZ_zlibCompileFlags      zlibCompileFlags
#  define CRZ_zlibVersion           zlibVersion
#  define CRZ_zlib_version          zlibVersion

#endif


#ifdef USE_PPPORT_H
#  define NEED_sv_2pvbyte
#  define NEED_sv_2pv_nolen
#  define NEED_sv_pvn_force_flags
#  include "ppport.h"
#endif

#if PERL_REVISION == 5 && PERL_VERSION == 9
    /* For Andreas */
#   define sv_pvbyte_force(sv,lp) sv_pvbyten_force(sv,lp)
#endif

#if PERL_REVISION == 5 && (PERL_VERSION < 8 || (PERL_VERSION == 8 && PERL_SUBVERSION < 4 ))

#    ifdef SvPVbyte_force
#        undef SvPVbyte_force
#    endif

#    define SvPVbyte_force(sv,lp) SvPV_force(sv,lp)

#endif

#ifndef SvPVbyte_nolen
#    define SvPVbyte_nolen SvPV_nolen
#endif



#if 0
#  ifndef SvPVbyte_nolen
#    define SvPVbyte_nolen SvPV_nolen
#  endif

#  ifndef SvPVbyte_force
#    define SvPVbyte_force(sv,lp) SvPV_force(sv,lp)
#  endif
#endif

#if PERL_REVISION == 5 && (PERL_VERSION >= 8 || (PERL_VERSION == 8 && PERL_SUBVERSION < 4 ))
#    define UTF8_AVAILABLE
#endif

typedef int                     DualType ;
typedef int                     int_undef ;

typedef struct di_stream {
    int      flags ;
#define FLAG_APPEND             1
#define FLAG_CRC32              2
#define FLAG_ADLER32            4
#define FLAG_CONSUME_INPUT      8
#define FLAG_LIMIT_OUTPUT       16
    uLong    crc32 ;
    uLong    adler32 ;
    CRZ_ZSTREAM stream;
    uLong    bufsize;
    SV *     dictionary ;
    uLong    dict_adler ;
    int      last_error ;
    bool     zip_mode ;
/* #define SETP_BYTE */
#ifdef SETP_BYTE
    /* SETP_BYTE only works with zlib up to 1.2.8 */
    bool     deflateParams_out_valid ;
    Bytef    deflateParams_out_byte;
#else
#define deflateParams_BUFFER_SIZE       0x40000
    uLong    deflateParams_out_length;
    Bytef*   deflateParams_out_buffer;
#endif
    int      Level;
    int      Method;
    int      WindowBits;
    int      MemLevel;
    int      Strategy;
    uLong    bytesInflated ;
    uLong    compressedBytes ;
    uLong    uncompressedBytes ;
#ifdef MAGIC_APPEND

#define WINDOW_SIZE 32768U

    bool     matchedEndBlock;
    Bytef*   window ;
    int      window_lastbit,  window_left,  window_full;
    unsigned window_have;
    off_t    window_lastoff, window_end;
    off_t    window_endOffset;

    uLong    lastBlockOffset ;
    unsigned char window_lastByte ;


#endif
} di_stream;

typedef di_stream * deflateStream ;
typedef di_stream * Compress__Raw__Zlib__deflateStream ;
typedef di_stream * inflateStream ;
typedef di_stream * Compress__Raw__Zlib__inflateStream ;
typedef di_stream * Compress__Raw__Zlib__inflateScanStream ;

#define ZMALLOC(to, typ) (to = (typ *)safecalloc(sizeof(typ), 1))

/* Figure out the Operating System */
#ifdef MSDOS
#  define OS_CODE  0x00
#endif

#if defined(AMIGA) || defined(AMIGAOS) || defined(__amigaos4__)
#  define OS_CODE  0x01
#endif

#if defined(VAXC) || defined(VMS)
#  define OS_CODE  0x02
#endif

#if 0 /* VM/CMS */
#  define OS_CODE  0x04
#endif

#if defined(ATARI) || defined(atarist)
#  define OS_CODE  0x05
#endif

#ifdef OS2
#  define OS_CODE  0x06
#endif

#if defined(MACOS) || defined(TARGET_OS_MAC)
#  define OS_CODE  0x07
#endif

#if 0 /* Z-System */
#  define OS_CODE  0x08
#endif

#if 0 /* CP/M */
#  define OS_CODE  0x09
#endif

#ifdef TOPS20
#  define OS_CODE  0x0a
#endif

#ifdef WIN32 /* Window 95 & Windows NT */
#  define OS_CODE  0x0b
#endif

#if 0 /* QDOS */
#  define OS_CODE  0x0c
#endif

#if 0 /* Acorn RISCOS */
#  define OS_CODE  0x0d
#endif

#if 0 /* ???  */
#  define OS_CODE  0x0e
#endif

#ifdef __50SERIES /* Prime/PRIMOS */
#  define OS_CODE  0x0F
#endif

/* Default to UNIX */
#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif

#ifndef GZIP_OS_CODE
#  define GZIP_OS_CODE OS_CODE
#endif


/* static const char * const my_z_errmsg[] = { */
static const char my_z_errmsg[][32] = {
    "need dictionary",     /* Z_NEED_DICT     2 */
    "stream end",          /* Z_STREAM_END    1 */
    "",                    /* Z_OK            0 */
    "file error",          /* Z_ERRNO        (-1) */
    "stream error",        /* Z_STREAM_ERROR (-2) */
    "data error",          /* Z_DATA_ERROR   (-3) */
    "insufficient memory", /* Z_MEM_ERROR    (-4) */
    "buffer error",        /* Z_BUF_ERROR    (-5) */
    "incompatible version",/* Z_VERSION_ERROR(-6) */
    ""};

#define setDUALstatus(var, err)                                         \
                sv_setnv(var, (double)err) ;                            \
                sv_setpv(var, ((err) ? GetErrorString(err) : "")) ;     \
                SvNOK_on(var);


#if defined(__SYMBIAN32__)
# define NO_WRITEABLE_DATA
#endif

/* Set TRACE_DEFAULT to a non-zero value to enable tracing */
#define TRACE_DEFAULT 0

#if defined(NO_WRITEABLE_DATA) || TRACE_DEFAULT == 0
#  define trace TRACE_DEFAULT
#else
  static int trace = TRACE_DEFAULT ;
#endif

/* Dodge PerlIO hiding of these functions. */
#undef printf

static char *
#ifdef CAN_PROTOTYPE
GetErrorString(int error_no)
#else
GetErrorString(error_no)
int error_no ;
#endif
{
    dTHX;
    char * errstr ;

    if (error_no == Z_ERRNO) {
        errstr = Strerror(errno) ;
    }
    else
        /* errstr = gzerror(fil, &error_no) ; */
        errstr = (char*) my_z_errmsg[2 - error_no];

    return errstr ;
}


#ifdef MAGIC_APPEND

/*
   The following two functions are taken almost directly from
   examples/gzappend.c. Only cosmetic changes have been made to conform to
   the coding style of the rest of the code in this file.
*/


/* return the greatest common divisor of a and b using Euclid's algorithm,
   modified to be fast when one argument much greater than the other, and
   coded to avoid unnecessary swapping */
static unsigned
#ifdef CAN_PROTOTYPE
gcd(unsigned a, unsigned b)
#else
gcd(a, b)
    unsigned a;
    unsigned b;
#endif
{
    unsigned c;

    while (a && b)
        if (a > b) {
            c = b;
            while (a - c >= c)
                c <<= 1;
            a -= c;
        }
        else {
            c = a;
            while (b - c >= c)
                c <<= 1;
            b -= c;
        }
    return a + b;
}

/* rotate list[0..len-1] left by rot positions, in place */
static void
#ifdef CAN_PROTOTYPE
rotate(unsigned char *list, unsigned len, unsigned rot)
#else
rotate(list, len, rot)
    unsigned char *list;
    unsigned len ;
    unsigned rot;
#endif
{
    unsigned char tmp;
    unsigned cycles;
    unsigned char *start, *last, *to, *from;

    /* normalize rot and handle degenerate cases */
    if (len < 2) return;
    if (rot >= len) rot %= len;
    if (rot == 0) return;

    /* pointer to last entry in list */
    last = list + (len - 1);

    /* do simple left shift by one */
    if (rot == 1) {
        tmp = *list;
        memmove(list, list + 1, len - 1);
        *last = tmp;
        return;
    }

    /* do simple right shift by one */
    if (rot == len - 1) {
        tmp = *last;
        memmove(list + 1, list, len - 1);
        *list = tmp;
        return;
    }

    /* otherwise do rotate as a set of cycles in place */
    cycles = gcd(len, rot);             /* number of cycles */
    do {
        start = from = list + cycles;   /* start index is arbitrary */
        tmp = *from;                    /* save entry to be overwritten */
        for (;;) {
            to = from;                  /* next step in cycle */
            from += rot;                /* go right rot positions */
            if (from > last) from -= len;   /* (pointer better not wrap) */
            if (from == start) break;   /* all but one shifted */
            *to = *from;                /* shift left */
        }
        *to = tmp;                      /* complete the circle */
    } while (--cycles);
}

#endif /* MAGIC_APPEND */

static void
#ifdef CAN_PROTOTYPE
DispHex(const void * ptr, int length)
#else
DispHex(ptr, length)
    const void * ptr;
    int length;
#endif
{
    char * p = (char*)ptr;
    int i;
    for (i = 0; i < length; ++i) {
        printf(" %02x", 0xFF & *(p+i));
    }
}


static void
#ifdef CAN_PROTOTYPE
DispStream(di_stream * s, const char * message)
#else
DispStream(s, message)
    di_stream * s;
    const char * message;
#endif
{

#if 0
    if (! trace)
        return ;
#endif

#define EnDis(f) (s->flags & f ? "Enabled" : "Disabled")

    printf("DispStream %p", s) ;
    if (message)
        printf("- %s \n", message) ;
    printf("\n") ;

    if (!s)  {
        printf("    stream pointer is NULL\n");
    }
    else     {
        printf("    stream           %p\n", &(s->stream));
        printf("           zalloc    %p\n", s->stream.zalloc);
        printf("           zfree     %p\n", s->stream.zfree);
        printf("           opaque    %p\n", s->stream.opaque);
        printf("           state     %p\n", s->stream.state);
        if (s->stream.msg)
            printf("           msg       %s\n", s->stream.msg);
        else
            printf("           msg       \n");
        printf("           next_in   %p", s->stream.next_in);
        if (s->stream.next_in){
            printf(" =>");
            DispHex(s->stream.next_in, 4);
        }
        printf("\n");

        printf("           next_out  %p", s->stream.next_out);
        if (s->stream.next_out){
            printf(" =>");
            DispHex(s->stream.next_out, 4);
        }
        printf("\n");

        printf("           avail_in  %lu\n",  (unsigned long)s->stream.avail_in);
        printf("           avail_out %lu\n",  (unsigned long)s->stream.avail_out);
        printf("           total_in  %ld\n",  s->stream.total_in);
        printf("           total_out %ld\n",  s->stream.total_out);
#if ! USE_ZLIB_NG
        printf("           adler     %ld\n",  s->stream.adler    );
#else
        printf("           adler     %u\n",  s->stream.adler    );
#endif
        printf("    bufsize          %ld\n",  s->bufsize);
        printf("    dictionary       %p\n",   s->dictionary);
        printf("    dict_adler       0x%ld\n",s->dict_adler);
        printf("    zip_mode         %d\n",   s->zip_mode);
        printf("    crc32            0x%x\n", (unsigned)s->crc32);
        printf("    adler32          0x%x\n", (unsigned)s->adler32);
        printf("    flags            0x%x\n", s->flags);
        printf("           APPEND    %s\n",   EnDis(FLAG_APPEND));
        printf("           CRC32     %s\n",   EnDis(FLAG_CRC32));
        printf("           ADLER32   %s\n",   EnDis(FLAG_ADLER32));
        printf("           CONSUME   %s\n",   EnDis(FLAG_CONSUME_INPUT));
        printf("           LIMIT     %s\n",   EnDis(FLAG_LIMIT_OUTPUT));


#ifdef MAGIC_APPEND
        printf("    window           %p\n", s->window);
#endif
        printf("\n");

    }
}

#ifdef AT_LEAST_ZLIB_1_2_5_2
voidpf my_zcalloc (voidpf opaque, unsigned items, unsigned size)
{
    PERL_UNUSED_VAR(opaque);
    /* TODO - put back to calloc */
    /* return safecalloc(items, size); */
    return safemalloc(items* size);
}


void my_zcfree (voidpf opaque, voidpf ptr)
{
    PERL_UNUSED_VAR(opaque);
    safefree(ptr);
    return;
}

#endif

static di_stream *
#ifdef CAN_PROTOTYPE
InitStream(void)
#else
InitStream()
#endif
{
    di_stream *s ;

    ZMALLOC(s, di_stream) ;

#ifdef AT_LEAST_ZLIB_1_2_5_2
    s->stream.zalloc = my_zcalloc;
    s->stream.zfree = my_zcfree;
#endif

    return s ;
}

static void
#ifdef CAN_PROTOTYPE
PostInitStream(di_stream * s, int flags, int bufsize, int windowBits)
#else
PostInitStream(s, flags, bufsize, windowBits)
    di_stream *s ;
    int flags ;
    int bufsize ;
    int windowBits ;
#endif
{
    s->bufsize = bufsize ;
    s->compressedBytes =
    s->uncompressedBytes =
    s->last_error = 0 ;
    s->flags    = flags ;
    s->zip_mode = (windowBits < 0) ;
    if (flags & FLAG_CRC32)
        s->crc32 = CRZ_crcInitial ;
    if (flags & FLAG_ADLER32)
        s->adler32 = CRZ_adlerInitial ;
}


static SV*
#ifdef CAN_PROTOTYPE
deRef(SV * sv, const char * string)
#else
deRef(sv, string)
SV * sv ;
char * string;
#endif
{
    dTHX;
    SvGETMAGIC(sv);

    if (SvROK(sv)) {
        sv = SvRV(sv) ;
        SvGETMAGIC(sv);
        switch(SvTYPE(sv)) {
            case SVt_PVAV:
            case SVt_PVHV:
            case SVt_PVCV:
                croak("%s: buffer parameter is not a SCALAR reference", string);
            default:
                break;
        }
        if (SvROK(sv))
            croak("%s: buffer parameter is a reference to a reference", string) ;
    }

    if (!SvOK(sv))
        sv = sv_2mortal(newSVpv("", 0));

    return sv ;
}

static SV*
#ifdef CAN_PROTOTYPE
deRef_l(SV * sv, const char * string)
#else
deRef_l(sv, string)
SV * sv ;
char * string ;
#endif
{
    dTHX;
    bool wipe = 0 ;
    STRLEN na;

    SvGETMAGIC(sv);
    wipe = ! SvOK(sv) ;

    if (SvROK(sv)) {
        sv = SvRV(sv) ;
        SvGETMAGIC(sv);
        wipe = ! SvOK(sv) ;

        switch(SvTYPE(sv)) {
            case SVt_PVAV:
            case SVt_PVHV:
            case SVt_PVCV:
                croak("%s: buffer parameter is not a SCALAR reference", string);
            default:
                break;
        }
        if (SvROK(sv))
            croak("%s: buffer parameter is a reference to a reference", string) ;
    }

    if (SvREADONLY(sv) && PL_curcop != &PL_compiling)
        croak("%s: buffer parameter is read-only", string);

    SvUPGRADE(sv, SVt_PV);

    if (wipe)
        sv_setpv(sv, "") ;
    else
        (void)SvPVbyte_force(sv, na) ;

    return sv ;
}

#if 0
int
flushToBuffer(di_stream* s, int flush)
{
    dTHX;
    int ret ;
    CRZ_ZSTREAM * strm = &s->stream;

    Bytef* output = s->deflateParams_out_buffer ;

    strm->next_in = NULL;
    strm->avail_in = 0;

    uLong total_output = 0;
    uLong have = 0;

    do
    {
        if (output)
            output = (unsigned char *)saferealloc(output, total_output + s->bufsize);
        else
            output = (unsigned char *)safemalloc(s->bufsize);

        strm->next_out  = output + total_output;
        strm->avail_out = s->bufsize;

        ret = deflate(strm, flush);    /* no bad return value */
        //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        if(ret == Z_STREAM_ERROR)
        {
            safefree(output);
            return ret;
        }
        have = s->bufsize - strm->avail_out;
        total_output += have;

        //fprintf(stderr, "FLUSH %s %d, return %d\n", flush_flags[flush], have, ret);

    } while (strm->avail_out == 0);

    s->deflateParams_out_buffer = output;
    s->deflateParams_out_length = total_output;

    return Z_OK;
}
#endif

#ifndef SETP_BYTE
int
flushParams(di_stream* s)
{
    dTHX;
    int ret ;
    CRZ_ZSTREAM * strm = &s->stream;

    Bytef* output = s->deflateParams_out_buffer ;
    uLong total_output = s->deflateParams_out_length;
    uLong have = 0;

    strm->next_in = NULL;
    strm->avail_in = 0;


    do
    {
        if (output)
            output = (unsigned char *)saferealloc(output, total_output + s->bufsize);
        else
            output = (unsigned char *)safemalloc(s->bufsize);

        strm->next_out  = output + total_output;
        strm->avail_out = s->bufsize;

        ret = CRZ_deflateParams(&(s->stream), s->Level, s->Strategy);
        /* fprintf(stderr, "deflateParams %d %s %lu\n", ret,
            GetErrorString(ret),  s->bufsize - strm->avail_out); */

        if (ret == Z_STREAM_ERROR)
            break;

        have = s->bufsize - strm->avail_out;
        total_output += have;


    } while (ret == Z_BUF_ERROR) ;

    if(ret == Z_STREAM_ERROR)
        safefree(output);
    else
    {
        s->deflateParams_out_buffer = output;
        s->deflateParams_out_length = total_output;
    }

    return ret;
}
#endif /* ! SETP_BYTE */

#include "constants.h"

MODULE = Compress::Raw::Zlib PACKAGE = Compress::Raw::Zlib        PREFIX = Zip_

REQUIRE:	1.924
PROTOTYPES:	DISABLE

INCLUDE: constants.xs

BOOT:
#if ! USE_ZLIB_NG
    /* Check this version of zlib is == 1 */
    if (CRZ_zlibVersion()[0] != '1')
        croak("Compress::Raw::Zlib needs zlib version 1.x\n") ;
#endif

    {
        /* Create the $os_code scalar */
        SV * os_code_sv = perl_get_sv("Compress::Raw::Zlib::gzip_os_code", GV_ADDMULTI) ;
        sv_setiv(os_code_sv, GZIP_OS_CODE) ;
    }

    {
        /* BUILD_ZLIB  */
        SV * os_code_sv = perl_get_sv("Compress::Raw::Zlib::BUILD_ZLIB", GV_ADDMULTI) ;
        sv_setiv(os_code_sv, Perl_crz_BUILD_ZLIB) ;
    }

#define Zip_zlib_version()	(const char*)CRZ_zlib_version()
const char*
Zip_zlib_version()

const char*
zlibng_version()

#define Zip_is_zlib_native()	(! (HAVE_ZLIB_NG_NATIVE || HAVE_ZLIB_NG_COMPAT))
bool
Zip_is_zlib_native()

#define Zip_is_zlibng_native()	(bool)HAVE_ZLIB_NG_NATIVE
bool
Zip_is_zlibng_native()

#define Zip_is_zlibng_compat()	(bool)HAVE_ZLIB_NG_COMPAT
bool
Zip_is_zlibng_compat()

#define Zip_is_zlibng()	(bool)(HAVE_ZLIB_NG_NATIVE || HAVE_ZLIB_NG_COMPAT)
bool
Zip_is_zlibng()

unsigned
ZLIB_VERNUM()
    CODE:
#ifdef ZLIB_VERNUM
        RETVAL = ZLIB_VERNUM ;
#elif USE_ZLIB_NG
        RETVAL = 0 ;
#else
        /* 1.1.4 => 0x1140 */
        RETVAL  = (CRZ_ZLIB_VERSION[0] - '0') << 12 ;
        RETVAL += (CRZ_ZLIB_VERSION[2] - '0') <<  8 ;
        RETVAL += (CRZ_ZLIB_VERSION[4] - '0') <<  4 ;
        if (strlen(CRZ_ZLIB_VERSION) > 5)
            RETVAL += (CRZ_ZLIB_VERSION[6] - '0')  ;
#endif
    OUTPUT:
        RETVAL


#ifndef AT_LEAST_ZLIB_1_2_1
#  define Zip_zlibCompileFlags  0
#else
#  define Zip_zlibCompileFlags  CRZ_zlibCompileFlags
#endif
uLong
Zip_zlibCompileFlags()

MODULE = Compress::Raw::Zlib	PACKAGE = Compress::Raw::Zlib	PREFIX = Zip_

#define Zip_adler32(buf, adler) CRZ_adler32(adler, buf, (uInt)len)

uLong
Zip_adler32(buf, adler=CRZ_adlerInitial)
        uLong    adler = NO_INIT
        STRLEN   len = NO_INIT
        Bytef *  buf = NO_INIT
	SV *	 sv = ST(0) ;
	INIT:
    	/* If the buffer is a reference, dereference it */
	sv = deRef(sv, "adler32") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(sv) && !sv_utf8_downgrade(sv, 1))
         croak("Wide character in Compress::Raw::Zlib::adler32");
#endif
	buf = (Byte*)SvPVbyte(sv, len) ;

	if (items < 2)
	  adler = CRZ_adlerInitial;
	else if (SvOK(ST(1)))
	  adler = SvUV(ST(1)) ;
	else
	  adler = CRZ_adlerInitial;
    OUTPUT:
        RETVAL

#define Zip_crc32(buf, crc, offset) CRZ_crc32(crc, buf+offset, (uInt)len-offset)

uLong
Zip_crc32(buf, crc=CRZ_crcInitial, offset=0)
        uLong    crc = NO_INIT
        STRLEN   len = NO_INIT
        Bytef *  buf = NO_INIT
        STRLEN   offset
	SV *	 sv = ST(0) ;
	INIT:
    	/* If the buffer is a reference, dereference it */
	sv = deRef(sv, "crc32") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(sv) && !sv_utf8_downgrade(sv, 1))
         croak("Wide character in Compress::Raw::Zlib::crc32");
#endif
	buf = (Byte*)SvPVbyte(sv, len) ;

	if (offset > len)
	  croak("Offset out of range in Compress::Raw::Zlib::crc32");

	if (items < 2)
	  crc = CRZ_crcInitial;
	else if (SvOK(ST(1)))
	  crc = SvUV(ST(1)) ;
	else
	  crc = CRZ_crcInitial;

uLong
crc32_combine(crc1, crc2, len2)
        uLong    crc1
        uLong    crc2
        z_off_t   len2
	CODE:
#ifndef AT_LEAST_ZLIB_1_2_2_1
        crc1 = crc1; crc2 = crc2 ; len2 = len2; /* Silence -Wall */
        croak("crc32_combine needs zlib 1.2.3 or better");
#else
        RETVAL = CRZ_crc32_combine(crc1, crc2, len2);
#endif
    OUTPUT:
        RETVAL


uLong
adler32_combine(adler1, adler2, len2)
        uLong    adler1
        uLong    adler2
        z_off_t   len2
	CODE:
#ifndef AT_LEAST_ZLIB_1_2_2_1
        adler1 = adler1; adler2 = adler2 ; len2 = len2; /* Silence -Wall */
        croak("adler32_combine needs zlib 1.2.3 or better");
#else
        RETVAL = CRZ_adler32_combine(adler1, adler2, len2);
#endif
    OUTPUT:
        RETVAL


MODULE = Compress::Raw::Zlib PACKAGE = Compress::Raw::Zlib

void
_deflateInit(flags,level, method, windowBits, memLevel, strategy, bufsize, dictionary)
    int flags
    int	level
    int method
    int windowBits
    int memLevel
    int strategy
    uLong bufsize
    SV* dictionary
  PPCODE:
    int err ;
    deflateStream s ;

    if (trace)
        warn("in _deflateInit(level=%d, method=%d, windowBits=%d, memLevel=%d, strategy=%d, bufsize=%ld dictionary=%p)\n",
	level, method, windowBits, memLevel, strategy, bufsize, dictionary) ;
    if ((s = InitStream() )) {

        s->Level      = level;
        s->Method     = method;
        s->WindowBits = windowBits;
        s->MemLevel   = memLevel;
        s->Strategy   = strategy;

        err = CRZ_deflateInit2(&(s->stream), level,
			   method, windowBits, memLevel, strategy);

        if (trace) {
            warn(" _deflateInit2 returned %d (state %p)\n", err, s);
            DispStream(s, "INIT");
        }

	/* Check if a dictionary has been specified */
	SvGETMAGIC(dictionary);
	if (err == Z_OK && SvPOK(dictionary) && SvCUR(dictionary)) {
#ifdef UTF8_AVAILABLE
            if (DO_UTF8(dictionary) && !sv_utf8_downgrade(dictionary, 1))
                croak("Wide character in Compress::Raw::Zlib::Deflate::new dicrionary parameter");
#endif
	    err = CRZ_deflateSetDictionary(&(s->stream), (const Bytef*) SvPVX(dictionary), SvCUR(dictionary)) ;
        if (trace)
            warn("deflateSetDictionary returned %d\n", err);
	    s->dict_adler = s->stream.adler ;
	}

        if (err != Z_OK) {
            Safefree(s) ;
            s = NULL ;
	}
	else
	    PostInitStream(s, flags, bufsize, windowBits) ;

    }
    else
        err = Z_MEM_ERROR ;

    {
        SV* obj = sv_setref_pv(sv_newmortal(),
            "Compress::Raw::Zlib::deflateStream", (void*)s);
        XPUSHs(obj);
    }
    if (GIMME_V == G_ARRAY) {
        SV * sv = sv_2mortal(newSViv(err)) ;
	setDUALstatus(sv, err);
        XPUSHs(sv) ;
    }

void
_inflateInit(flags, windowBits, bufsize, dictionary)
    int flags
    int windowBits
    uLong bufsize
    SV * dictionary
  ALIAS:
    _inflateScanInit = 1
  PPCODE:

    int err = Z_OK ;
    inflateStream s ;
#ifndef MAGIC_APPEND
    if (ix == 1)
        croak("inflateScanInit needs zlib 1.2.1 or better");
#endif
    if (trace)
        warn("in _inflateInit(windowBits=%d, bufsize=%lu, dictionary=%lu\n",
                windowBits, bufsize, (unsigned long)SvCUR(dictionary)) ;
    if ((s = InitStream() )) {

        s->WindowBits = windowBits;

        err = CRZ_inflateInit2(&(s->stream), windowBits);
        if (err != Z_OK) {
            Safefree(s) ;
            s = NULL ;
	}
	else if (sv_len(dictionary)) {
#ifdef AT_LEAST_ZLIB_1_2_2_1
        /* Zlib 1.2.2.1 or better allows a dictionary with raw inflate */
        if (s->WindowBits < 0) {
            STRLEN dlen;
            const Bytef* b = (const Bytef*)SvPVbyte(dictionary, dlen);
            err = CRZ_inflateSetDictionary(&(s->stream),
                b, dlen);
            if (err != Z_OK) {
                Safefree(s) ;
                s = NULL ;
            }
        }
        else
#endif
            /* Dictionary specified - take a copy for use in inflate */
	    s->dictionary = newSVsv(dictionary) ;
	}
	if (s) {
	    PostInitStream(s, flags, bufsize, windowBits) ;
#ifdef MAGIC_APPEND
            if (ix == 1)
            {
                s->window = (unsigned char *)safemalloc(WINDOW_SIZE);
            }
#endif
        }
    }
    else
	err = Z_MEM_ERROR ;

    {
        SV* obj = sv_setref_pv(sv_newmortal(),
                   ix == 1
                   ? "Compress::Raw::Zlib::inflateScanStream"
                   :  "Compress::Raw::Zlib::inflateStream",
                   (void*)s);
        XPUSHs(obj);
    }
    if (GIMME_V == G_ARRAY) {
        SV * sv = sv_2mortal(newSViv(err)) ;
	setDUALstatus(sv, err);
        XPUSHs(sv) ;
    }



MODULE = Compress::Raw::Zlib PACKAGE = Compress::Raw::Zlib::deflateStream

void
DispStream(s, message=NULL)
    Compress::Raw::Zlib::deflateStream   s
    const char *  message

DualType
deflateReset(s)
    Compress::Raw::Zlib::deflateStream   s
  CODE:
      RETVAL = CRZ_deflateReset(&(s->stream)) ;
      if (RETVAL == Z_OK) {
	  PostInitStream(s, s->flags, s->bufsize, s->WindowBits) ;
      }
    OUTPUT:
      RETVAL

DualType
deflate (s, buf, output)
    Compress::Raw::Zlib::deflateStream	s
    SV *	buf
    SV * 	output
    uInt	cur_length = NO_INIT
    uInt	increment = NO_INIT
    uInt	prefix    = NO_INIT
    int		RETVAL = 0;
    uLong     bufinc = NO_INIT
    STRLEN    origlen = NO_INIT
  CODE:
    bufinc = s->bufsize;

    /* If the input buffer is a reference, dereference it */
    buf = deRef(buf, "deflate") ;

    /* initialise the input buffer */
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(buf) && !sv_utf8_downgrade(buf, 1))
         croak("Wide character in Compress::Raw::Zlib::Deflate::deflate input parameter");
#endif
    s->stream.next_in = (Bytef*)SvPV_nomg(buf, origlen) ;
    s->stream.avail_in = origlen;

    if (s->flags & FLAG_CRC32)
        s->crc32 = CRZ_crc32(s->crc32, s->stream.next_in, s->stream.avail_in) ;

    if (s->flags & FLAG_ADLER32)
        s->adler32 = CRZ_adler32(s->adler32, s->stream.next_in, s->stream.avail_in) ;

    /* and retrieve the output buffer */
    output = deRef_l(output, "deflate") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(output) && !sv_utf8_downgrade(output, 1))
         croak("Wide character in Compress::Raw::Zlib::Deflate::deflate output parameter");
#endif

     if((s->flags & FLAG_APPEND) == FLAG_APPEND) {
         SvOOK_off(output);
     } else {
         SvCUR_set(output, 0);
     }
    prefix = cur_length =  SvCUR(output) ;
    s->stream.next_out = (Bytef*) SvPVX(output) + cur_length;
    increment =  SvLEN(output) -  cur_length;
    s->stream.avail_out =  increment;
#ifdef SETP_BYTE
    /* Check for saved output from deflateParams */
    if (s->deflateParams_out_valid) {
	*(s->stream.next_out) = s->deflateParams_out_byte;
	++ s->stream.next_out;
	-- s->stream.avail_out ;
	s->deflateParams_out_valid = FALSE;
    }
#else
    /* Check for saved output from deflateParams */
    if (s->deflateParams_out_length) {
        uLong plen = s->deflateParams_out_length ;
        /* printf("Copy %lu bytes saved data\n", plen); */
        if (s->stream.avail_out < plen) {
            /* printf("GROW from %d to %lu\n", s->stream.avail_out,
                        SvLEN(output) + plen - s->stream.avail_out);  */
             s->stream.next_out = (Bytef*) Sv_Grow(output, SvLEN(output) + plen - s->stream.avail_out) ;
             s->stream.next_out += cur_length;
        }

        Copy(s->deflateParams_out_buffer, s->stream.next_out, plen, Bytef) ;
        cur_length += plen;
        SvCUR_set(output, cur_length);
        s->stream.next_out += plen ;
        s->stream.avail_out = SvLEN(output) - cur_length ;
        increment = s->stream.avail_out;

        s->deflateParams_out_length = 0;
        Safefree(s->deflateParams_out_buffer);
        s->deflateParams_out_buffer = NULL;
    }
#endif
    RETVAL = Z_OK ;
    while (s->stream.avail_in != 0) {

        if (s->stream.avail_out == 0) {
	    /* out of space in the output buffer so make it bigger */
            s->stream.next_out = (Bytef*) Sv_Grow(output, SvLEN(output) + bufinc) ;
            cur_length += increment ;
            s->stream.next_out += cur_length ;
            increment = bufinc ;
            s->stream.avail_out = increment;
            bufinc *= 2 ;
        }

        if (trace) {
          printf("DEFLATE Avail In %d, Out %d\n", s->stream.avail_in, s->stream.avail_out);
          DispStream(s, "BEFORE");
          /* Perl_sv_dump(output); */
        }

        RETVAL = CRZ_deflate(&(s->stream), Z_NO_FLUSH);
        /*
        if (RETVAL != Z_STREAM_ERROR) {
            int done = increment -  s->stream.avail_out ;
            printf("std DEFLATEr returned %d '%s'  avail in %d, out %d wrote %d\n", RETVAL,
            GetErrorString(RETVAL), s->stream.avail_in, s->stream.avail_out, done);
        }
        */

        if (trace) {
            printf("DEFLATE returned %d %s, avail in %d, out %d\n", RETVAL,
           GetErrorString(RETVAL), s->stream.avail_in, s->stream.avail_out);
            DispStream(s, "AFTER");
        }

        if (RETVAL != Z_OK)
            break;
    }

    s->compressedBytes += cur_length + increment - prefix - s->stream.avail_out ;
    s->uncompressedBytes  += origlen - s->stream.avail_in  ;

    s->last_error = RETVAL ;
    if (RETVAL == Z_OK) {
        SvPOK_only(output);
        SvCUR_set(output, cur_length + increment - s->stream.avail_out) ;
        SvSETMAGIC(output);
    }
    OUTPUT:
	RETVAL


void
DESTROY(s)
    Compress::Raw::Zlib::deflateStream	s
  CODE:
    if (trace)
        printf("Compress::Raw::Zlib::deflateStream::DESTROY %p\n", s);
    CRZ_deflateEnd(&s->stream) ;
    if (s->dictionary)
	SvREFCNT_dec(s->dictionary) ;
#ifndef SETP_BYTE
    if (s->deflateParams_out_buffer)
        Safefree(s->deflateParams_out_buffer);
#endif
    Safefree(s) ;


DualType
flush(s, output, f=Z_FINISH)
    Compress::Raw::Zlib::deflateStream	s
    SV * output
    int  f
    uInt	cur_length = NO_INIT
    uInt	increment = NO_INIT
    uInt	prefix    = NO_INIT
    uLong     bufinc = NO_INIT
    uLong     availableout = NO_INIT
  CODE:
    bufinc = s->bufsize;



    /* retrieve the output buffer */
    output = deRef_l(output, "flush") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(output) && !sv_utf8_downgrade(output, 1))
         croak("Wide character in Compress::Raw::Zlib::Deflate::flush input parameter");
#endif
     if((s->flags & FLAG_APPEND) == FLAG_APPEND) {
         SvOOK_off(output);
     } else {
         SvCUR_set(output, 0);
     }
    prefix = cur_length =  SvCUR(output) ;
    s->stream.next_out = (Bytef*) SvPVX(output) + cur_length;
    increment =  SvLEN(output) -  cur_length;
    s->stream.avail_out =  increment;
#ifdef SETP_BYTE
    /* Check for saved output from deflateParams */
    if (s->deflateParams_out_valid) {
	*(s->stream.next_out) = s->deflateParams_out_byte;
	++ s->stream.next_out;
	-- s->stream.avail_out ;
	s->deflateParams_out_valid = FALSE;
    }
#else
    /* Check for saved output from deflateParams */
    if (s->deflateParams_out_length) {
        uLong plen = s->deflateParams_out_length ;
        /* printf("Copy %lu bytes saved data\n", plen); */
        if (s->stream.avail_out < plen) {
            /* printf("GROW from %d to %lu\n", s->stream.avail_out,
                        SvLEN(output) + plen - s->stream.avail_out); */
            s->stream.next_out = (Bytef*) Sv_Grow(output, SvLEN(output) + plen - s->stream.avail_out) ;
            s->stream.next_out += cur_length;
        }

        Copy(s->deflateParams_out_buffer, s->stream.next_out, plen, Bytef) ;
        cur_length += plen;
        SvCUR_set(output, cur_length);
        s->stream.next_out += plen ;
        s->stream.avail_out = SvLEN(output) - cur_length ;
        increment = s->stream.avail_out;

        s->deflateParams_out_length = 0;
        Safefree(s->deflateParams_out_buffer);
        s->deflateParams_out_buffer = NULL;
    }
#endif

    for (;;) {
        if (s->stream.avail_out == 0) {
            /* consumed all the available output, so extend it */
            s->stream.next_out = (Bytef*) Sv_Grow(output, SvLEN(output) + bufinc) ;
            cur_length += increment ;
            s->stream.next_out += cur_length ;
            increment = bufinc ;
            s->stream.avail_out = increment;
            bufinc *= 2 ;
        }

        availableout = s->stream.avail_out ;

        if (trace) {
          printf("flush (%d) DEFLATE Avail In %d, Out %d\n", f, s->stream.avail_in, s->stream.avail_out);
          DispStream(s, "BEFORE");
          /* Perl_sv_dump(output); */
        }

        RETVAL = CRZ_deflate(&(s->stream), f);
        /*
        if (RETVAL != Z_STREAM_ERROR) {
            int done = availableout -  s->stream.avail_out ;
            printf("flush DEFLATEr returned %d '%s'  avail in %d, out %d wrote %d\n", RETVAL,
            GetErrorString(RETVAL), s->stream.avail_in,
s->stream.avail_out, done);
        }
        */

        if (trace) {
            printf("flush DEFLATE returned %d '%s', avail in %d, out %d\n", RETVAL,
            GetErrorString(RETVAL), s->stream.avail_in, s->stream.avail_out);
            DispStream(s, "AFTER");
        }

        /* Ignore the second of two consecutive flushes: */
        if (availableout == s->stream.avail_out && RETVAL == Z_BUF_ERROR)
            RETVAL = Z_OK;

        /* deflate has finished flushing only when it hasn't used up
         * all the available space in the output buffer:
         */
        if (s->stream.avail_out != 0 || RETVAL != Z_OK )
            break;
    }

    RETVAL =  (RETVAL == Z_STREAM_END ? Z_OK : RETVAL) ;
    s->last_error = RETVAL ;

    s->compressedBytes    += cur_length + increment - prefix - s->stream.avail_out ;

    if (RETVAL == Z_OK) {
        SvPOK_only(output);
        SvCUR_set(output, cur_length + increment - s->stream.avail_out) ;
        SvSETMAGIC(output);
    }
    OUTPUT:
	RETVAL


DualType
_deflateParams(s, flags, level, strategy, bufsize)
  	Compress::Raw::Zlib::deflateStream	s
	int 	flags
	int	level
	int	strategy
    	uLong	bufsize
	bool changed = FALSE;
    CODE:
        /* printf("_deflateParams(Flags %d Level %d Strategy %d Bufsize %d)\n", flags, level, strategy, bufsize);
        printf("Before -- Level %d, Strategy %d, Bufsize %d\n", s->Level, s->Strategy, s->bufsize); */
        if (flags & 1 && level != s->Level) {
            s->Level = level ;
            changed = TRUE;
        }
        if (flags & 2 && strategy != s->Strategy) {
            s->Strategy = strategy ;
            changed = TRUE;
        }
        if (flags & 4)
            s->bufsize = bufsize;
        if (changed) {
#ifdef SETP_BYTE
            s->stream.avail_in = 0;
            s->stream.next_out = &(s->deflateParams_out_byte) ;
            s->stream.avail_out = 1;
            RETVAL = deflateParams(&(s->stream), s->Level, s->Strategy);
            s->deflateParams_out_valid =
            (RETVAL == Z_OK && s->stream.avail_out == 0) ;
#else
            /* printf("Level %d Strategy %d, Prev Len %d\n",
                s->Level, s->Strategy, s->deflateParams_out_length); */
            RETVAL = flushParams(s);
#endif
        }
        else
            RETVAL = Z_OK;
    OUTPUT:
        RETVAL


int
get_Level(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->Level ;
    OUTPUT:
	RETVAL

int
get_Strategy(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->Strategy ;
    OUTPUT:
	RETVAL


uLong
get_Bufsize(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->bufsize ;
    OUTPUT:
	RETVAL


int
status(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->last_error ;
    OUTPUT:
	RETVAL

uLong
crc32(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->crc32 ;
    OUTPUT:
	RETVAL

uLong
dict_adler(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->dict_adler ;
    OUTPUT:
	RETVAL

uLong
adler32(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = s->adler32 ;
    OUTPUT:
	RETVAL

uLong
compressedBytes(s)
    Compress::Raw::Zlib::deflateStream	s
    CODE:
        RETVAL = s->compressedBytes;
  OUTPUT:
	RETVAL

uLong
uncompressedBytes(s)
    Compress::Raw::Zlib::deflateStream	s
    CODE:
        RETVAL = s->uncompressedBytes;
  OUTPUT:
	RETVAL

uLong
total_in(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
        RETVAL = s->stream.total_in ;
    OUTPUT:
	RETVAL

uLong
total_out(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
        RETVAL = s->stream.total_out ;
    OUTPUT:
	RETVAL

char*
msg(s)
        Compress::Raw::Zlib::deflateStream   s
    CODE:
	RETVAL = (char*)s->stream.msg;
    OUTPUT:
	RETVAL

int
deflateTune(s, good_length, max_lazy, nice_length, max_chain)
            Compress::Raw::Zlib::deflateStream   s
            int good_length
            int max_lazy
            int nice_length
            int max_chain
    CODE:
#ifndef AT_LEAST_ZLIB_1_2_2_3
        good_length = good_length; max_lazy = max_lazy ; /* Silence -Wall */
        nice_length = nice_length; max_chain = max_chain; /* Silence -Wall */
        croak("deflateTune needs zlib 1.2.2.3 or better");
#else
	RETVAL = CRZ_deflateTune(&(s->stream), good_length, max_lazy, nice_length, max_chain);
#endif
    OUTPUT:
	RETVAL


MODULE = Compress::Raw::Zlib PACKAGE = Compress::Raw::Zlib::inflateStream

void
DispStream(s, message=NULL)
    Compress::Raw::Zlib::inflateStream   s
    const char *  message

DualType
inflateReset(s)
    Compress::Raw::Zlib::inflateStream   s
  CODE:
      RETVAL = CRZ_inflateReset(&(s->stream)) ;
      if (RETVAL == Z_OK) {
	  PostInitStream(s, s->flags, s->bufsize, s->WindowBits) ;
      }
    OUTPUT:
      RETVAL

DualType
inflate (s, buf, output, eof=FALSE)
    Compress::Raw::Zlib::inflateStream	s
    SV *	buf
    SV * 	output
    bool 	eof
    uInt	cur_length = 0;
    uInt	prefix_length = 0;
    int	    increment = 0;
    uLong   bufinc = NO_INIT
    STRLEN  na = NO_INIT ;
  PREINIT:
#ifdef UTF8_AVAILABLE
    bool	out_utf8  = FALSE;
#endif
    STRLEN	origlen;
  CODE:
    bufinc = s->bufsize;
    /* If the buffer is a reference, dereference it */
    buf = deRef(buf, "inflate") ;

    if (s->flags & FLAG_CONSUME_INPUT) {
        if (SvREADONLY(buf))
            croak("Compress::Raw::Zlib::Inflate::inflate input parameter cannot be read-only when ConsumeInput is specified");
        SvPV_force(buf, na);
    }
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(buf) && !sv_utf8_downgrade(buf, 1))
         croak("Wide character in Compress::Raw::Zlib::Inflate::inflate input parameter");
#endif

    /* initialise the input buffer */
    s->stream.next_in = (Bytef*)SvPV_nomg(buf, origlen) ;
    s->stream.avail_in = origlen ;

    /* and retrieve the output buffer */
    output = deRef_l(output, "inflate") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(output))
         out_utf8 = TRUE ;
    if (DO_UTF8(output) && !sv_utf8_downgrade(output, 1))
         croak("Wide character in Compress::Raw::Zlib::Inflate::inflate output parameter");
#endif
     if((s->flags & FLAG_APPEND) == FLAG_APPEND) {
         SvOOK_off(output);
     } else {
         SvCUR_set(output, 0);
     }

    /* Assume no output buffer - the code below will update if there is any available */
    s->stream.avail_out = 0;


    if (SvLEN(output)) {
        prefix_length = cur_length =  SvCUR(output) ;

        if (s->flags & FLAG_LIMIT_OUTPUT && SvLEN(output) - cur_length - 1 < bufinc)
        {
            Sv_Grow(output, bufinc + cur_length + 1) ;
        }

        /* Only setup the stream output pointers if there is spare
           capacity in the outout SV
        */
        if (SvLEN(output) > cur_length + 1)
        {
            s->stream.next_out = (Bytef*) SvPV_nomg_nolen(output) + cur_length;
            increment = SvLEN(output) -  cur_length - 1;
            s->stream.avail_out = increment;
        }
    }


    s->bytesInflated = 0;

    RETVAL = Z_OK;

    while (RETVAL == Z_OK) {
        if (s->stream.avail_out == 0) {
	    /* out of space in the output buffer so make it bigger */
            s->stream.next_out = (Bytef*) Sv_Grow(output, SvLEN(output) + bufinc +1) ;
            cur_length += increment ;
            s->stream.next_out += cur_length ;
            increment = bufinc ;
            s->stream.avail_out = increment;
            bufinc *= 2 ;
        }

        /* printf("INFLATE Availl In %d, Out %d\n", s->stream.avail_in,
 s->stream.avail_out);
DispStream(s, "BEFORE");
Perl_sv_dump(output); */
        RETVAL = CRZ_inflate(&(s->stream), Z_SYNC_FLUSH);
        /* printf("INFLATE returned %d %s, avail in %d, out %d\n", RETVAL,
 GetErrorString(RETVAL), s->stream.avail_in, s->stream.avail_out); */


        if (RETVAL == Z_NEED_DICT && s->dictionary) {
            STRLEN dlen;
            const Bytef* b = (const Bytef*)SvPV(s->dictionary, dlen) ;
            s->dict_adler = s->stream.adler ;
            RETVAL = CRZ_inflateSetDictionary(&(s->stream),
                b, dlen);
            if (RETVAL == Z_OK)
                continue;
        }

        if (s->flags & FLAG_LIMIT_OUTPUT &&
                (RETVAL == Z_OK || RETVAL == Z_BUF_ERROR )) {
            if (s->stream.avail_out == 0)
                RETVAL = Z_BUF_ERROR;
            break;
        }
        if (s->flags & FLAG_LIMIT_OUTPUT &&
                (RETVAL == Z_OK || RETVAL == Z_BUF_ERROR ))
            break;

        if (RETVAL == Z_STREAM_ERROR || RETVAL == Z_MEM_ERROR ||
            RETVAL == Z_DATA_ERROR   || RETVAL == Z_STREAM_END )
            break ;

        if (RETVAL == Z_BUF_ERROR) {
            if (s->stream.avail_out == 0)
                continue ;
            if (s->stream.avail_in == 0) {
                RETVAL = Z_OK ;
                break ;
            }
        }
    }
#ifdef NEED_DUMMY_BYTE_AT_END
    if (eof && RETVAL == Z_OK && (s->flags & FLAG_LIMIT_OUTPUT) == 0) {
        Bytef* nextIn =  (Bytef*)s->stream.next_in;
        uInt availIn =  s->stream.avail_in;
        s->stream.next_in = (Bytef*) " ";
        s->stream.avail_in = 1;
        if (s->stream.avail_out == 0) {
	    /* out of space in the output buffer so make it bigger */
            s->stream.next_out = Sv_Grow(output, SvLEN(output) + bufinc) ;
            cur_length += increment ;
            s->stream.next_out += cur_length ;
            increment = bufinc ;
            s->stream.avail_out = increment;
            bufinc *= 2 ;
        }
        RETVAL = CRZ_inflate(&(s->stream), Z_SYNC_FLUSH);
        s->stream.next_in = nextIn ;
        s->stream.avail_in  = availIn ;
    }
#else
    PERL_UNUSED_VAR(eof);
#endif

    s->last_error = RETVAL ;
    if (RETVAL == Z_OK || RETVAL == Z_STREAM_END || RETVAL == Z_BUF_ERROR || RETVAL == Z_DATA_ERROR) {
	   unsigned in ;

        s->bytesInflated = cur_length + increment - s->stream.avail_out - prefix_length;
        s->uncompressedBytes += s->bytesInflated ;
        s->compressedBytes   += origlen - s->stream.avail_in  ;

        SvPOK_only(output);
        SvCUR_set(output, prefix_length + s->bytesInflated) ;
	*SvEND(output) = '\0';
#ifdef UTF8_AVAILABLE
        if (out_utf8)
            sv_utf8_upgrade(output);
#endif
        SvSETMAGIC(output);

        if (s->flags & FLAG_CRC32 )
            s->crc32 = CRZ_crc32(s->crc32,
				(const Bytef*)SvPVX(output)+prefix_length,
            			SvCUR(output)-prefix_length) ;

        if (s->flags & FLAG_ADLER32)
            s->adler32 = CRZ_adler32(s->adler32,
				(const Bytef*)SvPVX(output)+prefix_length,
            			SvCUR(output)-prefix_length) ;

	/* fix the input buffer */
	if (s->flags & FLAG_CONSUME_INPUT || s->flags & FLAG_LIMIT_OUTPUT) {
	    in = s->stream.avail_in ;
	    SvCUR_set(buf, in) ;
	    if (in)
	        Move(s->stream.next_in, SvPVX(buf), in, char) ;
            *SvEND(buf) = '\0';
            SvSETMAGIC(buf);
	}

    }
    OUTPUT:
	RETVAL

uLong
inflateCount(s)
    Compress::Raw::Zlib::inflateStream	s
    CODE:
        RETVAL = s->bytesInflated;
  OUTPUT:
	RETVAL

uLong
compressedBytes(s)
    Compress::Raw::Zlib::inflateStream	s
    CODE:
        RETVAL = s->compressedBytes;
  OUTPUT:
	RETVAL

uLong
uncompressedBytes(s)
    Compress::Raw::Zlib::inflateStream	s
    CODE:
        RETVAL = s->uncompressedBytes;
  OUTPUT:
	RETVAL


DualType
inflateSync (s, buf)
    Compress::Raw::Zlib::inflateStream	s
    SV *	buf
  CODE:

    /* If the buffer is a reference, dereference it */
    buf = deRef(buf, "inflateSync") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(buf) && !sv_utf8_downgrade(buf, 1))
         croak("Wide character in Compress::Raw::Zlib::Inflate::inflateSync");
#endif

    /* initialise the input buffer */
    s->stream.next_in = (Bytef*)SvPV_force_nomg_nolen(buf) ;
    s->stream.avail_in = SvCUR(buf) ;

    /* inflateSync doesn't create any output */
    s->stream.next_out = (Bytef*) NULL;
    s->stream.avail_out = 0;

    RETVAL = CRZ_inflateSync(&(s->stream));
    s->last_error = RETVAL ;

    /* fix the input buffer */
    {
	unsigned in = s->stream.avail_in ;
 	SvCUR_set(buf, in) ;
 	if (in)
     	    Move(s->stream.next_in, SvPVX(buf), in, char) ;
        *SvEND(buf) = '\0';
        SvSETMAGIC(buf);
    }
    OUTPUT:
	RETVAL

void
DESTROY(s)
    Compress::Raw::Zlib::inflateStream	s
  CODE:
    CRZ_inflateEnd(&s->stream) ;
    if (s->dictionary)
	SvREFCNT_dec(s->dictionary) ;
#ifndef SETP_BYTE
    if (s->deflateParams_out_buffer)
        Safefree(s->deflateParams_out_buffer);
#endif
#ifdef MAGIC_APPEND
    if (s->window)
        Safefree(s->window);
#endif
    Safefree(s) ;


uLong
status(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
	RETVAL = s->last_error ;
    OUTPUT:
	RETVAL

uLong
crc32(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
	RETVAL = s->crc32 ;
    OUTPUT:
	RETVAL

uLong
dict_adler(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
	RETVAL = s->dict_adler ;
    OUTPUT:
	RETVAL

uLong
total_in(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
        RETVAL = s->stream.total_in ;
    OUTPUT:
	RETVAL

uLong
adler32(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
	RETVAL = s->adler32 ;
    OUTPUT:
	RETVAL

uLong
total_out(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
        RETVAL = s->stream.total_out ;
    OUTPUT:
	RETVAL

char*
msg(s)
	Compress::Raw::Zlib::inflateStream   s
    CODE:
	RETVAL = (char*)s->stream.msg;
    OUTPUT:
	RETVAL


uLong
get_Bufsize(s)
        Compress::Raw::Zlib::inflateStream   s
    CODE:
	RETVAL = s->bufsize ;
    OUTPUT:
	RETVAL

bool
set_Append(s, mode)
        Compress::Raw::Zlib::inflateStream   s
	bool	mode
    CODE:
        RETVAL = ((s->flags & FLAG_APPEND) == FLAG_APPEND);
	if (mode)
	    s->flags |= FLAG_APPEND ;
	else
	    s->flags &= ~FLAG_APPEND ;
    OUTPUT:
        RETVAL

MODULE = Compress::Raw::Zlib PACKAGE = Compress::Raw::Zlib::inflateScanStream

void
DESTROY(s)
    Compress::Raw::Zlib::inflateScanStream	s
  CODE:
    CRZ_inflateEnd(&s->stream) ;
    if (s->dictionary)
	SvREFCNT_dec(s->dictionary) ;
#ifndef SETP_BYTE
    if (s->deflateParams_out_buffer)
        Safefree(s->deflateParams_out_buffer);
#endif
#ifdef MAGIC_APPEND
    if (s->window)
        Safefree(s->window);
#endif
    Safefree(s) ;

void
DispStream(s, message=NULL)
    Compress::Raw::Zlib::inflateScanStream   s
    const char *  message

DualType
inflateReset(s)
    Compress::Raw::Zlib::inflateScanStream   s
  CODE:
      RETVAL = CRZ_inflateReset(&(s->stream)) ;
      if (RETVAL == Z_OK) {
	  PostInitStream(s, s->flags, s->bufsize, s->WindowBits) ;
      }
    OUTPUT:
      RETVAL

DualType
scan(s, buf, out=NULL, eof=FALSE)
    Compress::Raw::Zlib::inflateScanStream	s
    SV *	buf
    SV *	out
    bool	eof
    bool	eof_mode = FALSE;
    int    start_len = NO_INIT
  CODE:
    PERL_UNUSED_VAR(out);
    PERL_UNUSED_VAR(eof);
    /* If the input buffer is a reference, dereference it */
#ifndef MAGIC_APPEND
        buf = buf;
        croak("scan needs zlib 1.2.1 or better");
#else
    buf = deRef(buf, "inflateScan") ;
#ifdef UTF8_AVAILABLE
    if (DO_UTF8(buf) && !sv_utf8_downgrade(buf, 1))
        croak("Wide character in Compress::Raw::Zlib::InflateScan::scan input parameter");
#endif
    /* initialise the input buffer */
    s->stream.next_in = (Bytef*)SvPV_force_nomg_nolen(buf) ;
    s->stream.avail_in = SvCUR(buf) ;
    start_len = s->stream.avail_in ;
    s->bytesInflated = 0 ;
    do
    {
        if (s->stream.avail_in == 0) {
            RETVAL = Z_OK ;
            break ;
        }

        /* set up output to next available section of sliding window */
        s->stream.avail_out = WINDOW_SIZE - s->window_have;
        s->stream.next_out = s->window + s->window_have;

        /* DispStream(s, "before inflate\n"); */

        /* inflate and check for errors */
        RETVAL = CRZ_inflate(&(s->stream), Z_BLOCK);

        if (start_len > 1 && ! eof_mode)
            s->window_lastByte = *(s->stream.next_in - 1 ) ;

        if (RETVAL == Z_STREAM_ERROR || RETVAL == Z_MEM_ERROR ||
            RETVAL == Z_DATA_ERROR )
            break ;

        if (s->flags & FLAG_CRC32 )
            s->crc32 = CRZ_crc32(s->crc32, s->window + s->window_have,
                             WINDOW_SIZE - s->window_have - s->stream.avail_out);

        if (s->flags & FLAG_ADLER32)
            s->adler32 = CRZ_adler32(s->adler32, s->window + s->window_have,
                                 WINDOW_SIZE - s->window_have - s->stream.avail_out);

        s->uncompressedBytes =
        s->bytesInflated += WINDOW_SIZE - s->window_have - s->stream.avail_out;

        if (s->stream.avail_out)
            s->window_have = WINDOW_SIZE - s->stream.avail_out;
        else {
            s->window_have = 0;
            s->window_full = 1;
        }

        /* process end of block */
        if (s->stream.data_type & 128) {
            if (s->stream.data_type & 64) {
                s->window_left = s->stream.data_type & 0x1f;
            }
            else {
                s->window_lastbit = s->stream.data_type & 0x1f;
                s->lastBlockOffset = s->stream.total_in;
            }
        }

    } while (RETVAL != Z_STREAM_END);

    s->last_error = RETVAL ;
    s->window_lastoff = s->stream.total_in ;
    s->compressedBytes += SvCUR(buf) - s->stream.avail_in  ;

    if (RETVAL == Z_STREAM_END)
    {
        s->matchedEndBlock = 1 ;

        /* save the location of the end of the compressed data */
        s->window_end = SvCUR(buf) - s->stream.avail_in - 1 ;
        s->window_endOffset = s->stream.total_in ;
        if (s->window_left)
        {
            -- s->window_endOffset ;
        }

        /* if window wrapped, build dictionary from window by rotating */
        if (s->window_full) {
            rotate(s->window, WINDOW_SIZE, s->window_have);
            s->window_have = WINDOW_SIZE;
        }

        /* if (s->flags & FLAG_CONSUME_INPUT) { */
        if (1) {
            unsigned in = s->stream.avail_in ;
            SvCUR_set(buf, in) ;
            if (in)
                Move(s->stream.next_in, SvPVX(buf), in, char) ;
            *SvEND(buf) = '\0';
            SvSETMAGIC(buf);
        }
    }
#endif
  OUTPUT:
	RETVAL


uLong
getEndOffset(s)
    Compress::Raw::Zlib::inflateScanStream	s
    CODE:
#ifndef MAGIC_APPEND
        croak("getEndOffset needs zlib 1.2.1 or better");
#else
        RETVAL = s->window_endOffset;
#endif
  OUTPUT:
	RETVAL

uLong
inflateCount(s)
    Compress::Raw::Zlib::inflateScanStream	s
    CODE:
#ifndef MAGIC_APPEND
        croak("inflateCount needs zlib 1.2.1 or better");
#else
        RETVAL = s->bytesInflated;
#endif
  OUTPUT:
	RETVAL

uLong
compressedBytes(s)
    Compress::Raw::Zlib::inflateScanStream	s
    CODE:
        RETVAL = s->compressedBytes;
  OUTPUT:
	RETVAL

uLong
uncompressedBytes(s)
    Compress::Raw::Zlib::inflateScanStream	s
    CODE:
        RETVAL = s->uncompressedBytes;
  OUTPUT:
	RETVAL


uLong
getLastBlockOffset(s)
    Compress::Raw::Zlib::inflateScanStream	s
    CODE:
#ifndef MAGIC_APPEND
        croak("getLastBlockOffset needs zlib 1.2.1 or better");
#else
        RETVAL = s->lastBlockOffset - (s->window_lastbit != 0);
#endif
  OUTPUT:
	RETVAL

uLong
getLastBufferOffset(s)
    Compress::Raw::Zlib::inflateScanStream	s
    CODE:
#ifndef MAGIC_APPEND
        croak("getLastBufferOffset needs zlib 1.2.1 or better");
#else
        RETVAL = s->window_lastoff;
#endif
  OUTPUT:
	RETVAL

void
resetLastBlockByte(s, byte)
    Compress::Raw::Zlib::inflateScanStream	s
    unsigned char*                      byte
    CODE:
#ifndef MAGIC_APPEND
        croak("resetLastBlockByte needs zlib 1.2.1 or better");
#else
        if (byte != NULL)
            *byte = *byte ^ (1 << ((8 - s->window_lastbit) & 7));
#endif


void
_createDeflateStream(inf_s, flags,level, method, windowBits, memLevel, strategy, bufsize)
    Compress::Raw::Zlib::inflateScanStream	inf_s
    int flags
    int	level
    int method
    int windowBits
    int memLevel
    int strategy
    uLong bufsize
  PPCODE:
  {
#ifndef MAGIC_APPEND
        flags = flags;
        level = level ;
        method = method;
        windowBits = windowBits;
        memLevel = memLevel;
        strategy = strategy;
        bufsize= bufsize;
        croak("_createDeflateStream needs zlib 1.2.1 or better");
#else
    int err ;
    deflateStream s ;

    if (trace)
        warn("in _createDeflateStream(level=%d, method=%d, windowBits=%d, memLevel=%d, strategy=%d, bufsize=%lu\n",
	level, method, windowBits, memLevel, strategy, bufsize) ;
    if ((s = InitStream() )) {

        s->Level      = level;
        s->Method     = method;
        s->WindowBits = windowBits;
        s->MemLevel   = memLevel;
        s->Strategy   = strategy;

        err = CRZ_deflateInit2(&(s->stream), level,
			   method, windowBits, memLevel, strategy);

	if (err == Z_OK) {
	    err = CRZ_deflateSetDictionary(&(s->stream), inf_s->window, inf_s->window_have);
	    s->dict_adler = s->stream.adler ;
	}

        if (err != Z_OK) {
            Safefree(s) ;
            s = NULL ;
	}
	else {
	    PostInitStream(s, flags, bufsize, windowBits) ;
            s->crc32            = inf_s->crc32;
            s->adler32          = inf_s->adler32;
            s->stream.adler     = inf_s->stream.adler ;
            /* s->stream.total_out = inf_s->bytesInflated ; */
            s->stream.total_in  = inf_s->stream.total_out ;
            if (inf_s->window_left) {
                /* printf("** window_left %d, window_lastByte %d\n", inf_s->window_left, inf_s->window_lastByte); */
                CRZ_deflatePrime(&(s->stream), 8 - inf_s->window_left, inf_s->window_lastByte);
            }
        }
    }
    else
        err = Z_MEM_ERROR ;

    XPUSHs(sv_setref_pv(sv_newmortal(),
            "Compress::Raw::Zlib::deflateStream", (void*)s));
    if (GIMME_V == G_ARRAY) {
        SV * sv = sv_2mortal(newSViv(err)) ;
        setDUALstatus(sv, err);
        XPUSHs(sv) ;
    }
#endif
  }

DualType
status(s)
        Compress::Raw::Zlib::inflateScanStream   s
    CODE:
	RETVAL = s->last_error ;
    OUTPUT:
	RETVAL

uLong
crc32(s)
        Compress::Raw::Zlib::inflateScanStream   s
    CODE:
	RETVAL = s->crc32 ;
    OUTPUT:
	RETVAL


uLong
adler32(s)
        Compress::Raw::Zlib::inflateScanStream   s
    CODE:
	RETVAL = s->adler32 ;
    OUTPUT:
	RETVAL
