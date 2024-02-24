#include "atf-c.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC O_NOINHERIT
#endif

/* Small list implementation to avoid dependency to <sys/queue.h> */

#define STAILQ_ENTRY(s)         \
	struct {                \
		struct s *next; \
	}
#define STAILQ_HEAD(s, e)        \
	struct s {               \
		struct e *list;  \
		struct e **last; \
	}
#define STAILQ_HEAD_INITIALIZER(h)              \
	{                                       \
		.list = NULL, .last = &(h).list \
	}
#define STAILQ_INSERT_TAIL(h, e, n)         \
	do {                                \
		(e)->n.next = NULL;         \
		*((h)->last) = (e);         \
		(h)->last = &((e)->n.next); \
	} while (0)
#define STAILQ_FOREACH(e, h, n) /**/ \
	for ((e) = (h)->list; (e); (e) = (e)->n.next)

/* Internal error codes. */

#define MICROATF_ERROR_CODES(_, ...)                            \
	_(microatf_error_argument_parsing, 1, __VA_ARGS__)      \
	_(microatf_error_no_matching_test_case, 2, __VA_ARGS__) \
	_(microatf_error_result_file, 3, __VA_ARGS__)           \
	_(microatf_error_too_many_variables, 4, __VA_ARGS__)    \
	_(microatf_error_initialization_error, 5, __VA_ARGS__)

#define MICROATF_DEFINE_ERROR_CODE(ec, val, ...) \
	static inline atf_error_t ec(void) { return (void *)(uintptr_t)val; }

MICROATF_ERROR_CODES(MICROATF_DEFINE_ERROR_CODE)

enum microatf_expect_type {
	MICROATF_EXPECT_PASS,
	MICROATF_EXPECT_FAIL,
	MICROATF_EXPECT_EXIT,
	MICROATF_EXPECT_SIGNAL,
	MICROATF_EXPECT_DEATH,
	MICROATF_EXPECT_TIMEOUT,
};

typedef struct {
	char const *result_file_path;
	int result_file;
	bool do_close_result_file;
	atf_tc_t *test_case;

	enum microatf_expect_type expect;

	int fail_count;
	int expect_fail_count;
	int expect_previous_fail_count;
} microatf_context_t;

static microatf_context_t microatf_context_static;
static microatf_context_t *microatf_context = &microatf_context_static;

struct atf_tc_impl_s_ {
	char const *variables_key[128];
	char const *variables_value[128];
	size_t variables_size;
	char const *config_variables_key[128];
	char const *config_variables_value[128];
	size_t config_variables_size;
	STAILQ_ENTRY(atf_tc_s) entries;
};

atf_error_t
atf_tc_set_md_var(atf_tc_t *tc, char const *key, char const *value, ...)
{
	size_t key_length = strlen(key);

	for (size_t i = 0; i < tc->impl_->variables_size; ++i) {
		if (strncmp(tc->impl_->variables_key[i], /**/
			key, key_length) == 0) {
			tc->impl_->variables_value[i] = value;
			return atf_no_error();
		}
	}

	if (tc->impl_->variables_size == 128) {
		return microatf_error_too_many_variables();
	}

	tc->impl_->variables_key[tc->impl_->variables_size] = key;
	tc->impl_->variables_value[tc->impl_->variables_size] = value;

	++tc->impl_->variables_size;

	return atf_no_error();
}

const char *
atf_tc_get_md_var(atf_tc_t const *tc, const char *key)
{
	size_t key_length = strlen(key);

	for (size_t i = 0; i < tc->impl_->variables_size; ++i) {
		if (strncmp(tc->impl_->variables_key[i], /**/
			key, key_length) == 0) {
			return tc->impl_->variables_value[i];
		}
	}

	return NULL;
}

const char *
atf_tc_get_config_var(atf_tc_t const *tc, const char *key)
{
	size_t key_length = strlen(key);

	for (size_t i = 0; i < tc->impl_->config_variables_size; ++i) {
		if (strncmp(tc->impl_->config_variables_key[i], /**/
			key, key_length) == 0) {
			return tc->impl_->config_variables_value[i];
		}
	}

	return NULL;
}

/**/

STAILQ_HEAD(atf_tc_list_s, atf_tc_s);
typedef struct atf_tc_list_s atf_tc_list_t;

struct atf_tp_s {
	atf_tc_list_t tcs;
};

