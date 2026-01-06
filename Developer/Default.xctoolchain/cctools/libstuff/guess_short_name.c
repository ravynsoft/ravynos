/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef RLD
#include <string.h>
#include "stuff/bool.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/guess_short_name.h"

static char *look_back_for_slash(
    char *name,
    char *p);

/*
 * guess_short_name() is passed a name of a dynamic library and returns a guess
 * on what the short name is.  Then name is returned as a pointer to allocated
 * memory that can be free()'ed later.  The name of the dynamic library is
 * recognized as a framework if it has one of the two following forms:
 *	Foo.framework/Versions/A/Foo
 *	Foo.framework/Foo
 * Where A and Foo can be any string.  And may contain a trailing suffix
 * starting with an underbar.  If the name is recognized as a framework then
 * *is_framework is set to TRUE else it is set to FALSE.  If the name has a
 * suffix then *return_suffix is set to allocated memory that contains the
 * suffix else it is set to NULL.
 *
 * The name of the dynamic library is recognized as a library name if it has
 * one of the two following forms:
 *	libFoo.A.dylib
 *	libFoo.dylib
 * The library may have a suffix trailing the name Foo of the form:
 *	libFoo_profile.A.dylib
 *	libFoo_profile.dylib
 *
 * The name of the dynamic library is also recognized as a library name if it
 * has the following form:
 *	Foo.qtx
 *
 * If the name of the dynamic library is none of the forms above then NULL is
 * returned.
 */
/*
 * MDT 20190119 rdar://12400897
 * guess_short_name() will require suffixes be either "_debug" or "_profile".
 * If a '_' substring has any other value, guess_short_name() will assume that
 * substring is part of the short name. This is because a large number of dylib
 * authors -- 1st party and 3rd party -- don't realize '_' is a reserved
 * character in dylib names, and some cctools get confused.
 */
