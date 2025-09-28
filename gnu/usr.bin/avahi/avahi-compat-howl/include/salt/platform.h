#ifndef _sw_platform_h
#define _sw_platform_h

/*
 * Copyright 2003, 2004 Porchdog Software, Inc. All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without modification,
 *	are permitted provided that the following conditions are met:
 *
 *		1. Redistributions of source code must retain the above copyright notice,
 *		   this list of conditions and the following disclaimer.
 *		2. Redistributions in binary form must reproduce the above copyright notice,
 *		   this list of conditions and the following disclaimer in the documentation
 *		   and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY PORCHDOG SOFTWARE ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE HOWL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *	OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	The views and conclusions contained in the software and documentation are those
 *	of the authors and should not be interpreted as representing official policies,
 *	either expressed or implied, of Porchdog Software, Inc.
 */


#ifdef __cplusplus
extern "C"
{
#endif


#if defined(__VXWORKS__)

#	define HOWL_API
#	include <vxworks.h>
#	include <sysLib.h>

#	define sw_snooze(SECS)		taskDelay(sysClkRateGet() * SECS)

#elif defined(WIN32)

#	define WIN32_LEAN_AND_MEAN
#	define HOWL_API __stdcall
#	pragma warning(disable:4127)
#	include <windows.h>
#	include <stdlib.h>

typedef signed char				int8_t;
typedef unsigned char			u_int8_t;
typedef signed short				int16_t;
typedef unsigned short			u_int16_t;
typedef signed long				int32_t;
typedef unsigned long			u_int32_t;
typedef _int64						int64_t;
typedef _int64						u_int64_t;

#	define sw_snooze(SECS)		Sleep(SECS * 1000)

#else

#	define HOWL_API
#	if defined(HOWL_KERNEL)
#		include <howl_config.h>
#	endif
#	include <sys/types.h>
#	include <stdlib.h>
#	include <unistd.h>

#	define sw_snooze(SECS)		sleep(SECS)

#endif

#if defined(__sun)

#	define u_int8_t	uint8_t
#	define u_int16_t	uint16_t
#	define u_int32_t	uint32_t
#	define u_int64_t	uint64_t

#endif

typedef void				*	sw_opaque;
typedef void				*	sw_opaque_t;
typedef int8_t					sw_int8;
typedef u_int8_t				sw_uint8;
typedef u_int8_t				sw_bool;
typedef int16_t				sw_int16;
typedef u_int16_t				sw_uint16;
typedef int32_t				sw_int32;
typedef u_int32_t				sw_uint32;
typedef int64_t				sw_int64;
typedef u_int64_t				sw_uint64;
typedef char				*	sw_string;
typedef sw_uint8			*	sw_octets;
#if !defined(__VXWORKS__) || defined(__cplusplus)
typedef const char		*	sw_const_string;
typedef const u_int8_t	*	sw_const_octets;
#else
typedef char				*	sw_const_string;
typedef u_int8_t			*	sw_const_octets;
#endif
typedef size_t					sw_size_t;
typedef int						sw_result;



/* --------------------------------------------------------
 *
 * Endian-osity
 *
 * SW_ENDIAN is 0 for big endian platforms, 1
 * for little endian platforms.
 *
 * The macro WORDS_BIGENDIAN will be defined
 * by autoconf.  If you are using Howl on
 * a platform  that doesn't have autoconf, define
 * SW_ENDIAN directly
 * --------------------------------------------------------
 */

#if !defined(SW_ENDIAN)

#	if defined(WORDS_BIGENDIAN) && WORDS_BIGENDIAN == 1

#		define SW_ENDIAN	0

#	else

#		define SW_ENDIAN	1

#	endif

#endif


/* --------------------------------------------------------
 *
 * Strings
 *
 * These macros supports cross platform string functions
 * for the following OSes
 *
 * Win32
 * *NIX
 * PalmOS
 * VxWorks
 *
 * --------------------------------------------------------
 */

#if defined(WIN32)

#	include <string.h>

#	define sw_memset(ARG1, ARG2, ARG3)		memset((char*) ARG1, ARG2, ARG3)
#	define sw_memcpy(ARG1, ARG2, ARG3)		memmove((char*) ARG1, (char*) ARG2, ARG3)
#	define sw_memcmp(ARG1, ARG2, ARG3)		memcmp((char*) ARG1, ARG2, ARG3)
#	define sw_strcasecmp(ARG1, ARG2)			stricmp(ARG1, ARG2)
#	define sw_strncasecmp(ARG1, ARG2)		strnicmp(ARG1, ARG2)
#	define sw_strcat(ARG1, ARG2)				strcat(ARG1, ARG2)
#	define sw_strncat(ARG1, ARG2)				strncat(ARG1, ARG2)
#	define sw_strchr(ARG1, ARG2)				strchr(ARG1, ARG2)
#	define sw_strcmp(ARG1, ARG2)				strcmp(ARG1, ARG2)
#	define sw_strncmp(ARG1, ARG2)				strncmp(ARG1, ARG2)
#	define sw_strcoll(ARG1, ARG2)				strcoll(ARG1, ARG2)
#	define sw_strcpy(ARG1, ARG2)				(ARG2) ? strcpy(ARG1, ARG2) : strcpy(ARG1, "")
#	define sw_strncpy(ARG1, ARG2, N)			(ARG2) ? strncpy(ARG1, ARG2, N) : strcpy(ARG1, "")
#	define sw_strcspn(ARG1, ARG2)				strcspn(ARG1, ARG2)
#	define sw_strlen(ARG1)						strlen(ARG1)
#	define sw_strstr(ARG1, ARG2)				strstr(ARG1, ARG2)
#	define sw_strtok_r(ARG1, ARG2, ARG3)	strtok_r(ARG1, ARG2, ARG3)

#elif defined(__VXWORKS__)

#	include <string.h>

extern sw_int32
sw_strcasecmp(
		sw_const_string	arg1,
		sw_const_string	arg2);

extern sw_int32
sw_strncasecmp(
		sw_const_string	arg1,
		sw_const_string	arg2,
		sw_len				n);

extern sw_string
sw_strtok_r(
		sw_string			arg1,
		sw_const_string	arg2,
		sw_string		*	lasts);

#	define sw_memset(ARG1, ARG2, ARG3)		memset((char*) ARG1, ARG2, ARG3)
#	define sw_memcpy(ARG1, ARG2, ARG3)		memcpy((char*) ARG1, (char*) ARG2, ARG3)
#	define sw_memcmp(ARG1, ARG2, ARG3)		memcmp((char*) ARG1, ARG2, ARG3)
#	define sw_strcat(ARG1, ARG2)				strcat(ARG1, ARG2)
#	define sw_strncat(ARG1, ARG2)				strncat(ARG1, ARG2)
#	define sw_strchr(ARG1, ARG2)				strchr(ARG1, ARG2)
#	define sw_strcmp(ARG1, ARG2)				strcmp(ARG1, ARG2)
#	define sw_strncmp(ARG1, ARG2)				strncmp(ARG1, ARG2)
#	define sw_strcoll(ARG1, ARG2)				strcoll(ARG1, ARG2)
#	define sw_strcpy(ARG1, ARG2)				ARG2 ? strcpy(ARG1, ARG2) : strcpy(ARG1, "")
#	define sw_strncpy(ARG1, ARG2, N)			ARG2 ? strncpy(ARG1, ARG2, N) : strcpy(ARG1, "")
#	define sw_strcspn(ARG1, ARG2)				strcspn(ARG1, ARG2)
#	define sw_strlen(ARG1)						strlen(ARG1)
#	define sw_strstr(ARG1, ARG2)				strstr(ARG1, ARG2)

#elif defined(__PALMOS__)

#	include <StringMgr.h>

#	define sw_strcasecmp(ARG1, ARG2)			strcasecmp(ARG1, ARG2)
#	define sw_strncasecmp(ARG1, ARG2)		strncasecmp(ARG1, ARG2)
#	define sw_strcat(ARG1, ARG2)				StrCat(ARG1, ARG2)
#	define sw_strncat(ARG1, ARG2)				StrNCat(ARG1, ARG2)
#	define sw_strchr(ARG1, ARG2)				StrChr(ARG1, ARG2)
#	define sw_strcmp(ARG1, ARG2)				StrCampare(ARG1, ARG2)
#	define sw_strncmp(ARG1, ARG2)				StrNCompare(ARG1, ARG2)
#	define sw_strcoll(ARG1, ARG2)				strcoll(ARG1, ARG2)
#	define sw_strcpy(ARG1, ARG2)				ARG2 ? StrCopy(ARG1, ARG2) : StrCopy(ARG1, "")
#	define sw_strncpy(ARG1, ARG2, N)			ARG2 ? StrNCopy(ARG1, ARG2, N) : StrCopy(ARG1, "")
#	define sw_strcspn(ARG1, ARG2)				strcspn(ARG1, ARG2)
#	define sw_strlen(ARG1)						StrLen(ARG1)
#	define sw_strstr(ARG1, ARG2)				strstr(ARG1, ARG2)
#	define sw_strtok_r(ARG1, ARG2, ARG3)	strtok_r(ARG1, ARG2, ARG3)

#else

#	include <string.h>

#	if defined(__Lynx__)
		char * strchr(char*, int);
#	endif

#	define sw_memset(ARG1, ARG2, ARG3)		memset((char*) ARG1, ARG2, ARG3)
#	define sw_memcpy(ARG1, ARG2, ARG3)		memcpy((char*) ARG1, (char*) ARG2, ARG3)
#	define sw_memcmp(ARG1, ARG2, ARG3)		memcmp((char*) ARG1, ARG2, ARG3)
#	define sw_strcasecmp(ARG1, ARG2)			strcasecmp(ARG1, ARG2)
#	define sw_strncasecmp(ARG1, ARG2)		strncasecmp(ARG1, ARG2)
#	define sw_strcat(ARG1, ARG2)				strcat(ARG1, ARG2)
#	define sw_strncat(ARG1, ARG2)				strncat(ARG1, ARG2)
#	define sw_strchr(ARG1, ARG2)				strchr(ARG1, ARG2)
#	define sw_strcmp(ARG1, ARG2)				strcmp(ARG1, ARG2)
#	define sw_strncmp(ARG1, ARG2)				strncmp(ARG1, ARG2)
#	define sw_strcoll(ARG1, ARG2)				strcoll(ARG1, ARG2)
#	define sw_strcpy(ARG1, ARG2)				ARG2 ? strcpy(ARG1, ARG2) : strcpy(ARG1, "")
#	define sw_strncpy(ARG1, ARG2, N)			ARG2 ? strncpy(ARG1, ARG2, N) : strcpy(ARG1, "")
#	define sw_strcspn(ARG1, ARG2)				strcspn(ARG1, ARG2)
#	define sw_strlen(ARG1)						strlen(ARG1)
#	define sw_strstr(ARG1, ARG2)				strstr(ARG1, ARG2)
#	define sw_strtok_r(ARG1, ARG2, ARG3)	strtok_r(ARG1, ARG2, ARG3)

#endif


sw_string
sw_strdup(
		sw_const_string str);


/* --------------------------------------------------------
 *
 * Memory
 *
 * These macros support cross platform heap functions.
 * When compiling with DEBUG, some extra checking is
 * done which can aid in tracking down heap corruption
 * problems
 *
 * --------------------------------------------------------
 */

#if defined(NDEBUG)

#	define	sw_malloc(SIZE)		malloc(SIZE)
#	define	sw_realloc(MEM,SIZE)	realloc(MEM, SIZE)
#	define	sw_free(MEM)			if (MEM) free(MEM)

#else

#	define	sw_malloc(SIZE)		_sw_debug_malloc(SIZE, __SW_FUNCTION__, __FILE__, __LINE__)
#	define	sw_realloc(MEM,SIZE)	_sw_debug_realloc(MEM, SIZE, __SW_FUNCTION__, __FILE__, __LINE__)
#	define	sw_free(MEM)			if (MEM) _sw_debug_free(MEM, __SW_FUNCTION__, __FILE__, __LINE__)

#endif


sw_opaque HOWL_API
_sw_debug_malloc(
			sw_size_t			size,
			sw_const_string	function,
			sw_const_string	file,
			sw_uint32			line);


sw_opaque HOWL_API
_sw_debug_realloc(
			sw_opaque_t			mem,
			sw_size_t			size,
			sw_const_string	function,
			sw_const_string	file,
			sw_uint32			line);


void HOWL_API
_sw_debug_free(
			sw_opaque_t			mem,
			sw_const_string	function,
			sw_const_string	file,
			sw_uint32			line);



/* --------------------------------------------------------
 *
 * Sockets
 *
 * These macros and APIs support cross platform socket
 * calls.  I am relying on BSD APIs, but even with those
 * there are subtle and not so subtle platform differences
 *
 * --------------------------------------------------------
 */

#if defined(__VXWORKS__)

#	include <vxworks.h>
#	include <hostLib.h>
#	include <sockLib.h>
#	include <ioLib.h>
#	include <inetLib.h>

typedef int							sw_sockdesc_t;
typedef socklen_t					sw_socklen_t;

#elif defined(WIN32)

#	include <winsock2.h>

typedef SOCKET						sw_sockdesc_t;
typedef int							sw_socklen_t;

#	define SW_E_WOULDBLOCK		WSAEWOULDBLOCK
#	define SW_INVALID_SOCKET	INVALID_SOCKET
#	define SW_SOCKET_ERROR		SOCKET_ERROR

#	define sw_close_socket(X)	closesocket(X)

#else

#	if defined(sun)

#		include <unistd.h>

#	endif

#	include <sys/types.h>
#	include <signal.h>

#	if defined(__Lynx__)

#		include <socket.h>

#	else

#		include <sys/socket.h>

#	endif

#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <netdb.h>
#	include <arpa/inet.h>
#	include <stdlib.h>
#	include <unistd.h>
#	include <sys/ioctl.h>
#	include <stdio.h>
#	include <errno.h>

typedef sw_int32					sw_sockdesc_t;
typedef socklen_t					sw_socklen_t;

#	define SW_E_WOULDBLOCK		EWOULDBLOCK
#	define SW_INVALID_SOCKET	-1
#	define SW_SOCKET_ERROR		-1

#	define sw_close_socket(X)	close(X)

#endif


/* --------------------------------------------------------
 *
 * strerror()
 *
 * This function will print a string rep of a system error
 * code
 *
 * --------------------------------------------------------
 */

sw_const_string
sw_strerror();


/*
 * Obsolete types and macros.
 *
 * These are here for backwards compatibility, but will
 * be removed in the future
 */
#define sw_char	sw_int8
#define sw_uchar	sw_uint8
#define sw_octet	sw_uint8
#define sw_short	sw_int16
#define sw_ushort	sw_uint16
#define sw_long	sw_int32
#define sw_ulong	sw_uint32


#define SW_TRY(EXPR) { sw_result result; if ((result = EXPR) != SW_OKAY) return result; } ((void) 0)
#define SW_TRY_GOTO(EXPR)  { if ((result = EXPR) != SW_OKAY) goto exit; } ((void) 0)


#ifdef __cplusplus
}
#endif


#endif