static void
atf_tp_init(atf_tp_t *tp)
{
	*tp = (atf_tp_t) { .tcs = STAILQ_HEAD_INITIALIZER(tp->tcs) };
}

atf_error_t
microatf_tp_add_tc(atf_tp_t *tp, atf_tc_t *tst)
{
	char const *ident = tst->name;

	tst->impl_ = (struct atf_tc_impl_s_ *)&tst->impl_space_;

	atf_tc_set_md_var(tst, "ident", ident);
	if (tst->head != NULL) {
		tst->head(tst);
	}

	char const *new_ident = atf_tc_get_md_var(tst, "ident");
	if (new_ident == NULL || strcmp(new_ident, ident) != 0) {
		return microatf_error_initialization_error();
	}

	STAILQ_INSERT_TAIL(&tp->tcs, tst, impl_->entries);
	return 0;
}

MICROATF_ATTRIBUTE_FORMAT_PRINTF(4, 0)
static void
microatf_context_write_result_pack(microatf_context_t *context,
    char const *result, const int arg, char const *reason, va_list args)
{
	if (context->result_file < 0) {
		return;
	}

	if (context->do_close_result_file) {
		if (ftruncate(context->result_file, 0) < 0) {
			return;
		}
	}

	char line[1024];
	int line_size = 0;
	int r;

#define APP_IMPL(print_fun, ...)                                           \
	do {                                                               \
		r = print_fun(line + line_size, /**/                       \
		    (size_t)((int)sizeof(line) - line_size), __VA_ARGS__); \
		if (r < 0 || r >= ((int)sizeof(line) - line_size)) {       \
			return;                                            \
		}                                                          \
                                                                           \
		line_size += r;                                            \
	} while (0);

#define APP(...) APP_IMPL(snprintf, __VA_ARGS__)
#define VAPP(...) APP_IMPL(vsnprintf, __VA_ARGS__)

	APP("%s", result);

	if (arg != -1) {
		APP("(%d)", arg);
	}

	if (reason) {
		APP(": ");
		VAPP(reason, args);
	}

	APP("\n");

#undef VAPP
#undef APP

	(void)write(context->result_file, line, (size_t)line_size);
}

MICROATF_ATTRIBUTE_FORMAT_PRINTF(4, 5)
static void
microatf_context_write_result(microatf_context_t *context, char const *result,
    const int arg, char const *reason, ...)
{
	va_list args;
	va_start(args, reason);
	microatf_context_write_result_pack(context, result, arg, reason, args);
	va_end(args);
}

MICROATF_ATTRIBUTE_NORETURN
static void
microatf_context_exit(microatf_context_t *context, int exit_code)
{
	if (context->do_close_result_file && context->result_file >= 0) {
		close(context->result_file);
		context->result_file = -1;
	}

	exit(exit_code);
}

static void
microatf_context_validate_expect(microatf_context_t *context)
{
	if (context->expect == MICROATF_EXPECT_FAIL) {
		if (context->expect_fail_count ==
		    context->expect_previous_fail_count) {
			microatf_context_write_result(context, "failed", -1,
			    "Test case was expecting a failure but none were "
			    "raised");
			microatf_context_exit(context, EXIT_FAILURE);
		}
	} else if (context->expect) {
		microatf_context_write_result(context, "failed", -1,
		    "Test case continued");
		microatf_context_exit(context, EXIT_FAILURE);
	}
}

MICROATF_ATTRIBUTE_NORETURN
static void
microatf_context_pass(microatf_context_t *context)
{
	if (context->expect == MICROATF_EXPECT_FAIL) {
		microatf_context_write_result(context, "failed", -1,
		    "Test case was expecting a failure but got a pass "
		    "instead");
		microatf_context_exit(context, EXIT_FAILURE);
	} else if (context->expect == MICROATF_EXPECT_PASS) {
		microatf_context_write_result(microatf_context, "passed", -1,
		    NULL);
		microatf_context_exit(microatf_context, EXIT_SUCCESS);
	} else {
		microatf_context_write_result(context, "failed", -1,
		    "Test case asked to explicitly pass but was not expecting "
		    "such condition");
		microatf_context_exit(context, EXIT_FAILURE);
	}
}

