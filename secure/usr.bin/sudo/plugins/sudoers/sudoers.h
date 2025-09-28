/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1993-1996, 1998-2005, 2007-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#ifndef SUDOERS_SUDOERS_H
#define SUDOERS_SUDOERS_H

#include <sys/types.h>		/* for gid_t, mode_t, pid_t, size_t, uid_t */
#include <limits.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */

#define DEFAULT_TEXT_DOMAIN	"sudoers"

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_nss.h>
#include <sudo_plugin.h>
#include <sudo_queue.h>
#include <sudo_util.h>
#include <sudoers_debug.h>

#include <defaults.h>
#include <logging.h>
#include <parse.h>
#include <pivot.h>

/*
 * Info passed in from the sudo front-end.
 */
struct sudoers_open_info {
    char * const *settings;
    char * const *user_info;
    char * const *plugin_args;
};

/*
 * Supplementary group IDs for a user.
 */
struct gid_list {
    int ngids;
    GETGROUPS_T *gids;
};

/*
 * Supplementary group names for a user.
 */
struct group_list {
    int ngroups;
    char **groups;
};

/*
 * Parse configuration settings.
 */
struct sudoers_parser_config {
    const char *sudoers_path;
    int strict;
    int verbose;
    bool recovery;
    bool ignore_perms;
    mode_t sudoers_mode;
    uid_t sudoers_uid;
    gid_t sudoers_gid;
};
#define SUDOERS_PARSER_CONFIG_INITIALIZER {				\
    .sudoers_path = NULL,						\
    .strict = false,							\
    .verbose = 1,							\
    .recovery = true,							\
    .ignore_perms = false,						\
    .sudoers_mode = SUDOERS_MODE,					\
    .sudoers_uid = SUDOERS_UID,						\
    .sudoers_gid = SUDOERS_GID						\
}

/*
 * Settings passed in from the sudo front-end.
 */
struct sudoers_plugin_settings {
    const char *plugin_dir;
    const char *ldap_conf;
    const char *ldap_secret;
    unsigned int flags;
};
#define SUDOERS_PLUGIN_SETTINGS_INITIALIZER {				\
    .plugin_dir = _PATH_SUDO_PLUGIN_DIR,				\
    .ldap_conf = _PATH_LDAP_CONF,					\
    .ldap_secret = _PATH_LDAP_SECRET					\
}

/*
 * Info pertaining to the invoking user.
 */
struct sudoers_user_context {
    struct passwd *pw;
    struct stat *cmnd_stat;
    char *cwd;
    char *name;
    char *path;
    char *tty;
    char *ttypath;
    char *host;
    char *shost;
    char *prompt;
    char *cmnd;
    char *cmnd_args;
    char *cmnd_base;
    char *cmnd_dir;
    char *cmnd_list;
    char *ccname;
    struct gid_list *gid_list;
    char * const * envp;
    char * const * env_add;
    int   closefrom;
    int   lines;
    int   cols;
    int   timeout;
    mode_t umask;
    uid_t euid;
    uid_t uid;
    uid_t egid;
    uid_t gid;
    pid_t pid;
    pid_t ppid;
    pid_t sid;
    pid_t tcpgid;
};

/*
 * Info pertaining to the runas user.
 */
struct sudoers_runas_context {
    int execfd;
    int argc;
    char **argv;
    char **argv_saved;
    struct passwd *pw;
    struct group *gr;
    struct passwd *list_pw;
    char *chroot;
    char *class;
    char *cmnd;
    char *cmnd_saved;
    char *cwd;
    char *group;
    char *host;
    char *shost;
    char *user;
#ifdef HAVE_SELINUX
    char *role;
    char *type;
#endif
#ifdef HAVE_APPARMOR
    char *apparmor_profile;
#endif
#ifdef HAVE_PRIV_SET
    char *privs;
    char *limitprivs;
#endif
};

/*
 * Global configuration for the sudoers module.
 */
struct sudoers_context {
    struct sudoers_parser_config parser_conf;
    struct sudoers_plugin_settings settings;
    struct sudoers_user_context user;
    struct sudoers_runas_context runas;
    struct timespec submit_time;
    char *source;
    char *iolog_file;
    char *iolog_dir;
    char *iolog_path;
    int sudoedit_nfiles;
    unsigned int mode;
    char uuid_str[37];
};
#define SUDOERS_CONTEXT_INITIALIZER {					\
    SUDOERS_PARSER_CONFIG_INITIALIZER,					\
    SUDOERS_PLUGIN_SETTINGS_INITIALIZER,				\
}

