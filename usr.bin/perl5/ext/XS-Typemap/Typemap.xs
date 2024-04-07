/*
   XS code to test the typemap entries

   Copyright (C) 2001 Tim Jenness.
   All Rights Reserved

*/

#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"   /* std perl include */
#include "perl.h"     /* std perl include */
#include "XSUB.h"     /* XSUB include */

/* Prototypes for external functions */
FILE * xsfopen( const char * );
int xsfclose( FILE * );
int xsfprintf( FILE *, const char *);

/* Type definitions required for the XS typemaps */
typedef SV * SVREF; /* T_SVREF */
typedef int SysRet; /* T_SYSRET */
typedef int Int;    /* T_INT */
typedef int intRef; /* T_PTRREF */
typedef int intObj; /* T_PTROBJ */
typedef int intRefIv; /* T_REF_IV_PTR */
typedef int intArray; /* T_ARRAY */
typedef int intTINT; /* T_INT */
typedef int intTLONG; /* T_LONG */
typedef short shortOPQ;   /* T_OPAQUE */
typedef int intOpq;   /* T_OPAQUEPTR */
typedef unsigned intUnsigned; /* T_U_INT */
typedef PerlIO * inputfh; /* T_IN */
typedef PerlIO * outputfh; /* T_OUT */

/* A structure to test T_OPAQUEPTR and T_PACKED */
struct t_opaqueptr {
  int a;
  int b;
  double c;
};

typedef struct t_opaqueptr astruct;
typedef struct t_opaqueptr anotherstruct;

/* Some static memory for the tests */
static I32 xst_anint;
static intRef xst_anintref;
static intObj xst_anintobj;
static intRefIv xst_anintrefiv;
static intOpq xst_anintopq;

/* A different type to refer to for testing the different
 * AV*, HV*, etc typemaps */
typedef AV AV_FIXED;
typedef HV HV_FIXED;
typedef CV CV_FIXED;
typedef SVREF SVREF_FIXED;

/* Helper functions */

/* T_ARRAY - allocate some memory */
intArray * intArrayPtr( int nelem ) {
    intArray * array;
    Newx(array, nelem, intArray);
    return array;
}

/* test T_PACKED */
STATIC void
XS_pack_anotherstructPtr(SV *out, anotherstruct *in)
{
    dTHX;
    HV *hash = newHV();
    if (NULL == hv_stores(hash, "a", newSViv(in->a)))
      croak("Failed to store data in hash");
    if (NULL == hv_stores(hash, "b", newSViv(in->b)))
      croak("Failed to store data in hash");
    if (NULL == hv_stores(hash, "c", newSVnv(in->c)))
      croak("Failed to store data in hash");
    sv_setsv(out, sv_2mortal(newRV_noinc((SV*)hash)));
}

STATIC anotherstruct *
XS_unpack_anotherstructPtr(SV *in)
{
    dTHX; /* rats, this is expensive */
    /* this is similar to T_HVREF since we chose to use a hash */
    HV *inhash;
    SV **elem;
    anotherstruct *out;
    SV *const tmp = in;
    SvGETMAGIC(tmp);
    if (SvROK(tmp) && SvTYPE(SvRV(tmp)) == SVt_PVHV)
       inhash = (HV*)SvRV(tmp);
    else
        Perl_croak(aTHX_ "Argument is not a HASH reference");

    /* FIXME dunno if supposed to use perl mallocs here */
    Newxz(out, 1, anotherstruct);

    elem = hv_fetchs(inhash, "a", 0);
    if (elem == NULL)
      Perl_croak(aTHX_ "Shouldn't happen: hv_fetchs returns NULL");
    out->a = SvIV(*elem);

    elem = hv_fetchs(inhash, "b", 0);
    if (elem == NULL)
      Perl_croak(aTHX_ "Shouldn't happen: hv_fetchs returns NULL");
    out->b = SvIV(*elem);

    elem = hv_fetchs(inhash, "c", 0);
    if (elem == NULL)
      Perl_croak(aTHX_ "Shouldn't happen: hv_fetchs returns NULL");
    out->c = SvNV(*elem);

    return out;
}

