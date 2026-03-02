/* The purpose of this file is to generate a approval_plugin symbols,
 * with an I/O plugin context which is unique to it and its functions.
 * The callbacks inside are just wrappers around the real functions in python_plugin_approval.c,
 * their only purpose is to add the unique context to each separate approval_plugin call.
 */

#define PLUGIN_CTX APPROVAL_SYMBOL_NAME(plugin_ctx)
#define CALLBACK_CFUNC(func_name) APPROVAL_SYMBOL_NAME(_python_plugin_approval_ ## func_name)

extern struct approval_plugin APPROVAL_SYMBOL_NAME(python_approval);
static struct ApprovalPluginContext PLUGIN_CTX = { { NULL }, &APPROVAL_SYMBOL_NAME(python_approval) };


static int
CALLBACK_CFUNC(open)(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t sudo_printf, char * const settings[],
    char * const user_info[], int submit_optind,
    char * const submit_argv[], char * const submit_envp[],
    char * const plugin_options[], const char **errstr)
{
    return python_plugin_approval_open(&PLUGIN_CTX, version, conversation,
        sudo_printf, settings, user_info, submit_optind, submit_argv,
        submit_envp, plugin_options, errstr);
}

static void
CALLBACK_CFUNC(close)(void)
{
    python_plugin_approval_close(&PLUGIN_CTX);
}

static int
CALLBACK_CFUNC(check)(char * const command_info[], char * const run_argv[],
    char * const run_envp[], const char **errstr)
{
    return python_plugin_approval_check(&PLUGIN_CTX, command_info, run_argv,
        run_envp, errstr);
}

static int
CALLBACK_CFUNC(show_version)(int verbose)
{
    return python_plugin_approval_show_version(&PLUGIN_CTX, verbose);
}

struct approval_plugin APPROVAL_SYMBOL_NAME(python_approval) = {
    SUDO_APPROVAL_PLUGIN,
    SUDO_API_VERSION,
    CALLBACK_CFUNC(open),
    CALLBACK_CFUNC(close),
    CALLBACK_CFUNC(check),
    CALLBACK_CFUNC(show_version)
};

#undef PLUGIN_CTX
#undef CALLBACK_CFUNC
#undef APPROVAL_SYMBOL_NAME
