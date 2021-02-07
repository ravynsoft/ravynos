/* locale_alias - Test program to create a file of locale to language name
                  aliases

  Copyright (C) 2005 Free Software Foundation, Inc.

  Written: Adam Fedor <fedor@gnu.org>
  Date: Oct 2000

  AFAIK: This only works on machines that support setlocale.
  The files created may require hand editing.

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYINGv3.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#import	"common.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <locale.h>

#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSDictionary.h"
#import "GNUstepBase/GSLocale.h"

#define MAXSTRING 100

static int debug=1;

NSMutableDictionary *dict;

int
loc_read_file(const char *dir, const char *file)
{
  FILE *fp;
  char name[1000], *s;
  char buf[1000];
  char locale[MAXSTRING], language[MAXSTRING], country[MAXSTRING];

  if (strcmp(file, "POSIX") == 0)
    return 0;

  snprintf(name, sizeof(name), "%s/%s", dir, file);
  fp = fopen(name, "r");
  if (fp == NULL)
    return -1;

  language[0] = '\0';
  country[0] = '\0';
  while (NULL != fgets(buf, MAXSTRING, fp))
    {
      if (strstr(buf, "anguage") != NULL)
	{
	  sscanf(&buf[2], "%s", language);
	}
      if ((s = strstr(buf, "ocale for")) != NULL)
	{
	  strncpy(country, s + 10, sizeof(country) - 1);
	  country[sizeof(country) - 1] = '\0';
	  s = strchr(country, '\n');
	  if (s)
	    *s = '\0';
	}
      if (strlen(language) > 0)
	break;
    }

  strncpy(locale, file, sizeof(locale) - 1);
  locale[sizeof(locale) - 1] = '\0';
  if (strlen(country) > 0 && strcmp(country, language) != 0)
    {
      strncat(country, language, sizeof(country) - 1 - strlen(country));
      [dict setObject: [NSString stringWithUTF8String: country]
	    forKey: [NSString stringWithUTF8String: locale]];
    }
  locale[2] = '\0';
  [dict setObject: [NSString stringWithUTF8String: language]
	forKey: [NSString stringWithUTF8String: locale]];
  fclose(fp);
  return 0;
}

/* Go through all the files in the directory */
int
loc_get_files(const char *dir)
{
  struct dirent *dp;
  DIR *dirp;

  dirp = opendir(dir);
  while ((dp = readdir(dirp)) != NULL)
    {
      if (isalpha((dp->d_name)[0]))
	{
	  if (debug)
	    printf(" checking %s ...\n", dp->d_name);
	  loc_read_file(dir, dp->d_name);
	}
    }
  closedir(dirp);
  return 0;
}

int
main(int argc, char *argv[])
{
  NSString *lang;
  char *l;
  CREATE_AUTORELEASE_POOL(pool);

  l = setlocale(LC_ALL, "");
  printf("Locale is %s\n", l);

  /* Create Locale.aliases */
  dict = [NSMutableDictionary dictionary];
  loc_get_files("/usr/share/i18n/locales");
  [dict writeToFile: @"Locale.aliases" atomically: NO];

  /* Write out a skeleton file from the current locale */
  dict = GSDomainFromDefaultLocale();
  lang = GSLanguageFromLocale(GSSetLocale(0,NULL));
  if (lang == nil)
    lang = @"Locale";
  if (dict)
    [dict writeToFile: lang atomically: NO];

  DESTROY(pool);
  return 0;
}
