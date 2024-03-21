/* opc2c.c --- generate C opcode decoder code from from .opc file

   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   Contributed by Red Hat, Inc.

   This file is part of the GNU opcode library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include "libiberty.h"

static char * line_buf = NULL;
static int line_buf_size = 0;

#define LBUFINCR 100

char *
safe_fgets (FILE * f)
{
  char * line_ptr;

  if (line_buf == NULL)
    {
      line_buf = (char *) malloc (LBUFINCR);
      line_buf_size = LBUFINCR;
    }

  /* Points to last byte.  */
  line_ptr = line_buf + line_buf_size - 1;

  /* So we can see if fgets put a 0 there.  */
  *line_ptr = 1;
  if (fgets (line_buf, line_buf_size, f) == 0)
    return NULL;

  /* We filled the buffer?  */
  while (line_ptr[0] == 0 && line_ptr[-1] != '\n')
    {
      /* Make the buffer bigger and read more of the line.  */
      line_buf_size += LBUFINCR;
      line_buf = (char *) realloc (line_buf, line_buf_size);

      /* Points to last byte again.  */
      line_ptr = line_buf + line_buf_size - 1;
      /* So we can see if fgets put a 0 there.  */
      *line_ptr = 1;

      if (fgets (line_buf + line_buf_size - LBUFINCR - 1, LBUFINCR + 1, f) == 0)
	return NULL;
    }

  return line_buf;
}


static int errors = 0;

#define MAX_BYTES 10

typedef struct
{
  int varyno:16;
  int byte:8;
  int shift:8;
} VaryRef;

typedef struct
{
  char nbytes;
  char dbytes;
  char id[MAX_BYTES * 8 + 1];
  unsigned char var_start[MAX_BYTES * 8 + 1];
  struct
  {
    unsigned char decodable_mask;
    unsigned char decodable_bits;
  } b[MAX_BYTES];
  char * comment;
  char * syntax;
  int lineno;
  int nlines;
  char ** lines;
  struct Indirect * last_ind;
  int semantics_label;
  int nvaries;
  VaryRef * vary;
} opcode;

int n_opcodes;
opcode ** opcodes;
opcode * op;

typedef struct
{
  char * name;
  int nlen;
  unsigned char mask;
  int n_patterns;
  unsigned char * patterns;
} Vary;

Vary ** vary = 0;
int n_varies = 0;

unsigned char cur_bits[MAX_BYTES + 1];

const char * orig_filename;

FILE * sim_log = NULL;
#define lprintf if (sim_log) fprintf

opcode prefix_text, suffix_text;

typedef enum
{
  T_unused,
  T_op,
  T_indirect,
  T_done
} OpType;

typedef struct Indirect
{
  OpType type;
  union
  {
    struct Indirect * ind;
    opcode * op;
  } u;
} Indirect;

Indirect indirect[256];

static int
next_varybits (int bits, opcode * op, int byte)
{
  int mask = op->b[byte].decodable_mask;
  int i;

  for (i = 0; i < 8; i++)
    if (!(mask & (1 << i)))
      {
	if (bits & (1 << i))
	  {
	    bits &= ~(1 << i);
	  }
	else
	  {
	    bits |= (1 << i);
	    return bits;
	  }
      }
  return 0;
}

static int
valid_varybits (int bits, opcode * op, int byte)
{
  if (op->nvaries)
    {
      int vn;

      for (vn = 0; vn < op->nvaries; vn++)
	{
	  Vary * v;
	  int found = 0;
	  int i;
	  int ob;

	  if (byte != op->vary[vn].byte)
	    continue;
	  v = vary[op->vary[vn].varyno];
	  ob = (bits >> op->vary[vn].shift) & v->mask;
	  lprintf (sim_log, "varybits: vary %s ob %x\n", v->name, ob);

	  for (i = 0; i < v->n_patterns; i++)
	    if (ob == v->patterns[i])
	      {
		lprintf (sim_log, "  found at %d\n", i);
		found = 1;
		break;
	      }
	  if (!found)
	    return 0;
	}
    }
  return 1;
}

char *
prmb (int mask, int bits)
{
  static char buf[8][30];
  static int bn = 0;
  char * bp;

  bn = (bn + 1) % 8;
  bp = buf[bn];
  int i;
  for (i = 0; i < 8; i++)
    {
      int bit = 0x80 >> i;

      if (!(mask & bit))
	*bp++ = '-';
      else if (bits & bit)
	*bp++ = '1';
      else
	*bp++ = '0';
      if (i % 4 == 3)
	*bp++ = ' ';
    }
  *--bp = 0;
  return buf[bn];
}

