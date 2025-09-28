/* Win32API/File.xs */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
/*#include "patchlevel.h"*/

/* Uncomment the next line unless set "WRITE_PERL=>1" in Makefile.PL: */
#define NEED_newCONSTSUB
#include "ppport.h"

#ifdef WORD
# undef WORD
#endif

#define  WIN32_LEAN_AND_MEAN	/* Tell windows.h to skip much */
#include <wchar.h>
#include <windows.h>
#include <winioctl.h>

/*CONSTS_DEFINED*/

#ifndef INVALID_SET_FILE_POINTER
#   define INVALID_SET_FILE_POINTER	((DWORD)-1)
#endif

#define oDWORD DWORD

#if (PERL_REVISION <= 5 && PERL_VERSION < 5) || defined(__CYGWIN__)
# define win32_get_osfhandle _get_osfhandle
# ifdef __CYGWIN__
#  define win32_open_osfhandle(handle,mode) \
	(Perl_croak(aTHX_ "_open_osfhandle not implemented on Cygwin!"), -1)
# else
#  define win32_open_osfhandle _open_osfhandle
# endif
# ifdef _get_osfhandle
#  undef _get_osfhandle	/* stolen_get_osfhandle() isn't available here */
# endif
# ifdef _open_osfhandle
#  undef _open_osfhandle /* stolen_open_osfhandle() isn't available here */
# endif
#endif

#ifndef XST_mUV
# define XST_mUV(i,v)  (ST(i) = sv_2mortal(newSVuv(v))  )
#endif

#ifndef XSRETURN_UV
# define XSRETURN_UV(v) STMT_START { XST_mUV(0,v);  XSRETURN(1); } STMT_END
#endif

#ifndef DEBUGGING
# define	Debug(list)	/*Nothing*/
#else
# define	Debug(list)	ErrPrintf list
# include <stdarg.h>
    static void
    ErrPrintf( const char *sFmt, ... )
    {
      va_list pAList;
      static char *sEnv= NULL;
      DWORD uErr= GetLastError();
	if(  NULL == sEnv  ) {
	    if(  NULL == ( sEnv= getenv("DEBUG_WIN32API_FILE") )  )
		sEnv= "";
	}
	if(  '\0' == *sEnv  )
	    return;
	va_start( pAList, sFmt );
	vfprintf( stderr, sFmt, pAList );
	va_end( pAList );
	SetLastError( uErr );
    }
#endif /* DEBUGGING */


#include "buffers.h"	/* Include this after DEBUGGING setup finished */

static LONG uLastFileErr= 0;

static void
SaveErr( BOOL bFailed )
{
    if(  bFailed  ) {
	uLastFileErr= GetLastError();
    }
}

MODULE = Win32API::File		PACKAGE = Win32API::File

PROTOTYPES: DISABLE


LONG
_fileLastError( uError=0 )
	DWORD	uError
    CODE:
	if(  1 <= items  ) {
	    uLastFileErr= uError;
	}
	RETVAL= uLastFileErr;
    OUTPUT:
	RETVAL


BOOL
CloseHandle( hObject )
	HANDLE	hObject
    CODE:
        RETVAL = CloseHandle( hObject );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
CopyFileA( sOldFileName, sNewFileName, bFailIfExists )
	char *	sOldFileName
	char *	sNewFileName
	BOOL	bFailIfExists
    CODE:
        RETVAL = CopyFileA( sOldFileName, sNewFileName, bFailIfExists );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
CopyFileW( swOldFileName, swNewFileName, bFailIfExists )
	WCHAR *	swOldFileName
	WCHAR *	swNewFileName
	BOOL	bFailIfExists
    CODE:
        RETVAL = CopyFileW( swOldFileName, swNewFileName, bFailIfExists );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


HANDLE
CreateFileA( sPath, uAccess, uShare, pSecAttr, uCreate, uFlags, hModel )
	char *	sPath
	DWORD	uAccess
	DWORD	uShare
	void *	pSecAttr
	DWORD	uCreate
	DWORD	uFlags
	HANDLE	hModel
    CODE:
	RETVAL= CreateFileA( sPath, uAccess, uShare,
	  (LPSECURITY_ATTRIBUTES)pSecAttr, uCreate, uFlags, hModel );
	if(  INVALID_HANDLE_VALUE == RETVAL  ) {
	    SaveErr( 1 );
	    XSRETURN_NO;
	} else if(  0 == RETVAL  ) {
	    XSRETURN_PV( "0 but true" );
	} else {
	    XSRETURN_UV( PTR2UV(RETVAL) );
	}


