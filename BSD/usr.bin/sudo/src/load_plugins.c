/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sudo.h>
#include <sudo_plugin.h>
#include <sudo_plugin_int.h>
#include <sudo_dso.h>

#ifdef ENABLE_SUDO_PLUGIN_API
static bool
sudo_qualify_plugin(struct plugin_info *info, char *fullpath, size_t pathsize)
{
    const char *plugin_dir = sudo_conf_plugin_dir_path();
    int len;
    debug_decl(sudo_stat_plugin, SUDO_DEBUG_PLUGIN);

    if (info->path[0] == '/') {
	if (strlcpy(fullpath, info->path, pathsize) >= pathsize) {
	    errno = ENAMETOOLONG;
	    goto bad;
	}
    } else {
#ifdef STATIC_SUDOERS_PLUGIN
	/* Check static symbols. */
	if (strcmp(info->path, _PATH_SUDOERS_PLUGIN) == 0) {
	    if (strlcpy(fullpath, info->path, pathsize) >= pathsize) {
		errno = ENAMETOOLONG;
		goto bad;
	    }
	    /* Plugin is static, do not fully-qualify. */
	    debug_return_bool(true);
	}
#endif /* STATIC_SUDOERS_PLUGIN */

	if (plugin_dir == NULL) {
	    errno = ENOENT;
	    goto bad;
	}
	len = snprintf(fullpath, pathsize, "%s%s", plugin_dir, info->path);
	if (len < 0 || (size_t)len >= pathsize) {
	    errno = ENAMETOOLONG;
	    goto bad;
	}
    }
    debug_return_bool(true);
bad:
    sudo_warnx(U_("error in %s, line %d while loading plugin \"%s\""),
	_PATH_SUDO_CONF, info->lineno, info->symbol_name);
    if (info->path[0] != '/' && plugin_dir != NULL)
	sudo_warn("%s%s", plugin_dir, info->path);
    else
	sudo_warn("%s", info->path);
    debug_return_bool(false);
}
#else
static bool
sudo_qualify_plugin(struct plugin_info *info, char *fullpath, size_t pathsize)
{
    debug_decl(sudo_qualify_plugin, SUDO_DEBUG_PLUGIN);
    (void)strlcpy(fullpath, info->path, pathsize);
    debug_return_bool(true);
}
#endif /* ENABLE_SUDO_PLUGIN_API */

