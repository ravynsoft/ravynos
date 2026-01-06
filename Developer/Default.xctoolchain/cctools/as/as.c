/* as.c - GAS main program.
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*
 * Main program for AS; a 32-bit assembler of GNU.
 * Understands command arguments.
 * Has a few routines that don't fit in other modules because they
 * are shared.
 *
 *
 *			bugs
 *
 * : initialisers
 *	Since no-one else says they will support them in future: I
 * don't support them now.
 *
 */

#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "as.h"
#include "input-scrub.h"
#include "symbols.h"
#include "sections.h"
#include "read.h"
#include "md.h"
#include "messages.h"
#include "xmalloc.h"
#include "layout.h"
#include "write_object.h"
#include "dwarf2dbg.h"
#include "stuff/arch.h"

/* Used for --gdwarf2 to generate dwarf2 debug info for assembly source files */
enum debug_info_type debug_type = DEBUG_NONE;

/* ['x'] TRUE if "-x" seen. */
char flagseen[128] = { 0 };

/* TRUE if -force_cpusubtype_ALL is specified */
int force_cpusubtype_ALL = 0;

/* set to the corresponding cpusubtype if -arch flag is specified */
cpu_subtype_t archflag_cpusubtype = -1;
char *specific_archflag = NULL;

/* TRUE if the .subsections_via_symbols directive was seen */
int subsections_via_symbols = 0;

/*
 * .include "file" looks in source file dir, then stack.
 * -I directories are added to the end, then the defaults are added.
 */
struct directory_stack include_defaults[] = {
#ifdef __OPENSTEP__
    { 0, "/NextDeveloper/Headers/" },
    { 0, "/LocalDeveloper/Headers/" },
#endif
    /* No default path for Rhapsody or MacOS X */
    { 0, NULL }
};
struct directory_stack *include = NULL;	/* First dir to search */
static struct directory_stack *include_tail = NULL;	/* Last in chain */

/* this is only used here, and in dwarf2dbg.c as the producer */
char version_string[] = "GNU assembler version 1.38";

/* this is set here, and used in dwarf2dbg.c as the apple_flags */
char *apple_flags = NULL;

/*
 * The list of signals to catch if not ignored.
 */
static int sig[] = { SIGHUP, SIGINT, SIGPIPE, SIGTERM, 0};
static void got_sig(
    int sig);

static void perform_an_assembly_pass(
    int argc,
    char **argv);

/* used by error calls (exported) */
char *progname = NULL;

/* non-NULL if AS_SECURE_LOG_FILE is set */
const char *secure_log_file = NULL;