void
microatf_fail_check(char const *msg, ...)
{
	microatf_context_t *context = microatf_context;

	if (context->expect == MICROATF_EXPECT_FAIL) {
		fprintf(stderr, "*** Expected check failure: ");

		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);

		fprintf(stderr, "\n");

		context->expect_fail_count++;
	} else if (context->expect == MICROATF_EXPECT_PASS) {
		fprintf(stderr, "*** Check failed: ");

		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);

		fprintf(stderr, "\n");

		context->fail_count++;
	} else {
		va_list args;
		va_start(args, msg);
		char const *file = va_arg(args, char const *);
		int line = va_arg(args, int);
		microatf_context_write_result(context, "failed", -1,
		    "%s:%d: %s", file, line,
		    "Test case raised a failure but was not expecting one");
		va_end(args);

		microatf_context_exit(context, EXIT_FAILURE);
	}
}

void
microatf_fail_require(char const *msg, ...)
{
	microatf_context_t *context = microatf_context;

	if (context->expect == MICROATF_EXPECT_FAIL) {
		va_list args;
		va_start(args, msg);
		microatf_context_write_result_pack(context, /**/
		    "expected_failure", -1, msg, args);
		va_end(args);

		microatf_context_exit(context, EXIT_SUCCESS);
	} else if (context->expect == MICROATF_EXPECT_PASS) {
		va_list args;
		va_start(args, msg);
		microatf_context_write_result_pack(context, /**/
		    "failed", -1, msg, args);
		va_end(args);

		microatf_context_exit(context, EXIT_FAILURE);
	} else {
		va_list args;
		va_start(args, msg);
		char const *file = va_arg(args, char const *);
		int line = va_arg(args, int);
		microatf_context_write_result(context, "failed", -1,
		    "%s:%d: %s", file, line,
		    "Test case raised a failure but was not expecting one");
		va_end(args);

		microatf_context_exit(context, EXIT_FAILURE);
	}
}

void
atf_tc_expect_timeout(const char *msg, ...)
{
	microatf_context_validate_expect(microatf_context);
	microatf_context->expect = MICROATF_EXPECT_TIMEOUT;

	va_list args;
	va_start(args, msg);
	microatf_context_write_result_pack(microatf_context, "expected_timeout",
	    -1, msg, args);
	va_end(args);
}

void
atf_tc_expect_exit(const int exitcode, const char *msg, ...)
{
	microatf_context_validate_expect(microatf_context);
	microatf_context->expect = MICROATF_EXPECT_EXIT;

	va_list args;
	va_start(args, msg);
	microatf_context_write_result_pack(microatf_context, /**/
	    "expected_exit", exitcode, msg, args);
	va_end(args);
}

void
atf_tc_expect_signal(const int signal, const char *msg, ...)
{
	microatf_context_validate_expect(microatf_context);
	microatf_context->expect = MICROATF_EXPECT_SIGNAL;

	va_list args;
	va_start(args, msg);
	microatf_context_write_result_pack(microatf_context, /**/
	    "expected_signal", signal, msg, args);
	va_end(args);
}

void
atf_tc_expect_fail(const char *msg, ...)
{
	(void)msg;

	microatf_context_validate_expect(microatf_context);
	microatf_context->expect = MICROATF_EXPECT_FAIL;

	microatf_context->expect_previous_fail_count =
	    microatf_context->expect_fail_count;
}

void
atf_tc_skip(const char *reason, ...)
{
	microatf_context_t *context = microatf_context;

	if (context->expect == MICROATF_EXPECT_PASS) {
		va_list args;
		va_start(args, reason);
		microatf_context_write_result_pack(context, /**/
		    "skipped", -1, reason, args);
		va_end(args);

		microatf_context_exit(context, EXIT_SUCCESS);
	} else {
		microatf_context_write_result(context, "failed", -1,
		    "Can only skip a test case when running in "
		    "expect pass mode");
		microatf_context_exit(context, EXIT_FAILURE);
	}
}

