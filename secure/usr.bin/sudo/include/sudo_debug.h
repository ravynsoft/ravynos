/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2017 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SUDO_DEBUG_H
#define SUDO_DEBUG_H

#include <sys/types.h>		/* for id_t, mode_t, size_t, ssize_t, time_t */
#include <stdarg.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <sudo_queue.h>

/*
 * List of debug files and flags for use in registration.
 */
struct sudo_debug_file {
    TAILQ_ENTRY(sudo_debug_file) entries;
    char *debug_file;
    char *debug_flags;
};
struct sudo_conf_debug_file_list;

/*
 * The priority and subsystem are encoded in a single 32-bit value.
 * The lower 4 bits are the priority and the top 26 bits are the subsystem.
 * This allows for 16 priorities and a very large number of subsystems.
 * Bit 5 is used as a flag to specify whether to log the errno value.
 * Bit 6 specifies whether to log the function, file and line number data.
 */

/*
 * Sudo debug priorities, ordered least to most verbose,
 * in other words, highest to lowest priority.  Max pri is 15.
 * Note: order must match sudo_debug_priorities[]
 */
#define SUDO_DEBUG_CRIT		1	/* critical errors */
#define SUDO_DEBUG_ERROR	2	/* non-critical errors */
#define SUDO_DEBUG_WARN		3	/* non-fatal warnings */
#define SUDO_DEBUG_NOTICE	4	/* non-error condition notices */
#define SUDO_DEBUG_DIAG		5	/* diagnostic messages */
#define SUDO_DEBUG_INFO		6	/* informational message */
#define SUDO_DEBUG_TRACE	7	/* log function enter/exit */
#define SUDO_DEBUG_DEBUG	8	/* very verbose debugging */

/* Flag to include string version of errno in debug info. */
#define SUDO_DEBUG_ERRNO	(1<<4)

/* Flag to include function, file and line number in debug info. */
#define SUDO_DEBUG_LINENO	(1<<5)

/*
 * Sudo debug subsystems.
 * This includes subsystems in the sudoers plugin.
 * Note: order must match sudo_debug_subsystems[]
 */
#define SUDO_DEBUG_ARGS		( 1<<6)    /* command line argument handling */
#define SUDO_DEBUG_CONV		( 2<<6)    /* user conversation */
#define SUDO_DEBUG_EDIT		( 3<<6)    /* sudoedit */
#define SUDO_DEBUG_EVENT	( 4<<6)    /* event handling */
#define SUDO_DEBUG_EXEC		( 5<<6)    /* command execution */
#define SUDO_DEBUG_HOOKS	( 6<<6)    /* hook functions */
#define SUDO_DEBUG_MAIN		( 7<<6)    /* sudo main() */
#define SUDO_DEBUG_NETIF	( 8<<6)    /* network interface functions */
#define SUDO_DEBUG_PCOMM	( 9<<6)    /* plugin communications */
#define SUDO_DEBUG_PLUGIN	(10<<6)    /* main plugin functions */
#define SUDO_DEBUG_PTY		(11<<6)    /* pseudo-tty */
#define SUDO_DEBUG_SELINUX	(12<<6)    /* selinux */
#define SUDO_DEBUG_UTIL		(13<<6)    /* utility functions */
#define SUDO_DEBUG_UTMP		(14<<6)    /* utmp file ops */
#define SUDO_DEBUG_APPARMOR     (15<<6)    /* AppArmor */
#define SUDO_DEBUG_ALL		0xffff0000 /* all subsystems */

/* Error return for sudo_debug_register().  */
#define SUDO_DEBUG_INSTANCE_ERROR	-2

/* Initializer for instance index to indicate that debugging is not setup. */
#define SUDO_DEBUG_INSTANCE_INITIALIZER	-1

/* Extract priority number and convert to an index. */
#define SUDO_DEBUG_PRI(n) (((n) & 0x0f) - 1)

/* Extract subsystem number and convert to an index. */
#define SUDO_DEBUG_SUBSYS(n) (((n) >> 6) - 1)

/*
 * Wrapper for sudo_debug_enter() that declares __func__ as needed
 * and sets sudo_debug_subsys for sudo_debug_exit().
 */
#ifdef HAVE___FUNC__
# define debug_decl_func(funcname)
# define debug_decl_vars(funcname, subsys)				       \
    const unsigned int sudo_debug_subsys = (subsys)
#else
# define debug_decl_func(funcname)					       \
    const char __func__[] = #funcname;
