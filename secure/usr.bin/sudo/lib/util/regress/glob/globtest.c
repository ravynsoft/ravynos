/*	$OpenBSD: globtest.c,v 1.1 2008/10/01 23:04:36 millert Exp $	*/

/*
 * Public domain, 2008, Todd C. Miller <Todd.Miller@sudo.ws>
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_GLOB
# include <glob.h>
#else
# include <compat/glob.h>
#endif
#include <errno.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_util.h>

#define MAX_RESULTS	256

struct gl_entry {
	int flags;
	int nresults;
	char pattern[1024];
	char *results[MAX_RESULTS];
};

int test_glob(struct gl_entry *);
sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char **argv)
{
	FILE *fp = stdin;
	char buf[2048], *cp, *ep;
	int ch, errors = 0, ntests = 0, lineno;
	struct gl_entry entry;
	size_t len;

	initprogname(argc > 0 ? argv[0] : "globtest");

	while ((ch = getopt(argc, argv, "v")) != -1) {
	    switch (ch) {
	    case 'v':
		/* ignore */
		break;
	    default:
		fprintf(stderr, "usage: %s [-v]\n", getprogname());
		return EXIT_FAILURE;
	    }
	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		if ((fp = fopen(argv[0], "r")) == NULL) {
			perror(argv[0]);
			return EXIT_FAILURE;
		}
	}

	/*
	 * Read in test file, which is formatted thusly:
	 *
	 * [pattern] <flags>
	 * result1
	 * result2
	 * result3
	 * ...
	 *
	 */
	lineno = 0;
	memset(&entry, 0, sizeof(entry));
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		lineno++;
		len = strlen(buf);
		if (len > 0) {
			if (buf[len - 1] != '\n') {
				fputs("globtest: missing newline at EOF\n", stderr);
				return EXIT_FAILURE;
			}
			buf[--len] = '\0';
		}
		if (len == 0)
			continue; /* blank line */

		if (buf[0] == '[') {
			/* check previous pattern */
			if (entry.pattern[0]) {
				errors += test_glob(&entry);
				ntests++;
			}

			/* start new entry */
			if ((cp = strrchr(buf + 1, ']')) == NULL) {
				fprintf(stderr,
				    "globtest: invalid entry on line %d\n",
				    lineno);
				return EXIT_FAILURE;
			}
			len = cp - buf - 1;
			if (len >= sizeof(entry.pattern)) {
				fprintf(stderr,
				    "globtest: pattern too big on line %d\n",
				    lineno);
				return EXIT_FAILURE;
			}
			memcpy(entry.pattern, buf + 1, len);
			entry.pattern[len] = '\0';

			cp += 2;
			if (*cp++ != '<') {
				fprintf(stderr,
				    "globtest: invalid entry on line %d\n",
				    lineno);
				return EXIT_FAILURE;
			}
			ep = strchr(cp, '>');
			if (ep == NULL) {
				fprintf(stderr,
				    "globtest: invalid entry on line %d\n",
				    lineno);
				return EXIT_FAILURE;
			}
			*ep = '\0';
			entry.flags = 0;
			for ((cp = strtok_r(cp, "|", &ep)); cp != NULL; (cp = strtok_r(NULL, "|", &ep))) {
				if (strcmp(cp, "GLOB_APPEND") == 0)
					entry.flags |= GLOB_APPEND;
				else if (strcmp(cp, "GLOB_DOOFFS") == 0)
					entry.flags |= GLOB_DOOFFS;
				else if (strcmp(cp, "GLOB_ERR") == 0)
					entry.flags |= GLOB_ERR;
				else if (strcmp(cp, "GLOB_MARK") == 0)
					entry.flags |= GLOB_MARK;
				else if (strcmp(cp, "GLOB_NOCHECK") == 0)
					entry.flags |= GLOB_NOCHECK;
				else if (strcmp(cp, "GLOB_NOSORT") == 0)
					entry.flags |= GLOB_NOSORT;
				else if (strcmp(cp, "GLOB_NOESCAPE") == 0)
					entry.flags |= GLOB_NOESCAPE;
				else if (strcmp(cp, "GLOB_BRACE") == 0)
					entry.flags |= GLOB_BRACE;
				else if (strcmp(cp, "GLOB_TILDE") == 0)
					entry.flags |= GLOB_TILDE;
				else if (strcmp(cp, "NONE") != 0) {
					fprintf(stderr,
					    "globtest: invalid flags on line %d\n",
					    lineno);
					return EXIT_FAILURE;
				}
			}
			entry.nresults = 0;
			continue;
		}
		if (!entry.pattern[0]) {
			fprintf(stderr, "globtest: missing entry on line %d\n",
			    lineno);
			return EXIT_FAILURE;
		}

		if (entry.nresults + 1 > MAX_RESULTS) {
			fprintf(stderr,
			    "globtest: too many results for %s, max %d\n",
			    entry.pattern, MAX_RESULTS);
			return EXIT_FAILURE;
		}
		entry.results[entry.nresults++] = strdup(buf);
	}
	if (entry.pattern[0]) {
		errors += test_glob(&entry); /* test last pattern */
		ntests++;
	}
        if (ntests != 0) {
		printf("glob: %d test%s run, %d errors, %d%% success rate\n",
		    ntests, ntests == 1 ? "" : "s", errors,
		    (ntests - errors) * 100 / ntests);
        }
	return errors;
}

static int
test_glob(struct gl_entry *entry)
{
	glob_t gl;
	char **ap;
	int nmatches = 0, i = 0;

	if (glob(entry->pattern, entry->flags, NULL, &gl) != 0) {
		fprintf(stderr, "glob failed: %s: %s\n", entry->pattern,
		    strerror(errno));
		return 1;
	}

	for (ap = gl.gl_pathv; *ap != NULL; ap++)
		nmatches++;

	if (nmatches != entry->nresults)
		goto mismatch;

	for (i = 0; i < entry->nresults; i++) {
		if (strcmp(gl.gl_pathv[i], entry->results[i]) != 0)
			goto mismatch;
		free(entry->results[i]);
	}
	return 0;
 mismatch:
	if (nmatches != entry->nresults) {
		fprintf(stderr,
		    "globtest: mismatch in number of results (found %d, expected %d) for pattern %s\n",
		    nmatches, entry->nresults, entry->pattern);
	} else {
		fprintf(stderr, "globtest: mismatch for pattern %s, flags 0x%x "
		    "(found \"%s\", expected \"%s\")\n", entry->pattern, entry->flags,
		    gl.gl_pathv[i], entry->results[i]);
		while (i < entry->nresults)
			free(entry->results[i++]);
	}
	return 1;
}