int
microatf_tp_main(int argc, char **argv, atf_error_t (*add_tcs_hook)(atf_tp_t *))
{
	atf_error_t ec;

	bool list_tests = false;
	char const *test_case_name = NULL;
	char const *result_file_path = NULL;
	char const *srcdir_path = NULL;
	char const *config_variables[128];
	size_t config_variables_size = 0;

	int ch;
	while ((ch = getopt(argc, argv, "lr:s:v:")) != -1) {
		switch (ch) {
		case 'l':
			list_tests = true;
			break;
		case 'r':
			result_file_path = optarg;
			break;
		case 's':
			srcdir_path = optarg;
			break;
		case 'v':
			if (config_variables_size == 128) {
				ec = microatf_error_too_many_variables();
				goto out;
			}
			config_variables[config_variables_size] = optarg;
			++config_variables_size;
			break;
		case '?':
		default:
			ec = microatf_error_argument_parsing();
			goto out;
		}
	}
	argc -= optind;
	argv += optind;

	/* TODO: What to do with this? */
	(void)srcdir_path;

	if (list_tests) {
		if (argc != 0) {
			ec = microatf_error_argument_parsing();
			goto out;
		}
	} else {
		if (argc != 1) {
			ec = microatf_error_argument_parsing();
			goto out;
		}
		test_case_name = argv[0];
	}

	atf_tp_t tp;
	atf_tp_init(&tp);

	ec = add_tcs_hook(&tp);
	if (ec) {
		goto out;
	}

	if (list_tests) {
		printf("Content-Type: application/X-atf-tp; version=\"1\"\n\n");

		bool print_newline = false;

		atf_tc_t *tc;
		STAILQ_FOREACH (tc, &tp.tcs, impl_->entries) {
			if (print_newline) {
				printf("\n");
			}
			print_newline = true;

			for (size_t i = 0; i < tc->impl_->variables_size; ++i) {
				char const *key_end =
				    strchr(tc->impl_->variables_key[i], '=');
				ptrdiff_t key_length = key_end ?
					  (key_end - tc->impl_->variables_key[i]) :
					  (ptrdiff_t)strlen(
					tc->impl_->variables_key[i]);
				printf("%.*s: %s\n", (int)key_length,
				    tc->impl_->variables_key[i],
				    tc->impl_->variables_value[i]);
			}
		}

		return 0;
	}

	atf_tc_t *matching_tc = NULL;

	atf_tc_t *tc;
	STAILQ_FOREACH (tc, &tp.tcs, impl_->entries) {
		if (strcmp(tc->name, test_case_name) == 0) {
			matching_tc = tc;
			break;
		}
	}

	if (!matching_tc) {
		ec = microatf_error_no_matching_test_case();
		goto out;
	}

	bool do_close_result_file = false;
	int result_file;

	if (!result_file_path) {
		result_file_path = "/dev/stdout";
	}

	if (strcmp(result_file_path, "/dev/stdout") == 0) {
		result_file = STDOUT_FILENO;
	} else if (strcmp(result_file_path, "/dev/stderr") == 0) {
		result_file = STDERR_FILENO;
	} else {
		do_close_result_file = true;

		result_file = open(result_file_path,
		    O_WRONLY | O_CREAT | O_TRUNC | O_APPEND | O_CLOEXEC, 0666);
		if (result_file < 0) {
			ec = microatf_error_result_file();
			goto out;
		}
	}

	/* Run the test case. */

	microatf_context_static = (microatf_context_t) {
		.result_file_path = result_file_path,
		.result_file = result_file,
		.do_close_result_file = do_close_result_file,
		.test_case = matching_tc,
	};

	for (size_t i = 0; i < config_variables_size; ++i) {
		matching_tc->impl_->config_variables_key[i] =
		    config_variables[i];
		matching_tc->impl_->config_variables_value[i] =
		    strchr(config_variables[i], '=');
		if (!matching_tc->impl_->config_variables_value[i]) {
			ec = microatf_error_argument_parsing();
			goto out;
		}
		++matching_tc->impl_->config_variables_value[i];
	}
	matching_tc->impl_->config_variables_size = config_variables_size;

	matching_tc->body(matching_tc);

	/**/

	microatf_context_validate_expect(microatf_context);

	if (microatf_context->fail_count > 0) {
		microatf_context_write_result(microatf_context, /**/
		    "failed", -1, "Some checks failed");
		microatf_context_exit(microatf_context, EXIT_FAILURE);
	} else if (microatf_context->expect_fail_count > 0) {
		microatf_context_write_result(microatf_context,
		    "expected_failure", -1, "Some checks failed as expected");
		microatf_context_exit(microatf_context, EXIT_SUCCESS);
	} else {
		microatf_context_pass(microatf_context);
	}

out:
	return ec ? 1 : 0;
}

atf_error_t
atf_no_error(void)
{
	return NULL;
}

bool
atf_is_error(atf_error_t const ec)
{
	return ec != atf_no_error();
}