/*
 * sudo_get_gidlist() type values
 */
#define ENTRY_TYPE_ANY		0x00
#define ENTRY_TYPE_QUERIED	0x01
#define ENTRY_TYPE_FRONTEND	0x02

/*
 * sudoers_plugin_settings.flag values
 */
#define RUNAS_USER_SPECIFIED	0x01U
#define RUNAS_GROUP_SPECIFIED	0x02U
#define CAN_INTERCEPT_SETID	0x04U
#define HAVE_INTERCEPT_PTRACE	0x08U
#define USER_INTERCEPT_SETID	0x10U

/*
 * Return values for sudoers_lookup(), also used as arguments for log_auth()
 * Note: cannot use '0' as a value here.
 */
#define VALIDATE_ERROR		0x001U
#define VALIDATE_SUCCESS	0x002U
#define VALIDATE_FAILURE	0x004U
#define FLAG_CHECK_USER		0x010U
#define FLAG_NO_USER		0x020U
#define FLAG_NO_HOST		0x040U
#define FLAG_NO_CHECK		0x080U
#define FLAG_NO_USER_INPUT	0x100U
#define FLAG_BAD_PASSWORD	0x200U
#define FLAG_INTERCEPT_SETID	0x400U

/*
 * Return values for check_user() (rowhammer resistant).
 */
#undef AUTH_SUCCESS
#define AUTH_SUCCESS		0x52a2925	/* 0101001010100010100100100101 */
#undef AUTH_FAILURE
#define AUTH_FAILURE		0xad5d6da	/* 1010110101011101011011011010 */
#undef AUTH_ERROR
#define AUTH_ERROR		0x1fc8d3ac	/* 11111110010001101001110101100 */

/*
 * find_path()/set_cmnd() return values
 */
#define FOUND			0
#define NOT_FOUND		1
#define NOT_FOUND_DOT		2
#define NOT_FOUND_ERROR		3
#define NOT_FOUND_PATH		4

/*
 * Various modes sudo can be in (based on arguments) in hex
 */
#define MODE_RUN		0x00000001U
#define MODE_EDIT		0x00000002U
#define MODE_VALIDATE		0x00000004U
#define MODE_INVALIDATE		0x00000008U
#define MODE_VERSION		0x00000010U
#define MODE_HELP		0x00000020U
#define MODE_LIST		0x00000040U
#define MODE_CHECK		0x00000080U
#define MODE_ERROR		0x00000100U
#define MODE_MASK		0x0000ffffU

/* Mode flags */
#define MODE_ASKPASS		0x00010000U
#define MODE_SHELL		0x00020000U
#define MODE_LOGIN_SHELL	0x00040000U
#define MODE_IMPLIED_SHELL	0x00080000U
#define MODE_RESET_HOME		0x00100000U
#define MODE_PRESERVE_GROUPS	0x00200000U
#define MODE_PRESERVE_ENV	0x00400000U
#define MODE_NONINTERACTIVE	0x00800000U
#define MODE_IGNORE_TICKET	0x01000000U
#define MODE_UPDATE_TICKET	0x02000000U
#define MODE_POLICY_INTERCEPTED	0x04000000U

/* Mode bits allowed for intercepted commands. */
#define MODE_INTERCEPT_MASK	(MODE_RUN|MODE_NONINTERACTIVE|MODE_IGNORE_TICKET|MODE_POLICY_INTERCEPTED)

/*
 * Used with set_perms()
 */
#define PERM_INITIAL		0x00
#define PERM_ROOT		0x01
#define PERM_USER		0x02
#define PERM_FULL_USER		0x03
#define PERM_SUDOERS		0x04
#define PERM_RUNAS		0x05
#define PERM_TIMESTAMP		0x06
#define PERM_IOLOG		0x07

/* Default sudoers uid/gid/mode if not set by the Makefile. */
#ifndef SUDOERS_UID
# define SUDOERS_UID	0
#endif
#ifndef SUDOERS_GID
# define SUDOERS_GID	0
#endif
#ifndef SUDOERS_MODE
# define SUDOERS_MODE	0600
#endif

struct sudo_lbuf;
struct passwd;
struct stat;
struct timespec;

/*
 * Function prototypes
 */
/* goodpath.c */
bool sudo_goodpath(const char *path, struct stat *sbp);

/* findpath.c */
int find_path(const char *infile, char **outfile, struct stat *sbp,
    const char *path, bool ignore_dot, char * const *allowlist);

