/* Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support (steve@cygnus.com).

   This file is part of GNU binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char writecode;
static char *it;
static int code;
static char * repeat;
static char *oldrepeat;
static char *name;
static int rdepth;
static char *names[] = {" ","[n]","[n][m]"};
static char *pnames[]= {"","*","**"};

static void yyerror (const char *s);
extern int yylex (void);
%}


%union {
 int i;
 char *s;
}
%token COND
%token REPEAT
%token '(' ')'
%token <s> TYPE
%token <s> NAME
%token <i> NUMBER UNIT
%type <i> attr_size
%type <s> attr_desc attr_id attr_type
%%

top:  {
  switch (writecode)
    {
    case 'i':
      printf("#ifdef SYSROFF_SWAP_IN\n");
      break;
    case 'p':
      printf("#ifdef SYSROFF_p\n");
      break;
    case 'd':
      break;
    case 'g':
      printf("#ifdef SYSROFF_SWAP_OUT\n");
      break;
    case 'c':
      printf("#ifdef SYSROFF_PRINT\n");
      printf("#include <stdio.h>\n");
      printf("#include <stdlib.h>\n");
      printf("#include <ansidecl.h>\n");
      break;
    }
 }
it_list {
  switch (writecode) {
  case 'i':
  case 'p':
  case 'g':
  case 'c':
    printf("#endif\n");
    break;
  case 'd':
    break;
  }
}

  ;


it_list: it it_list
  |
  ;

it:
	'(' NAME NUMBER
      {
	it = $2; code = $3;
	switch (writecode)
	  {
	  case 'd':
	    printf("\n\n\n#define IT_%s_CODE 0x%x\n", it,code);
	    printf("struct IT_%s;\n", it);
	    printf("extern void sysroff_swap_%s_in (struct IT_%s *);\n",
		   $2, it);
	    printf("extern void sysroff_swap_%s_out (FILE *, struct IT_%s *);\n",
		   $2, it);
	    printf("extern void sysroff_print_%s_out (struct IT_%s *);\n",
		   $2, it);
	    printf("struct IT_%s { \n", it);
	    break;
	  case 'i':
	    printf("void sysroff_swap_%s_in (struct IT_%s * ptr)\n",$2,it);
	    printf("{\n");
	    printf("\tunsigned char raw[255];\n");
	    printf("\tint idx = 0;\n");
	    printf("\tint size;\n");
	    printf("\tmemset(raw,0,255);\n");
	    printf("\tmemset(ptr,0,sizeof(*ptr));\n");
	    printf("\tsize = fillup(raw);\n");
	    break;
	  case 'g':
	    printf("void sysroff_swap_%s_out (FILE * ffile, struct IT_%s * ptr)\n",$2,it);
	    printf("{\n");
	    printf("\tunsigned char raw[255];\n");
	    printf("\tint idx = 16;\n");
	    printf("\tmemset (raw, 0, 255);\n");
	    printf("\tcode = IT_%s_CODE;\n", it);
	    break;
	  case 'o':
	    printf("void sysroff_swap_%s_out (bfd * abfd, struct IT_%s * ptr)\n",$2, it);
	    printf("{\n");
	    printf("\tint idx = 0;\n");
	    break;
	  case 'c':
	    printf("void sysroff_print_%s_out (struct IT_%s *ptr)\n",$2,it);
	    printf("{\n");
	    printf("itheader(\"%s\", IT_%s_CODE);\n",$2,$2);
	    break;

	  case 't':
	    break;
	  }

      }
	it_field_list
')'
{
  switch (writecode) {
  case 'd':
    printf("};\n");
    break;
  case 'g':
    printf("\tchecksum(ffile,raw, idx, IT_%s_CODE);\n", it);
    /* Fall through.  */
  case 'i':
  case 'o':
  case 'c':
    printf("}\n");
  }

  free (it);
}
;



it_field_list:
		it_field it_field_list
	|	cond_it_field it_field_list
	|	repeat_it_field it_field_list
	|
	;

repeat_it_field: '(' REPEAT NAME
	{
	  rdepth++;
	  switch (writecode)
	    {
	    case 'c':
	      if (rdepth==1)
	      printf("\tprintf(\"repeat %%d\\n\", %s);\n",$3);
	      if (rdepth==2)
	      printf("\tprintf(\"repeat %%d\\n\", %s[n]);\n",$3);
	      /* Fall through.  */
	    case 'i':
	    case 'g':
	    case 'o':

	      if (rdepth==1)
		{
	      printf("\t{ int n; for (n = 0; n < %s; n++) {\n",    $3);
	    }
	      if (rdepth == 2) {
	      printf("\t{ int m; for (m = 0; m < %s[n]; m++) {\n",    $3);
	    }

	      break;
	    }

	  oldrepeat = repeat;
         repeat = $3;
	}

	 it_field_list ')'

	{
	  free (repeat);

	  repeat = oldrepeat;
	  oldrepeat =0;
	  rdepth--;
	  switch (writecode)
	    {
	    case 'i':
	    case 'g':
	    case 'o':
	    case 'c':
	  printf("\t}}\n");
	}
	}
       ;