# define debug_decl_vars(funcname, subsys)				       \
    debug_decl_func(funcname)						       \
    const unsigned int sudo_debug_subsys = (subsys)
#endif
#define debug_decl(funcname, subsys)					       \
    debug_decl_vars((funcname), (subsys));				       \
    sudo_debug_enter(__func__, __FILE__, __LINE__, sudo_debug_subsys)

/*
 * Different flavors of sudo_debug_exit() macros.
 */
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
# define sudo_debug_enter(_func, _file, _line, _sys)			       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "-> %s @ %s:%d", (_func), (_file), (_line));		       \
    } while (0)

# define sudo_debug_exit(_func, _file, _line, _sys)			       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d", (_func), (_file), (_line));		       \
    } while (0)

# define sudo_debug_exit_int(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %d", (_func), (_file), (_line), (_ret));	       \
    } while (0)

# define sudo_debug_exit_uint(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %u", (_func), (_file), (_line), (_ret));	       \
    } while (0)

# define sudo_debug_exit_long(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %ld", (_func), (_file), (_line), (_ret));	       \
    } while (0)

# if SIZEOF_ID_T == 8
#  define sudo_debug_exit_id_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %lld", (_func), (_file), (_line), (long long)(_ret));\
    } while (0)
# else
#  define sudo_debug_exit_id_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %d", (_func), (_file), (_line), (int)(_ret));    \
    } while (0)
# endif

# define sudo_debug_exit_size_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %zu", (_func), (_file), (_line), (_ret));	       \
    } while (0)

# define sudo_debug_exit_ssize_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %zd", (_func), (_file), (_line), (_ret));	       \
    } while (0)

# if SIZEOF_TIME_T == 8
#  define sudo_debug_exit_time_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %lld", (_func), (_file), (_line), (long long)(_ret));\
    } while (0)
# else
#  define sudo_debug_exit_time_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %d", (_func), (_file), (_line), (int)(_ret));    \
    } while (0)
# endif

# define sudo_debug_exit_mode_t(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %d", (_func), (_file), (_line), (int)(_ret));    \
    } while (0)

# define sudo_debug_exit_bool(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %s", (_func), (_file), (_line), (_ret) ? "true": "false");\
    } while (0)

# define sudo_debug_exit_str(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %s", (_func), (_file), (_line), (_ret) ? (_ret) : "(null)");\
    } while (0)

# define sudo_debug_exit_str_masked(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	const char _stars[] = "********************************************************************************"; \
	const size_t _len = (_ret) ? strlen(_ret) : sizeof("(null)") - 1;      \
	const char *_s = (_ret) ? _stars : "(null)";			       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %.*s", (_func), (_file), (_line), (int)_len, _s);\
    } while (0)

# define sudo_debug_exit_ptr(_func, _file, _line, _sys, _ret)		       \
    do {								       \
	sudo_debug_printf2(NULL, NULL, 0, (_sys) | SUDO_DEBUG_TRACE,	       \
	    "<- %s @ %s:%d := %p", (_func), (_file), (_line), (_ret));	       \
    } while (0)
#else /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */
# define sudo_debug_enter(_a, _b, _c, _d)		((void)&(_d))
# define sudo_debug_exit(_a, _b, _c, _d)		((void)&(_d))
# define sudo_debug_exit_int(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_uint(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_long(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_id_t(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_size_t(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_ssize_t(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_time_t(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_mode_t(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_bool(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_str(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_str_masked(_a, _b, _c, _d, _e)	((void)&(_d))
# define sudo_debug_exit_ptr(_a, _b, _c, _d, _e)	((void)&(_d))
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */

/*
 * Wrappers for sudo_debug_exit() and friends.
 */
#define debug_return							       \
    do {								       \
	sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);      \
	return;								       \
    } while (0)

