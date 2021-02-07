/* wgetopt

   Copyright (C) 2005 Free Software Foundation, Inc.

   This file is part of the GNU Objective C User Interface library.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.

*/

#if (defined __MINGW__)
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
getopt(int argc, char **argv, char *options)
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
#endif