/* test T_PACKEDARRAY */
STATIC void
XS_pack_anotherstructPtrPtr(SV *out, anotherstruct **in, UV cnt)
{
    dTHX;
    UV i;
    AV *ary = newAV();
    for (i = 0; i < cnt; ++i) {
        HV *hash = newHV();
        if (NULL == hv_stores(hash, "a", newSViv(in[i]->a)))
          croak("Failed to store data in hash");
        if (NULL == hv_stores(hash, "b", newSViv(in[i]->b)))
          croak("Failed to store data in hash");
        if (NULL == hv_stores(hash, "c", newSVnv(in[i]->c)))
          croak("Failed to store data in hash");
        av_push(ary, newRV_noinc((SV*)hash));
    }
    sv_setsv(out, sv_2mortal(newRV_noinc((SV*)ary)));
}

STATIC anotherstruct **
XS_unpack_anotherstructPtrPtr(SV *in)
{
    dTHX; /* rats, this is expensive */
    /* this is similar to T_HVREF since we chose to use a hash */
    HV *inhash;
    AV *inary;
    SV **elem;
    anotherstruct **out;
    UV nitems, i;
    SV *tmp;

    /* safely deref the input array ref */
    tmp = in;
    SvGETMAGIC(tmp);
    if (SvROK(tmp) && SvTYPE(SvRV(tmp)) == SVt_PVAV)
        inary = (AV*)SvRV(tmp);
    else
        Perl_croak(aTHX_ "Argument is not an ARRAY reference");

    nitems = av_count(inary);

    /* FIXME dunno if supposed to use perl mallocs here */
    /* N+1 elements so we know the last one is NULL */
    Newxz(out, nitems+1, anotherstruct*);

    /* WARNING: in real code, we'd have to Safefree() on exception, but
     *          since we're testing perl, if we croak() here, stuff is
     *          rotten anyway! */
    for (i = 0; i < nitems; ++i) {
        Newxz(out[i], 1, anotherstruct);
        elem = av_fetch(inary, i, 0);
        if (elem == NULL)
            Perl_croak(aTHX_ "Shouldn't happen: av_fetch returns NULL");
        tmp = *elem;
        SvGETMAGIC(tmp);
        if (SvROK(tmp) && SvTYPE(SvRV(tmp)) == SVt_PVHV)
            inhash = (HV*)SvRV(tmp);
        else
            Perl_croak(aTHX_ "Array element %" UVuf
                             " is not a HASH reference", i);

        elem = hv_fetchs(inhash, "a", 0);
        if (elem == NULL)
            Perl_croak(aTHX_ "Shouldn't happen: hv_fetchs returns NULL");
        out[i]->a = SvIV(*elem);

        elem = hv_fetchs(inhash, "b", 0);
        if (elem == NULL)
            Perl_croak(aTHX_ "Shouldn't happen: hv_fetchs returns NULL");
        out[i]->b = SvIV(*elem);

        elem = hv_fetchs(inhash, "c", 0);
        if (elem == NULL)
            Perl_croak(aTHX_ "Shouldn't happen: hv_fetchs returns NULL");
        out[i]->c = SvNV(*elem);
    }

    return out;
}

/* no special meaning as far as typemaps are concerned,
 * just for convenience */
void
XS_release_anotherstructPtrPtr(anotherstruct **in)
{
    unsigned int i;
    for (i = 0; in[i] != NULL; i++)
        Safefree(in[i]);
    Safefree(in);
}


MODULE = XS::Typemap   PACKAGE = XS::Typemap

PROTOTYPES: DISABLE

TYPEMAP: <<END_OF_TYPEMAP

