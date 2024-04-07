/* const2perl.h -- For converting C constants into Perl constant subs
 *	(usually via XS code but can just write Perl code to stdout). */


/* #ifndef _INCLUDE_CONST2PERL_H
 * #define _INCLUDE_CONST2PERL_H 1 */

#ifndef CONST2WRITE_PERL	/* Default is "const to .xs": */

# define newconst( sName, sFmt, xValue, newSV )	\
		newCONSTSUB( mHvStash, sName, newSV )

# define noconst( const )	av_push( mAvExportFail, newSVpv(#const,0) )

# define setuv(u)	do {				\
	mpSvNew= newSViv(0); sv_setuv(mpSvNew,u);	\
    } while( 0 )

#else

/* #ifdef __cplusplus
 * # undef printf
 * # undef fprintf
 * # undef stderr
 * # define stderr (&_iob[2])
 * # undef iobuf
 * # undef malloc
 * #endif */

# include <stdio.h>	/* Probably already included, but shouldn't hurt */
# include <errno.h>	/* Possibly already included, but shouldn't hurt */

# define newconst( sName, sFmt, xValue, newSV )	\
		printf( "sub %s () { " sFmt " }\n", sName, xValue )

# define noconst( const )	printf( "push @EXPORT_FAIL, '%s';\n", #const )

# define setuv(u)	/* Nothing */

# ifndef IVdf
#  define IVdf "ld"
# endif
# ifndef UVuf
#  define UVuf "lu"
# endif
# ifndef UVxf
#  define UVxf "lX"
# endif
# ifndef NV_DIG
#  define NV_DIG 15
# endif

static char *
escquote( const char *sValue )
{
    Size_t lLen= 1+2*strlen(sValue);
    char *sEscaped= (char *) malloc( lLen );
    char *sNext= sEscaped;
    if(  NULL == sEscaped  ) {
	fprintf( stderr, "Can't allocate %"UVuf"-byte buffer (errno=%d)\n",
	  U_V(lLen), _errno );
	exit( 1 );
    }
    while(  '\0' != *sValue  ) {
	switch(  *sValue  ) {
	 case '\'':
	 case '\\':
	    *(sNext++)= '\\';
	}
	*(sNext++)= *(sValue++);
    }
    *sNext= *sValue;
    return( sEscaped );
}

#endif


#ifdef __cplusplus

class _const2perl {
 public:
    char msBuf[64];	/* Must fit sprintf of longest NV */
#ifndef CONST2WRITE_PERL
    HV *mHvStash;
    AV *mAvExportFail;
    SV *mpSvNew;
    _const2perl::_const2perl( char *sModName ) {
	mHvStash= gv_stashpv( sModName, TRUE );
	SV **pSv= hv_fetch( mHvStash, "EXPORT_FAIL", 11, TRUE );
	GV *gv;
	char *sVarName= (char *) malloc( 15+strlen(sModName) );
	strcpy( sVarName, sModName );
	strcat( sVarName, "::EXPORT_FAIL" );
	gv= gv_fetchpv( sVarName, 1, SVt_PVAV );
	mAvExportFail= GvAVn( gv );
    }
#else
    _const2perl::_const2perl( char *sModName ) {
	;	/* Nothing to do */
    }
#endif /* CONST2WRITE_PERL */
    void mkconst( char *sName, unsigned long uValue ) {
	setuv(uValue);
	newconst( sName, "0x%"UVxf, uValue, mpSvNew );
    }
    void mkconst( char *sName, unsigned int uValue ) {
	setuv(uValue);
	newconst( sName, "0x%"UVxf, uValue, mpSvNew );
    }
    void mkconst( char *sName, unsigned short uValue ) {
	setuv(uValue);
	newconst( sName, "0x%"UVxf, uValue, mpSvNew );
    }
    void mkconst( char *sName, long iValue ) {
	newconst( sName, "%"IVdf, iValue, newSViv(iValue) );
    }
    void mkconst( char *sName, int iValue ) {
	newconst( sName, "%"IVdf, iValue, newSViv(iValue) );
    }
    void mkconst( char *sName, short iValue ) {
	newconst( sName, "%"IVdf, iValue, newSViv(iValue) );
    }
    void mkconst( char *sName, double nValue ) {
	newconst( sName, "%s",
	  Gconvert(nValue,NV_DIG,0,msBuf), newSVnv(nValue) );
    }
    void mkconst( char *sName, char *sValue ) {
	newconst( sName, "'%s'", escquote(sValue), newSVpv(sValue,0) );
    }
    void mkconst( char *sName, const void *pValue ) {
	setuv((UV)pValue);
	newconst( sName, "0x%"UVxf, (UV)(pValue), mpSvNew );
    }
/*#ifdef HAS_QUAD
 * HAS_QUAD only means pack/unpack deal with them, not that SVs can.
 *    void mkconst( char *sName, Quad_t *qValue ) {
 *	newconst( sName, "0x%"QVxf, qValue, newSVqv(qValue) );
 *    }
 *#endif / * HAS_QUAD */
};

#define START_CONSTS( sModName )	_const2perl const2( sModName );
#define const2perl( const )		const2.mkconst( #const, const )

#else	/* __cplusplus */

# ifndef CONST2WRITE_PERL
#  define START_CONSTS( sModName )					\
	    HV *mHvStash= gv_stashpv( sModName, TRUE );			\
	    AV *mAvExportFail;						\
	    SV *mpSvNew;						\
	    { char *sVarName= malloc( 15+strlen(sModName) );		\
	      GV *gv;							\
		strcpy( sVarName, sModName );				\
		strcat( sVarName, "::EXPORT_FAIL" );			\
		gv= gv_fetchpv( sVarName, 1, SVt_PVAV );		\
		mAvExportFail= GvAVn( gv );				\
	    }
# else
#  define START_CONSTS( sModName )	/* Nothing */
# endif

#define const2perl( const )	do {	 				\
	if(  const < 0  ) {						\
	    newconst( #const, "%"IVdf, const, newSViv((IV)const) );	\
	} else {							\
	    setuv( (UV)const );						\
	    newconst( #const, "0x%"UVxf, const, mpSvNew ); 		\
	}								\
    } while( 0 )

#endif	/* __cplusplus */


//Example use:
//#include <const2perl.h>
//  {
//    START_CONSTS( "Package::Name" )	/* No ";" */
//#ifdef $const
//    const2perl( $const );
//#else
//    noconst( $const );
//#endif
//  }
// sub ? { my( $sConstName )= @_;
//    return $sConstName;	# "#ifdef $sConstName"
//    return FALSE;		# Same as above
//    return "HAS_QUAD";	# "#ifdef HAS_QUAD"
//    return "#if 5.04 <= VERSION";
//    return "#if 0";
//    return 1;		# No #ifdef
/* #endif / * _INCLUDE_CONST2PERL_H */
