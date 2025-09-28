/* buffers.h -- Version 1.11 */

/* The following abbreviations are used at start of parameter names
 * to indicate the type of data:
 *	s	string (char * or WCHAR *) [PV]
 *	sw	wide string (WCHAR *) [PV]
 *	p	pointer (usually to some structure) [PV]
 *	a	array (packed array as in C) (usually of some structure) [PV]
 *		    called a "vector" or "vect" in some places.
 *	n	generic number [IV, UV, or NV]
 *	iv	signed integral value [IV]
 *	u	unsigned integral value [UV]
 *	d	floating-point number (double) [NV]
 *	b	boolean (bool) [IV]
 *	c	count of items [UV]
 *	l	length (in bytes) [UV]
 *	lw	length in WCHARs [UV]
 *	h	a handle [IV]
 *	r	record (structure) [PV]
 *	sv	Perl scalar (s, i, u, d, n, or rv) [SV]
 *	rv	Perl reference (usually to scalar) [RV]
 *	hv	reference to Perl hash [HV]
 *	av	reference to Perl array [AV]
 *	cv	Perl code reference [PVCV]
 *
 * Unusual combined types:
 *	pp	single pointer (to non-Perl data) packed into string [PV]
 *	pap	vector of pointers (to non-Perl data) packed into string [PV]
 *
 * Whether a parameter is for input data, output data, or both is usually
 * not reflected by the data type prefix.  In cases where this is not
 * obvious nor reflected in the variable name proper, you can use
 * the following in front of the data type prefix:
 *	i	an input parameter given to API (usually omitted)
 *	o	an Output parameter taken from API
 *	io	Input given to API then overwritten with Output taken from API
 */