# Typemap file for typemap testing
# includes bonus typemap entries
# Mainly so that all the standard typemaps can be exercised even when
# there is not a corresponding type explicitly identified in the standard
# typemap

svtype           T_ENUM
intRef *         T_PTRREF
intRef           T_IV
intObj *         T_PTROBJ
intObj           T_IV
intRefIv *       T_REF_IV_PTR
intRefIv         T_IV
intArray *       T_ARRAY
intOpq           T_IV
intOpq   *       T_OPAQUEPTR
intUnsigned      T_U_INT
intTINT          T_INT
intTLONG         T_LONG
shortOPQ         T_OPAQUE
shortOPQ *       T_OPAQUEPTR
astruct *        T_OPAQUEPTR
anotherstruct *  T_PACKED
anotherstruct ** T_PACKEDARRAY
AV_FIXED *	 T_AVREF_REFCOUNT_FIXED
HV_FIXED *	 T_HVREF_REFCOUNT_FIXED
CV_FIXED *	 T_CVREF_REFCOUNT_FIXED
SVREF_FIXED	 T_SVREF_REFCOUNT_FIXED
inputfh          T_IN
outputfh         T_OUT

END_OF_TYPEMAP


## T_SV

SV *
T_SV( sv )
  SV * sv
 CODE:
  /* create a new sv for return that is a copy of the input
     do not simply copy the pointer since the SV will be marked
     mortal by the INPUT typemap when it is pushed back onto the stack */
  RETVAL = sv_mortalcopy( sv );
  /* increment the refcount since the default INPUT typemap mortalizes
     by default and we don't want to decrement the ref count twice
     by mistake */
  SvREFCNT_inc(RETVAL);
 OUTPUT:
  RETVAL

void
T_SV_output(sv)
  SV *sv
 CODE:
 sv = sv_2mortal(newSVpvn("test", 4));
 OUTPUT:
  sv

## T_SVREF

SVREF
T_SVREF( svref )
  SVREF svref
 CODE:
  RETVAL = svref;
 OUTPUT:
  RETVAL


## T_SVREF_FIXED

SVREF_FIXED
T_SVREF_REFCOUNT_FIXED( svref )
  SVREF_FIXED svref
 CODE:
  SvREFCNT_inc(svref);
  RETVAL = svref;
 OUTPUT:
  RETVAL

void
T_SVREF_REFCOUNT_FIXED_output( OUT svref )
  SVREF_FIXED svref
 CODE:
  svref = newSVpvn("test", 4);

## T_AVREF

AV *
T_AVREF( av )
  AV * av
 CODE:
  RETVAL = av;
 OUTPUT:
  RETVAL


## T_AVREF_REFCOUNT_FIXED

AV_FIXED*
T_AVREF_REFCOUNT_FIXED( av )
  AV_FIXED * av
 CODE:
  SvREFCNT_inc(av);
  RETVAL = av;
 OUTPUT:
  RETVAL

void
T_AVREF_REFCOUNT_FIXED_output( OUT avref)
  AV_FIXED *avref;
 CODE:
  avref = newAV();
  av_push(avref, newSVpvs("test"));

## T_HVREF

HV *
T_HVREF( hv )
  HV * hv
 CODE:
  RETVAL = hv;
 OUTPUT:
  RETVAL


## T_HVREF_REFCOUNT_FIXED

HV_FIXED*
T_HVREF_REFCOUNT_FIXED( hv )
  HV_FIXED * hv
 CODE:
  SvREFCNT_inc(hv);
  RETVAL = hv;
 OUTPUT:
  RETVAL

void
T_HVREF_REFCOUNT_FIXED_output( OUT hvref)
  HV_FIXED *hvref;
 CODE:
  hvref = newHV();
  hv_stores(hvref, "test", newSVpvs("value"));

## T_CVREF

CV *
T_CVREF( cv )
  CV * cv
 CODE:
  RETVAL = cv;
 OUTPUT:
  RETVAL


## T_CVREF_REFCOUNT_FIXED

