/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#if !defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mman.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#include <signal.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mman.h>

#endif /* __APPLE__ */

#include "ctf_headers.h"
#include "ctftools.h"
#include "ctfmerge.h"
#include "traverse.h"
#include "memory.h"
#include "fifo.h"
#include "barrier.h"

const char *progname;
static char *outfile = NULL;
static char *tmpname = NULL;
static int dynsym;
int debug_level = DEBUG_LEVEL;

/*
 * Given several files containing CTF data, merge and uniquify that data into
 * a single CTF section in an output file.
 *
 */

void
usage(void)
{
	(void) fprintf(stderr,
	    "Usage: %s [-fgstv] -l label | -L labelenv -o outfile file ...\n"
	    "       %s [-fgstv] -l label | -L labelenv -o outfile -d uniqfile\n"
	    "       %*s [-g] [-D uniqlabel] file ...\n"
	    "       %s [-fgstv] -l label | -L labelenv -o outfile -w withfile "
	    "file ...\n"
	    "       %s [-g] -c srcfile destfile\n"
	    "       %s [-fgstv] -l label | -L labelenv -o master_macho_file -Z raw_ctf_outfile file ...\n"
	    "\n"
	    "  Note: if -L labelenv is specified and labelenv is not set in\n"
	    "  the environment, a default value is used.\n",
	    progname, progname, (int)strlen(progname), " ",
	    progname, progname, progname);
}

static void
terminate_cleanup(void)
{
	int dounlink = getenv("CTFMERGE_TERMINATE_NO_UNLINK") ? 0 : 1;

	if (tmpname != NULL && dounlink)
		unlink(tmpname);

	if (outfile == NULL)
		return;

	if (dounlink) {
		fprintf(stderr, "Removing %s\n", outfile);
		unlink(outfile);
	}
}

static void
copy_ctf_data(char *srcfile, char *destfile, int keep_stabs)
{
	tdata_t *srctd;

	if (read_ctf(&srcfile, 1, NULL, read_ctf_save_cb, &srctd, 1) == 0)
		terminate("No CTF data found in source file %s\n", srcfile);

	tmpname = mktmpname(destfile, ".ctf");
	write_ctf(srctd, destfile, tmpname, CTF_COMPRESS | keep_stabs);
	if (rename(tmpname, destfile) != 0) {
		terminate("Couldn't rename temp file %s to %s", tmpname,
		    destfile);
	}
	free(tmpname);
	tdata_free(srctd);
}

static int
strcompare(const void *p1, const void *p2)
{
	char *s1 = *((char **)p1);
	char *s2 = *((char **)p2);

	return (strcmp(s1, s2));
}

static int
merge_ctf_cb(tdata_t *td, char *name, void *arg)
{
	return (ctfmerge_add_td(td, name));
}