#define debug_return_int(ret)						       \
    do {								       \
	int sudo_debug_ret = (ret);					       \
	sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys,   \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_uint(ret)						       \
    do {								       \
	unsigned int sudo_debug_ret = (ret);				       \
	sudo_debug_exit_uint(__func__, __FILE__, __LINE__, sudo_debug_subsys,  \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_id_t(ret)					       \
    do {								       \
	id_t sudo_debug_ret = (ret);				       \
	sudo_debug_exit_id_t(__func__, __FILE__, __LINE__, sudo_debug_subsys,\
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_size_t(ret)					       \
    do {								       \
	size_t sudo_debug_ret = (ret);				       \
	sudo_debug_exit_size_t(__func__, __FILE__, __LINE__, sudo_debug_subsys,\
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_ssize_t(ret)					       \
    do {								       \
	ssize_t sudo_debug_ret = (ret);				       \
	sudo_debug_exit_ssize_t(__func__, __FILE__, __LINE__, sudo_debug_subsys,\
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_time_t(ret)					       \
    do {								       \
	time_t sudo_debug_ret = (ret);				       \
	sudo_debug_exit_time_t(__func__, __FILE__, __LINE__, sudo_debug_subsys,\
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_mode_t(ret)					       \
    do {								       \
	mode_t sudo_debug_ret = (ret);				       \
	sudo_debug_exit_mode_t(__func__, __FILE__, __LINE__, sudo_debug_subsys,\
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_long(ret)						       \
    do {								       \
	long sudo_debug_ret = (ret);					       \
	sudo_debug_exit_long(__func__, __FILE__, __LINE__, sudo_debug_subsys,  \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_bool(ret)						       \
    do {								       \
	bool sudo_debug_ret = (ret);					       \
	sudo_debug_exit_bool(__func__, __FILE__, __LINE__, sudo_debug_subsys,  \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_str(ret)						       \
    do {								       \
	char *sudo_debug_ret = (ret);					       \
	sudo_debug_exit_str(__func__, __FILE__, __LINE__, sudo_debug_subsys,   \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_const_str(ret)					       \
    do {								       \
	const char *sudo_debug_ret = (ret);				       \
	sudo_debug_exit_str(__func__, __FILE__, __LINE__, sudo_debug_subsys,   \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_str_masked(ret)					       \
    do {								       \
	char *sudo_debug_ret = (ret);					       \
	sudo_debug_exit_str_masked(__func__, __FILE__, __LINE__,	       \
	    sudo_debug_subsys, sudo_debug_ret);			       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_ptr(ret)						       \
    do {								       \
	void *sudo_debug_ret = (ret);					       \
	sudo_debug_exit_ptr(__func__, __FILE__, __LINE__, sudo_debug_subsys,   \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

#define debug_return_const_ptr(ret)					       \
    do {								       \
	const void *sudo_debug_ret = (ret);				       \
	sudo_debug_exit_ptr(__func__, __FILE__, __LINE__, sudo_debug_subsys,   \
	    sudo_debug_ret);						       \
	return sudo_debug_ret;						       \
    } while (0)

/*
 * Variadic macros are a C99 feature but GNU cpp has supported
 * a (different) version of them for a long time.
 */
#if defined(NO_VARIADIC_MACROS)
# define sudo_debug_printf sudo_debug_printf_nvm
#elif defined(__GNUC__) && __GNUC__ == 2
# define sudo_debug_printf(pri, fmt...) \
    sudo_debug_printf2(__func__, __FILE__, __LINE__, (pri)|sudo_debug_subsys, \
    fmt)
#else
# define sudo_debug_printf(pri, ...) \
    sudo_debug_printf2(__func__, __FILE__, __LINE__, (pri)|sudo_debug_subsys, \
    __VA_ARGS__)
#endif

#define sudo_debug_execve(pri, path, argv, envp) \
    sudo_debug_execve2((pri)|sudo_debug_subsys, (path), (argv), (envp))

#define sudo_debug_write(fd, str, len, errnum) \
    sudo_debug_write2(fd, NULL, NULL, 0, (str), (len), (errnum))

sudo_dso_public int sudo_debug_deregister_v1(int instance_id);
sudo_dso_public void sudo_debug_enter_v1(const char *func, const char *file, int line, unsigned int subsys);
sudo_dso_public void sudo_debug_execve2_v1(unsigned int level, const char *path, char *const argv[], char *const envp[]);
sudo_dso_public void sudo_debug_exit_v1(const char *func, const char *file, int line, unsigned int subsys);
sudo_dso_public void sudo_debug_exit_bool_v1(const char *func, const char *file, int line, unsigned int subsys, bool ret);
sudo_dso_public void sudo_debug_exit_int_v1(const char *func, const char *file, int line, unsigned int subsys, int ret);
sudo_dso_public void sudo_debug_exit_uint_v1(const char *func, const char *file, int line, unsigned int subsys, unsigned int ret);
sudo_dso_public void sudo_debug_exit_long_v1(const char *func, const char *file, int line, unsigned int subsys, long ret);
sudo_dso_public void sudo_debug_exit_ptr_v1(const char *func, const char *file, int line, unsigned int subsys, const void *ret);
sudo_dso_public void sudo_debug_exit_id_t_v1(const char *func, const char *file, int line, unsigned int subsys, id_t ret);
sudo_dso_public void sudo_debug_exit_size_t_v1(const char *func, const char *file, int line, unsigned int subsys, size_t ret);
sudo_dso_public void sudo_debug_exit_ssize_t_v1(const char *func, const char *file, int line, unsigned int subsys, ssize_t ret);
sudo_dso_public void sudo_debug_exit_str_v1(const char *func, const char *file, int line, unsigned int subsys, const char *ret);
sudo_dso_public void sudo_debug_exit_str_masked_v1(const char *func, const char *file, int line, unsigned int subsys, const char *ret);
sudo_dso_public void sudo_debug_exit_time_t_v1(const char *func, const char *file, int line, unsigned int subsys, time_t ret);
sudo_dso_public void sudo_debug_exit_mode_t_v1(const char *func, const char *file, int line, unsigned int subsys, mode_t ret);
sudo_dso_public pid_t sudo_debug_fork_v1(void);
sudo_dso_public int sudo_debug_get_active_instance_v1(void);
sudo_dso_public int sudo_debug_get_fds_v1(unsigned char **fds);
sudo_dso_public int sudo_debug_get_instance_v1(const char *program);
sudo_dso_public int sudo_debug_parse_flags_v1(struct sudo_conf_debug_file_list *debug_files, const char *entry);
sudo_dso_public void sudo_debug_printf2_v1(const char *func, const char *file, int line, unsigned int level, const char * restrict fmt, ...) sudo_printf0like(5, 6);
sudo_dso_public void sudo_debug_printf_nvm_v1(int pri, const char * restrict fmt, ...) sudo_printf0like(2, 3);
sudo_dso_public int sudo_debug_register_v1(const char *program, const char *const subsystems[], unsigned int ids[], struct sudo_conf_debug_file_list *debug_files);
sudo_dso_public int sudo_debug_register_v2(const char *program, const char *const subsystems[], unsigned int ids[], struct sudo_conf_debug_file_list *debug_files, int minfd);
sudo_dso_public int sudo_debug_set_active_instance_v1(int inst);
sudo_dso_public void sudo_debug_update_fd_v1(int ofd, int nfd);
sudo_dso_public void sudo_debug_vprintf2_v1(const char *func, const char *file, int line, unsigned int level, const char * restrict fmt, va_list ap) sudo_printf0like(5, 0);
sudo_dso_public void sudo_debug_write2_v1(int fd, const char *func, const char *file, int line, const char *str, unsigned int len, int errnum);
sudo_dso_public bool sudo_debug_needed_v1(unsigned int level);

#define sudo_debug_needed(level) sudo_debug_needed_v1((level)|sudo_debug_subsys)
#define sudo_debug_deregister(_a) sudo_debug_deregister_v1((_a))
#define sudo_debug_execve2(_a, _b, _c, _d) sudo_debug_execve2_v1((_a), (_b), (_c), (_d))
#define sudo_debug_fork() sudo_debug_fork_v1()
#define sudo_debug_get_active_instance() sudo_debug_get_active_instance_v1()
#define sudo_debug_get_fds(_a) sudo_debug_get_fds_v1((_a))
#define sudo_debug_get_instance(_a) sudo_debug_get_instance_v1((_a))
#define sudo_debug_parse_flags(_a, _b) sudo_debug_parse_flags_v1((_a), (_b))
#define sudo_debug_printf2 sudo_debug_printf2_v1
#define sudo_debug_printf_nvm sudo_debug_printf_nvm_v1
#define sudo_debug_register(_a, _b, _c, _d, _e) sudo_debug_register_v2((_a), (_b), (_c), (_d), (_e))
#define sudo_debug_set_active_instance(_a) sudo_debug_set_active_instance_v1((_a))
#define sudo_debug_update_fd(_a, _b) sudo_debug_update_fd_v1((_a), (_b))
#define sudo_debug_vprintf2(_a, _b, _c, _d, _e, _f) sudo_debug_vprintf2_v1((_a), (_b), (_c), (_d), (_e), (_f))
#define sudo_debug_write2(_a, _b, _c, _d, _e, _f, _g) sudo_debug_write2_v1((_a), (_b), (_c), (_d), (_e), (_f), (_g))

#endif /* SUDO_DEBUG_H */