/* resolve_cmnd.c */
int resolve_cmnd(struct sudoers_context *ctx, const char *infile,
    char **outfile, const char *path);

/* check.c */
int check_user(struct sudoers_context *ctx, unsigned int validated, unsigned int mode);
bool user_is_exempt(const struct sudoers_context *ctx);

/* check_util.c */
int check_user_runchroot(const char *runchroot);
int check_user_runcwd(const char *runcwd);

/* prompt.c */
char *expand_prompt(const struct sudoers_context *ctx, const char *restrict old_prompt, const char *restrict auth_user);

/* sudo_auth.c */
bool sudo_auth_needs_end_session(void);
int verify_user(const struct sudoers_context *ctx, struct passwd *pw, char *prompt, unsigned int validated, struct sudo_conv_callback *callback);
int sudo_auth_begin_session(const struct sudoers_context *ctx, struct passwd *pw, char **user_env[]);
int sudo_auth_end_session(void);
int sudo_auth_init(const struct sudoers_context *ctx, struct passwd *pw, unsigned int mode);
int sudo_auth_approval(const struct sudoers_context *ctx, struct passwd *pw, unsigned int validated, bool exempt);
int sudo_auth_cleanup(const struct sudoers_context *ctx, struct passwd *pw, bool force);

/* set_perms.c */
bool rewind_perms(void);
bool set_perms(const struct sudoers_context *, int);
bool restore_perms(void);
int pam_prep_user(struct passwd *);

/* defaults.c */
void dump_defaults(void);
void dump_auth_methods(void);

/* getspwuid.c */
char *sudo_getepw(const struct passwd *);

/* pwutil.c */
typedef struct cache_item * (*sudo_make_pwitem_t)(uid_t uid, const char *user);
typedef struct cache_item * (*sudo_make_gritem_t)(gid_t gid, const char *group);
typedef struct cache_item * (*sudo_make_gidlist_item_t)(const struct passwd *pw, int ngids, GETGROUPS_T *gids, char * const *gidstrs, unsigned int type);
typedef struct cache_item * (*sudo_make_grlist_item_t)(const struct passwd *pw, char * const *groups);
typedef bool (*sudo_valid_shell_t)(const char *shell);
sudo_dso_public struct group *sudo_getgrgid(gid_t);
sudo_dso_public struct group *sudo_getgrnam(const char *);
sudo_dso_public void sudo_gr_addref(struct group *);
sudo_dso_public void sudo_gr_delref(struct group *);
bool user_in_group(const struct passwd *, const char *);
struct group *sudo_fakegrnam(const char *);
struct group *sudo_mkgrent(const char *group, gid_t gid, ...);
struct gid_list *sudo_get_gidlist(const struct passwd *pw, unsigned int type);
struct group_list *sudo_get_grlist(const struct passwd *pw);
struct passwd *sudo_fakepwnam(const char *, gid_t);
struct passwd *sudo_mkpwent(const char *user, uid_t uid, gid_t gid, const char *home, const char *shell);
struct passwd *sudo_getpwnam(const char *);
struct passwd *sudo_getpwuid(uid_t);
void sudo_endspent(void);
void sudo_freegrcache(void);
void sudo_freepwcache(void);
void sudo_gidlist_addref(struct gid_list *);
void sudo_gidlist_delref(struct gid_list *);
void sudo_grlist_addref(struct group_list *);
void sudo_grlist_delref(struct group_list *);
void sudo_pw_addref(struct passwd *);
void sudo_pw_delref(struct passwd *);
int  sudo_set_gidlist(struct passwd *pw, int ngids, GETGROUPS_T *gids, char * const *gidstrs, unsigned int type);
int  sudo_set_grlist(struct passwd *pw, char * const *groups);
int  sudo_pwutil_get_max_groups(void);
void sudo_pwutil_set_max_groups(int);
void sudo_pwutil_set_backend(sudo_make_pwitem_t, sudo_make_gritem_t, sudo_make_gidlist_item_t, sudo_make_grlist_item_t, sudo_valid_shell_t);
void sudo_setspent(void);
bool user_shell_valid(const struct passwd *pw);

/* timestr.c */
char *get_timestr(time_t, int);

/* boottime.c */
bool get_boottime(struct timespec *);

/* iolog.c */
bool cb_maxseq(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);
bool cb_iolog_user(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);
bool cb_iolog_group(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);
bool cb_iolog_mode(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);

/* iolog_path_escapes.c */
struct iolog_path_escape;
extern const struct iolog_path_escape *sudoers_iolog_path_escapes;

