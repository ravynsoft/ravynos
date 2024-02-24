#ifndef MICROATF_ATF_C_H_
#define MICROATF_ATF_C_H_

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**/

#ifdef __cplusplus
#define MICROATF_ALIGNAS alignas
#else
#define MICROATF_ALIGNAS _Alignas
#endif
#define MICROATF_ATTRIBUTE_UNUSED __attribute__((__unused__))
#define MICROATF_ATTRIBUTE_FORMAT_PRINTF(a, b) \
	__attribute__((__format__(__printf__, a, b)))
#define MICROATF_ATTRIBUTE_NORETURN __attribute__((__noreturn__))

/**/

struct atf_error;
typedef struct atf_error *atf_error_t;

/**/

struct atf_tc_impl_s_;
struct atf_tc_s;
typedef struct atf_tc_s atf_tc_t;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wpedantic"
struct atf_tc_s {
	MICROATF_ALIGNAS(8192) char const *name;
	void (*head)(atf_tc_t *);
	void (*body)(atf_tc_t const *);
	struct atf_tc_impl_s_ *impl_;
	MICROATF_ALIGNAS(max_align_t) unsigned char impl_space_[];
};
#pragma GCC diagnostic pop

atf_error_t atf_tc_set_md_var(atf_tc_t *tc, /**/
    char const *key, char const *value, ...);

const char *atf_tc_get_md_var(atf_tc_t const *tc, const char *key);
const char *atf_tc_get_config_var(atf_tc_t const *tc, const char *key);

/**/

struct atf_tp_s;
typedef struct atf_tp_s atf_tp_t;

atf_error_t microatf_tp_add_tc(atf_tp_t *tp, atf_tc_t *tc);

/**/

MICROATF_ATTRIBUTE_FORMAT_PRINTF(1, 2)
void microatf_fail_check(char const *msg, ...);

MICROATF_ATTRIBUTE_NORETURN
MICROATF_ATTRIBUTE_FORMAT_PRINTF(1, 2)
void microatf_fail_require(char const *msg, ...);

/**/

MICROATF_ATTRIBUTE_FORMAT_PRINTF(1, 2)
void atf_tc_expect_timeout(const char *msg, ...);

MICROATF_ATTRIBUTE_FORMAT_PRINTF(2, 3)
void atf_tc_expect_exit(const int exitcode, const char *msg, ...);

MICROATF_ATTRIBUTE_FORMAT_PRINTF(2, 3)
void atf_tc_expect_signal(const int signal, const char *msg, ...);

MICROATF_ATTRIBUTE_FORMAT_PRINTF(1, 2)
void atf_tc_expect_fail(const char *msg, ...);

/**/

MICROATF_ATTRIBUTE_NORETURN
MICROATF_ATTRIBUTE_FORMAT_PRINTF(1, 2)
void atf_tc_skip(const char *reason, ...);

/**/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define ATF_REQUIRE_MSG(expression, fmt, ...)                     \
	do {                                                      \
		if (!(expression)) {                              \
			microatf_fail_require("%s:%d: " fmt, /**/ \
			    __FILE__, __LINE__, ##__VA_ARGS__);   \
		}                                                 \
	} while (0)

#define ATF_CHECK_MSG(expression, fmt, ...)                     \
	do {                                                    \
		if (!(expression)) {                            \
			microatf_fail_check("%s:%d: " fmt, /**/ \
			    __FILE__, __LINE__, ##__VA_ARGS__); \
		}                                               \
	} while (0)

#pragma clang diagnostic pop
#pragma GCC diagnostic pop

#define ATF_REQUIRE(expression) \
	ATF_REQUIRE_MSG((expression), "%s not met", #expression)

#define ATF_CHECK(expression) \
	ATF_CHECK_MSG((expression), "%s not met", #expression)

#define ATF_REQUIRE_EQ(expected, actual) \
	ATF_REQUIRE_MSG((expected) == (actual), "%s != %s", #expected, #actual)

#define ATF_CHECK_EQ(expected, actual) \
	ATF_CHECK_MSG((expected) == (actual), "%s != %s", #expected, #actual)

#define ATF_REQUIRE_ERRNO(exp_errno, bool_expr)              \
	do {                                                 \
		ATF_REQUIRE_MSG((bool_expr),	 /**/        \
		    "Expected true value in %s", /**/        \
		    #bool_expr);                             \
		int ec = errno;                              \
		ATF_REQUIRE_MSG(ec == (exp_errno),	/**/ \
		    "Expected errno %d, got %d, in %s", /**/ \
		    (exp_errno), ec, #bool_expr);            \
	} while (0)

#define ATF_TC_WITHOUT_HEAD(tc)                                \
	static void microatf_tc_##tc##_body(atf_tc_t const *); \
	static atf_tc_t microatf_tc_##tc = {                   \
		#tc,                                           \
		NULL,                                          \
		microatf_tc_##tc##_body,                       \
		NULL,                                          \
	}

#define ATF_TC(tc)                                             \
	static void microatf_tc_##tc##_head(atf_tc_t *);       \
	static void microatf_tc_##tc##_body(atf_tc_t const *); \
	static atf_tc_t microatf_tc_##tc = {                   \
		#tc,                                           \
		microatf_tc_##tc##_head,                       \
		microatf_tc_##tc##_body,                       \
		NULL,                                          \
	}

#define ATF_TC_HEAD(tc, tcptr)               \
	static void microatf_tc_##tc##_head( \
	    atf_tc_t *tcptr MICROATF_ATTRIBUTE_UNUSED)

#define ATF_TC_BODY(tc, tcptr)               \
	static void microatf_tc_##tc##_body( \
	    atf_tc_t const *tcptr MICROATF_ATTRIBUTE_UNUSED)

#define ATF_TP_ADD_TCS(tps)                                               \
	static atf_error_t microatf_tp_add_tcs(atf_tp_t *);               \
	int main(int argc, char **argv)                                   \
	{                                                                 \
		return microatf_tp_main(argc, argv, microatf_tp_add_tcs); \
	}                                                                 \
	static atf_error_t microatf_tp_add_tcs(atf_tp_t *tps)

#define ATF_TP_ADD_TC(tp, tc)                                               \
	do {                                                                \
		atf_error_t ec = microatf_tp_add_tc(tp, &microatf_tc_##tc); \
		if (atf_is_error(ec)) {                                     \
			return ec;                                          \
		}                                                           \
	} while (0)

int microatf_tp_main(int argc, char **argv,
    atf_error_t (*add_tcs_hook)(atf_tp_t *));

atf_error_t atf_no_error(void);
bool atf_is_error(atf_error_t const);

#ifdef __cplusplus
}
#endif

#endif