static int
op_cmp (const void *va, const void *vb)
{
  const opcode * a = *(const opcode **) va;
  const opcode * b = *(const opcode **) vb;

  if (a->nbytes != b->nbytes)
    return a->nbytes - b->nbytes;

  return strcmp (a->id, b->id);
}

void
dump_lines (opcode * op, int level, Indirect * ind)
{
  char * varnames[40];
  int i, vn = 0;

  if (op->semantics_label)
    {
      printf ("%*sgoto op_semantics_%d;\n", level, "", op->semantics_label);
      return;
    }

  if (ind != op->last_ind)
    {
      static int labelno = 0;
      labelno++;
      printf ("%*sop_semantics_%d:\n", level, "", labelno);
      op->semantics_label = labelno;
    }

  if (op->comment)
    {
      level += 2;
      printf ("%*s{\n", level, "");
      printf ("%*s  %s\n", level, "", op->comment);
    }

  for (i = 0; i < op->nbytes * 8;)
    {
      if (isalpha (op->id[i]))
	{
	  int byte = i >> 3;
	  int mask = 0;
	  int shift = 0;
	  char name[33];
	  char * np = name;

	  while (op->id[i] && isalpha (op->id[i]))
	    {
	      mask = (mask << 1) | 1;
	      shift = 7 - (i & 7);
	      *np++ = op->id[i++];
	      if (op->var_start[i])
		break;
	    }
	  *np = 0;
	  varnames[vn++] = strdup (name);
	  printf ("#line %d \"%s\"\n", op->lineno + 1, orig_filename);
	  if (mask & ~0xff)
	    {
	      fprintf (stderr, "Error: variable %s spans bytes: %s\n",
		       name, op->comment);
	      errors++;
	    }
	  else if (shift && (mask != 0xff))
	    printf ("%*s  int %s AU = (op[%d] >> %d) & 0x%02x;\n",
		    level, "", name, byte, shift, mask);
	  else if (mask != 0xff)
	    printf ("%*s  int %s AU = op[%d] & 0x%02x;\n",
		    level, "", name, byte, mask);
	  else
	    printf ("%*s  int %s AU = op[%d];\n", level, "", name, byte);
	}
      else
	i++;
    }

  if (op->comment)
    {
      printf ("%*s  if (trace)\n", level, "");
      printf ("%*s    {\n", level, "");
      printf ("%*s      printf (\"\\033[33m%%s\\033[0m ", level, "");
      for (i = 0; i < op->nbytes; i++)
	printf (" %%02x");
      printf ("\\n\"");
      printf (",\n%*s             \"%s\"", level, "", op->comment);
      for (i = 0; i < op->nbytes; i++)
	{
	  if (i == 0)
	    printf (",\n%*s             op[%d]", level, "", i);
	  else
	    printf (", op[%d]", i);
	}
      printf (");\n");
      for (i = 0; i < vn; i++)
	printf ("%*s      printf (\"  %s = 0x%%x%s\", %s);\n", level, "",
		varnames[i], (i < vn - 1) ? "," : "\\n", varnames[i]);
      printf ("%*s    }\n", level, "");
    }

  if (op->syntax)
    printf ("%*s  SYNTAX(\"%s\");\n", level, "", op->syntax);

  printf ("#line %d \"%s\"\n", op->lineno + 1, orig_filename);

  for (i = 0; i < op->nlines; i++)
    if (op->lines[i][0] == '\n')
      printf ("%s", op->lines[i]);
    else
      printf ("%*s%s", level, "", op->lines[i]);

  if (op->comment)
    printf ("%*s}\n", level, "");
}

void
store_opcode_bits (opcode * op, int byte, Indirect * ind)
{
  int bits = op->b[byte].decodable_bits;

  do
    {
      if (!valid_varybits (bits, op, byte))
	continue;

      switch (ind[bits].type)
	{
	case T_unused:
	  if (byte == op->dbytes - 1)
	    {
	      ind[bits].type = T_op;
	      ind[bits].u.op = op;
	      op->last_ind = ind;
	      break;
	    }
	  else
	    {
	      int i2;

	      ind[bits].type = T_indirect;
	      ind[bits].u.ind = (Indirect *) malloc (256 * sizeof (Indirect));
	      for (i2 = 0; i2 < 256; i2++)
		ind[bits].u.ind[i2].type = T_unused;
	      store_opcode_bits (op, byte + 1, ind[bits].u.ind);
	    }
	  break;

	case T_indirect:
	  if (byte < op->dbytes - 1)
	    store_opcode_bits (op, byte + 1, ind[bits].u.ind);
	  break;

	case T_op:
	  break;

	case T_done:
	  break;
	}
    }
  while ((bits = next_varybits (bits, op, byte)) != 0);
}