cond_it_field: '(' COND NAME
	{
	  switch (writecode)
	    {
	    case 'i':
	    case 'g':
	    case 'o':
	    case 'c':
	      printf("\tif (%s) {\n", $3);
	      break;
	    }

	  free ($3);
	}

	 it_field_list ')'
	{
	  switch (writecode)
	    {
	    case 'i':
	    case 'g':
	    case 'o':
	    case 'c':
	  printf("\t}\n");
	}
	}
       ;

it_field:
	'(' attr_desc '(' attr_type attr_size ')' attr_id
	{name = $7; }
	enums ')'
	{
	  char *desc = $2;
	  char *type = $4;
	  int size = $5;
	  char *id = $7;
char *p = names[rdepth];
char *ptr = pnames[rdepth];
	  switch (writecode)
	    {
	    case 'g':
	      if (size % 8)
		{

		  printf("\twriteBITS(ptr->%s%s,raw,&idx,%d);\n",
			 id,
			 names[rdepth], size);

		}
	      else {
		printf("\twrite%s(ptr->%s%s,raw,&idx,%d,ffile);\n",
		       type,
		       id,
		       names[rdepth],size/8);
		}
	      break;
	    case 'i':
	      {

		if (rdepth >= 1)

		  {
		    printf("if (!ptr->%s) ptr->%s = (%s*)xcalloc(%s, sizeof(ptr->%s[0]));\n",
			   id,
			   id,
			   type,
			   repeat,
			   id);
		  }

		if (rdepth == 2)
		  {
		    printf("if (!ptr->%s[n]) ptr->%s[n] = (%s**)xcalloc(%s[n], sizeof(ptr->%s[n][0]));\n",
			   id,
			   id,
			   type,
			   repeat,
			   id);
		  }

	      }

	      if (size % 8)
		{
		  printf("\tptr->%s%s = getBITS(raw,&idx, %d,size);\n",
			 id,
			 names[rdepth],
			 size);
		}
	      else {
		printf("\tptr->%s%s = get%s(raw,&idx, %d,size);\n",
		       id,
		       names[rdepth],
		       type,
		       size/8);
		}
	      break;
	    case 'o':
	      printf("\tput%s(raw,%d,%d,&idx,ptr->%s%s);\n", type,size/8,size%8,id,names[rdepth]);
	      break;
	    case 'd':
	      if (repeat)
		printf("\t/* repeat %s */\n", repeat);

		  if (type[0] == 'I') {
		  printf("\tint %s%s; \t/* %s */\n",ptr,id, desc);
		}
		  else if (type[0] =='C') {
		  printf("\tchar %s*%s;\t /* %s */\n",ptr,id, desc);
		}
	      else {
		printf("\tbarray %s%s;\t /* %s */\n",ptr,id, desc);
	      }
		  break;
		case 'c':
	      printf("tabout();\n");
		  printf("\tprintf(\"/*%-30s*/ ptr->%s = \");\n", desc, id);

		  if (type[0] == 'I')
		  printf("\tprintf(\"%%d\\n\",ptr->%s%s);\n", id,p);
		  else   if (type[0] == 'C')
		  printf("\tprintf(\"%%s\\n\",ptr->%s%s);\n", id,p);

		  else   if (type[0] == 'B')
		    {
		  printf("\tpbarray(&ptr->%s%s);\n", id,p);
		}
	      else abort();
		  break;
		}

	  free (desc);
	  free (id);
	}

	;


attr_type:
	 TYPE { $$ = $1; }
 	|  { $$ = "INT";}
	;

attr_desc:
	'(' NAME ')'
	{ $$ = $2; }
	;

attr_size:
	 NUMBER UNIT
	{ $$ = $1 * $2; }
	;


attr_id:
		'(' NAME ')'	{ $$ = $2; }
	|	{ $$ = strdup ("dummy");}
	;

enums:
	| '(' enum_list ')' ;

enum_list:
	|
	enum_list '(' NAME NAME ')' {
	  switch (writecode)
	    {
	    case 'd':
	      printf("#define %s %s\n", $3,$4);
	      break;
	    case 'c':
		printf("if (ptr->%s%s == %s) { tabout(); printf(\"%s\\n\");}\n", name, names[rdepth],$4,$3);
	    }

	  free ($3);
	  free ($4);
	}

	;



%%
/* four modes

   -d write structure definitions for sysroff in host format
   -i write functions to swap into sysroff format in
   -o write functions to swap into sysroff format out
   -c write code to print info in human form */

int yydebug;

int
main (int ac, char **av)
{
  yydebug=0;
  if (ac > 1)
    writecode = av[1][1];
if (writecode == 'd')
  {
    printf("typedef struct { unsigned char *data; int len; } barray; \n");
    printf("typedef  int INT;\n");
    printf("typedef  char * CHARS;\n");

  }
  yyparse();
return 0;
}

static void
yyerror (const char *s)
{
  fprintf(stderr, "%s\n" , s);
}