int
main(
int argc,
char **argv,
char **envp)
{
    int	work_argc;	/* variable copy of argc */
    char **work_argv;	/* variable copy of argv */
    char *arg;		/* an arg to program */
    char a;		/* an arg flag (after -) */
    char *out_file_name;/* name of object file, argument to -o if specified */
    int i, apple_flags_size;
    struct directory_stack *dirtmp;

	progname = argv[0];

	/*
	 * Set up to catch the signals listed in sig[] that are not ignored.
	 */
	for(i = 0; sig[i] != 0; i++)
	    if(signal(sig[i], SIG_IGN) != SIG_IGN)
		signal(sig[i], got_sig);
	/*
	 * Set the default for the flags that will be parsed.
	 */
	memset(flagseen, '\0', sizeof(flagseen)); /* aint seen nothing yet */
	out_file_name = "a.out";	/* default .o file */

	/* This is the -dynamic flag, which is now the default */
	flagseen[(int)'k'] = TRUE;

	if(getenv("RC_DEBUG_OPTIONS") != NULL){
	    apple_flags_size = 1;
	    for(i = 0; i < argc; i++)
		apple_flags_size += strlen(argv[i]) + 2;
	    apple_flags = xmalloc(apple_flags_size);
	    apple_flags_size = 0;
	    for(i = 0; i < argc; i++){
		strcpy(apple_flags + apple_flags_size, argv[i]);
		apple_flags_size += strlen(argv[i]);
		apple_flags[apple_flags_size++] = ' ';
	    }
	    apple_flags[apple_flags_size] = '\0';
	}

	/*
	 * Parse arguments, but we are only interested in flags.
	 * When we find a flag, we process it then make it's argv[] NULL.
	 * This helps any future argv[] scanners avoid what we processed.
	 * Since it is easy to do here we interpret the special arg "-"
	 * to mean "use stdin" and we set that argv[] pointing to "".
	 * After we have munged argv[], the only things left are source file
	 * name(s) and ""(s) denoting stdin. These file names are used
	 * (perhaps more than once) later.
	 */
	work_argc = argc - 1;		/* don't count argv[0] */
	work_argv = argv + 1;		/* skip argv[0] */
	for( ; work_argc-- ; work_argv++){

	    /* work_argv points to this argument */
	    arg = *work_argv;

	    /* Filename. We need it later. */
	    if(*arg != '-')
		continue;

	    if(strcmp(arg, "--gstabs") == 0){
		/* generate stabs for debugging assembly code */
		flagseen[(int)'g'] = TRUE;
		*work_argv = NULL; /* NULL means 'not a file-name' */
		continue;
	    }
	    if(strcmp(arg, "--gdwarf2") == 0 || strcmp(arg, "-gdwarf-2") == 0){
		debug_type = DEBUG_DWARF2;
		*work_argv = NULL; /* NULL means 'not a file-name' */
		continue;
	    }
	    if(strncmp(arg, "-mcpu", 5) == 0){
		/* ignore -mcpu as it is only used with clang(1)'s integrated
		   assembler, but the as(1) driver will pass it. */
		*work_argv = NULL; /* NULL means 'not a file-name' */
		continue;
	    }

	    /* Keep scanning args looking for flags. */
	    if (arg[1] == '-' && arg[2] == 0) {
		/* "--" as an argument means read STDIN */
		/* on this scan, we don't want to think about filenames */
		*work_argv = "";	/* Code that means 'use stdin'. */
		continue;
	    }

	    /* This better be a switch ( -l where l is a letter. */
	    arg++;		/* -> letter. */

	    /* scan all the 1-char flags */
	    while((a = *arg)){
		arg ++;	/* arg -> after letter. */
		a &= 0x7F;	/* ascii only please */
		if(flagseen[(int)a] && (a != 'I') && (a != 'a') && (a != 'f') &&
		   (a != 'd') && (a != 's') && (a != 'k'))
			as_warn("%s: Flag option -%c has already been seen!",
				progname, a);
		if(a != 'f' && a != 'n' && a != 'g')
		    flagseen[(int)a] = TRUE;
		switch(a){
		case 'f':
		    if(strcmp(arg-1, "force_cpusubtype_ALL") == 0){
			force_cpusubtype_ALL = 1;
			arg = "";	/* Finished with this arg. */
			break;
		    }
		    /* -f means fast - no need for "app" preprocessor. */
		    flagseen[(int)a] = TRUE;
		    break;

		case 'L': /* -L means keep L* symbols */
		    break;

		case 'o':
		    if(*arg != '\0') /* Rest of argument is object file-name. */
			out_file_name = arg;
		    else if(work_argc){	/* Want next arg for a file-name. */
			*work_argv = NULL; /* This is not a file-name. */
			work_argc--;
			out_file_name = *++work_argv;
		    }
		    else
			as_fatal("%s: I expected a filename after -o. \"%s\" "
				"assumed.", progname, out_file_name);
		    arg = "";	/* Finished with this arg. */
		    break;

		case 'R':
		    /* -R means put data into text segment */
		    as_fatal("%s: -R option not supported (use the "
			    ".const directive)", progname);
		    flagseen['R'] = FALSE;
		    break;

		case 'v':
		    fprintf(stderr, APPLE_INC_VERSION " %s, ", apple_version);
		    fprintf(stderr, "%s\n", version_string);
		    if(*arg && strcmp(arg,"ersion"))
			as_fatal("Unknown -v option ignored");
		    while(*arg)
			arg++;	/* Skip the rest */
		    break;

		case 'W':
		    /* -W means don't warn about things */
		    break;

		case 'I':
		    /* Add directory to path for includes */
		    dirtmp = (struct directory_stack *)
			xmalloc(sizeof(struct directory_stack));
		    /* New one goes on the end */
		    dirtmp->next = 0;
		    if(include == 0)
			include = dirtmp;
		    else
			include_tail->next = dirtmp;
		    /* Tail follows the last one */
		    include_tail = dirtmp;
		    /* Rest of argument is include file-name. */
		    if(*arg)
			dirtmp->fname = arg;
		    else if (work_argc){
			/* Want next arg for a file-name. */
			/* This is not a file-name. */
			*work_argv = NULL;
			work_argc--;
			dirtmp->fname = *++work_argv;
		    }
		    else
			as_fatal("I expected a filename after -I.");
		    arg = "";	/* Finished with this arg. */
		    break;

		case 'g':
		    /* -g no longer means generate stabs for debugging
		       assembly code but to generate dwarf2 for assembly code.
		       If stabs if really wanted then --gstabs can be used. */
		    debug_type = DEBUG_DWARF2;
		    break;

		case 'n':
#ifdef PPC
		    if(strcmp(arg-1, "no_ppc601") == 0)
			goto unknown_flag;
#endif
		    /* no default .text section */
		    flagseen[(int)a] = TRUE;
		    break;

#ifdef PPC
		case 'p':
		    if(strcmp(arg-1, "ppcasm") == 0)
			goto unknown_flag;
#endif

		case 'd':
		    if(strcmp(arg-1, "dynamic") == 0){
			arg = "";	/* Finished with this arg. */
			flagseen[(int)'k'] = TRUE;
			break;
		    }
		    goto unknown_flag;

		case 's':
		    if(strcmp(arg-1, "static") == 0){
			arg = "";	/* Finished with this arg. */
			flagseen[(int)'k'] = FALSE;
			break;
		    }
		    goto unknown_flag;

		case 'N':
		    if(strcmp(arg-1, "NEXTSTEP-deployment-target") == 0){
			arg = "";	/* Finished with this arg. */
			/* Want next arg for a <release_tag> */
			if(work_argc){
			    /* This, "-NEXTST..." is not a file-name. */
			    *work_argv = NULL;
			    work_argc--;
			    work_argv++;
			    if(strcmp(*work_argv, "3.3") == 0){
				flagseen[(int)'k'] = TRUE;
			    }
			    else if(strcmp(*work_argv, "3.2") == 0){
				flagseen[(int)'k'] = FALSE;
			    }
			    else{
				as_fatal("I expected '3.2' or '3.3' after "
				    "-NEXTSTEP-deployment-target.");
			    }
			}
			else
			    as_fatal("I expected a <release_tag> "
				     "after -NEXTSTEP-deployment-target.");
			break;
		    }
		    goto unknown_flag;

		case 'k':
		    /* use new features incompatible with 3.2 */
		    break;

		case 'V':
		    /* as driver's -V, verbose, flag */
		    break;

		case 'a':
		    if(strcmp(arg-1, "arch_multiple") == 0){
			arch_multiple = 1;
			arg = "";	/* Finished with this arg. */
			break;
		    }
		    else if(strcmp(arg-1, "arch") == 0){
			arg = "";	/* Finished with this arg. */
			/* Want next arg for a <arch_type> */
			if(work_argc){
			    /* This, "-arch" is not a file-name. */
			    *work_argv = NULL;
			    work_argc--;
			    work_argv++;
#ifdef M68K
			    if(strcmp(*work_argv, "m68030") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
				    CPU_SUBTYPE_MC68030_ONLY)
				    as_fatal("can't specify both "
					"-arch m68030 and -arch "
					"m68040");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_MC68030_ONLY;
			    }
			    else if(strcmp(*work_argv,
					    "m68040") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
				    CPU_SUBTYPE_MC68040)
				    as_fatal("can't specify both "
					"-arch m68030 and -arch "
					"m68040");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_MC68040;
			    }
			    else if(strcmp(*work_argv, "m68k") != 0)
				as_fatal("I expected 'm68k', "
				    "'m68030' or 'm68040' after "
				    "-arch for this assembler.");
#endif
#ifdef M88K
			    if(strcmp(*work_argv, "m88k") != 0)
				as_fatal("I expected 'm88k' after "
				       "-arch for this assembler.");
#endif
#if defined(PPC) && !defined(ARCH64)
			    if(strcmp(*work_argv, "ppc601") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_601)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_601;
			    }
			    else if(strcmp(*work_argv,
					   "ppc603") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_603)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_603;
			    }
			    else if(strcmp(*work_argv,
					   "ppc603e") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_603e)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_603e;
			    }
			    else if(strcmp(*work_argv,
					   "ppc603ev") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_603ev)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_603ev;
			    }
			    else if(strcmp(*work_argv,
					   "ppc604") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_604)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_604;
			    }
			    else if(strcmp(*work_argv,
					   "ppc604e") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_604e)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_604e;
			    }
			    else if(strcmp(*work_argv,
					   "ppc750") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_750)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_750;
			    }
			    else if(strcmp(*work_argv,
					   "ppc7400") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_7400)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_7400;
			    }
			    else if(strcmp(*work_argv,
					   "ppc7450") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_7450)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_7450;
			    }
			    else if(strcmp(*work_argv,
					   "ppc970") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_970)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_970;
			    }
			    else if(strcmp(*work_argv, "ppc") != 0 &&
			    	    strcmp(*work_argv, "m98k") != 0)
				as_fatal("I expected 'ppc' after "
				       "-arch for this assembler.");