/* env.c */
char **env_get(void);
bool env_merge(const struct sudoers_context *ctx, char * const envp[]);
bool env_swap_old(void);
void env_free(void);
bool env_init(char * const envp[]);
bool init_envtables(void);
bool insert_env_vars(char * const envp[]);
bool read_env_file(const struct sudoers_context *ctx, const char *path, bool overwrite, bool restricted);
bool rebuild_env(const struct sudoers_context *ctx);
bool validate_env_vars(const struct sudoers_context *ctx, char * const envp[]);
int sudo_setenv(const char *var, const char *val, int overwrite);
int sudo_unsetenv(const char *var);
char *sudo_getenv(const char *name);
char *sudo_getenv_nodebug(const char *name);
int sudo_putenv_nodebug(char *str, bool dupcheck, bool overwrite);
int sudo_unsetenv_nodebug(const char *var);
int sudoers_hook_getenv(const char *name, char **value, void *closure);
int sudoers_hook_putenv(char *string, void *closure);
int sudoers_hook_setenv(const char *name, const char *value, int overwrite, void *closure);
int sudoers_hook_unsetenv(const char *name, void *closure);
void register_env_file(void * (*ef_open)(const char *), void (*ef_close)(void *), char * (*ef_next)(void *, int *), bool sys);

/* env_pattern.c */
bool matches_env_pattern(const char *pattern, const char *var, bool *full_match);

/* sudoers_cb.c */
void set_callbacks(void);
bool cb_log_input(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);
bool cb_log_output(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);

/* sudoers.c */
FILE *open_sudoers(const char *, char **, bool, bool *);
bool cb_runas_default(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);
int set_cmnd_path(struct sudoers_context *ctx, const char *runchroot);
void set_cmnd_status(struct sudoers_context *ctx, const char *runchroot);
int sudoers_init(void *info, sudoers_logger_t logger, char * const envp[]);
int sudoers_check_cmnd(int argc, char *const argv[], char *env_add[], void *closure);
int sudoers_list(int argc, char *const argv[], const char *list_user, int verbose);
int sudoers_validate_user(void);
void sudoers_cleanup(void);
bool sudoers_override_umask(void);
const struct sudoers_context *sudoers_get_context(void);
bool sudoers_set_mode(unsigned int flags, unsigned int mask);

/* sudoers_ctx_free.c */
void sudoers_ctx_free(struct sudoers_context *ctx);

/* policy.c */
unsigned int sudoers_policy_deserialize_info(struct sudoers_context *ctx, void *v, struct defaults_list *defaults);
bool sudoers_policy_store_result(struct sudoers_context *ctx, bool accepted, char *argv[], char *envp[], mode_t cmnd_umask, char *iolog_path, void *v);
bool sudoers_tty_present(struct sudoers_context *ctx);
extern sudo_conv_t sudo_conv;
extern sudo_printf_t sudo_printf;
extern struct sudo_plugin_event * (*plugin_event_alloc)(void);

/* group_plugin.c */
void group_plugin_unload(void);
int group_plugin_query(const char *user, const char *group,
    const struct passwd *pwd);
bool cb_group_plugin(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);

/* editor.c */
char *find_editor(int nfiles, char * const *files, int *argc_out,
    char ***argv_out, char * const *allowlist, const char **env_editor);

/* exptilde.c */
bool expand_tilde(char **path, const char *user);

/* gc.c */
enum sudoers_gc_types {
    GC_UNKNOWN,
    GC_VECTOR,
    GC_PTR
};
bool sudoers_gc_add(enum sudoers_gc_types type, void *ptr);
bool sudoers_gc_remove(enum sudoers_gc_types type, void *ptr);
void sudoers_gc_init(void);
void sudoers_gc_run(void);

/* canon_path.c */
char *canon_path(const char *inpath);
void canon_path_free(char *resolved);
void canon_path_free_cache(void);

/* strlcpy_unesc.c */
size_t strlcpy_unescape(char *restrict dst, const char *restrict src, size_t size);

/* strvec_join.c */
char *strvec_join(char *const argv[], char sep, size_t (*cpy)(char *, const char *, size_t));

/* unesc_str.c */
void unescape_string(char *str);

/* serialize_list.c */
char *serialize_list(const char *varname, struct list_members *members);

/* sethost.c */
bool sudoers_sethost(struct sudoers_context *ctx, const char *host, const char *remhost);

#endif /* SUDOERS_SUDOERS_H */