HANDLE
CreateFileW( swPath, uAccess, uShare, pSecAttr, uCreate, uFlags, hModel )
	WCHAR *	swPath
	DWORD	uAccess
	DWORD	uShare
	void *	pSecAttr
	DWORD	uCreate
	DWORD	uFlags
	HANDLE	hModel
    CODE:
	RETVAL= CreateFileW( swPath, uAccess, uShare,
	  (LPSECURITY_ATTRIBUTES)pSecAttr, uCreate, uFlags, hModel );
	if(  INVALID_HANDLE_VALUE == RETVAL  ) {
	    SaveErr( 1 );
	    XSRETURN_NO;
	} else if(  0 == RETVAL  ) {
	    XSRETURN_PV( "0 but true" );
	} else {
	    XSRETURN_UV( PTR2UV(RETVAL) );
	}


BOOL
DefineDosDeviceA( uFlags, sDosDeviceName, sTargetPath )
	DWORD	uFlags
	char *	sDosDeviceName
	char *	sTargetPath
    CODE:
        RETVAL = DefineDosDeviceA( uFlags, sDosDeviceName, sTargetPath );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
DefineDosDeviceW( uFlags, swDosDeviceName, swTargetPath )
	DWORD	uFlags
	WCHAR *	swDosDeviceName
	WCHAR *	swTargetPath
    CODE:
        RETVAL = DefineDosDeviceW( uFlags, swDosDeviceName, swTargetPath );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
DeleteFileA( sFileName )
	char *	sFileName
    CODE:
        RETVAL = DeleteFileA( sFileName );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
DeleteFileW( swFileName )
	WCHAR *	swFileName
    CODE:
        RETVAL = DeleteFileW( swFileName );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
DeviceIoControl( hDevice, uIoControlCode, pInBuf, lInBuf, opOutBuf, lOutBuf, olRetBytes, pOverlapped )
	HANDLE	hDevice
	DWORD	uIoControlCode
	char *	pInBuf
	DWORD	lInBuf		= init_buf_l($arg);
	char *	opOutBuf	= NO_INIT
	DWORD	lOutBuf		= init_buf_l($arg);
	oDWORD	&olRetBytes
	void *	pOverlapped
    CODE:
	if(  NULL != pInBuf  ) {
	    if(  0 == lInBuf  ) {
		lInBuf= SvCUR(ST(2));
	    } else if(  SvCUR(ST(2)) < lInBuf  ) {
		croak( "%s: pInBuf shorter than specified (%d < %d)",
		  "Win32API::File::DeviceIoControl", SvCUR(ST(2)), lInBuf );
	    }
	}
	grow_buf_l( opOutBuf,ST(4),char *, lOutBuf,ST(5) );
	RETVAL= DeviceIoControl( hDevice, uIoControlCode, pInBuf, lInBuf,
		  opOutBuf, lOutBuf, &olRetBytes, (LPOVERLAPPED)pOverlapped );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL
	opOutBuf	trunc_buf_l( RETVAL, opOutBuf,ST(4), olRetBytes );
	olRetBytes


HANDLE
FdGetOsFHandle( ivFd )
	int	ivFd
    CODE:
	RETVAL= (HANDLE) win32_get_osfhandle( ivFd );
	SaveErr( INVALID_HANDLE_VALUE == RETVAL );
    OUTPUT:
	RETVAL


DWORD
GetDriveTypeA( sRootPath )
	char *	sRootPath
    CODE:
        RETVAL = GetDriveTypeA( sRootPath );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


DWORD
GetDriveTypeW( swRootPath )
	WCHAR *	swRootPath
    CODE:
        RETVAL = GetDriveTypeW( swRootPath );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


DWORD
GetFileAttributesA( sPath )
	char *	sPath
    CODE:
        RETVAL = GetFileAttributesA( sPath );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


