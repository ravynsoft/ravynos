/*
 * The assembler driver as and runs the assembler for the "-arch <arch_flag>"
 * (if given) in ../libexec/as/<arch_flag>/as or
 * ../local/libexec/as/<arch_flag>/as.  Or runs the assembler for the host
 * architecture as returned by get_arch_from_host().  The driver only checks to
 * make sure their are not multiple arch_flags and then passes all flags to the
 * assembler it will run.
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include "libc.h"
#include <sys/file.h>
#include <mach/mach.h>
#include "stuff/arch.h"
#include "stuff/errors.h"
#include "stuff/execute.h"
#include "stuff/allocate.h"
#include "stuff/port.h" /* cctools-port: find_executable() */
#include <mach-o/dyld.h>

/* used by error calls (exported) */
char *progname = NULL;



int
main(
int argc,
char **argv,
char **envp)
{
    const char *LIB = ASLIBEXECDIR;
    const char *LOCALLIB = ASLIBEXECDIR;
    const char *AS = "/as";

    int i, j;
    uint32_t count, verbose, run_clang;
    char *p, c, *arch_name, *as, *as_local;
    char **new_argv;
    const char *CLANG = "clang";
    char *prefix, buf[MAXPATHLEN], resolved_name[PATH_MAX];
    uint32_t bufsize;
    struct arch_flag arch_flag;
    const struct arch_flag *arch_flags, *family_arch_flag;
    enum bool oflag_specified, qflag, Qflag, some_input_files;

	progname = argv[0];
	arch_name = NULL;
	verbose = 0;
	run_clang = 0;
	oflag_specified = FALSE;
	qflag = FALSE;
	Qflag = FALSE;
	some_input_files = FALSE;
	/*
	 * Construct the prefix to the assembler driver.
	 */
	bufsize = MAXPATHLEN;
	p = buf;
	i = _NSGetExecutablePath(p, &bufsize);
	if(i == -1){
	    p = allocate(bufsize);
	    _NSGetExecutablePath(p, &bufsize);
	}
	prefix = realpath(p, resolved_name);
	if(prefix == NULL)
	    system_fatal("realpath(3) for %s failed", p);
	p = rindex(prefix, '/');
	if(p != NULL)
	    p[1] = '\0';
	/*
	 * Process the assembler flags exactly like the assembler would (except
	 * let the assembler complain about multiple flags, bad combinations of
	 * flags, unknown single letter flags and the like).  The main thing
	 * here is to parse out the "-arch <arch_flag>" and to do so the
	 * multiple argument and multiple character flags need to be known how
	 * to be stepped over correctly.
	 */
	for(i = 1; i < argc; i++){
	    /*
	     * The assembler flags start with '-' except that "--" is recognized
	     * as assemble from stdin and that flag "--" is not allowed to be
	     * grouped with other flags (so "-a-" is not the same as "-a --").
	     */
	    if(argv[i][0] == '-' &&
	       !(argv[i][1] == '-' && argv[i][2] == '\0')){
		/*
		 * Treat a single "-" as reading from stdin input also.
		 */
		if(argv[i][1] == '\0')
		    some_input_files = TRUE;
		/*
		 * the assembler allows single letter flags to be grouped
		 * together so "-abc" is the same as "-a -b -c".  So that
		 * logic must be followed here.
		 */
		for(p = &(argv[i][1]); (c = *p); p++){
		    /*
		     * The assembler simply ignores the high bit of flag
		     * characters and not treat them as different characters
		     * as they are (but the argument following the flag
		     * character is not treated this way).  So it's done
		     * here as well to match it.
		     */
		    c &= 0x7F;
		    switch(c){
		    /*
		     * Flags that take a single argument.  The argument is the
		     * rest of the current argument if there is any or the it is
		     * the next argument.  Again errors like missing arguments
		     * are not handled here but left to the assembler.
		     */
		    case 'o':	/* -o name */
			oflag_specified = TRUE;
		    case 'I':	/* -I directory */
		    case 'm':	/* -mc68000, -mc68010 and mc68020 */
		    case 'N':	/* -NEXTSTEP-deployment-target */
			/*
			 * We want to skip the next argv if the value is not
			 * contained in this argv eg: -I dir .
			 */
			if(p[1] == '\0')
			    i++;
			/*
			 * And in case the value is contained in this argv
			 * (eg: -Idir), skip the rest.
			 */
			while(p[1])
			    p++;
			p = " "; /* Finished with this arg. */
			break;
	    	    case 'g':
			if(strcmp(p, "gstabs") == 0 ||
	    		   strcmp(p, "gdwarf2") == 0 ||
			   strcmp(p, "gdwarf-2") == 0){
			    p = " "; /* Finished with this arg. */
			}
			break;
		    case 'd':
			if(strcmp(p, "dynamic") == 0){
			    p = " "; /* Finished with this arg. */
			}
			break;
		    case 's':
			if(strcmp(p, "static") == 0){
			    p = " "; /* Finished with this arg. */
			}
			break;
		    case 'a':
		        if(strcmp(p, "arch_multiple") == 0){
			    p = " "; /* Finished with this arg. */
			}
			if(strcmp(p, "arch") == 0){
			    if(i + 1 >= argc)
				fatal("missing argument to %s option", argv[i]);
			    if(arch_name != NULL)
				fatal("more than one %s option (not allowed, "
				      "use cc(1) instead)", argv[i]);
			    arch_name = argv[i+1];
			    p = " "; /* Finished with this arg. */
			    i++;
			    break;
			}
			/* fall through for non "-arch" */
		    case 'f':
			if(strcmp(p, "force_cpusubtype_ALL") == 0){
			    p = " "; /* Finished with this arg. */
			    break;
			}
		    case 'k':
		    case 'v':
		    case 'W':
		    case 'L':
		    case 'l':
		    default:
			/* just recognize it, do nothing */
			break;
		    case 'q':
			qflag = TRUE;
			break;
		    case 'Q':
			Qflag = TRUE;
			break;
		    case 'V':
			verbose = 1;
			break;
		    }
		}
	    }
	    else{
		some_input_files = TRUE;
	    }
	}

	/*
	 * Construct the name of the assembler to run from the given -arch
	 * <arch_flag> or if none then from the value returned from
	 * get_arch_from_host().
	 */
	if(arch_name == NULL){
	    if(get_arch_from_host(&arch_flag, NULL)){
#if __LP64__
		/*
		 * If runing as a 64-bit binary and on an Intel x86 host
		 * default to the 64-bit assember.
		 */
		if(arch_flag.cputype == CPU_TYPE_I386)
		    arch_flag = *get_arch_family_from_cputype(CPU_TYPE_X86_64);
#endif /* __LP64__ */
		arch_name = arch_flag.name;
	    }
	    else
		fatal("unknown host architecture (can't determine which "
		      "assembler to run)");
	}
	else{
	    /*
	     * Convert a possible machine specific architecture name to a
	     * family name to base the name of the assembler to run.
	     */
	    if(get_arch_from_flag(arch_name, &arch_flag) != 0){
		family_arch_flag =
			get_arch_family_from_cputype(arch_flag.cputype);
		if(family_arch_flag != NULL)
		    arch_name = (char *)(family_arch_flag->name);
	    }

	}

	if(qflag == TRUE && Qflag == TRUE){
	    printf("%s: can't specifiy both -q and -Q\n", progname);
	    exit(1);
	}
	/*
	 * If the environment variable AS_INTEGRATED_ASSEMBLER is set then set
	 * the qflag to call clang(1) with -integrated-as unless the -Q flag is
	 * set and do this for the supported architectures.
	 */
	if(Qflag == FALSE &&
           getenv("AS_INTEGRATED_ASSEMBLER") != NULL &&
	   (arch_flag.cputype == CPU_TYPE_X86_64 ||
	    arch_flag.cputype == CPU_TYPE_I386 ||
	    arch_flag.cputype == CPU_TYPE_ARM64 ||
	    arch_flag.cputype == CPU_TYPE_ARM64_32 ||
	    arch_flag.cputype == CPU_TYPE_ARM)){
	    qflag = TRUE;
	}
	if(qflag == TRUE &&
	   (arch_flag.cputype != CPU_TYPE_X86_64 &&
	    arch_flag.cputype != CPU_TYPE_I386 &&
	    arch_flag.cputype != CPU_TYPE_ARM64 &&
	    arch_flag.cputype != CPU_TYPE_ARM64_32 &&
	    arch_flag.cputype != CPU_TYPE_ARM)){
	    printf("%s: can't specifiy -q with -arch %s\n", progname,
		   arch_flag.name);
	    exit(1);
	}

	/*
	 * When the target assembler is for arm64, for now:
	 *   rdar://8913781 ARM64: cctools 'as' driver should invoke clang
	 *		    for ARM64 assembly files
	 * use clang.  Later for:
	 *   rdar://8928193 ARM64: Standalone 'as' driver
	 * when there is and llvm-mc based standalone 'as' driver and it is
 	 * in the usual place as the other target assemblers this use of clang
	 * will be removed.
	 */ 
	if(arch_flag.cputype == CPU_TYPE_ARM64 ||
           arch_flag.cputype == CPU_TYPE_ARM64_32){
	    if(Qflag == TRUE){
		printf("%s: can't specifiy -Q with -arch %s\n", progname, 
                       arch_flag.cputype == CPU_TYPE_ARM64 ? "arm64" : "arm64_32");
		exit(1);
	    }
	    run_clang = 1;
	}

	/*
	 * Use the LLVM integrated assembler as the default with the as(1)
	 * driver for Intel (64-bit & 32-bit) as well as ARM for 32-bit too
	 * (64-bit ARM handled above) via running clang.
	 */
	if(arch_flag.cputype == CPU_TYPE_X86_64 ||
	   arch_flag.cputype == CPU_TYPE_I386 ||
	   arch_flag.cputype == CPU_TYPE_ARM)
	    run_clang = 1;

#ifndef DISABLE_CLANG_AS /* cctools-port */
	if(getenv("CCTOOLS_NO_CLANG_AS") != NULL) /* cctools-port */
	    run_clang = 0;

	/*
	 * Use the clang as the assembler if is the default or asked to with
	 * the -q flag. But don't use it asked to use the system assembler
	 * with the -Q flag.
	 */
	if((run_clang || qflag) && !Qflag &&
	   (arch_flag.cputype == CPU_TYPE_X86_64 ||
	    arch_flag.cputype == CPU_TYPE_I386 ||
	    arch_flag.cputype == CPU_TYPE_ARM64 ||
	    arch_flag.cputype == CPU_TYPE_ARM64_32 ||
	    arch_flag.cputype == CPU_TYPE_ARM)){
#if 0 /* cctools port */
	    as = makestr(prefix, CLANG, NULL);
#endif
	    /* cctools-port start */
#ifndef __APPLE__
	    char *target_triple = getenv("CCTOOLS_CLANG_AS_TARGET_TRIPLE");
#endif /* ! __APPLE__ */
	    as = find_executable("clang");
	    /* cctools-port end */
	    if(!as || access(as, F_OK) != 0){ /* cctools-port: added  !as || */
		printf("%s: assembler (%s) not installed\n", progname,
		       as ? as : "clang"); /* cctools-port:
					      added  ? as : "clang" */
		exit(1);
	    }
	    new_argv = allocate((argc + 10) * sizeof(char *)); /* cctools-port:
								  + 8 -> + 10 */
	    new_argv[0] = as;
	    j = 1;
	    /*
	     * Add "-x assembler" in case the input does not end in .s this must
	     * come before "-" or the clang driver will issue an error:
	     * "error: -E or -x required when input is from standard input"
	     */
	    new_argv[j] = "-x";
	    j++;
	    new_argv[j] = "assembler";
	    j++;
	    /*
	     * If we have not seen some some_input_files or a "-" or "--" to
	     * indicate we are assembling stdin add a "-" so clang will
	     * assemble stdin as as(1) would.
	     */
	    if(some_input_files == FALSE){
		new_argv[j] = "-";
		j++;
	    }
	    for(i = 1; i < argc; i++){
		/*
		 * Translate as(1) use of "--" for stdin to clang's use of "-".
		 */
		if(strcmp(argv[i], "--") == 0){
		    new_argv[j] = "-";
		    j++;
		}
		/*
		 * Do not pass command line argument that are Unknown to
		 * to clang.
		 */
		else if(strcmp(argv[i], "-V") != 0 &&
		   strcmp(argv[i], "-q") != 0 &&
		   strcmp(argv[i], "-Q") != 0){
		    new_argv[j] = argv[i];
		    j++;
		}
	    }
	    /*
	     * clang requires a "-o a.out" if not -o is specified.
	     */
	    if(oflag_specified == FALSE){
		new_argv[j] = "-o";
		j++;
		new_argv[j] = "a.out";
		j++;
	    }
	    /* Add -integrated-as or clang will run as(1). */
	    new_argv[j] = "-integrated-as";
	    j++;
	    /* Add -c or clang will run ld(1). */
	    new_argv[j] = "-c";
	    j++;
	    /* cctools-port start */
#ifndef __APPLE__
	    new_argv[j] = "-target";
	    j++;
	    new_argv[j] = target_triple ? target_triple : "unknown-apple-darwin";
	    j++;
#endif /* ! __APPLE__ */
	    /* cctools-port end */
	    new_argv[j] = NULL;
	    if(execute(new_argv, verbose))
		exit(0);
	    else
		exit(1);
	}
#endif /* ! DISABLE_CLANG_AS */

	/*
	 * If this assembler exist try to run it else print an error message.
	 */
	as = makestr(LIB, arch_name, AS, NULL);
	new_argv = allocate((argc + 1) * sizeof(char *));
	new_argv[0] = as;
	j = 1;
	for(i = 1; i < argc; i++){
	    /*
	     * Do not pass command line argument that are unknown to as.
	     */
	    if(strcmp(argv[i], "-q") != 0 &&
	       strcmp(argv[i], "-Q") != 0){
		new_argv[j] = argv[i];
		j++;
	    }
	}
	new_argv[j] = NULL;
	if(access(as, F_OK) == 0){
	    argv[0] = as;
	    if(execute(new_argv, verbose))
		exit(0);
	    else
		exit(1);
	}
	as_local = makestr(LOCALLIB, arch_name, AS, NULL);
	new_argv[0] = as_local;
	if(access(as_local, F_OK) == 0){
	    argv[0] = as_local;
	    if(execute(new_argv, verbose))
		exit(0);
	    else
		exit(1);
	}
	printf("%s: assembler (%s or %s) for architecture %s not installed\n",
	       progname, as, as_local, arch_name);
	arch_flags = get_arch_flags();
	count = 0;
	for(i = 0; arch_flags[i].name != NULL; i++){
	    as = makestr(LIB, arch_flags[i].name, AS, NULL);
	    if(access(as, F_OK) == 0){
		if(count == 0)
		    printf("Installed assemblers are:\n");
		printf("%s for architecture %s\n", as, arch_flags[i].name);
		count++;
	    }
	    else{
		as_local = makestr(LOCALLIB, arch_flags[i].name, AS,
				   NULL);
		if(access(as_local, F_OK) == 0){
		    if(count == 0)
			printf("Installed assemblers are:\n");
		    printf("%s for architecture %s\n", as_local,
			   arch_flags[i].name);
		    count++;
		}
	    }
	}
	if(count == 0)
	    printf("%s: no assemblers installed\n", progname);
	exit(1);
}