#endif /* defined(PPC) && !defined(ARCH64) */
#if defined(PPC) && defined(ARCH64)
			    if(strcmp(*work_argv,
					   "ppc970-64") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_POWERPC_970)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_POWERPC_970;
			    }
			    else
#endif
#if defined(PPC) && defined(ARCH64)
			    if(strcmp(*work_argv, "ppc64") != 0)
			      as_fatal("I expected 'ppc64' after "
				       "-arch for this assembler.");
#endif
#ifdef I860
			    if(strcmp(*work_argv, "i860") != 0)
				as_fatal("I expected 'i860' after "
				       "-arch for this assembler.");
#endif
#if defined(I386) && !defined(ARCH64)
			    if(strcmp(*work_argv, "i486") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_486)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_486;
			    }
			    else if(strcmp(*work_argv,
					   "i486SX") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_486SX)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_486SX;
			    }
			    else if(strcmp(*work_argv, "i586") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_586)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_586;
			    }
			    else if(strcmp(*work_argv, "pentium") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_PENT)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_PENT;
			    }
			    else if(strcmp(*work_argv, "pentpro") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_PENTPRO)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_PENTPRO;
			    }
			    else if(strcmp(*work_argv, "i686") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_PENTPRO)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_PENTPRO;
			    }
			    else if(strcmp(*work_argv, "pentIIm3") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_PENTII_M3)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_PENTII_M3;
			    }
			    else if(strcmp(*work_argv, "pentIIm5") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_PENTII_M5)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_PENTII_M5;
			    }
			    else if(strcmp(*work_argv, "pentium4") ==0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_PENTIUM_4)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_PENTIUM_4;
			    }
			    else if(strcmp(*work_argv, "i386") != 0)
				as_fatal("I expected 'i386', 'i486', 'i486SX', "
				   "'i586', 'pentium', 'i686', 'pentpro', "
				   "'pentIIm3', or 'pentIIm5' after -arch "
				   "for this assembler.");