void
emit_indirect (Indirect * ind, int byte)
{
  int unsup = 0;
  int j, n, mask;

  mask = 0;
  for (j = 0; j < 256; j++)
    {
      switch (ind[j].type)
	{
	case T_indirect:
	  mask = 0xff;
	  break;
	case T_op:
	  mask |= ind[j].u.op->b[byte].decodable_mask;
	  break;
	case T_done:
	case T_unused:
	  break;
	}
    }

  printf ("%*s  GETBYTE ();\n", byte * 6, "");
  printf ("%*s  switch (op[%d] & 0x%02x)\n", byte * 6, "", byte, mask);
  printf ("%*s  {\n", byte * 6, "");

  for (j = 0; j < 256; j++)
    if ((j & ~mask) == 0)
      {
	switch (ind[j].type)
	  {
	  case T_done:
	    break;
	  case T_unused:
	    unsup = 1;
	    break;
	  case T_op:
	    for (n = j; n < 256; n++)
	      if ((n & ~mask) == 0
		  && ind[n].type == T_op && ind[n].u.op == ind[j].u.op)
		{
		  ind[n].type = T_done;
		  printf ("%*s    case 0x%02x:\n", byte * 6, "", n);
		}
	    for (n = byte; n < ind[j].u.op->nbytes - 1; n++)
	      printf ("%*s      GETBYTE();\n", byte * 6, "");
	    dump_lines (ind[j].u.op, byte * 6 + 6, ind);
	    printf ("%*s      break;\n", byte * 6, "");
	    break;
	  case T_indirect:
	    printf ("%*s    case 0x%02x:\n", byte * 6, "", j);
	    emit_indirect (ind[j].u.ind, byte + 1);
	    printf ("%*s      break;\n", byte * 6, "");
	    break;
	  }
      }
  if (unsup)
    printf ("%*s    default: UNSUPPORTED(); break;\n", byte * 6, "");
  printf ("%*s  }\n", byte * 6, "");
}

static char *
pv_dup (char * p, char * ep)
{
  int n = ep - p;
  char *rv = (char *) malloc (n + 1);

  memcpy (rv, p, n);
  rv[n] = 0;
  return rv;
}

static unsigned char
str2mask (char * str, char * ep)
{
  unsigned char rv = 0;

  while (str < ep)
    {
      rv *= 2;
      if (*str == '1')
	rv += 1;
      str++;
    }
  return rv;
}

static void
process_vary (char * line)
{
  char * cp;
  char * ep;
  Vary * v = (Vary *) malloc (sizeof (Vary));

  n_varies++;
  if (vary)
    vary = (Vary **) realloc (vary, n_varies * sizeof (Vary *));
  else
    vary = (Vary **) malloc (n_varies * sizeof (Vary *));
  vary[n_varies - 1] = v;

  cp = line;

  for (cp = line; isspace (*cp); cp++);
  for (ep = cp; *ep && !isspace (*ep); ep++);

  v->name = pv_dup (cp, ep);
  v->nlen = strlen (v->name);
  v->mask = (1 << v->nlen) - 1;

  v->n_patterns = 0;
  v->patterns = (unsigned char *) malloc (1);
  while (1)
    {
      for (cp = ep; isspace (*cp); cp++);
      if (!isdigit (*cp))
	break;
      for (ep = cp; *ep && !isspace (*ep); ep++);
      v->n_patterns++;
      v->patterns = (unsigned char *) realloc (v->patterns, v->n_patterns);
      v->patterns[v->n_patterns - 1] = str2mask (cp, ep);
    }
}

static int
fieldcmp (opcode * op, int bit, char *name)
{
  int n = strlen (name);

  if (memcmp (op->id + bit, name, n) == 0
      && (!isalpha (op->id[bit + n]) || op->var_start[bit + n]))
    return 1;
  return 0;
}

