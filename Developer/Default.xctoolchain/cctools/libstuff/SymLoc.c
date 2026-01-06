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
#import <libc.h>
#import <ctype.h>
#import <sys/types.h>
#ifdef __OPENSTEP__
#define _POSIX_SOURCE
#endif
#import <dirent.h>
#import <pwd.h>
#import "stuff/bool.h"
#import "stuff/errors.h"
#import "stuff/allocate.h"
#import "stuff/SymLoc.h"

const char *
symLocForDylib(const char *installName, const char *releaseName,
enum bool *found_project,
enum bool disablewarnings,
enum bool no_error_if_missing)
{
	return(LocForDylib(installName, releaseName, "Symbols", found_project,
			   disablewarnings, no_error_if_missing));
}

const char *
dstLocForDylib(const char *installName, const char *releaseName,
enum bool *found_project,
enum bool disablewarnings,
enum bool no_error_if_missing)
{
	return(LocForDylib(installName, releaseName, "Roots", found_project,
			   disablewarnings, no_error_if_missing));
}

// caller is responsible for freeing the returned string (using free(3)) 
const char *
LocForDylib(const char *installName, const char *releaseName,
const char *dirname,
enum bool *found_project,
enum bool disablewarnings,
enum bool no_error_if_missing)
{
    struct passwd	*passwd		= NULL;
    struct dirent	*dp		= NULL;
    FILE		*file 		= NULL;
    DIR			*dirp		= NULL;
    DIR			*dirp2		= NULL;
    char		*line		= NULL;
    char		*c		= NULL;
    char		*v		= NULL;
    char		*viewMap	= NULL;
    size_t		 releaseLen	= strlen(releaseName);
    char		 buf[MAXPATHLEN+MAXNAMLEN+64];
    char		 readbuf[MAXPATHLEN+64];
    char		 viewPath[MAXPATHLEN];
    char		 dylibList[MAXPATHLEN];
    char		 installNameList[MAXPATHLEN];

    *found_project = FALSE;

    // check parameters
    if (!installName || !*installName || !releaseName || !*releaseName) {
        fatal("internal error symLocForDylib(): Null or empty parameter");
        return NULL;
    }

    viewMap = getenv("RC_VIEW_MAP_LOCATION");
    if(!viewMap) {
        // find ~rc's home directory
        if (!(passwd = getpwnam("rc"))) {
            system_error("symLocForDylib(): getpwnam(\"rc\") returns NULL");
            return NULL;
        }
        strcpy(buf, passwd->pw_dir);
        // open release-to-view file
        strcat(buf, "/Data/release_to_view.map");
    } else {  
        strcpy(buf, viewMap);
    }
    if (!(file = fopen(buf, "r"))) {
        system_error("symLocForDylib(): Can't fopen %s", buf);
        return NULL;
    }

    // parse release-to-view file
    *viewPath = '\0';
    while ((line = fgets(buf, sizeof(buf), file))) {
        if (!strncmp(line, releaseName, releaseLen) && isspace(line[releaseLen])) {
            c = &line[releaseLen] + 1;
            while (isspace(*c)) c++;
            for (v = &viewPath[0]; !isspace(*c); c++, v++) *v = *c;
            *v = '\0';
            break;
        }
    }
    if(fclose(file) != 0)
	system_error("fclose() failed");
    if (!*viewPath) {
        error("symLocForDylib(): Can't locate view path for release %s",
	      releaseName);
        return NULL;
    }

    // open DylibProjects directory
    strcpy(dylibList, viewPath);
    c = &dylibList[strlen(dylibList)];
    strcpy(c, "/.BuildData/DylibProjects");
    if (!(dirp = opendir(dylibList))) {
        system_error("symLocForDylib(): Can't opendir %s", buf);
        return NULL;
    }

    // open the InstallNames directory
    strcpy(installNameList, viewPath);
    c = &installNameList[strlen(installNameList)];
    strcpy(c, "/.BuildData/InstallNames");
    if (!(dirp2 = opendir(installNameList))) {
        system_error("symLocForDylib(): Can't opendir %s", buf);
        return NULL;
    }
    c = NULL;

    // read DylibProjects entries
    *buf = '\0';
    v = &dylibList[strlen(dylibList)];
    *v = '/';
    v++;
    while ((dp = readdir(dirp))) {

        // skip "." and ".."
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;

        // open file
        strcpy(v, dp->d_name);
        if (!(file = fopen(dylibList, "r"))) {
            system_error("symLocForDylib(): Can't fopen %s", dylibList);
	    if(closedir(dirp) != 0)
		system_error("closedir() failed");
            return NULL;
        }

        // parse file
        while ((line = fgets(readbuf, sizeof(readbuf), file))) {
            if (!*line || *line == '(' || *line == ')') continue;
            while (*line == ' ') line++;
            if (*line != '"') {
                warning("symLocForDylib(): %s contains malformed line",
			dp->d_name);
                continue;
            }
            line++;
            for (c = &buf[0]; *line && *line != '"'; *c++ = *line++);
            if (*line != '"') {
                warning("symLocForDylib(): %s contains malformed line",
		        dp->d_name);
                continue;
            }
            *c = '\0';
            if (!strcmp(buf, installName)) {
                c = allocate(strlen(viewPath) + strlen(releaseName) +
			     strlen(dirname) + strlen(dp->d_name) + 32);
                sprintf(c, "%s/Updates/Built%s/%s/%s", viewPath, releaseName,
			dirname, dp->d_name);
                break;
            } else {
                c = NULL;
            }
        }
        if(fclose(file) != 0)
	    system_error("fclose() failed");
        if (c) break;
    }
    if(closedir(dirp) != 0)
	system_error("closedir() failed");

    if(!c) {    
         // read InstallNames entries
        *buf = '\0';
        v = &installNameList[strlen(installNameList)];
        *v = '/';
        v++;

        while ((dp = readdir(dirp2))) {

            // skip "." and ".."
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;

            // open file
            strcpy(v, dp->d_name);
            if (!(file = fopen(installNameList, "r"))) {
                system_error("symLocForDylib(): Can't fopen %s", installNameList);
                if(closedir(dirp) != 0)
                    system_error("closedir() failed");
                return NULL;
            }

            // parse file
            while ((line = fgets(readbuf, sizeof(readbuf), file))) {
                if (!*line || *line == '(' || *line == ')') continue;
                while (*line == ' ') line++;
                if (*line != '"') {
                    warning("symLocForDylib(): %s contains malformed line",
                        dp->d_name);
                    continue;
                }
                line++;
                for (c = &buf[0]; *line && *line != '"'; *c++ = *line++);
                if (*line != '"') {
                    warning("symLocForDylib(): %s contains malformed line",
                        dp->d_name);
                    continue;
                }
                *c = '\0';
                if (!strcmp(buf, installName)) {
                    c = allocate(strlen(viewPath) + strlen(releaseName) +
                             strlen(dirname) + strlen(dp->d_name) + 32);
                    sprintf(c, "%s/Updates/Built%s/%s/%s", viewPath, releaseName,
                        dirname, dp->d_name);
                    break;
                } else {
                    c = NULL;
                }
            }
            if(fclose(file) != 0)
                system_error("fclose() failed");
            if (c) break;
        }
   } 

    // process return value
    if (!c) {
	if(no_error_if_missing == FALSE)
	    error("Can't find project that builds %s", installName);
        return NULL;
    } else {
	*found_project = TRUE;
        return c;
    }
}
#endif /* !defined(RLD) */