CV_FIXED *
T_CVREF_REFCOUNT_FIXED( cv )
  CV_FIXED * cv
 CODE:
  SvREFCNT_inc(cv);
  RETVAL = cv;
 OUTPUT:
  RETVAL

void
T_CVREF_REFCOUNT_FIXED_output( OUT cvref)
  CV_FIXED *cvref;
 CODE:
  cvref = get_cv("XSLoader::load", 0);
  SvREFCNT_inc(cvref);

## T_SYSRET

# Test a successful return

SysRet
T_SYSRET_pass()
 CODE:
  RETVAL = 0;
 OUTPUT:
  RETVAL

# Test failure

SysRet
T_SYSRET_fail()
 CODE:
  RETVAL = -1;
 OUTPUT:
  RETVAL

## T_UV

unsigned int
T_UV( uv )
  unsigned int uv
 CODE:
  RETVAL = uv;
 OUTPUT:
  RETVAL


## T_IV

long
T_IV( iv )
  long iv
 CODE:
  RETVAL = iv;
 OUTPUT:
  RETVAL


## T_INT

intTINT
T_INT( i )
  intTINT i
 CODE:
  RETVAL = i;
 OUTPUT:
  RETVAL


## T_ENUM

# The test should return the value for SVt_PVHV.
# 11 at the present time but we can't not rely on this
# for testing purposes.

svtype
T_ENUM()
 CODE:
  RETVAL = SVt_PVHV;
 OUTPUT:
  RETVAL


## T_BOOL

bool
T_BOOL( in )
  bool in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL

bool
T_BOOL_2( in )
  bool in
 CODE:
    PERL_UNUSED_VAR(RETVAL);
 OUTPUT:
   in

void
T_BOOL_OUT( out, in )
  bool out
  bool in
 CODE:
 out = in;
 OUTPUT:
   out

## T_U_INT

intUnsigned
T_U_INT( uint )
  intUnsigned uint
 CODE:
  RETVAL = uint;
 OUTPUT:
  RETVAL


## T_SHORT

short
T_SHORT( s )
  short s
 CODE:
  RETVAL = s;
 OUTPUT:
  RETVAL


## T_U_SHORT

U16
T_U_SHORT( in )
  U16 in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_LONG

intTLONG
T_LONG( in )
  intTLONG in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL

## T_U_LONG

U32
T_U_LONG( in )
  U32 in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_CHAR

char
T_CHAR( in );
  char in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_U_CHAR

unsigned char
T_U_CHAR( in );
  unsigned char in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_FLOAT

float
T_FLOAT( in )
  float in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_NV

NV
T_NV( in )
  NV in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_DOUBLE

double
T_DOUBLE( in )
  double in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL


## T_PV

char *
T_PV( in )
  char * in
 CODE:
  RETVAL = in;
 OUTPUT:
  RETVAL

char *
T_PV_null()
 CODE:
  RETVAL = NULL;
 OUTPUT:
  RETVAL


## T_PTR

# Pass in a value. Store the value in some static memory and
# then return the pointer

void *
T_PTR_OUT( in )
  int in;
 CODE:
  xst_anint = in;
  RETVAL = &xst_anint;
 OUTPUT:
  RETVAL

# pass in the pointer and return the value

int
T_PTR_IN( ptr )
  void * ptr
 CODE:
  RETVAL = *(int *)ptr;
 OUTPUT:
  RETVAL


## T_PTRREF

# Similar test to T_PTR
# Pass in a value. Store the value in some static memory and
# then return the pointer

intRef *
T_PTRREF_OUT( in )
  intRef in;
 CODE:
  xst_anintref = in;
  RETVAL = &xst_anintref;
 OUTPUT:
  RETVAL

# pass in the pointer and return the value

intRef
T_PTRREF_IN( ptr )
  intRef * ptr
 CODE:
  RETVAL = *ptr;
 OUTPUT:
  RETVAL


## T_PTROBJ