#endif
#if defined(I386) && defined(ARCH64)
			    if(strcmp(*work_argv, "x86_64") != 0)
			      as_fatal("I expected 'x86_64' after "
				       "-arch for this assembler.");
#endif
#ifdef HPPA
			    if(strcmp(*work_argv, "hppa") != 0)
				as_fatal("I expected 'hppa' after "
					 "-arch for this assembler.");
#endif
#ifdef SPARC
			    if(strcmp(*work_argv, "sparc") != 0)
				as_fatal("I expected 'sparc' after "
					 "-arch for this assembler.");
#endif
#ifdef ARM
			    if(strcmp(*work_argv,
					   "arm") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V4T)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V4T;
			    }
			    else if(strcmp(*work_argv,
					   "armv4t") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V4T)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V4T;
			    }
			    else if(strcmp(*work_argv,
					   "armv5") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V5TEJ)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V5TEJ;
			    }
			    else if(strcmp(*work_argv,
					   "xscale") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_XSCALE)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_XSCALE;
			    }
			    else if(strcmp(*work_argv,
					   "armv6") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V6)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V6;
			    }
			    else if(strcmp(*work_argv,
					   "armv6m") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V6M)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V6M;
			    }
			    else if(strcmp(*work_argv,
					   "armv7") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V7)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V7;
			    }
			    else if(strcmp(*work_argv,
					   "armv7f") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V7F)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V7F;
			    }
			    else if(strcmp(*work_argv,
					   "armv7s") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V7S)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V7S;
			    }
			    else if(strcmp(*work_argv,
					   "armv7k") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V7K)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V7K;
			    }
			    else if(strcmp(*work_argv,
					   "armv7m") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V7M)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V7M;
			    }
			    else if(strcmp(*work_argv,
					   "armv7em") == 0){
				if(archflag_cpusubtype != -1 &&
				   archflag_cpusubtype !=
					CPU_SUBTYPE_ARM_V7EM)
				    as_fatal("can't specify more "
				       "than one -arch flag ");
				specific_archflag = *work_argv;
				archflag_cpusubtype =
				    CPU_SUBTYPE_ARM_V7EM;
			    }
			    else
				as_fatal("I expected 'arm' after "
					 "-arch for this assembler.");
