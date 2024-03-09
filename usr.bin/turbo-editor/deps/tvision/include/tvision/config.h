/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   CONFIG.H                                                              */
/*                                                                         */
/*   miscellaneous system-wide configuration parameters                    */
/*   FOR INTERNAL USE ONLY                                                 */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __CONFIG_H )
#define __CONFIG_H

#if !defined( __LIMITS_H )
#include <limits.h>
#endif  // __LIMITS_H

const int eventQSize = 16;
const int keyEventQSize = 3;
const int maxCollectionSize = (int)(( (unsigned long) UINT_MAX - 16)/sizeof( void * ));

const int maxViewWidth = 132;

const int maxFindStrLen    = 80;
const int maxReplaceStrLen = 80;

#endif  // __CONFIG_H