/* Buffer arguments are usually followed by an argument (or two) specifying
 * their size and/or returning the size of data written.  The size can be
 * measured in bytes ["lSize"] or in characters [for (char *) buffers such as
 * for *A() routines, these sizes are also called "lSize", but are called
 * "lwSize" for (WCHAR *) buffers, UNICODE strings, such as for *W() routines].
 *
 * Before calling the actual C function, you must make sure the Perl variable
 * actually has a big enough buffer allocated, and, if the user didn't want
 * to specify a buffer size, set the buffer size to be correct.  This is what
 * the grow_*() macros are for.  They also handle special meanings of the
 * buffer size argument [described below].
 *
 * Once the actual C function returns, you must set the Perl variable to know
 * the size of the written data.  This is what the trunc_*() macros are for.
 *
 * The size sometimes does and sometimes doesn't include the trailing '\0'
 * [or L'\0'], so we always add or subtract 1 in the appropriate places so
 * we don't care about this detail.
 *
 * A call may  1) request a pointer to the buffer size which means that
 * the buffer size will be overwritten with the size of the data written;
 * 2) have an extra argument which is a pointer to the place to write the
 * size of the written data;  3) provide the size of the written data in
 * the function's return value;  4) format the data so that the length
 * can be determined by examining the data [such as with '\0'-terminated
 * strings];  or  5) write fixed-length data [usually sizeof(STRUCT)].
 * This obviously determines what you should use in the trunc_*() macro
 # to specify the size of the output value.
 *
 * The user can pass in an empty list reference, C<[]>, to indicate C<NULL>
 * for the pointer to the buffer which means that they don't want that data.
 *
 * The user can pass in C<[]> or C<0> to indicate that they don't care about
 * the buffer size [we aren't programming in C here, after all] and just try
 * to get the data.  This will work if either the buffer already allocated for
 * the SV [scalar value] is large enough to hold the data or the API provides
 * an easy way to determine the required size [and the XS code uses it].
 *
 * If the user passes in a numeric value for a buffer size, then the XS
 * code makes sure that the buffer is at least large enough to hold a value
 * of that size and then passes in how large the buffer is.  So the buffer
 * size passed to the API call is the larger of the size requested by the
 * user and the size of the buffer already allocated to the SV.
 *
 * The user can also pass in a string consisting of a leading "=" followed
 * by digits for a buffer size.  This means just use the size specified after
 * the equals sign, even if the allocated buffer is larger.  The XS code will
 * still allocate a large enough buffer before the first call.
 *
 * If the function is nice enough to tell us that a buffer was too small
 * [usually via ERROR_MORE_DATA] _and_ how large the buffer needs to be,
 * then the XS code should enlarge the buffer(s) and repeat the call [once].
 * This resizing is _not_ done for buffers whose size was specified with a
 * leading "=".
 *
 * Only grow_buf() and perhaps trunc_buf() can be used in a typemap file.
 * The other macros would be used in the parameter declarations or INPUT:
 * section [grow_*()], the INIT: section [init_*()], or the OUTPUT: section
 * [trunc_*()].
 *
 * Buffer arguments should be initialised with C<= NO_INIT> [or C<= NULL;>].
 *
 * See also the F<typemap> file.  C<oDWORD>, for example, is for an output-
 * only parameter of type C<DWORD> and you should simply C<#define> it to be
 * C<DWORD>.  In F<typemap>, C<oDWORD> is treated differently than C<DWORD>
 * in two ways.
 *
 * First, if C<undef> is passed in, a C<DWORD> could generate a warning
 * when it gets converted to 0 while C<oDWORD> will never generate such a
 * warning for C<undef>.  This first difference doesn't apply if specific
 * initialization is specified for the variable, as in C<= init_buf_l($var);>.
 * In particular, the init_*() macros also convert C<undef> to 0 without
 * ever producing a warning.
 *
 * Second, passing in a read-only SV for a C<oDWORD> parameter will generate
 * a fatal error on output when we try to update the SV.  For C<DWORD>, we
 * won't update a read-only SV since passing in a literal constant for a
 * buffer size is a useful thing to do even though it prevents us from
 * returning the size of data written via that SV.  Since we should use a
 * trunc_*() macro to output the actual data, the user should be able to
 * determine the size of data written based on the size of the scalar we
 * output anyway.
 *
 * This second difference doesn't apply unless the parameter is listed in
 * the OUTPUT: section without specific output instructions.  We define
 * no macros for outputting buffer length parameters so be careful to use
 * C<oDWORD> [for example] for them if and only if they are output-only.
 *
 * Note that C<oDWORD> is the same as C<DWORD> in that, if a defined value
 * is passed in, it is used [and can generate a warning if the value is
 * "not numeric"].  So although C<oDWORD> is for output-only parameters,
 * we still initialize the C variable before calling the API.  This is good
 * in case the parameter isn't always strictly output-only due to upgrades,
 * bugs, etc.
 *
 * Here is a made-up example that shows several cases:
 *
 * # Actual GetDataW() returns length of data written to ioswName, not bool.
 * bool
 * GetDataW( ioswName, ilwName, oswText, iolwText, opJunk, opRec, ilRec, olRec )
 *	WCHAR *	ioswName	= NO_INIT
 *	DWORD	ilwName		= NO_INIT
 *	WCHAR *	oswText		= NO_INIT
 *	DWORD	&iolwText	= init_buf_l($arg);
 *	void *	opJunk		= NO_INIT
 *	BYTE *	opRec		= NO_INIT
 *	DWORD	ilRec		= init_buf_l($arg);
 *	oDWORD	&olRec
 * PREINIT:
 *	DWORD	olwName;
 * INIT:
 *	grow_buf_lw( ioswName,ST(0), ilwName,ST(1) );
 *	grow_buf_lw( oswText,ST(2), iolwText,ST(3) );
 *	grow_buf_typ( opJunk,ST(4),void *, LONG_STRUCT_TYPEDEF );
 *	grow_buf_l( opRec,ST(5),BYTE *, ilRec,ST(6) );
 * CODE:
 *	olwName= GetDataW( ioswName, ilwName, oswText, &iolwText,
 *			   (LONG_STRUCT_TYPEDEF *)opJunk, opRec, &iolRec );
 *	if(  0 == olwName  &&  ERROR_MORE_DATA == GetLastError()
 *	 &&  ( autosize(ST(1)) || autosize(ST(3)) || autosize(ST(6)) )  ) {
 *	    if(  autosize(ST(1))  )
 *		grow_buf_lw( ioswName,ST(0), ilwName,ST(1) );
 *	    if(  autosize(ST(3))  )
 *		grow_buf_lw( oswText,ST(2), iolwText,ST(3) );
 *	    if(  autosize(ST(6))  )
 *		grow_buf_l( opRec,ST(5),BYTE *, iolRec,ST(6) );
 *	    olwName= GetDataW( ioswName, ilwName, oswText, &iolwText,
 *			       (LONG_STRUCT_TYPEDEF *)opJunk, opRec, &iolRec );
 *	}
 *	RETVAL=  0 != olwName;
 * OUTPUT:
 *	RETVAL
 *	ioswName	trunc_buf_lw( RETVAL, ioswName,ST(0), olwName );
 *	oswText		trunc_buf_lw( RETVAL, oswText,ST(2), iolwText );
 *	iolwText
 *	opJunk		trunc_buf_typ(RETVAL,opJunk,ST(4),LONG_STRUCT_TYPEDEF);
 *	opRec		trunc_buf_l( RETVAL, opRec,ST(5), olRec );
 *	olRec
 *
 * The above example would be more complex and less efficient if we used
 * C<DWORD * iolwText> in place of C<DWORD  &iolwText>.  The only possible
 * advantage would be that C<NULL> would be passed in for C<iolwText> if
 * _both_ C<$oswText> and C<$iolwText> were specified as C<[]>.  The *_pl*()
 * macros are defined [and C<DWORD *> specified in F<typemap>] so we can
 * handle those cases but it is usually better to use the *_l*() macros
 * instead by specifying C<&> instead of C<*>.  Using C<&> instead of C<*>
 * is usually better when dealing with scalars, even if they aren't buffer
 * sizes.  But you must use C<*> if it is important for that parameter to
 * be able to pass C<NULL> to the underlying API.
 *
 * In Win32API::, we try to use C<*> for buffer sizes of optional buffers
 * and C<&> for buffer sizes of required buffers.
 *
 * For parameters that are pointers to things other than buffers or buffer
 * sizes, we use C<*> for "important" parameters [so that using C<[]>
 * generates an error rather than fetching the value and just throwing it
 * away], and for optional parameters [in case specifying C<NULL> is or
 * becomes important].  Otherwise we use C<&> [for "unimportant" but
 * required parameters] so the user can specify C<[]> if they don't care
 * about it.  The output handle of an "open" routine is "important".
 */