#endif
			}
			else
			    as_fatal("I expected an <arch_type> "
				     "after -arch.");
			break;
		    }
		    /* fall through for non -arch flag */
		default:
unknown_flag:
		    --arg;
		    if(md_parse_option(&arg, &work_argc, &work_argv) == 0)
			as_fatal("%s: I don't understand '%c' flag!", progname,
				a);
		    if(arg && *arg)
			arg++;
		    break;
		}
	    }
	    /*
	     * We have just processed a "-..." arg, which was not a
	     * file-name. Smash it so the
	     * things that look for filenames won't ever see it.
	     *
	     * Whatever work_argv points to, it has already been used
	     * as part of a flag, so DON'T re-use it as a filename.
	     */
	    *work_argv = NULL; /* NULL means 'not a file-name' */
	}
	if(flagseen['g'] == TRUE && flagseen['n'] == TRUE)
	    as_fatal("-g can't be specified if -n is specified");
	/*
	 * If we haven't seen a -force_cpusubtype_ALL or an -arch flag for a
	 * specific architecture then let the machine instructions in the
	 * assembly determine the cpusubtype of the output file.
	 */
	if(force_cpusubtype_ALL_for_cputype(md_cputype) == TRUE)
	    force_cpusubtype_ALL = TRUE;
	if(force_cpusubtype_ALL && specific_archflag)
	    archflag_cpusubtype = -1;

	/*
	 * Test to see if the AS_SECURE_LOG_FILE environment
	 * variable is set and save the value.
	 */
	secure_log_file = getenv("AS_SECURE_LOG_FILE");

	/*
	 * Call the initialization routines.
	 */
	symbol_begin();			/* symbols.c */
	sections_begin();		/* sections.c */
	read_begin();			/* read.c */
#ifdef PPC
	if(flagseen[(int)'p'] == TRUE)
	    ppcasm_read_begin();	/* read.c */
#endif /* PPC */
	md_begin();			/* MACHINE.c */
	input_scrub_begin();		/* input_scrub.c */

	/* Here with flags set up in flagseen[]. */
	perform_an_assembly_pass(argc, argv); /* Assemble it. */

	if(seen_at_least_1_file() && bad_error != TRUE){
	    /*
	     * If we've been collecting dwarf2 .debug_line info, either for
	     * assembly debugging or on behalf of the compiler, emit it now.
	     */
	    dwarf2_finish();

	    layout_addresses();
	    write_object(out_file_name);
	}

	input_scrub_end();
	md_end();			/* MACHINE.c */

	return(bad_error);		/* WIN */
}
 
/*			perform_an_assembly_pass()
 *
 * Here to attempt 1 pass over each input file.
 * We scan argv[*] looking for filenames or exactly "" which is
 * shorthand for stdin. Any argv that is NULL is not a file-name.
 * We set need_pass_2 TRUE if, after this, we still have unresolved
 * expressions of the form (unknown value)+-(unknown value).
 *
 * Note the un*x semantics: there is only 1 logical input file, but it
 * may be a catenation of many 'physical' input files.
 */
static
void
perform_an_assembly_pass(
int argc,
char **argv)
{
    char *buffer;		/* Where each bufferful of lines will start. */
    int saw_a_file;

	saw_a_file = 0;

	argv++;			/* skip argv[0] */
	argc--;			/* skip argv[0] */
	while(argc--){
	    if(*argv){		/* Is it a file-name argument? */
		/* argv -> "" if stdin desired, else -> filename */
		if((buffer = input_scrub_new_file(*argv))){
		    saw_a_file++;
		    read_a_source_file(buffer);
		}
	    }
	    argv++;			/* completed that argv */
	}
	if(!saw_a_file)
	    if((buffer = input_scrub_new_file("")))
		    read_a_source_file(buffer);
}

static
void
got_sig(
int sig)
{
	as_bad("Interrupted by signal %d",sig);
	exit(1);
}
