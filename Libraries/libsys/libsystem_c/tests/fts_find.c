#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <fts.h>
#include <sys/types.h>
#include <sys/stat.h>

// Can ASan fts by uncommenting below
//#include "../gen/fts.c"

#ifndef DARWINTEST
#define fts_find_main main
#else
#include <darwintest.h>
#endif

int fts_find_main(int argc, char *argv[]);

#ifndef DARWINTEST
static char *
stat_str(struct stat *st)
{
	static char charbuf[256];
	snprintf(charbuf, sizeof(charbuf), "dev: %d, mode: %x, nlink: %d, ino: %lld, "
			"owner: %d/%d, rdev: %d, mtime: %ld, ctime: %ld, btime: %ld, "
			"size: %lld, blocks: %lld, blksize: %d, flags: %d, gen: %d",
			st->st_dev, st->st_mode, st->st_nlink, st->st_ino, st->st_uid,
			st->st_gid, st->st_rdev, st->st_mtimespec.tv_sec,
			st->st_ctimespec.tv_sec, st->st_birthtimespec.tv_sec, st->st_size,
			st->st_blocks, st->st_blksize, st->st_flags, st->st_gen);
	return charbuf;
}
#endif // DARWINTEST

int
fts_find_main(int argc, char *argv[])
{
	FTS *fts;
	FTSENT *ftse;

	bool print_children = false;
	int fts_options = FTS_COMFOLLOW | FTS_XDEV;
	optind = 1;
	optreset = 1;

	int ch;
	while ((ch = getopt(argc, argv, "lpcdsS")) != -1){
		switch (ch){
			case 'l':
				fts_options |= FTS_LOGICAL;
				break;
			case 'p':
				fts_options |= FTS_PHYSICAL;
				break;
			case 'c':
				print_children = true;
				break;
			case 'd':
				fts_options |= FTS_NOCHDIR;
				break;
			case 's':
				fts_options |= FTS_NOSTAT;
				break;
			case 'S':
				fts_options |= FTS_NOSTAT_TYPE;
				break;
			case '?':
				fprintf(stderr, "Usage: %s (-l|-p) [-c] [-d] [-s|-S] <path> ...\n", argv[0]);
				exit(EX_USAGE);
		}
	}

	if ((fts_options & (FTS_LOGICAL|FTS_PHYSICAL)) == 0){
		fprintf(stderr, "Usage: %s (-l|-p) [-c] [-s|-S] <path> ...\n", argv[0]);
		exit(EX_USAGE);
	}

	argc -= optind;
	argv += optind;

	char **args = alloca((size_t)(argc + 1)*sizeof(char*));
	for (int i = 0; i < argc; i++){
		args[i] = argv[i];
	}
	args[argc] = NULL;
	fts = fts_open_b(args, fts_options, ^(const FTSENT **a, const FTSENT **b){
		return strcmp((*a)->fts_name, (*b)->fts_name);
	});
	if (!fts) err(EX_DATAERR, "fts_open_b");

	while ((ftse = fts_read(fts)) != NULL) {
#ifndef DARWINTEST
		if (!print_children || (ftse->fts_info & FTS_D)){
			printf("%s (%s): 0x%x\n", ftse->fts_path, ftse->fts_name, ftse->fts_info);
			if (!(fts_options & (FTS_NOSTAT|FTS_NOSTAT_TYPE))) printf("\t\t%s\n", stat_str(ftse->fts_statp));
		}
#endif // DARWINTEST
		if (print_children){
			FTSENT *child = fts_children(fts, 0);
			while (child){
#ifndef DARWINTEST
				if (child->fts_info & FTS_F){
					printf("\t%s (%s): 0x%x\n", child->fts_path, child->fts_name, child->fts_info);
					if (!(fts_options & (FTS_NOSTAT|FTS_NOSTAT_TYPE))) printf("\t\t%s\n", stat_str(child->fts_statp));
				}
#endif // DARWINTEST
				child = child->fts_link;
			}
		}
	}

	(void)fts_close(fts);
	return 0;
}

#ifdef DARWINTEST
T_DECL(fts_find, "A find(1) example in fts"){
	int fts_argc = 3;
	char *fts_argv[] = {"fts_find", "-lc", "/System", NULL};
	if (fts_find_main(fts_argc, fts_argv) == 0){
		T_PASS("fts_find() completed successfully");
	} else {
		T_FAIL("fts_find() exited with error");
	}
}

T_DECL(fts_find_empty_path, "Test result for empty path"){
	char *paths[] = {"/System", "", NULL};

	FTS *fts = fts_open_b(paths, 0, ^(const FTSENT **a, const FTSENT **b){
		return strcmp((*a)->fts_name, (*b)->fts_name);
	});
	if (fts == NULL) {
		T_FAIL("fts_open() failed");
		return;
	}

	// The first entry name should be the empty string, because of the sort
	// order. The second entry should be "System".
	FTSENT *entry = fts_read(fts);
	T_ASSERT_NOTNULL(entry, "First fts_read() returned NULL");
	T_ASSERT_EQ_STR(entry->fts_name, "", "First entry name is empty");
	T_ASSERT_EQ((int)entry->fts_info, FTS_NS, "First fts_info is FTS_NS");
	T_ASSERT_EQ(entry->fts_errno, ENOENT, "First fts_errno is ENOENT");

	entry = fts_read(fts);
	T_ASSERT_NOTNULL(entry, "Second fts_read() returned NULL");
	T_ASSERT_EQ_STR(entry->fts_name, "System", "Second entry name is System");
	T_ASSERT_EQ((int)entry->fts_info, FTS_D, "Second fts_info is FTS_D");
	T_ASSERT_EQ(entry->fts_errno, 0, "Second fts_errno is 0");

	fts_close(fts);
}
#endif // DARWINTEST