#ifndef Debug
# define	Debug(list)	/*Nothing*/
#endif

/*#ifndef CAST
 *# ifdef __cplusplus
 *#  define   CAST(type,expr)	static_cast<type>(expr)
 *# else*/
#  define   CAST(type,expr)	(type)(expr)
/*# endif
 *#endif*/

/* Is an argument C<[]>, meaning we should pass C<NULL>? */
#define null_arg(sv)	(  SvROK(sv)  &&  SVt_PVAV == SvTYPE(SvRV(sv))	\
			   &&  -1 == av_len((AV*)SvRV(sv))  )

#define PV_or_null(sv)	( null_arg(sv) ? NULL : SvPV_nolen(sv) )

/* Minimum buffer size to use when no buffer existed: */
#define MIN_GROW_SIZE	128

#ifdef Debug
/* Used in Debug() messages to show which macro call is involved: */
#define string(arg) #arg
#endif

/* Simplify using SvGROW() for byte-sized buffers: */
#define lSvGROW(sv,n)	SvGROW( sv, 0==(n) ? MIN_GROW_SIZE : (n)+1 )

/* Simplify using SvGROW() for WCHAR-sized buffers: */
#define lwSvGROW(sv,n)	CAST( WCHAR *,		\
	SvGROW( sv, sizeof(WCHAR)*( 0==(n) ? MIN_GROW_SIZE : (n)+1 ) ) )

/* Whether the buffer size we got lets us change what buffer size we use: */
#define autosize(sv)	(!(  SvOK(sv)  &&  ! SvROK(sv)		\
			 &&  SvPV_nolen(sv)  &&  '=' == *SvPV_nolen(sv)  ))

/* Get the IV/UV for a parameter that might be C<[]> or C<undef>: */
#define optIV(sv)	( null_arg(sv) ? 0 : !SvOK(sv) ? 0 : SvIV(sv) )
#define optUV(sv)	( null_arg(sv) ? 0 : !SvOK(sv) ? 0 : SvUV(sv) )

/* Allocate temporary storage that will automatically be freed later: */
#ifndef TempAlloc	/* Can be C<#define>d to be C<_alloca>, for example */
# define TempAlloc( size )	sv_grow( sv_newmortal(), size )
#endif

