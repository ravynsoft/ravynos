/* Test/example program for the base library

   Copyright (C) 2005 Free Software Foundation, Inc.
   
  Copying and distribution of this file, with or without modification,
  are permitted in any medium without royalty provided the copyright
  notice and this notice are preserved.

   This file is part of the GNUstep Base Library.
*/

/* A simple implementation of getopt() */
static int
indexof(char c, char *string)
{
  int i;

  for (i = 0; i < strlen(string); i++)
    {
      if (string[i] == c)
	{
	  return i;
	}
    }
  return -1;
}

static char *optarg;
static int optind;
static char
wgetopt(int argc, char **argv, char *options)
{
  static char	*arg;
  int		index;
  char		retval = '\0';

  optarg = NULL;
  if (optind == 0)
    {
      optind = 1;
    }
  while (optind < argc)
    {
      arg = argv[optind];
      if (strlen(arg) == 2)
	{
	  if (arg[0] == '-')
	    {
	      if ((index = indexof(arg[1], options)) != -1)
		{
		  retval = arg[1];
		  if (index < strlen(options))
		    {
		      if (options[index+1] == ':')
			{
			  if (optind < argc-1)
			    {
			      optind++;
			      optarg = argv[optind];
			    }
			  else
			    {
			      return -1; /* ':' given, but argv exhausted */
			    }
			}
		    }
		}
	    }
	}
      optind++;
      return retval;
    }
  return -1;
}
