/* make_strings

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by:  Alexander Malmberg <alexander@malmberg.org>
   Created: 2002

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

#import "common.h"
#import "Foundation/NSString.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSArray.h"

#include "make_strings.h"

#include "StringsFile.h"

#include "SourceEntry.h"
#include "StringsEntry.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif


int verbose, aggressive_import, aggressive_match, aggressive_remove;


typedef struct
{
  const char *func_name;
  int num_args;
  int key_index, comment_index, table_index;
} loc_func_t;

/*
  List of functions we should look for (easy to extend).
*/
static loc_func_t loc_funcs[]= 
  {
    {"_"                                  , 1,  0, -1, -1}, 
    {"NSLocalizedString"                  , 2,  0, 1, -1}, 
    {"NSLocalizedStringFromTable"         , 3,  0, 2, 1}, 
    {"NSLocalizedStringFromTableInBundle" , 4,  0, 3, 1}, 

    {"__"                                 , 1,  0, -1, -1}, 
    {"NSLocalizedStaticString"            , 2,  0, 1, -1}, 
    {}, 
  };
#define MAX_ARGS 4

static char *nilp = 0;

static int
isname1(unsigned char ch)
{
  if (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
    return 1;
  return 0;
}

static int
isname(unsigned char ch)
{
  if (isname1(ch)) return 1;
  if (ch >= '0' && ch <= '9') return 1;
  return 0;
}


/*
  This function attempts to parse the specified file and adds any calls to
  tables. To avoid excessive complexity and to easily handle comments and
  strings everywhere, it's written as a state machine.

  Things that can fool it:
  - Stupid use of pre-processor stuff, like '#define foo bar('.
  Solution: Don't do that.

  - Nested localization calls, like:
  'NSLocalizedString(@"foo", NSLocalizedString(@"bar", @"zot"))'.
  Solution: don't do that (with the current functions, there should never
  be any reason to).

*/

#define add_arg_ch(ch)\
    {\
      if (arg_len[num_args]+1 >= arg_size[num_args])\
	{\
	  arg_size[num_args] += 512;\
	  args[num_args] = realloc(args[num_args], arg_size[num_args]);\
	  if (!args[num_args])\
	    {\
	      NSLog(@"out of memory!\n");\
	      exit(1);\
	    }\
	}\
      args[num_args][arg_len[num_args]++] = ch;\
      args[num_args][arg_len[num_args]] = 0;\
    }

static int
ParseFile(const char *filename, NSMutableDictionary *tables)
{
  FILE *f;
  NSString *filenamestr;

  /*
    0: normal parsing
    1: in '//' comment
    2: in '/ *' comment
    3: in string
    5: parsing potentially interesting name
    6: parsing uninteresting name
    7: got name, wait for '('
    8: parsing arguments
  */
  int state, old_state, skip;

  int ch, nch;

  int name = -1;
  int nindex = 0;

  int cur_line;

  unsigned char *args[MAX_ARGS];
  int arg_size[MAX_ARGS], arg_len[MAX_ARGS];
  int arg_ok[MAX_ARGS];
  int num_args;
  int i;

  int depth = 0;



  filenamestr = [NSString stringWithCString: filename
    encoding: [NSString defaultCStringEncoding]];
  if (verbose)
    printf("Parsing '%s'.\n", filename);
  f = fopen(filename, "rt");
  if (!f)
    {
      NSLog(@"Unable to open '%@': %m\n", filenamestr);
      return 1;
    }

  old_state = state = 0;
  skip = 0;
  cur_line = 1;
  num_args = -1;

  for (i = 0; i < MAX_ARGS; i++)
    {
      args[i] = NULL;
      arg_size[i] = 0;
    }

  nch = fgetc(f);
  while (!feof(f))
    {
      ch = nch;
      if (ch == EOF) break;
      nch = fgetc(f);
      // printf("ch = %02x '%c'  state =%i skip =%i\n", ch, ch, state, skip);
      if (skip)
	{
	  skip--;
	  continue;
	}

      if (ch == '\n') cur_line++;

      if (state == 3)
	{
	  if (ch!= '"' && num_args!=-1 && arg_ok[num_args])
	    add_arg_ch(ch);

	  if (ch == '\\')
	    {
	      if (num_args!= -1 && arg_ok[num_args])
		add_arg_ch(nch);
	      skip = 1;
	    }
	  else if (ch == '"')
	    state = old_state;
	  continue;
	}

      if (state == 1)
	{
	  if (ch == '\n' || ch == '\r')
	    state = old_state;
	  continue;
	}

      if (state == 2)
	{
	  if (ch == '*' && nch == '/')
	    {
	      state = old_state;
	      skip = 1;
	    }
	  continue;
	}

      if (ch == '"')
	{
	  old_state = state;
	  state = 3;
	  continue;
	}

      if (ch == '/' && nch == '/')
	{
	  old_state = state;
	  state = 1;
	  continue;
	}
      if (ch == '/' && nch == '*')
	{
	  old_state = state;
	  state = 2;
	  /* skip a character so we'll parse '/ * /' correctly */
	  skip = 1;
	  continue;
	}

      if (state == 0 && isname1(ch))
	{
	  for (name = 0;loc_funcs[name].func_name;name++)
	    if (ch == loc_funcs[name].func_name[0])
	      break;
	  if (loc_funcs[name].func_name)
	    {
	      state = 5;
	      nindex = 1;
	    }
	  else
	    state = 6;
	  continue;
	}

      if (state == 6)
	{
	  if (!isname(ch))
	    state = 0;
	  continue;
	}

      if (state == 5)
	{
	  if (isname(ch))
	    {
	      int old_name;
	      if (loc_funcs[name].func_name[nindex] == ch)
		{
		  nindex++;
		  continue;
		}

	      old_name = name;
	      for (name++;loc_funcs[name].func_name;name++)
		{
		  if (!strncmp(loc_funcs[old_name].func_name,
                    loc_funcs[name].func_name, nindex)
                    && loc_funcs[name].func_name[nindex] == ch)
		    break;
		}
	      if (loc_funcs[name].func_name)
		nindex++;
	      else
		state = 6;
	      continue;
	    }
	  if (loc_funcs[name].func_name[nindex]!= 0)
	    {
	      int old_name = name;
	      for (name++;loc_funcs[name].func_name;name++)
		{
		  if (!strncmp(loc_funcs[old_name].func_name,
                    loc_funcs[name].func_name, nindex)
                    && loc_funcs[name].func_name[nindex] == 0)
		    break;
		}
	    }
	  if (!loc_funcs[name].func_name)
	    {
	      state = 0;
	      continue;
	    }
	  else
	    {
	      /* printf("found call to '%s' at line %i\n",
               * loc_funcs[name].func_name, cur_line);
               */
	      state = 7;
	    }
	}

      if (state == 7)
	{
	  if (ch == '(')
	    {
	      num_args = 0;
	      depth = 0;
	      state = 8;
	      nilp = "nil";
	      for (i = 0; i < MAX_ARGS; i++)
		{
		  arg_len[i] = 0;
		  arg_ok[i] = 1;
		}
	      /* printf(" start arg list, want %i args\n",
               * loc_funcs[name].num_args);
               */
	    }
	  else if (ch > 32)
	    state = 0;
	  continue;
	}

      if (state == 8)
	{
	  if (depth)
	    {
	      if (ch == ')' || ch == ']' || ch == '}')
		depth--;
	      continue;
	    }
	  if (ch == '(' || ch == '[' || ch == '{')
	    {
	      arg_ok[num_args] = 0;
	      depth++;
	      continue;
	    }
	  if (ch == ')')
	    {
	      loc_func_t *lf = &loc_funcs[name];

/*{
printf("got call to '%s', %i args\n", loc_funcs[name].func_name, num_args);
for (i = 0;i<num_args;i++)
printf("  %3i : %i '%s'\n", i, arg_ok[i], args[i]);
}*/

	      if (arg_ok[lf->key_index]
		&& (lf->comment_index == -1 || arg_ok[lf->comment_index])
		&& (lf->table_index == -1 || arg_ok[lf->table_index]))
		{
		  SourceEntry	*e;
		  NSString	*key, *comment, *table;

		  /* TODO: let user specify source file encoding */
		  key = [NSString stringWithCString:
		    (char*)args[lf->key_index]];

		  if (lf->comment_index == -1 || !arg_len[lf->comment_index])
		    comment = nil;
		  else
		    comment = [NSString stringWithCString:
		      (char*)args[lf->comment_index]];

		  if (lf->table_index == -1
		    || (arg_ok[lf->table_index]
		      && (args[lf->table_index] == 0
			|| strcmp("nil", (char*)args[lf->table_index]) == 0)))
		    {
		      table = @"Localizable"; /* TODO: customizable? */
		    }
		  else
		    {
		      table = [NSString stringWithCString:
		        (char*)args[lf->table_index]];
		    }
		  e = [[SourceEntry alloc] initWithKey: key
					       comment: comment
						  file: filenamestr
						  line: cur_line];
		  [tables addEntry: e toTable: table];
		  [e release];
		}
	      else
		{
		  NSLog(@"unable to parse call to '%s' at %s:%i\n", 
		    lf->func_name, filename, cur_line);
		}

	      num_args = -1;
	      state = 0;
	      continue;
	    }
	  if (ch == ',')
	    {
	      num_args++;
	      if (num_args == MAX_ARGS)
		state = 0;
	      nilp = "nil";
	      continue;
	    }
          if (nilp)
	    {
 	      if (ch == *nilp)
 		{
 		  nilp++;
 		  if (*nilp == '\0')
 		    {
 		      args[num_args] = (unsigned char *)strdup("nil");
 		      arg_len[num_args] = 3;
 		      arg_size[num_args] = 4;
 		      nilp = 0;
 		    }
 		  continue;
 		}
 	    }
	  if (ch > 32 && ch != '@')
	    arg_ok[num_args] = 0;
	  continue;
	}
    }

  for (i = 0; i<MAX_ARGS; i++)
    if (args[i])
      free(args[i]);

  fclose(f);
  return 0;
}


@implementation NSMutableDictionary (make_strings)
- (void) addEntry: (SourceEntry *)e  toTable: (NSString *)table
{
  NSMutableArray *a;
  a = [self objectForKey: table];
  if (!a)
    {
      a = [[NSMutableArray alloc] init];
      [self setObject: a forKey: table];
      [a release];
    }
  [a addObject: e];
}
@end


static void
UpdateTable(NSArray *source_table, NSString *filename)
{
  int i, c;
  StringsFile *sf;

  if (verbose)
    printf("Updating '%s'.\n", [filename cString]);
  sf = [[StringsFile alloc] initWithFile: filename];

  c = [source_table count];
  for (i = 0; i < c; i++)
    {
      [sf addSourceEntry: [source_table objectAtIndex: i]];
    }

  [sf writeToFile: filename];
  DESTROY(sf);
}


static void
HandleLanguage(NSString *language_name, NSMutableDictionary *source_entries)
{
  NSEnumerator *e;
  NSString *table_name;
  NSString *filename;
  CREATE_AUTORELEASE_POOL(arp);

  if (verbose)
    printf("Updating language '%s'.\n", [language_name cString]);
  for (e = [source_entries keyEnumerator];(table_name =[e nextObject]);)
    {
      filename = [[[NSString stringWithFormat: @"%@.lproj", language_name]
		  stringByAppendingPathComponent:
		    [NSString stringWithFormat: @"%@.strings", table_name]]
		 stringByStandardizingPath];
      UpdateTable([source_entries objectForKey: table_name], filename);
    }

  DESTROY(arp);
}


int main(int argc, char **argv)
{
  CREATE_AUTORELEASE_POOL(arp);

  NSMutableDictionary *source_entries;
  NSMutableArray *languages = [NSMutableArray arrayWithCapacity: 10];

  int error;

  {
    int i, j;
    char *c;
    for (j = i = 1; i < argc; i++)
      {
	c = argv[i];
	if (!strcmp (c, "--help"))
	  {
	    printf ("Syntax: %s [--help] [--verbose] [--aggressive-import] [--aggressive-match] [--aggressive-remove] [-L languages] files.[hmc...]\n", 
		    argv[0]);
	    printf("\n");
	    printf("Example: %s -L \"English Swedish German\" *.[hm]\n", 
		   argv[0]);
	    return 0;
	  }
	else if (!strcmp (c, "--verbose"))
	  {
	    verbose = 1;
	  }
	else if (!strcmp (c, "--aggressive-import"))
	  {
	    aggressive_import = 1;
	    aggressive_match = 1;
	  }
	else if (!strcmp (c, "--aggressive-match"))
	  {
	    aggressive_match = 1;
	  }
	else if (!strcmp(c, "--aggressive-remove"))
	  {
	    aggressive_remove = 1;
	    aggressive_match = 1;
	  }
	else if (!strcmp (c, "-L"))
	  {
	    char *d, *d2;
	    if (++i == argc)
	      {
		NSLog (@"syntax error\n");
		return 1;
	      }
            d =  argv[i];
            while (1)
	      {
		d2 = strchr (d, ' ');
		if (d2)
		  *d2 = 0;
		[languages addObject: [NSString stringWithCString: d]];
		d = d2 + 1;
		if (!d2)
		  break;
	      }
	  }
	else
	  {
	    argv[j++] = c;
	  }
      }
    argc = j;
  }

  /* Remove any empty language from the list.  */
  {
    int k;

    for (k = [languages count] - 1; k >= 0; k--)
      {
	NSString *language = [languages objectAtIndex: k];
	if ([language isEqualToString: @""])
	  {
	    [languages removeObjectAtIndex: k];
	  }
      }
  }


  if (![languages count])
    {
      NSLog (@"No languages specified!\n");
      return 1;
    }
  if (argc == 1)
    {
      NSLog (@"No files specified!\n");
      return 1;
    }

  source_entries = [[NSMutableDictionary alloc] init];
  error = 0;
  {
    int i;

    for (i = 1; i < argc; i++)
      error += ParseFile (argv[i], source_entries);
  }

  if (!error)
    {
      int i, c = [languages count];

      for (i = 0; i < c; i++)
	{
	  HandleLanguage ([languages objectAtIndex: i], source_entries);
	}
    }

  DESTROY(arp);

  if (error)
    return 1;
  else
    return 0;
}

