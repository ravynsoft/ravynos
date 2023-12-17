/* accept - listen for and accept a remote network connection on a given port */

/*
   Copyright (C) 2020 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bashtypes.h"
#include <errno.h>
#include <time.h>
#include <limits.h>
#include "typemax.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "loadables.h"

static int accept_bind_variable (char *, int);

int
accept_builtin (list)
     WORD_LIST *list;
{
  SHELL_VAR *v;
  intmax_t iport;
  int opt;
  char *tmoutarg, *fdvar, *rhostvar, *rhost, *bindaddr;
  unsigned short uport;
  int servsock, clisock;
  struct sockaddr_in server, client;
  socklen_t clientlen;
  struct timeval timeval;
  struct linger linger = { 0, 0 };

  rhostvar = tmoutarg = fdvar = rhost = bindaddr = (char *)NULL;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "b:r:t:v:")) != -1)
    {
      switch (opt)
	{
	case 'b':
	  bindaddr = list_optarg;
	  break;
	case 'r':
	  rhostvar = list_optarg;
	  break;
	case 't':
	  tmoutarg = list_optarg;
	  break;
	case 'v':
	  fdvar = list_optarg;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  
  list = loptend;

  /* Validate input and variables */
  if (tmoutarg)
    {
      long ival, uval;
      opt = uconvert (tmoutarg, &ival, &uval, (char **)0);
      if (opt == 0 || ival < 0 || uval < 0)
	{
	  builtin_error ("%s: invalid timeout specification", tmoutarg);
	  return (EXECUTION_FAILURE);
	}
      timeval.tv_sec = ival;
      timeval.tv_usec = uval;
      /* XXX - should we warn if ival == uval == 0 ? */
    }

  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (legal_number (list->word->word, &iport) == 0 || iport < 0 || iport > TYPE_MAXIMUM (unsigned short))
    {
      builtin_error ("%s: invalid port number", list->word->word);
      return (EXECUTION_FAILURE);
    }
  uport = (unsigned short)iport;

  if (fdvar == 0)
    fdvar = "ACCEPT_FD";

  unbind_variable (fdvar);
  if (rhostvar)
    unbind_variable (rhostvar);
    
  if ((servsock = socket (AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
    {
      builtin_error ("cannot create socket: %s", strerror (errno));
      return (EXECUTION_FAILURE);
    }

  memset ((char *)&server, 0, sizeof (server));
  server.sin_family = AF_INET;
  server.sin_port = htons(uport);
  server.sin_addr.s_addr = bindaddr ? inet_addr (bindaddr) : htonl(INADDR_ANY);

  if (server.sin_addr.s_addr == INADDR_NONE)
    {
      builtin_error ("invalid address: %s", strerror (errno));
      return (EXECUTION_FAILURE);
    }

  opt = 1;
  setsockopt (servsock, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof (opt));
  setsockopt (servsock, SOL_SOCKET, SO_LINGER, (void *)&linger, sizeof (linger));

  if (bind (servsock, (struct sockaddr *)&server, sizeof (server)) < 0)
    {
      builtin_error ("socket bind failure: %s", strerror (errno));
      close (servsock);
      return (EXECUTION_FAILURE);
    }

  if (listen (servsock, 1) < 0)
    {
      builtin_error ("listen failure: %s", strerror (errno));
      close (servsock);
      return (EXECUTION_FAILURE);
    }

  if (tmoutarg)
    {
      fd_set iofds;

      FD_ZERO(&iofds);
      FD_SET(servsock, &iofds);

      opt = select (servsock+1, &iofds, 0, 0, &timeval);
      if (opt < 0)
	builtin_error ("select failure: %s", strerror (errno));
      if (opt <= 0)
	{
	  close (servsock);
	  return (EXECUTION_FAILURE);
	}
    }

  clientlen = sizeof (client);
  if ((clisock = accept (servsock, (struct sockaddr *)&client, &clientlen)) < 0)
    {
      builtin_error ("client accept failure: %s", strerror (errno));
      close (servsock);
      return (EXECUTION_FAILURE);
    }

  close (servsock);

  accept_bind_variable (fdvar, clisock);  
  if (rhostvar)
    {
      rhost = inet_ntoa (client.sin_addr);
      v = builtin_bind_variable (rhostvar, rhost, 0);
      if (v == 0 || readonly_p (v) || noassign_p (v))
	builtin_error ("%s: cannot set variable", rhostvar);
    }

  return (EXECUTION_SUCCESS);
}

static int
accept_bind_variable (varname, intval)
     char *varname;
     int intval;
{
  SHELL_VAR *v;
  char ibuf[INT_STRLEN_BOUND (int) + 1], *p;

  p = fmtulong (intval, 10, ibuf, sizeof (ibuf), 0);
  v = builtin_bind_variable (varname, p, 0);		/* XXX */
  if (v == 0 || readonly_p (v) || noassign_p (v))
    builtin_error ("%s: cannot set variable", varname);
  return (v != 0);
}

char *accept_doc[] = {
	"Accept a network connection on a specified port.",
	""
	"This builtin allows a bash script to act as a TCP/IP server.",
	"",
	"Options, if supplied, have the following meanings:",
	"    -b address    use ADDRESS as the IP address to listen on; the",
	"                  default is INADDR_ANY",
	"    -t timeout    wait TIMEOUT seconds for a connection. TIMEOUT may",
	"                  be a decimal number including a fractional portion",
	"    -v varname    store the numeric file descriptor of the connected",
	"                  socket into VARNAME. The default VARNAME is ACCEPT_FD",
	"    -r rhost      store the IP address of the remote host into the shell",
	"                  variable RHOST, in dotted-decimal notation",
	"",
	"If successful, the shell variable ACCEPT_FD, or the variable named by the",
	"-v option, will be set to the fd of the connected socket, suitable for",
	"use as 'read -u$ACCEPT_FD'. RHOST, if supplied, will hold the IP address",
	"of the remote client. The return status is 0.",
	"",
	"On failure, the return status is 1 and ACCEPT_FD (or VARNAME) and RHOST,",
	"if supplied, will be unset.",
	"",
	"The server socket fd will be closed before accept returns.",
	(char *) NULL
};

struct builtin accept_struct = {
	"accept",		/* builtin name */
	accept_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	accept_doc,		/* array of long documentation strings. */
	"accept [-b address] [-t timeout] [-v varname] [-r addrvar ] port",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