int
main(int argc, char **argv)
{
	tdata_t *mstrtd, *savetd;
	char *uniqfile = NULL, *uniqlabel = NULL;
	char *withfile = NULL;
	char *raw_ctf_file = NULL;
	char *label = NULL;
	char **ifiles, **tifiles;
	int verbose = 0, docopy = 0;
	int write_fuzzy_match = 0;
	int keep_stabs = 0;
	int require_ctf = 0;
	int nifiles, nielems;
	int c, i, idx, tidx, err;
	
	progname = basename(argv[0]);

	if (getenv("CTFMERGE_DEBUG_LEVEL"))
		debug_level = atoi(getenv("CTFMERGE_DEBUG_LEVEL"));

	err = 0;
	while ((c = getopt(argc, argv, ":cd:D:fgl:L:o:tvw:sZ:")) != EOF) {
		switch (c) {
		case 'c':
			docopy = 1;
			break;
		case 'd':
			/* Uniquify against `uniqfile' */
			uniqfile = optarg;
			break;
		case 'D':
			/* Uniquify against label `uniqlabel' in `uniqfile' */
			uniqlabel = optarg;
			break;
		case 'f':
			write_fuzzy_match = CTF_FUZZY_MATCH;
			break;
		case 'g':
			keep_stabs = CTF_KEEP_STABS;
			break;
		case 'l':
			/* Label merged types with `label' */
			label = optarg;
			break;
		case 'L':
			/* Label merged types with getenv(`label`) */
			if ((label = getenv(optarg)) == NULL)
				label = CTF_DEFAULT_LABEL;
			break;
		case 'o':
			/* Place merged types in CTF section in `outfile' */
			outfile = optarg;
			break;
		case 't':
			/* Insist *all* object files built from C have CTF */
			require_ctf = 1;
			break;
		case 'v':
			/* More debugging information */
			verbose = 1;
			break;
		case 'w':
			/* Additive merge with data from `withfile' */
			withfile = optarg;
			break;
		case 's':
			/* use the dynsym rather than the symtab */
			dynsym = CTF_USE_DYNSYM;
			break;
		case 'Z':
			/* Write raw CTF data by itself */
			raw_ctf_file = optarg;
			break;
		default:
			usage();
			exit(2);
		}
	}

	/* Validate arguments */
	if (docopy) {
		if (uniqfile != NULL || uniqlabel != NULL || label != NULL ||
		    outfile != NULL || withfile != NULL || dynsym != 0)
			err++;

		if (argc - optind != 2)
			err++;
	} else {
		if (uniqfile != NULL && withfile != NULL)
			err++;

		if (uniqlabel != NULL && uniqfile == NULL)
			err++;

		if (outfile == NULL || label == NULL)
			err++;

		if (argc - optind == 0)
			err++;
	}

	if ((uniqfile != NULL || withfile != NULL) && raw_ctf_file != NULL)
		err++;
		
	if (err) {
		usage();
		exit(2);
	}

	if (getenv("STRIPSTABS_KEEP_STABS") != NULL)
		keep_stabs = CTF_KEEP_STABS;

	if (uniqfile && access(uniqfile, R_OK) != 0) {
		warning("Uniquification file %s couldn't be opened and "
		    "will be ignored.\n", uniqfile);
		uniqfile = NULL;
	}
	if (withfile && access(withfile, R_OK) != 0) {
		warning("With file %s couldn't be opened and will be "
		    "ignored.\n", withfile);
		withfile = NULL;
	}
	if (outfile && access(outfile, R_OK|W_OK) != 0)
		terminate("Cannot open output file %s for r/w", outfile);

	if (raw_ctf_file && access(raw_ctf_file, F_OK) != -1)
		terminate("Raw CTF output file %s already exists", raw_ctf_file);
		
	/*
	 * This is ugly, but we don't want to have to have a separate tool
	 * (yet) just for copying an ELF section with our specific requirements,
	 * so we shoe-horn a copier into ctfmerge.
	 */
	if (docopy) {
		copy_ctf_data(argv[optind], argv[optind + 1], keep_stabs);

		exit(0);
	}

	set_terminate_cleanup(terminate_cleanup);

	/* Sort the input files and strip out duplicates */
	nifiles = argc - optind;
	ifiles = xmalloc(sizeof (char *) * nifiles);
	tifiles = xmalloc(sizeof (char *) * nifiles);

	for (i = 0; i < nifiles; i++)
		tifiles[i] = argv[optind + i];
	qsort(tifiles, nifiles, sizeof (char *), (int (*)())strcompare);

	ifiles[0] = tifiles[0];
	for (idx = 0, tidx = 1; tidx < nifiles; tidx++) {
		if (strcmp(ifiles[idx], tifiles[tidx]) != 0)
			ifiles[++idx] = tifiles[tidx];
	}
	nifiles = idx + 1;

	/* Make sure they all exist */
	if ((nielems = count_files(ifiles, nifiles)) < 0)
		terminate("Some input files were inaccessible\n");

	/* Prepare for the merge */
	ctfmerge_prepare(nielems);

	/*
	 * Start the merge
	 *
	 * We're reading everything from each of the object files, so we
	 * don't need to specify labels.
	 */
	if (read_ctf(ifiles, nifiles, NULL, merge_ctf_cb,
	    NULL, require_ctf) == 0) {
		/*
		 * If we're verifying that C files have CTF, it's safe to
		 * assume that in this case, we're building only from assembly
		 * inputs.
		 */
		if (require_ctf)
			exit(0);
		terminate("No ctf sections found to merge\n");
	}

	mstrtd = ctfmerge_done();

	/*
	 * All requested files have been merged, with the resulting tree in
	 * mstrtd.  savetd is the tree that will be placed into the output file.
	 *
	 * Regardless of whether we're doing a normal uniquification or an
	 * additive merge, we need a type tree that has been uniquified
	 * against uniqfile or withfile, as appropriate.
	 *
	 * If we're doing a uniquification, we stuff the resulting tree into
	 * outfile.  Otherwise, we add the tree to the tree already in withfile.
	 */

	if (verbose || debug_level) {
		debug(2, "Statistics for td %p\n", (void *)mstrtd);

		iidesc_stats(mstrtd->td_iihash);
	}

	if (uniqfile != NULL || withfile != NULL) {
		char *reffile, *reflabel = NULL;
		tdata_t *reftd;

		if (uniqfile != NULL) {
			reffile = uniqfile;
			reflabel = uniqlabel;
		} else
			reffile = withfile;

		if (read_ctf(&reffile, 1, reflabel, read_ctf_save_cb,
		    &reftd, require_ctf) == 0) {
			terminate("No CTF data found in reference file %s\n",
			    reffile);
		}

		savetd = tdata_new();

		if (CTF_TYPE_ISCHILD(reftd->td_nextid))
			terminate("No room for additional types in master\n");

		savetd->td_nextid = withfile ? reftd->td_nextid :
		    CTF_INDEX_TO_TYPE(1, TRUE);
		merge_into_master(NULL, mstrtd, reftd, savetd, 0);

		tdata_label_add(savetd, label, CTF_LABEL_LASTIDX);

		if (withfile) {
			/*
			 * savetd holds the new data to be added to the withfile
			 */
			tdata_t *withtd = reftd;

			tdata_merge(withtd, savetd);

			savetd = withtd;
		} else {
			char uniqname[MAXPATHLEN];
			labelent_t *parle;

			parle = tdata_label_top(reftd);

			savetd->td_parlabel = parle->le_name;

			strncpy(uniqname, reffile, sizeof (uniqname));
			uniqname[MAXPATHLEN - 1] = '\0';
			savetd->td_parname = atom_get(basename(uniqname));
		}

	} else {
		/*
		 * No post processing.  Write the merged tree as-is into the
		 * output file.
		 */
		tdata_label_free(mstrtd);
		tdata_label_add(mstrtd, label, CTF_LABEL_LASTIDX);

		savetd = mstrtd;
	}

	tmpname = mktmpname(outfile, ".ctf");
	write_ctf(savetd, outfile, tmpname,
		  CTF_COMPRESS | write_fuzzy_match | dynsym | keep_stabs);
	if (rename(tmpname, outfile) != 0)
	    terminate("Couldn't rename output temp file %s", tmpname);
	free(tmpname);
		
	return (0);
}