DWORD
GetFileAttributesW( swPath )
	WCHAR *	swPath
    CODE:
        RETVAL = GetFileAttributesW( swPath );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


DWORD
GetFileType( hFile )
	HANDLE	hFile
    CODE:
        RETVAL = GetFileType( hFile );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
GetHandleInformation( hObject, ouFlags )
	HANDLE		hObject
	oDWORD *	ouFlags
    CODE:
        RETVAL = GetHandleInformation( hObject, ouFlags );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL
	ouFlags


DWORD
GetLogicalDrives()
    CODE:
        RETVAL = GetLogicalDrives();
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


DWORD
GetLogicalDriveStringsA( lBufSize, osBuffer )
	DWORD	lBufSize	= init_buf_l($arg);
	char *	osBuffer	= NO_INIT
    CODE:
	grow_buf_l( osBuffer,ST(1),char *, lBufSize,ST(0) );
	RETVAL= GetLogicalDriveStringsA( lBufSize, osBuffer );
	if(  lBufSize < RETVAL  &&  autosize(ST(0))  ) {
	    lBufSize= RETVAL;
	    grow_buf_l( osBuffer,ST(1),char *, lBufSize,ST(0) );
	    RETVAL= GetLogicalDriveStringsA( lBufSize, osBuffer );
	}
	if(  0 == RETVAL  ||  lBufSize < RETVAL  ) {
	    SaveErr( 1 );
	} else {
	    trunc_buf_l( 1, osBuffer,ST(1), RETVAL );
	}
    OUTPUT:
	RETVAL
	osBuffer	;/* The code for this appears above. */


DWORD
GetLogicalDriveStringsW( lwBufSize, oswBuffer )
	DWORD	lwBufSize	= init_buf_lw($arg);
	WCHAR *	oswBuffer	= NO_INIT
    CODE:
	grow_buf_lw( oswBuffer,ST(1), lwBufSize,ST(0) );
	RETVAL= GetLogicalDriveStringsW( lwBufSize, oswBuffer );
	if(  lwBufSize < RETVAL  &&  autosize(ST(0))  ) {
	    lwBufSize= RETVAL;
	    grow_buf_lw( oswBuffer,ST(1), lwBufSize,ST(0) );
	    RETVAL= GetLogicalDriveStringsW( lwBufSize, oswBuffer );
	}
	if(  0 == RETVAL  ||  lwBufSize < RETVAL  ) {
	    SaveErr( 1 );
	} else {
	    trunc_buf_lw( 1, oswBuffer,ST(1), RETVAL );
	}
    OUTPUT:
	RETVAL
	oswBuffer	;/* The code for this appears above. */


BOOL
GetVolumeInformationA( sRootPath, osVolName, lVolName, ouSerialNum, ouMaxNameLen, ouFsFlags, osFsType, lFsType )
	char *	sRootPath
	char *	osVolName	= NO_INIT
	DWORD	lVolName	= init_buf_l($arg);
	oDWORD	&ouSerialNum	= optUV($arg);
	oDWORD	&ouMaxNameLen	= optUV($arg);
	oDWORD	&ouFsFlags	= optUV($arg);
	char *	osFsType	= NO_INIT
	DWORD	lFsType		= init_buf_l($arg);
    CODE:
	grow_buf_l( osVolName,ST(1),char *, lVolName,ST(2) );
	grow_buf_l( osFsType,ST(6),char *, lFsType,ST(7) );
	RETVAL= GetVolumeInformationA( sRootPath, osVolName, lVolName,
		  &ouSerialNum, &ouMaxNameLen, &ouFsFlags, osFsType, lFsType );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL
	osVolName	trunc_buf_z( RETVAL, osVolName,ST(1) );
	osFsType	trunc_buf_z( RETVAL, osFsType,ST(6) );
	ouSerialNum
	ouMaxNameLen
	ouFsFlags