static void
log_indirect (Indirect * ind, int byte)
{
  int i, j;
  char * last_c = 0;

  for (i = 0; i < 256; i++)
    {

      for (j = 0; j < byte; j++)
	fprintf (sim_log, "%s ", prmb (255, cur_bits[j]));
      fprintf (sim_log, "%s ", prmb (255, i));

      switch (ind[i].type)
	{
	case T_op:
	case T_done:
	  if (last_c && (ind[i].u.op->comment == last_c))
	    fprintf (sim_log, "''\n");
	  else
	    fprintf (sim_log, "%s\n", ind[i].u.op->comment);
	  last_c = ind[i].u.op->comment;
	  break;
	case T_unused:
	  fprintf (sim_log, "unused\n");
	  break;
	case T_indirect:
	  fprintf (sim_log, "indirect\n");
	  cur_bits[byte] = i;
	  log_indirect (ind[i].u.ind, byte + 1);
	  last_c = 0;
	  break;
	}
    }
}

int
main (int argc, char ** argv)
{
  char * line;
  FILE * in;
  int lineno = 0;
  int i;
  VaryRef * vlist;
  int skipping_section = 0;

  if (argc < 2)
    {
      fprintf (stderr, "usage: opc2c infile.opc > outfile.opc\n");
      exit (1);
    }

  orig_filename = lbasename (argv[1]);
  in = fopen (argv[1], "r");
  if (!in)
    {
      fprintf (stderr, "Unable to open file %s for reading: %s\n", argv[1],
	       xstrerror (errno));
      exit (1);
    }

  n_opcodes = 0;
  opcodes = (opcode **) malloc (sizeof (opcode *));
  op = &prefix_text;
  op->lineno = 0;
  while ((line = safe_fgets (in)) != 0)
    {
      lineno++;
      if (strncmp (line, "/*?* ", 5) == 0)
	{
	  skipping_section = 1;
	  continue;
	}
      if (strncmp (line, "  /** ", 6) == 0
	  && (isdigit (line[6]) || memcmp (line + 6, "VARY", 4) == 0))
	line += 2;
      if (line[0] == '/' && line[1] == '*' && line[2] == '*')
	{
	  skipping_section = 0;
	  if (strncmp (line, "/** */", 6) == 0)
	    {
	      op = &suffix_text;
	      op->lineno = lineno;
	    }
	  else if (strncmp (line, "/** VARY ", 9) == 0)
	    process_vary (line + 9);
	  else
	    {
	      char * lp;
	      int i, bit, byte;
	      int var_start = 1;

	      n_opcodes++;
	      opcodes =
		(opcode **) realloc (opcodes, n_opcodes * sizeof (opcode *));
	      op = (opcode *) malloc (sizeof (opcode));
	      opcodes[n_opcodes - 1] = op;

	      op->nbytes = op->dbytes = 0;
	      memset (op->id, 0, sizeof (op->id));
	      memset (op->var_start, 0, sizeof (op->var_start));
	      for (i = 0; i < MAX_BYTES; i++)
		{
		  op->b[i].decodable_mask = 0;
		  op->b[i].decodable_bits = 0;
		}
	      op->comment = strdup (line);
	      op->comment[strlen (op->comment) - 1] = 0;
	      while (op->comment[0] && isspace (op->comment[0]))
		op->comment++;
	      op->lineno = lineno;
	      op->nlines = 0;
	      op->lines = 0;
	      op->last_ind = 0;
	      op->semantics_label = 0;
	      op->nvaries = 0;
	      op->vary = 0;

	      i = 0;
	      for (lp = line + 4; *lp; lp++)
		{
		  bit = 7 - (i & 7);
		  byte = i >> 3;

		  if (strncmp (lp, "*/", 2) == 0)
		    break;
		  else if ((lp[0] == ' ' && lp[1] == ' ') || (lp[0] == '\t'))
		    {
		      while (*lp == ' ' || *lp == '\t')
			lp ++;
		      op->syntax = strdup (lp);
		      lp = strstr (op->syntax, "*/");
		      if (lp)
			{
			  *lp-- = 0;
			  while ((*lp == ' ' || *lp == '\t')
				 && lp > op->syntax)
			    *lp-- = 0;
			}
		      break;
		    }
		  else if (*lp == ' ')
		    var_start = 1;
		  else
		    {
		      if (*lp == '0' || *lp == '1')
			{
			  op->b[byte].decodable_mask |= 1 << bit;
			  var_start = 1;
			  if (op->dbytes < byte + 1)
			    op->dbytes = byte + 1;
			}
		      else if (var_start)
			{
			  op->var_start[i] = 1;
			  var_start = 0;
			  if (op->dbytes < byte + 1)
			    op->dbytes = byte + 1;
			}
		      if (*lp == '1')
			op->b[byte].decodable_bits |= 1 << bit;

		      op->nbytes = byte + 1;
		      op->id[i++] = *lp;
		    }
		}
	    }
	}
      else if (!skipping_section)
	{
	  op->nlines++;
	  if (op->lines)
	    op->lines =
	      (char **) realloc (op->lines, op->nlines * sizeof (char *));
	  else
	    op->lines = (char **) malloc (op->nlines * sizeof (char *));
	  op->lines[op->nlines - 1] = strdup (line);
	}
    }

  {
    int i, j;
    for (i = 0; i < n_varies; i++)
      {
	Vary *v = vary[i];
	lprintf (sim_log, "V[%s] %d\n", v->name, v->nlen);
	for (j = 0; j < v->n_patterns; j++)
	  lprintf (sim_log, "  P %02x\n", v->patterns[j]);
      }
  }

  for (i = n_opcodes - 2; i >= 0; i--)
    {
      if (opcodes[i]->nlines == 0)
	{
	  opcodes[i]->nlines = opcodes[i + 1]->nlines;
	  opcodes[i]->lines = opcodes[i + 1]->lines;
	}
    }

  for (i = 0; i < 256; i++)
    indirect[i].type = T_unused;

  qsort (opcodes, n_opcodes, sizeof (opcodes[0]), op_cmp);

  vlist = (VaryRef *) malloc (n_varies * sizeof (VaryRef));

  for (i = 0; i < n_opcodes; i++)
    {
      int j, b, v;

      for (j = 0; j < opcodes[i]->nbytes; j++)
	lprintf (sim_log, "%s ",
		 prmb (opcodes[i]->b[j].decodable_mask,
		       opcodes[i]->b[j].decodable_bits));
      lprintf (sim_log, " %s\n", opcodes[i]->comment);

      for (j = 0; j < opcodes[i]->nbytes; j++)
	{
	  for (b = 0; b < 8; b++)
	    if (isalpha (opcodes[i]->id[j * 8 + b]))
	      for (v = 0; v < n_varies; v++)
		if (fieldcmp (opcodes[i], j * 8 + b, vary[v]->name))
		  {
		    int nv = opcodes[i]->nvaries++;
		    if (nv)
		      opcodes[i]->vary =
			(VaryRef *) realloc (opcodes[i]->vary,
					     (nv + 1) * sizeof (VaryRef));
		    else
		      opcodes[i]->vary =
			(VaryRef *) malloc ((nv + 1) * sizeof (VaryRef));

		    opcodes[i]->vary[nv].varyno = v;
		    opcodes[i]->vary[nv].byte = j;
		    opcodes[i]->vary[nv].shift = 8 - b - vary[v]->nlen;
		    lprintf (sim_log, "[vary %s shift %d]\n",
			     vary[v]->name, opcodes[i]->vary[nv].shift);
		  }

	}
    }

  for (i = 0; i < n_opcodes; i++)
    {
      int i2;
      int bytes = opcodes[i]->dbytes;

      lprintf (sim_log, "\nmask:");
      for (i2 = 0; i2 < opcodes[i]->nbytes; i2++)
	lprintf (sim_log, " %02x", opcodes[i]->b[i2].decodable_mask);
      lprintf (sim_log, "%*s%s\n", 13 - 3 * opcodes[i]->nbytes, "",
	       opcodes[i]->comment);

      lprintf (sim_log, "bits:");
      for (i2 = 0; i2 < opcodes[i]->nbytes; i2++)
	lprintf (sim_log, " %02x", opcodes[i]->b[i2].decodable_bits);
      lprintf (sim_log, "%*s(%s) %d byte%s\n", 13 - 3 * opcodes[i]->nbytes,
	       "", opcodes[i]->id, bytes, bytes == 1 ? "" : "s");

      store_opcode_bits (opcodes[i], 0, indirect);
    }

  printf ("/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */\n");

  dump_lines (&prefix_text, 0, 0);

  emit_indirect (indirect, 0);

  dump_lines (&suffix_text, 0, 0);

  if (sim_log)
    log_indirect (indirect, 0);

  return errors;
}