# Similar test to T_PTRREF
# Pass in a value. Store the value in some static memory and
# then return the pointer

intObj *
T_PTROBJ_OUT( in )
  intObj in;
 CODE:
  xst_anintobj = in;
  RETVAL = &xst_anintobj;
 OUTPUT:
  RETVAL

# pass in the pointer and return the value

MODULE = XS::Typemap  PACKAGE = intObjPtr

intObj
T_PTROBJ_IN( ptr )
  intObj * ptr
 CODE:
  RETVAL = *ptr;
 OUTPUT:
  RETVAL

MODULE = XS::Typemap PACKAGE = XS::Typemap


## T_REF_IV_REF
## NOT YET


## T_REF_IV_PTR

# Similar test to T_PTROBJ
# Pass in a value. Store the value in some static memory and
# then return the pointer

intRefIv *
T_REF_IV_PTR_OUT( in )
  intRefIv in;
 CODE:
  xst_anintrefiv = in;
  RETVAL = &xst_anintrefiv;
 OUTPUT:
  RETVAL

# pass in the pointer and return the value

MODULE = XS::Typemap  PACKAGE = intRefIvPtr

intRefIv
T_REF_IV_PTR_IN( ptr )
  intRefIv * ptr
 CODE:
  RETVAL = *ptr;
 OUTPUT:
  RETVAL


MODULE = XS::Typemap PACKAGE = XS::Typemap

## T_PTRDESC
## NOT YET


## T_REFREF
## NOT YET


## T_REFOBJ
## NOT YET


## T_OPAQUEPTR

intOpq *
T_OPAQUEPTR_IN( val )
  intOpq val
 CODE:
  xst_anintopq = val;
  RETVAL = &xst_anintopq;
 OUTPUT:
  RETVAL

intOpq
T_OPAQUEPTR_OUT( ptr )
  intOpq * ptr
 CODE:
  RETVAL = *ptr;
 OUTPUT:
  RETVAL

short
T_OPAQUEPTR_OUT_short( ptr )
  shortOPQ * ptr
 CODE:
  RETVAL = *ptr;
 OUTPUT:
  RETVAL

# Test it with a structure
astruct *
T_OPAQUEPTR_IN_struct( a,b,c )
  int a
  int b
  double c
 PREINIT:
  struct t_opaqueptr test;
 CODE:
  test.a = a;
  test.b = b;
  test.c = c;
  RETVAL = &test;
 OUTPUT:
  RETVAL

void
T_OPAQUEPTR_OUT_struct( test )
  astruct * test
 PPCODE:
  XPUSHs(sv_2mortal(newSViv(test->a)));
  XPUSHs(sv_2mortal(newSViv(test->b)));
  XPUSHs(sv_2mortal(newSVnv(test->c)));


## T_OPAQUE

shortOPQ
T_OPAQUE_IN( val )
  int val
 CODE:
  RETVAL = (shortOPQ)val;
 OUTPUT:
  RETVAL

IV
T_OPAQUE_OUT( val )
  shortOPQ val
 CODE:
  RETVAL = (IV)val;
 OUTPUT:
  RETVAL

array(int,3)
T_OPAQUE_array( a,b,c)
  int a
  int b
  int c
 PREINIT:
  int array[3];
 CODE:
  array[0] = a;
  array[1] = b;
  array[2] = c;
  RETVAL = array;
 OUTPUT:
  RETVAL


## T_PACKED

void
T_PACKED_in(in)
  anotherstruct *in;
 PPCODE:
  mXPUSHi(in->a);
  mXPUSHi(in->b);
  mXPUSHn(in->c);
  Safefree(in);
  XSRETURN(3);

anotherstruct *
T_PACKED_out(a, b ,c)
  int a;
  int b;
  double c;
 CODE:
  Newxz(RETVAL, 1, anotherstruct);
  RETVAL->a = a;
  RETVAL->b = b;
  RETVAL->c = c;
 OUTPUT: RETVAL
 CLEANUP:
  Safefree(RETVAL);