/* Initialize a buffer size argument of type (DWORD *): */
#define init_buf_pl( plSize, svSize, tpSize )		STMT_START {	\
	if(  null_arg(svSize)  )					\
	    plSize= NULL;						\
	else {								\
	    STRLEN n_a;							\
	    *( plSize= CAST( tpSize, TempAlloc(sizeof(*plSize)) ) )=	\
	      autosize(svSize) ? optUV(svSize)				\
	        : strtoul( 1+SvPV(svSize,n_a), NULL, 10 );		\
	} } STMT_END
/* In INPUT section put ": init_buf_pl($var,$arg,$type);" after var name. */

/* Initialize a buffer size argument of type DWORD: */
#define init_buf_l( svSize )						\
	(  null_arg(svSize) ? 0 : autosize(svSize) ? optUV(svSize)	\
	   : strtoul( 1+SvPV_nolen(svSize), NULL, 10 )  )
/* In INPUT section put "= init_buf_l($arg);" after variable name. */

/* Lengths in WCHARs are initialized the same as lengths in bytes: */
#define init_buf_plw	init_buf_pl
#define init_buf_lw	init_buf_l

/* grow_buf_pl() and grow_buf_plw() are included so you can define
 * parameters of type C<DWORD *>, for example.  In practice, it is
 * usually better to define such parameters as "DWORD &". */

/* Grow a buffer where we have a pointer to its size in bytes: */
#define	grow_buf_pl( sBuf,svBuf,tpBuf, plSize,svSize,tpSize ) STMT_START { \
	Debug(("grow_buf_pl( %s==0x%lX,[%s:%ld/%ld, %s==0x%lX:%ld,[%s )\n",\
	  string(sBuf),sBuf,strchr(string(svBuf),'('),SvPOK(svBuf)?	\
	  SvCUR(svBuf):-1,SvPOK(svBuf)?SvLEN(svBuf):-1,string(plSize),	\
	  plSize,plSize?*plSize:-1,strchr(string(svSize),'(')));	\
	if(  null_arg(svBuf)  ) {					\
	    sBuf= NULL;							\
	} else {							\
	    STRLEN n_a;							\
	    if(  NULL == plSize  )					\
		*( plSize= CAST(tpSize,TempAlloc(sizeof(*plSize))) )= 0;\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    sBuf= CAST( tpBuf, lSvGROW( svBuf, *plSize ) );		\
	    if(  autosize(svSize)  )   *plSize= SvLEN(svBuf) - 1;	\
	    Debug(("more buf_pl( %s==0x%lX,[%s:%ld/%ld, %s==0x%lX:%ld,[%s )\n",\
	      string(sBuf),sBuf,strchr(string(svBuf),'('),SvPOK(svBuf)?	\
	      SvCUR(svBuf):-1,SvPOK(svBuf)?SvLEN(svBuf):-1,string(plSize),\
	      plSize,plSize?*plSize:-1,strchr(string(svSize),'(')));	\
	} } STMT_END

/* Grow a buffer where we have a pointer to its size in WCHARs: */
#define	grow_buf_plw( sBuf,svBuf, plwSize,svSize,tpSize ) STMT_START {	\
	if(  null_arg(svBuf)  ) {					\
	    sBuf= NULL;							\
	} else {							\
	    STRLEN n_a;							\
	    if(  NULL == plwSize  )					\
		*( plwSize= CAST(tpSize,TempAlloc(sizeof(*plwSize))) )= 0;\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    sBuf= lwSvGROW( svBuf, *plwSize );				\
	    if(  autosize(svSize)  )					\
		*plwSize= SvLEN(svBuf)/sizeof(WCHAR) - 1;		\
	} } STMT_END

/* Grow a buffer where we have its size in bytes: */
#define	grow_buf_l( sBuf,svBuf,tpBuf, lSize,svSize )	STMT_START {	\
	if(  null_arg(svBuf)  ) {					\
	    sBuf= NULL;							\
	} else {							\
	    STRLEN n_a;							\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    sBuf= CAST( tpBuf, lSvGROW( svBuf, lSize ) );		\
	    if(  autosize(svSize)  )   lSize= SvLEN(svBuf) - 1;		\
	} } STMT_END