BOOL
GetVolumeInformationW( swRootPath, oswVolName, lwVolName, ouSerialNum, ouMaxNameLen, ouFsFlags, oswFsType, lwFsType )
	WCHAR *	swRootPath
	WCHAR *	oswVolName	= NO_INIT
	DWORD	lwVolName	= init_buf_lw($arg);
	oDWORD	&ouSerialNum	= optUV($arg);
	oDWORD	&ouMaxNameLen	= optUV($arg);
	oDWORD	&ouFsFlags	= optUV($arg);
	WCHAR *	oswFsType	= NO_INIT
	DWORD	lwFsType	= init_buf_lw($arg);
    CODE:
	grow_buf_lw( oswVolName,ST(1), lwVolName,ST(2) );
	grow_buf_lw( oswFsType,ST(6), lwFsType,ST(7) );
	RETVAL= GetVolumeInformationW( swRootPath, oswVolName, lwVolName,
	  &ouSerialNum, &ouMaxNameLen, &ouFsFlags, oswFsType, lwFsType );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL
	oswVolName	trunc_buf_zw( RETVAL, oswVolName,ST(1) );
	oswFsType	trunc_buf_zw( RETVAL, oswFsType,ST(6) );
	ouSerialNum
	ouMaxNameLen
	ouFsFlags


BOOL
IsRecognizedPartition( ivPartitionType )
	int	ivPartitionType
    CODE:
        RETVAL = IsRecognizedPartition( ivPartitionType );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
IsContainerPartition( ivPartitionType )
	int	ivPartitionType
    CODE:
        RETVAL = IsContainerPartition( ivPartitionType );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
MoveFileA( sOldName, sNewName )
	char *	sOldName
	char *	sNewName
    CODE:
        RETVAL = MoveFileA( sOldName, sNewName );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
MoveFileW( swOldName, swNewName )
	WCHAR *	swOldName
	WCHAR *	swNewName
    CODE:
        RETVAL = MoveFileW( swOldName, swNewName );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
MoveFileExA( sOldName, sNewName, uFlags )
	char *	sOldName
	char *	sNewName
	DWORD	uFlags
    CODE:
        RETVAL = MoveFileExA( sOldName, sNewName, uFlags );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
MoveFileExW( swOldName, swNewName, uFlags )
	WCHAR *	swOldName
	WCHAR *	swNewName
	DWORD	uFlags
    CODE:
        RETVAL = MoveFileExW( swOldName, swNewName, uFlags );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


long
OsFHandleOpenFd( hOsFHandle, uMode )
	long	hOsFHandle
	DWORD	uMode
    CODE:
	RETVAL= win32_open_osfhandle( hOsFHandle, uMode );
	if(  RETVAL < 0  ) {
	    SaveErr( 1 );
	    XSRETURN_NO;
	} else if(  0 == RETVAL  ) {
	    XSRETURN_PV( "0 but true" );
	} else {
	    XSRETURN_IV( (IV) RETVAL );
	}


DWORD
QueryDosDeviceA( sDeviceName, osTargetPath, lTargetBuf )
	char *	sDeviceName
	char *	osTargetPath	= NO_INIT
	DWORD	lTargetBuf	= init_buf_l($arg);
    CODE:
	grow_buf_l( osTargetPath,ST(1),char *, lTargetBuf,ST(2) );
	RETVAL= QueryDosDeviceA( sDeviceName, osTargetPath, lTargetBuf );
	SaveErr( 0 == RETVAL );
    OUTPUT:
	RETVAL
	osTargetPath	trunc_buf_l( 1, osTargetPath,ST(1), RETVAL );


DWORD
QueryDosDeviceW( swDeviceName, oswTargetPath, lwTargetBuf )
	WCHAR *	swDeviceName
	WCHAR *	oswTargetPath	= NO_INIT
	DWORD	lwTargetBuf	= init_buf_lw($arg);
    CODE:
	grow_buf_lw( oswTargetPath,ST(1), lwTargetBuf,ST(2) );
	RETVAL= QueryDosDeviceW( swDeviceName, oswTargetPath, lwTargetBuf );
	SaveErr( 0 == RETVAL );
    OUTPUT:
	RETVAL
	oswTargetPath	trunc_buf_lw( 1, oswTargetPath,ST(1), RETVAL );