## T_PACKEDARRAY

void
T_PACKEDARRAY_in(in)
  anotherstruct **in;
 PREINIT:
  unsigned int i = 0;
 PPCODE:
  while (in[i] != NULL) {
    mXPUSHi(in[i]->a);
    mXPUSHi(in[i]->b);
    mXPUSHn(in[i]->c);
    ++i;
  }
  XS_release_anotherstructPtrPtr(in);
  XSRETURN(3*i);

anotherstruct **
T_PACKEDARRAY_out(...)
 PREINIT:
  unsigned int i, nstructs, count_anotherstructPtrPtr;
 CODE:
  if ((items % 3) != 0)
    croak("Need nitems divisible by 3");
  nstructs = (unsigned int)(items / 3);
  count_anotherstructPtrPtr = nstructs;
  Newxz(RETVAL, nstructs+1, anotherstruct *);
  for (i = 0; i < nstructs; ++i) {
    Newxz(RETVAL[i], 1, anotherstruct);
    RETVAL[i]->a = SvIV(ST(3*i));
    RETVAL[i]->b = SvIV(ST(3*i+1));
    RETVAL[i]->c = SvNV(ST(3*i+2));
  }
 OUTPUT: RETVAL
 CLEANUP:
  XS_release_anotherstructPtrPtr(RETVAL);


## T_DATAUNIT
## NOT YET


## T_CALLBACK
## NOT YET


## T_ARRAY

# Test passes in an integer array and returns it along with
# the number of elements
# Pass in a dummy value to test offsetting

# Problem is that xsubpp does XSRETURN(1) because we arent
# using PPCODE. This means that only the first element
# is returned. KLUGE this by using CLEANUP to return before the
# end.
# Note: I read this as: The "T_ARRAY" typemap is really rather broken,
#       at least for OUTPUT. That is apart from the general design
#       weaknesses. --Steffen

intArray *
T_ARRAY( dummy, array, ... )
  int dummy = 0;
  intArray * array
 PREINIT:
  U32 size_RETVAL;
 CODE:
  dummy += 0; /* Fix -Wall */
  size_RETVAL = ix_array;
  RETVAL = array;
 OUTPUT:
  RETVAL
 CLEANUP:
  Safefree(array);
  XSRETURN(size_RETVAL);


## T_STDIO

FILE *
T_STDIO_open( file )
  const char * file
 CODE:
  RETVAL = xsfopen( file );
 OUTPUT:
  RETVAL

void
T_STDIO_open_ret_in_arg( file, io)
  const char * file
  FILE * io = NO_INIT
 CODE:
  io = xsfopen( file );
 OUTPUT:
  io

SysRet
T_STDIO_close( f )
  PerlIO * f
 PREINIT:
  FILE * stream;
 CODE:
  /* Get the FILE* */
  stream = PerlIO_findFILE( f );  
  /* Release the FILE* from the PerlIO system so that we do
     not close the file twice */
  PerlIO_releaseFILE(f,stream);
  /* Must release the file before closing it */
  RETVAL = xsfclose( stream );
 OUTPUT:
  RETVAL

int
T_STDIO_print( stream, string )
  FILE * stream
  const char * string
 CODE:
  RETVAL = xsfprintf( stream, string );
 OUTPUT:
  RETVAL


## T_INOUT

PerlIO *
T_INOUT(in)
  PerlIO *in;
 CODE:
  RETVAL = in; /* silly test but better than nothing */
 OUTPUT: RETVAL


## T_IN

inputfh
T_IN(in)
  inputfh in;
 CODE:
  RETVAL = in; /* silly test but better than nothing */
 OUTPUT: RETVAL


## T_OUT

outputfh
T_OUT(in)
  outputfh in;
 CODE:
  RETVAL = in; /* silly test but better than nothing */
 OUTPUT: RETVAL