/* Grow a buffer where we have its size in WCHARs: */
#define	grow_buf_lw( swBuf,svBuf, lwSize,svSize )	STMT_START {	\
	if(  null_arg(svBuf)  ) {					\
	    swBuf= NULL;						\
	} else {							\
	    STRLEN n_a;							\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    swBuf= lwSvGROW( svBuf, lwSize );				\
	    if(  autosize(svSize)  )					\
		lwSize= SvLEN(svBuf)/sizeof(WCHAR) - 1;			\
	} } STMT_END

/* Grow a buffer that contains the declared fixed data type: */
#define	grow_buf( pBuf,svBuf, tpBuf )			STMT_START {	\
	if(  null_arg(svBuf)  ) {					\
	    pBuf= NULL;							\
	} else {							\
	    STRLEN n_a;							\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    pBuf= CAST( tpBuf, SvGROW( svBuf, sizeof(*pBuf) ) );	\
	} } STMT_END

/* Grow a buffer that contains a fixed data type other than that declared: */
#define	grow_buf_typ( pBuf,svBuf,tpBuf, Type )		STMT_START {	\
	if(  null_arg(svBuf)  ) {					\
	    pBuf= NULL;							\
	} else {							\
	    STRLEN n_a;							\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    pBuf= CAST( tpBuf, SvGROW( svBuf, sizeof(Type) ) );	\
	} } STMT_END

/* Grow a buffer that contains a list of items of the declared data type: */
#define	grow_vect( pBuf,svBuf,tpBuf, cItems )		STMT_START {	\
	if(  null_arg(svBuf)  ) {					\
	    pBuf= NULL;							\
	} else {							\
	    STRLEN n_a;							\
	    if(  ! SvOK(svBuf)  )    sv_setpvn(svBuf,"",0);		\
	    (void) SvPV_force( svBuf, n_a );				\
	    pBuf= CAST( tpBuf, SvGROW( svBuf, sizeof(*pBuf)*cItems ) );	\
	} } STMT_END

/* If call succeeded, set data length to returned length (in bytes): */
#define	trunc_buf_l( bOkay, sBuf,svBuf, lSize )		STMT_START {	\
	if(  bOkay  &&  NULL != sBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, lSize );					\
	} } STMT_END

/* Same as above except we have a pointer to the returned length: */
#define	trunc_buf_pl( bOkay, sBuf,svBuf, plSize )			\
	trunc_buf_l( bOkay, sBuf,svBuf, *plSize )

/* If call succeeded, set data length to returned length (in WCHARs): */
#define	trunc_buf_lw( bOkay, sBuf,svBuf, lwSize )	STMT_START {	\
	if(  bOkay  &&  NULL != sBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, (lwSize)*sizeof(WCHAR) );			\
	} } STMT_END

/* Same as above except we have a pointer to the returned length: */
#define	trunc_buf_plw( bOkay, swBuf,svBuf, plwSize )			\
	trunc_buf_lw( bOkay, swBuf,svBuf, *plwSize )

/* Set data length for a buffer that contains the declared fixed data type: */
#define	trunc_buf( bOkay, pBuf,svBuf )			STMT_START {	\
	if(  bOkay  &&  NULL != pBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, sizeof(*pBuf) );				\
	} } STMT_END

/* Set data length for a buffer that contains some other fixed data type: */
#define	trunc_buf_typ( bOkay, pBuf,svBuf, Type )	STMT_START {	\
	if(  bOkay  &&  NULL != pBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, sizeof(Type) );				\
	} } STMT_END

/* Set length for buffer that contains list of items of the declared type: */
#define	trunc_vect( bOkay, pBuf,svBuf, cItems )		STMT_START {	\
	if(  bOkay  &&  NULL != pBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, sizeof(*pBuf)*cItems );			\
	} } STMT_END

/* Set data length for a buffer where a '\0'-terminate string was stored: */
#define	trunc_buf_z( bOkay, sBuf,svBuf )		STMT_START {	\
	if(  bOkay  &&  NULL != sBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, strlen(sBuf) );				\
	} } STMT_END

/* Set data length for a buffer where a L'\0'-terminate string was stored: */
#define	trunc_buf_zw( bOkay, sBuf,svBuf )		STMT_START {	\
	if(  bOkay  &&  NULL != sBuf  ) {				\
	    SvPOK_only( svBuf );					\
	    SvCUR_set( svBuf, wcslen(sBuf)*sizeof(WCHAR) );		\
	} } STMT_END