static bool
fill_container(struct plugin_container *container, void *handle,
     const char *path, struct generic_plugin *plugin, struct plugin_info *info)
{
    debug_decl(fill_container, SUDO_DEBUG_PLUGIN);

    if ((container->path = strdup(path)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_bool(false);
    }
    container->handle = handle;
    container->name = info->symbol_name;
    container->options = info->options;
    container->debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;
    container->u.generic = plugin;
    container->debug_files = sudo_conf_debug_files(path);

    /* Zero out info strings that the container now owns. */
    info->symbol_name = NULL;
    info->options = NULL;

    debug_return_bool(true);
}

static struct plugin_container *
new_container(void *handle, const char *path, struct generic_plugin *plugin,
    struct plugin_info *info)
{
    struct plugin_container *container;
    debug_decl(new_container, SUDO_DEBUG_PLUGIN);

    if ((container = calloc(1, sizeof(*container))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    if (!fill_container(container, handle, path, plugin, info))
	goto bad;

    debug_return_ptr(container);
bad:
    free(container);
    debug_return_ptr(NULL);
}

static bool
plugin_exists(struct plugin_container_list *plugins, const char *symbol_name)
{
    struct plugin_container *container;
    debug_decl(plugin_exists, SUDO_DEBUG_PLUGIN);

    TAILQ_FOREACH(container, plugins, entries) {
	if (strcmp(container->name, symbol_name) == 0)
	    debug_return_bool(true);
    }
    debug_return_bool(false);
}

typedef struct generic_plugin * (plugin_clone_func)(void);

static struct generic_plugin *
sudo_plugin_try_to_clone(void *so_handle, const char *symbol_name)
{
    debug_decl(sudo_plugin_try_to_clone, SUDO_DEBUG_PLUGIN);
    struct generic_plugin *plugin = NULL;
    plugin_clone_func *clone_func;
    char *clone_func_name = NULL;

    if (asprintf(&clone_func_name, "%s_clone", symbol_name) < 0) {
        sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
        goto cleanup;
    }

    clone_func = (plugin_clone_func *)sudo_dso_findsym(so_handle,
	clone_func_name);
    if (clone_func) {
        plugin = (*clone_func)();
    }

cleanup:
    free(clone_func_name);
    debug_return_ptr(plugin);
}

static bool
sudo_insert_plugin(struct plugin_container_list *plugin_list, void *handle,
     const char *path, struct generic_plugin *plugin, struct plugin_info *info)
{
    struct plugin_container *container;
    debug_decl(sudo_insert_plugin, SUDO_DEBUG_PLUGIN);

    if (plugin_exists(plugin_list, info->symbol_name)) {
	plugin = sudo_plugin_try_to_clone(handle, info->symbol_name);
	if (plugin == NULL) {
	    sudo_warnx(U_("ignoring duplicate plugin \"%s\" in %s, line %d"),
		info->symbol_name, _PATH_SUDO_CONF, info->lineno);
	    sudo_dso_unload(handle);
	    goto done;
	}
    }

    if ((container = new_container(handle, path, plugin, info)) == NULL)
	debug_return_bool(false);
    TAILQ_INSERT_TAIL(plugin_list, container, entries);

done:
    debug_return_bool(true);
}

/*
 * Load the plugin specified by "info".
 */
static bool
sudo_load_plugin(struct plugin_info *info, bool quiet)
{
    struct generic_plugin *plugin;
    char path[PATH_MAX];
    void *handle = NULL;
    bool ret = false;
    debug_decl(sudo_load_plugin, SUDO_DEBUG_PLUGIN);

    /* Fill in path from info and plugin dir. */
    if (!sudo_qualify_plugin(info, path, sizeof(path)))
	goto done;

    /* Open plugin and map in symbol */
    handle = sudo_dso_load(path, SUDO_DSO_LAZY|SUDO_DSO_GLOBAL);
    if (!handle) {
	if (!quiet) {
	    const char *errstr = sudo_dso_strerror();
	    sudo_warnx(U_("error in %s, line %d while loading plugin \"%s\""),
		_PATH_SUDO_CONF, info->lineno, info->symbol_name);
	    sudo_warnx(U_("unable to load %s: %s"), path,
		errstr ? errstr : "unknown error");
	}
	goto done;
    }
    plugin = sudo_dso_findsym(handle, info->symbol_name);
    if (!plugin) {
	if (!quiet) {
	    sudo_warnx(U_("error in %s, line %d while loading plugin \"%s\""),
		_PATH_SUDO_CONF, info->lineno, info->symbol_name);
	    sudo_warnx(U_("unable to find symbol \"%s\" in %s"),
		info->symbol_name, path);
	}
	goto done;
    }

    if (SUDO_API_VERSION_GET_MAJOR(plugin->version) != SUDO_API_VERSION_MAJOR) {
	if (!quiet) {
	    sudo_warnx(U_("error in %s, line %d while loading plugin \"%s\""),
		_PATH_SUDO_CONF, info->lineno, info->symbol_name);
	    sudo_warnx(U_("incompatible plugin major version %d (expected %d) found in %s"),
		SUDO_API_VERSION_GET_MAJOR(plugin->version),
		SUDO_API_VERSION_MAJOR, path);
	}
	goto done;
    }

    switch (plugin->type) {
    case SUDO_POLICY_PLUGIN:
	if (policy_plugin.handle != NULL) {
	    /* Ignore duplicate entries. */
	    if (strcmp(policy_plugin.name, info->symbol_name) == 0) {
		if (!quiet) {
		    sudo_warnx(U_("ignoring duplicate plugin \"%s\" in %s, line %d"),
			info->symbol_name, _PATH_SUDO_CONF, info->lineno);
		}
	    } else {
		if (!quiet) {
		    sudo_warnx(U_("ignoring policy plugin \"%s\" in %s, line %d"),
			info->symbol_name, _PATH_SUDO_CONF, info->lineno);
		    sudo_warnx("%s",
			U_("only a single policy plugin may be specified"));
		}
		goto done;
	    }
	    ret = true;
	    goto done;
	}
	if (!fill_container(&policy_plugin, handle, path, plugin, info))
	    goto done;
	break;
    case SUDO_IO_PLUGIN:
	if (!sudo_insert_plugin(&io_plugins, handle, path, plugin, info))
	    goto done;
	break;
    case SUDO_AUDIT_PLUGIN:
	if (!sudo_insert_plugin(&audit_plugins, handle, path, plugin, info))
	    goto done;
	break;
    case SUDO_APPROVAL_PLUGIN:
	if (!sudo_insert_plugin(&approval_plugins, handle, path, plugin, info))
	    goto done;
	break;
    default:
	if (!quiet) {
	    sudo_warnx(U_("error in %s, line %d while loading plugin \"%s\""),
		_PATH_SUDO_CONF, info->lineno, info->symbol_name);
	    sudo_warnx(U_("unknown plugin type %d found in %s"), plugin->type, path);
	}
	goto done;
    }

    /* Handle is either in use or has been closed. */
    handle = NULL;

    ret = true;

done:
    if (handle != NULL)
	sudo_dso_unload(handle);
    debug_return_bool(ret);
}

static void
free_plugin_info(struct plugin_info *info)
{
    free(info->path);
    free(info->symbol_name);
    if (info->options != NULL) {
	int i = 0;
	while (info->options[i] != NULL)
	    free(info->options[i++]);
	free(info->options);
    }
    free(info);
}

static void
sudo_register_hooks(void)
{
    struct plugin_container *container;
    debug_decl(sudo_register_hooks, SUDO_DEBUG_PLUGIN);

    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 2)) {
	if (policy_plugin.u.policy->register_hooks != NULL) {
	    sudo_debug_set_active_instance(policy_plugin.debug_instance);
	    policy_plugin.u.policy->register_hooks(SUDO_HOOK_VERSION,
		register_hook);
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}
    }

    TAILQ_FOREACH(container, &io_plugins, entries) {
	if (container->u.io->version >= SUDO_API_MKVERSION(1, 2)) {
	    if (container->u.io->register_hooks != NULL) {
		sudo_debug_set_active_instance(container->debug_instance);
		container->u.io->register_hooks(SUDO_HOOK_VERSION,
		    register_hook);
		sudo_debug_set_active_instance(sudo_debug_instance);
	    }
	}
    }

    TAILQ_FOREACH(container, &audit_plugins, entries) {
	if (container->u.audit->register_hooks != NULL) {
	    sudo_debug_set_active_instance(container->debug_instance);
	    container->u.audit->register_hooks(SUDO_HOOK_VERSION,
		register_hook);
	    sudo_debug_set_active_instance(sudo_debug_instance);
	}
    }

    debug_return;
}

static void
sudo_init_event_alloc(void)
{
    struct plugin_container *container;
    debug_decl(sudo_init_event_alloc, SUDO_DEBUG_PLUGIN);

    if (policy_plugin.u.policy->version >= SUDO_API_MKVERSION(1, 15))
	policy_plugin.u.policy->event_alloc = sudo_plugin_event_alloc;

    TAILQ_FOREACH(container, &io_plugins, entries) {
	if (container->u.io->version >= SUDO_API_MKVERSION(1, 15))
	    container->u.io->event_alloc = sudo_plugin_event_alloc;
    }
    TAILQ_FOREACH(container, &audit_plugins, entries) {
	if (container->u.audit->version >= SUDO_API_MKVERSION(1, 17))
	    container->u.audit->event_alloc = sudo_plugin_event_alloc;
    }

    debug_return;
}

/*
 * Load the specified symbol from the sudoers plugin.
 * Used to provide a default plugin when none are specified in sudo.conf.
 */
static bool
sudo_load_sudoers_plugin(const char *symbol_name, bool optional)
{
    struct plugin_info *info;
    bool ret = false;
    debug_decl(sudo_load_sudoers_plugin, SUDO_DEBUG_PLUGIN);

    /* Default policy plugin */
    info = calloc(1, sizeof(*info));
    if (info == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    info->symbol_name = strdup(symbol_name);
    info->path = strdup(_PATH_SUDOERS_PLUGIN);
    if (info->symbol_name == NULL || info->path == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	free_plugin_info(info);
	goto done;
    }
    /* info->options = NULL; */
    ret = sudo_load_plugin(info, optional);
    free_plugin_info(info);

done:
    debug_return_bool(ret);
}

/*
 * Load the plugins listed in sudo.conf.
 */
bool
sudo_load_plugins(void)
{
    struct plugin_info_list *plugins;
    struct plugin_info *info, *next;
    bool ret = false;
    debug_decl(sudo_load_plugins, SUDO_DEBUG_PLUGIN);

    /* Walk the plugin list from sudo.conf, if any and free it. */
    plugins = sudo_conf_plugins();
    TAILQ_FOREACH_SAFE(info, plugins, entries, next) {
	ret = sudo_load_plugin(info, false);
	if (!ret)
	    goto done;
	free_plugin_info(info);
    }
    TAILQ_INIT(plugins);

    /*
     * If no policy plugin, fall back to the default (sudoers).
     * If there is also no I/O log plugin, use sudoers for that too.
     */
    if (policy_plugin.handle == NULL) {
	/* Default policy plugin */
	ret = sudo_load_sudoers_plugin("sudoers_policy", false);
	if (!ret)
	    goto done;

	/* Default audit plugin, optional (sudoers < 1.9.1 lack this) */
	(void)sudo_load_sudoers_plugin("sudoers_audit", true);

	/* Default I/O plugin */
	if (TAILQ_EMPTY(&io_plugins)) {
	    ret = sudo_load_sudoers_plugin("sudoers_io", false);
	    if (!ret)
		goto done;
	}
    } else if (strcmp(policy_plugin.name, "sudoers_policy") == 0) {
	/*
	 * If policy plugin is sudoers_policy but there is no sudoers_audit
	 * loaded, load it too, if possible.
	 */
	if (!plugin_exists(&audit_plugins, "sudoers_audit")) {
	    if (sudo_load_sudoers_plugin("sudoers_audit", true)) {
		/*
		 * Move the plugin options from sudoers_policy to sudoers_audit
		 * since the audit module is now what actually opens sudoers.
		 */
		if (policy_plugin.options != NULL) {
		    TAILQ_LAST(&audit_plugins, plugin_container_list)->options =
			policy_plugin.options;
		    policy_plugin.options = NULL;
		}
	    }
	}
    }

    /* TODO: check all plugins for open function too */
    if (policy_plugin.u.policy->check_policy == NULL) {
	sudo_warnx(U_("policy plugin %s does not include a check_policy method"),
	    policy_plugin.name);
	ret = false;
	goto done;
    }

    /* Set event_alloc() in plugins. */
    sudo_init_event_alloc();

    /* Install hooks (XXX - later, after open). */
    sudo_register_hooks();

done:
    debug_return_bool(ret);
}