BOOL
ReadFile( hFile, opBuffer, lBytes, olBytesRead, pOverlapped )
	HANDLE	hFile
	BYTE *	opBuffer	= NO_INIT
	DWORD	lBytes		= init_buf_l($arg);
	oDWORD	&olBytesRead
	void *	pOverlapped
    CODE:
	grow_buf_l( opBuffer,ST(1),BYTE *, lBytes,ST(2) );
	/* Don't read more bytes than asked for if buffer is already big: */
	lBytes= init_buf_l(ST(2));
	if(  0 == lBytes  &&  autosize(ST(2))  ) {
	    lBytes= SvLEN( ST(1) ) - 1;
	}
	RETVAL= ReadFile( hFile, opBuffer, lBytes, &olBytesRead,
		  (LPOVERLAPPED)pOverlapped );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL
	opBuffer	trunc_buf_l( RETVAL, opBuffer,ST(1), olBytesRead );
	olBytesRead


BOOL
GetOverlappedResult( hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait)
	HANDLE hFile
	LPOVERLAPPED lpOverlapped
	LPDWORD lpNumberOfBytesTransferred
	BOOL bWait
    CODE:
    	RETVAL= GetOverlappedResult( hFile, lpOverlapped,
	 lpNumberOfBytesTransferred, bWait);
	SaveErr( !RETVAL );
    OUTPUT:
    	RETVAL
	lpOverlapped
	lpNumberOfBytesTransferred

DWORD
GetFileSize( hFile, lpFileSizeHigh )
	HANDLE hFile
	LPDWORD lpFileSizeHigh
    CODE:
    	RETVAL= GetFileSize( hFile, lpFileSizeHigh );
	SaveErr( NO_ERROR != GetLastError() );
    OUTPUT:
    	RETVAL
	lpFileSizeHigh

UINT
SetErrorMode( uNewMode )
	UINT	uNewMode


LONG
SetFilePointer( hFile, ivOffset, ioivOffsetHigh, uFromWhere )
	HANDLE	hFile
	LONG	ivOffset
	LONG *	ioivOffsetHigh
	DWORD	uFromWhere
    CODE:
	RETVAL= SetFilePointer( hFile, ivOffset, ioivOffsetHigh, uFromWhere );
	if(  RETVAL == INVALID_SET_FILE_POINTER && (GetLastError() != NO_ERROR)  ) {
	    SaveErr( 1 );
	    XST_mNO(0);
	} else if(  0 == RETVAL  ) {
	    XST_mPV(0,"0 but true");
	} else {
	    XST_mIV(0,RETVAL);
	}
    OUTPUT:
	ioivOffsetHigh


BOOL
SetHandleInformation( hObject, uMask, uFlags )
	HANDLE	hObject
	DWORD	uMask
	DWORD	uFlags
    CODE:
        RETVAL = SetHandleInformation( hObject, uMask, uFlags );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL


BOOL
WriteFile( hFile, pBuffer, lBytes, ouBytesWritten, pOverlapped )
	HANDLE		hFile
	BYTE *		pBuffer
	DWORD		lBytes		= init_buf_l($arg);
	oDWORD	&ouBytesWritten
	void *		pOverlapped
    CODE:
	/* SvCUR(ST(1)) might "panic" if pBuffer isn't valid */
	if(  0 == lBytes  ) {
	    lBytes= SvCUR(ST(1));
	} else if(  SvCUR(ST(1)) < lBytes  ) {
	    croak( "%s: pBuffer value too short (%d < %d)",
	      "Win32API::File::WriteFile", SvCUR(ST(1)), lBytes );
	}
	RETVAL= WriteFile( hFile, pBuffer, lBytes,
		  &ouBytesWritten, (LPOVERLAPPED)pOverlapped );
	SaveErr( !RETVAL );
    OUTPUT:
	RETVAL
	ouBytesWritten

void
GetStdHandle(fd)
    DWORD fd
PPCODE:
#ifdef _WIN64
    XSRETURN_IV((DWORD_PTR)GetStdHandle(fd));
#else
    XSRETURN_IV((DWORD)GetStdHandle(fd));
#endif

void
SetStdHandle(fd,handle)
    DWORD fd
    HANDLE handle
PPCODE:
    if (SetStdHandle(fd, handle))
	XSRETURN_YES;
    else
	XSRETURN_NO;
