#include <string>
#include <sstream>
#include <iostream>

using namespace std;

#define	NELEMENTS(a)	(sizeof (a)/sizeof *(a))

#define	NO_RESULT	(-1)
#define	PASS		1
#define	FAIL		0

#define	DBG	bhole

void
bhole(...)
{
}

class line_category
{
public:
	const char *key;
	char line[99999];
	void (*test)(line_category &);
	int test_result;
} lc;

void
deft(line_category &l)
{
	l.test_result = PASS;
}

void
pass_fail(line_category &l)
{
	if(FAIL!=l.test_result)
		l.test_result = strnstr(l.line, "FAIL", 4)? FAIL: PASS;
}

void
stderr_output(line_category &l)
{
	if(FAIL==l.test_result)
		return;

	if(l.line[0])
		l.test_result = FAIL;
}

void
exit_test(line_category &l)
{
	if(!atoi(l.line)) {
		DBG("exit_test(%s)==%d\n", l.line, atoi(l.line));
		l.test_result = PASS;
	}
}

#define	STDOUT	"stdout: "
#define	STDERR	"stderr: "
#define	CWD	"cwd: "
#define	CMD	"cmd: "
#define	SEXIT	"exit: "
line_category line_categories[] = {
	{ CWD, "" , deft, NO_RESULT},	/* must be first */
	{ CMD, "", deft, NO_RESULT},
	{ STDOUT, "", pass_fail, NO_RESULT},
	{ STDERR, "", stderr_output, NO_RESULT },
	{ SEXIT, "", exit_test, NO_RESULT },
};

static line_category no_line_category = { "none", "no test", deft, NO_RESULT };

line_category &
retrieve(line_category &l, const char *s)
{
	unsigned j;
	line_category *lp = &l;
	//int final_result = PASS;

	for(j=0; j<NELEMENTS(line_categories); ++j,++lp) {//TODO: remove NELEMENTS
		if(!strcmp(lp->key, s)) {
			char *p;

			for(p=(char *)lp->line; *p; ++p) {
				switch(*p) {
				case '\0':
					break;
				case '"':
				case '<':
					*p = ' ';
					// fall thru
				default:
					continue;
				}
			}
DBG("FOUND line_categories[j].line==%s\n", lp->line);
			return line_categories[j];
		}
	}

	return no_line_category;
}

void
xml_string_print(FILE *strm, const char *s)
{
	fputc('"', strm);
	for( ; ; ++s) {
		switch(*s) {
		case '\0':
			break;
		case '&':
			fputs("&amp;", strm);
			continue;
		default:
			fputc(*s, strm);
			continue;
		}
		break;
	}
	fputc('"', strm);
}

//	
//	FAIL if stderr non-zero
//	FAIL if stdout=="FAIL"
//	UNRESOLVED if make exit non-zero
//	PASS otherwise
//	

static int cnt;
void
dump_test(void)
{
	unsigned j;
	int final_result = PASS;

	for(j=0; j<NELEMENTS(line_categories); ++j) {
		if(line_categories[j].line[0]) {
			line_categories[j].line[strlen(line_categories[j].line)-1] = '\0';
			DBG("%s%s RESULT %d\n"
				, line_categories[j].key
				, line_categories[j].line
				, line_categories[j].test_result
				);

			if(PASS==final_result) {
				final_result = line_categories[j].test_result;
				if(NO_RESULT==line_categories[j].test_result) {
					final_result = NO_RESULT;
				} else if(FAIL==line_categories[j].test_result) {
					final_result = FAIL;
				}
			}
		}
	}

	printf("<test name=");
	xml_string_print(stdout, retrieve(line_categories[0], CMD).line);
	printf(" result=\"");
	if(NO_RESULT==final_result) {
		printf("UNRESOLVED");
	} else if(FAIL==final_result) {
		printf("FAIL");
	} else {
		printf("PASS");
	}
	printf("\" ");

	char *s = retrieve(line_categories[0], CWD).line;
	if(*s) {
		char detail[9999];
		char fn[9999];
		FILE *strm;

		strncpy(fn, s, sizeof fn);
		strncat(fn, "/comment.txt", sizeof fn);
		strm = fopen(fn, "r");
		if(strm) {
			if(fgets(detail, -1+sizeof detail, strm)) {
				detail[strlen(detail)-1] = '\0';
				printf("detail=");
				xml_string_print(stdout, detail);
			}
		}
	}
	printf(">\n");

	printf("    <diagnostics>\n");
	s = retrieve(line_categories[0], STDERR).line;
	if(*s) {
		printf("      <diagnostic line=\"%d\" message=", ++cnt);
		xml_string_print(stdout, s);
		printf(" severity=\"ERROR\"/>\n");
	}
#if 1
	s = retrieve(line_categories[0], STDOUT).line;
	if(*s) {
		printf("      <diagnostic line=\"%d\" message=", ++cnt);
		xml_string_print(stdout, s);
		printf(" severity=\"note\"/>\n");
	}
#endif
	printf("    </diagnostics>\n");
	printf("</test>\n\n");

	for(j=0; j<NELEMENTS(line_categories); ++j) {
		line_categories[j].line[0] = '\0';
		line_categories[j].test_result = NO_RESULT;
	}
}

int
main(int argc, char **argv)
{
	int firsttime = 1;


	if(argc>1)
		cnt = atoi(argv[1]);

	for(;;) {
		char line[99999];
		int i;

		line[0] = '\0';
		fgets(line, sizeof line, stdin);
		if(feof(stdin)) {
			dump_test();
			break;
		}

		for(i=0; ; ++i) {
			size_t len = strlen(line_categories[i].key);

			//DBG("strnstr(%s, %s, %u)\n", line, line_categories[i].key, len);
			if(strnstr(line, line_categories[i].key, len)) {
				if(firsttime)
					firsttime = 0;
				else if(0==i)
					dump_test();

				char *lp = &line[len];
				//DBG("%s%s", line_categories[i].key, lp);
				strncpy(line_categories[i].line, lp, sizeof line_categories[i].line);
				line_categories[i].test(line_categories[i]);
				break;
			}

			if(i==NELEMENTS(line_categories)-1) {
				DBG("BADLINE:%s", line);
				break;
			}
		}
	}
	return 0;
}
