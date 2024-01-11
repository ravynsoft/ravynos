#ifndef _salt_debug_h
#define _salt_debug_h

/*
 * Copyright 2003, 2004 Porchdog Software. All rights reserved.
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
 *	either expressed or implied, of Porchdog Software.
 */

#include <salt/platform.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C"
{
#endif


#define SW_LOG_WARNING     1 << 0
#define SW_LOG_ERROR       1 << 1
#define SW_LOG_NOTICE      1 << 2
#define SW_LOG_VERBOSE     1 << 3
#define SW_LOG_OFF         0x0


#if (defined( __GNUC__))

#	if ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 3)))

#		define  __C99_VA_ARGS__	1

#		define  __GNU_VA_ARGS__	0

#	else

#		define  __C99_VA_ARGS__	0

#		define  __GNU_VA_ARGS__	1

#	endif

#else

#	define  __C99_VA_ARGS__		0

#	define  __GNU_VA_ARGS__		0

#endif


# if ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 9)))

#	define	__SW_FUNCTION__			__func__

#elif (defined( __GNUC__))

#	define	__SW_FUNCTION__			__PRETTY_FUNCTION__

#elif( defined(_MSC_VER ) && !defined(_WIN32_WCE))

#	define	__SW_FUNCTION__			__FUNCTION__

#else

#	define	__SW_FUNCTION__			""

#endif


#define sw_check(expr, label, action)			\
do 														\
{															\
	if (!(expr)) 										\
	{														\
		{													\
			action;										\
		}													\
		goto label;										\
	}														\
} while (0)


#define sw_check_log(expr, label, action)		\
do 														\
{															\
	if (!(expr)) 										\
	{														\
		sw_print_assert(0, NULL, __FILE__, __SW_FUNCTION__, __LINE__);	\
		{													\
			action;										\
		}													\
		goto label;										\
	}														\
} while (0)


#define sw_check_okay(code, label)				\
do 														\
{															\
	if ((int) code != 0) 							\
	{														\
		goto label;										\
	}														\
} while (0)


#define sw_check_okay_log(code, label)			\
do 														\
{															\
	if ((int) code != 0) 							\
	{														\
		sw_print_assert((int) code, NULL, __FILE__, __SW_FUNCTION__, __LINE__);	\
		goto label;										\
	}														\
} while ( 0 )


#define sw_translate_error(expr, errno)		((expr) ? 0 : (errno))


#if defined(WIN32)

#	define sw_socket_errno()		(int) WSAGetLastError()
#	define sw_set_socket_errno(X)	WSASetLastError(X)
#	define sw_system_errno()		(int) GetLastError()
#	define sw_set_system_errno(X)	SetLastError(X)

#else

#	define sw_socket_errno()		errno
#	define sw_set_socket_errno(X)	errno = X
#	define sw_system_errno()		errno
#	define sw_set_system_errno(X)	errno = X

#endif


#if !defined(NDEBUG)

#	define sw_assert(X)		\
									\
	do								\
	{								\
		if (!(X))				\
		{							\
			sw_print_assert( 0, #X, __FILE__, __SW_FUNCTION__, __LINE__); \
		}							\
	} while( 0 )

#else

#	define sw_assert(X)

#endif


void HOWL_API
sw_print_assert(
		int					code,
		sw_const_string	assert_string,
		sw_const_string	file,
		sw_const_string	func,
		int					line);


#if !defined(NDEBUG)

void HOWL_API
sw_print_debug(
		int					level,
		sw_const_string	format,
		...);

#	if (__C99_VA_ARGS__)

#		define  sw_debug(...)			sw_print_debug(__VA_ARGS__)

#	else

#		define  sw_debug					sw_print_debug

#	endif

#else

#	if (__C99_VA_ARGS__)

#		define  sw_debug(...)

#	else

#		define  sw_debug					while( 0 )

#	endif

#endif


#define SW_UNUSED_PARAM(X)	(void) (X)


#if defined(__cplusplus)
}
#endif


#endif