__private_extern__
char *
guess_short_name(
char *name,
enum bool *is_framework,
char **return_suffix)
{
    char *foo, *a, *b, *c, *d, *suffix, *guess;
    unsigned long l, s;

	*is_framework = FALSE;
	*return_suffix = NULL;
  
	/* pull off the last component and make foo point to it */
	a = strrchr(name, '/');
	if(a == NULL)
	    goto guess_library;
	if(a == name)
	    goto guess_library;
	foo = a + 1;
	l = strlen(foo);
	
	/* look for a suffix starting with a '_' */
	suffix = strrchr(foo, '_');
	if(suffix != NULL){
	    s = strlen(suffix);
	    if(suffix == foo || s < 2)
		suffix = NULL;
	    else if (0 != strncmp("_debug", suffix, 6) &&
		     0 != strncmp("_profile", suffix, 8)) {
		suffix = NULL;
	    }
	    else{
		l -= s;
		*return_suffix = allocate(s + 1);
		strncpy(*return_suffix, suffix, s);
		(*return_suffix)[s] = '\0';
	    }
	}

	/* first look for the form Foo.framework/Foo */
	b = look_back_for_slash(name, a);
	if(b == NULL){
	    if(strncmp(name, foo, l) == 0 &&
	       strncmp(name + l, ".framework/", sizeof(".framework/")-1 ) == 0){
		guess = allocate(l + 1);
		strncpy(guess, name, l);
		guess[l] = '\0';
		*is_framework = TRUE;
		return(guess);
	    }
	    else
		goto guess_library;
	}
	else{
	    if(strncmp(b+1, foo, l) == 0 &&
	       strncmp(b+1 + l, ".framework/", sizeof(".framework/")-1 ) == 0){
		guess = allocate(l + 1);
		strncpy(guess, b+1, l);
		guess[l] = '\0';
		*is_framework = TRUE;
		return(guess);
	    }
	}

	/* next look for the form Foo.framework/Versions/A/Foo */
	if(b == name)
	    goto guess_library;
	c = look_back_for_slash(name, b);
	if(c == NULL ||
	   c == name ||
	   strncmp(c+1, "Versions/", sizeof("Versions/")-1) != 0)
	    goto guess_library;
	d = look_back_for_slash(name, c);
	if(d == NULL){
	    if(strncmp(name, foo, l) == 0 &&
	       strncmp(name + l, ".framework/", sizeof(".framework/")-1 ) == 0){
		guess = allocate(l + 1);
		strncpy(guess, name, l);
		guess[l] = '\0';
		*is_framework = TRUE;
		return(guess);
	    }
	    else
		goto guess_library;
	}
	else{
	    if(strncmp(d+1, foo, l) == 0 &&
	       strncmp(d+1 + l, ".framework/", sizeof(".framework/")-1 ) == 0){
		guess = allocate(l + 1);
		strncpy(guess, d+1, l);
		guess[l] = '\0';
		*is_framework = TRUE;
		return(guess);
	    }
	    else
		goto guess_library;
	}

guess_library:
	/* pull off the suffix after the "." and make a point to it */
	a = strrchr(name, '.');
	if(a == NULL)
	    return(NULL);
	if(a == name)
	    return(NULL);
	if(strcmp(a, ".dylib") != 0)
	    goto guess_qtx;

	/* first pull off the version letter for the form Foo.A.dylib */
	if(a - name >= 3 && a[-2] == '.')
	    a = a - 2;

	b = look_back_for_slash(name, a);
	if(b == name)
	    return(NULL);
	if(b == NULL){
	    /* ignore any suffix after an underbar
	       like Foo_profile.A.dylib */
	    c = strchr(name, '_');
	    if (c != NULL && c != name &&
	       (0 == strncmp("_debug", c, 6) ||
		0 == strncmp("_profile", c, 8)))
	    {
		l = c - name;
		suffix = c;
		for(s = 0; suffix[s] != '.'; s++)
		    ;
		*return_suffix = allocate(s + 1);
		strncpy(*return_suffix, suffix, s);
		(*return_suffix)[s] = '\0';
	    }
	    else
		l = a - name;
	    /* there are incorrect library names of the form:
	       libATS.A_profile.dylib so check for these */
	    if(l >= 3 && name[l-2] == '.')
		l = l - 2;
	    guess = allocate(l + 1);
	    strncpy(guess, name, l);
	    guess[l] = '\0';
	    return(guess);
	}
	else{
	    /* ignore any suffix after an underbar
	       like Foo_profile.A.dylib */
	    c = strrchr(b+1, '_');
	    if (c != NULL && c != b+1 &&
		(0 == strncmp("_debug", c, 6) ||
		 0 == strncmp("_profile", c, 8)))
	    {
		l = c - (b+1);
		suffix = c;
		for(s = 0; suffix[s] != '.'; s++)
		    ;
		*return_suffix = allocate(s + 1);
		strncpy(*return_suffix, suffix, s);
		(*return_suffix)[s] = '\0';
	    }
	    else
		l = a - (b+1);
	    /* there are incorrect library names of the form:
	       libATS.A_profile.dylib so check for these */
	    if(l >= 3 && b[1+l-2] == '.')
		l = l - 2;
	    guess = allocate(l + 1);
	    strncpy(guess, b+1, l);
	    guess[l] = '\0';
	    return(guess);
	}

guess_qtx:
	a = strrchr(name, '.');
	if(strcmp(a, ".qtx") != 0)
	    return(NULL);

	b = look_back_for_slash(name, a);
	if(b == name)
	    return(NULL);
	if(b == NULL){
	    l = a - name;
	    /* there are library names of the form:
	       QT.A.qtx so check for these */
	    if(l >= 3 && name[l-2] == '.')
		l = l - 2;
	    guess = allocate(l + 1);
	    strncpy(guess, name, l);
	    guess[l + 1] = '\0';
	    return(guess);
	}
	else{
	    l = a - (b+1);
	    /* there are library names of the form:
	       QT.A.qtx so check for these */
	    if(l >= 3 && b[1+l-2] == '.')
		l = l - 2;
	    guess = allocate(l + 1);
	    strncpy(guess, b+1, l);
	    guess[l + 1] = '\0';
	    return(guess);
	}
}

/*
 * look_back_for_slash() is passed a string name and an end point in name to
 * start looking for '/' before the end point.  It returns a pointer to the
 * '/' back from the end point or NULL if there is none.
 */
static
char *
look_back_for_slash(
char *name,
char *p)
{
	for(p = p - 1; p >= name; p--){
	    if(*p == '/')
		return(p);
	}
	return(NULL);
}
#endif /* !defined(RLD) */
