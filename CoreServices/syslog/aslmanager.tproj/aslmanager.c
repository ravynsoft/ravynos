/*
 * Copyright (c) 2007-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <asl.h>
#include <asl_msg.h>
#include <asl_msg_list.h>
#include <asl_store.h>
#include <errno.h>
#include <vproc_priv.h>
#include <os/transaction_private.h>

#include "asl_common.h"
#include "daemon.h"

/* global */
bool dryrun;
uint32_t debug;
FILE *debugfp;
dispatch_queue_t work_queue;

static dispatch_queue_t server_queue;
static time_t module_ttl;
static xpc_connection_t listener;
static bool main_task_enqueued;
static bool initial_main_task = true;
static dispatch_source_t sig_term_src;

/* wait 5 minutes to run main task after being invoked by XPC */
#define MAIN_TASK_INITIAL_DELAY 300

/*
 * Used to set config parameters.
 * Line format "= name value"
 */
static void
_aslmanager_set_param(asl_out_dst_data_t *dst, char *s)
{
	char **l;
	uint32_t count;

	if (s == NULL) return;
	if (s[0] == '\0') return;

	/* skip '=' and whitespace */
	if (*s == '=') s++;
	while ((*s == ' ') || (*s == '\t')) s++;

	l = explode(s, " \t");
	if (l == NULL) return;

	for (count = 0; l[count] != NULL; count++);

	/* name is required */
	if (count == 0)
	{
		free_string_list(l);
		return;
	}

	/* value is required */
	if (count == 1)
	{
		free_string_list(l);
		return;
	}

	if (!strcasecmp(l[0], "aslmanager_debug"))
	{
		/* = debug level */
		set_debug(DEBUG_ASL, l[1]);
	}
	else if (!strcasecmp(l[0], "store_ttl"))
	{
		/* = store_ttl days */
		dst->ttl[LEVEL_ALL] = asl_core_str_to_time(l[1], SECONDS_PER_DAY);
	}
	else if (!strcasecmp(l[0], "module_ttl"))
	{
		/* = module_ttl days */
		module_ttl = asl_core_str_to_time(l[1], SECONDS_PER_DAY);
	}
	else if (!strcasecmp(l[0], "max_store_size"))
	{
		/* = max_file_size bytes */
		dst->all_max = asl_core_str_to_size(l[1]);
	}
	else if (!strcasecmp(l[0], "archive"))
	{
		free(dst->rotate_dir);
		dst->rotate_dir = NULL;

		/* = archive {0|1} path */
		if (!strcmp(l[1], "1"))
		{
			if (l[2] == NULL) dst->rotate_dir = strdup(PATH_ASL_ARCHIVE);
			else dst->rotate_dir = strdup(l[2]);
		}
	}
	else if (!strcasecmp(l[0], "store_path"))
	{
		/* = archive path */
		free(dst->path);
		dst->path = strdup(l[1]);
	}
	else if (!strcasecmp(l[0], "archive_mode"))
	{
		dst->mode = strtol(l[1], NULL, 0);
		if ((dst->mode == 0) && (errno == EINVAL)) dst->mode = 0400;
	}

	free_string_list(l);
}

int
cli_main(int argc, char *argv[])
{
	int i, work;
	asl_out_module_t *mod, *m;
	asl_out_rule_t *r;
	asl_out_dst_data_t store, opts, *asl_store_dst = NULL;
	const char *mname = NULL;
	bool quiet = false;

#if !TARGET_OS_SIMULATOR
	if (geteuid() != 0)
	{
		if (argc == 0) debug = DEBUG_ASL;
		else debug = DEBUG_STDERR;

		debug_log(ASL_LEVEL_ERR, "aslmanager must be run by root\n");
		exit(1);
	}
#endif

	module_ttl = DEFAULT_TTL;

	/* cobble up a dst_data with defaults and parameter settings */
	memset(&store, 0, sizeof(store));
	store.ttl[LEVEL_ALL] = DEFAULT_TTL;
	store.all_max = DEFAULT_MAX_SIZE;

	memset(&opts, 0, sizeof(opts));
	opts.ttl[LEVEL_ALL] = DEFAULT_TTL;
	opts.all_max = DEFAULT_MAX_SIZE;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-q"))
		{
			quiet = true;
		}
		else if (!strcmp(argv[i], "-dd"))
		{
			quiet = true;
		}
		else if (!strcmp(argv[i], "-s"))
		{
			if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
			{
				store.path = strdup(argv[++i]);
				asl_store_dst = &store;
			}
		}
	}

	if (!quiet)
	{
		char *path = NULL;
		int status = asl_make_database_dir(NULL, NULL);
		if (status == 0) status = asl_make_database_dir(ASL_INTERNAL_LOGS_DIR, &path);
		if (status == 0)
		{
			char tstamp[32], *str = NULL;

			asl_make_timestamp(time(NULL), MODULE_NAME_STYLE_STAMP_LCL_B, tstamp, sizeof(tstamp));
			asprintf(&str, "%s/aslmanager.%s", path, tstamp);

			if (str != NULL)
			{
				if (status == 0) debugfp = fopen(str, "w");
				if (debugfp != NULL) debug |= DEBUG_FILE;
				free(str);
			}
		}
		free(path);
	}

	/* get parameters from asl.conf */
	mod = asl_out_module_init();

	if (mod != NULL)
	{
		for (r = mod->ruleset; (r != NULL) && (asl_store_dst == NULL); r = r->next)
		{
			if ((r->dst != NULL) && (r->action == ACTION_OUT_DEST) && (!strcmp(r->dst->path, PATH_ASL_STORE)))
				asl_store_dst = r->dst;
		}

		for (r = mod->ruleset; r != NULL; r = r->next)
		{
			if (r->action == ACTION_SET_PARAM)
			{
				if (r->query == NULL) _aslmanager_set_param(asl_store_dst, r->options);
			}
		}
	}

	work = DO_ASLDB | DO_MODULE;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-a"))
		{
			if (asl_store_dst == NULL) asl_store_dst = &store;

			if (((i + 1) < argc) && (argv[i + 1][0] != '-')) asl_store_dst->rotate_dir = strdup(argv[++i]);
			else asl_store_dst->rotate_dir = strdup(PATH_ASL_ARCHIVE);
			asl_store_dst->mode = 0400;
		}
		else if (!strcmp(argv[i], "-store_ttl"))
		{
			if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
			{
				if (asl_store_dst == NULL) asl_store_dst = &store;
				asl_store_dst->ttl[LEVEL_ALL] = asl_core_str_to_time(argv[++i], SECONDS_PER_DAY);
			}
		}
		else if (!strcmp(argv[i], "-module_ttl"))
		{
			if (((i + 1) < argc) && (argv[i + 1][0] != '-')) module_ttl = asl_core_str_to_time(argv[++i], SECONDS_PER_DAY);
		}
		else if (!strcmp(argv[i], "-ttl"))
		{
			if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
			{
				opts.ttl[LEVEL_ALL] = asl_core_str_to_time(argv[++i], SECONDS_PER_DAY);

				if (asl_store_dst == NULL) asl_store_dst = &store;
				asl_store_dst->ttl[LEVEL_ALL] = opts.ttl[LEVEL_ALL];

				module_ttl = opts.ttl[LEVEL_ALL];
			}
		}
		else if (!strcmp(argv[i], "-size"))
		{
			if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
			{
				opts.all_max = asl_core_str_to_size(argv[++i]);

				if (asl_store_dst == NULL) asl_store_dst = &store;
				asl_store_dst->all_max = opts.all_max;
			}
		}
		else if (!strcmp(argv[i], "-checkpoint"))
		{
			work |= DO_CHECKPT;
		}
		else if (!strcmp(argv[i], "-module"))
		{
			work &= ~DO_ASLDB;

			/* optional name follows -module */
			if ((i +1) < argc)
			{
				if (argv[i + 1][0] != '-') mname = argv[++i];
			}
		}
		else if (!strcmp(argv[i], "-asldb"))
		{
			work = DO_ASLDB;
		}
		else if (!strcmp(argv[i], "-d"))
		{
			if (((i + i) < argc) && (argv[i+1][0] != '-')) set_debug(DEBUG_STDERR, argv[++i]);
			else set_debug(DEBUG_STDERR, NULL);
		}
		else if (!strcmp(argv[i], "-dd"))
		{
			dryrun = true;

			if (((i + i) < argc) && (argv[i+1][0] != '-')) set_debug(DEBUG_STDERR, argv[++i]);
			else set_debug(DEBUG_STDERR, "l7");
		}
	}

	if (asl_store_dst != NULL && asl_store_dst->path == NULL) asl_store_dst->path = strdup(PATH_ASL_STORE);

	debug_log(ASL_LEVEL_ERR, "aslmanager starting%s\n", dryrun ? " dryrun" : "");

	if (work & DO_ASLDB) process_asl_data_store(asl_store_dst, &opts);

	if (work & DO_MODULE)
	{
		if (work & DO_CHECKPT) checkpoint(mname);

		if (mod != NULL)
		{
			for (m = mod; m != NULL; m = m->next)
			{
				if (mname == NULL)
				{
					process_module(m, NULL);
				}
				else if ((m->name != NULL) && (!strcmp(m->name, mname)))
				{
					process_module(m, &opts);
				}
			}
		}
	}

	asl_out_module_free(mod);

	debug_log(ASL_LEVEL_NOTICE, "----------------------------------------\n");
	debug_log(ASL_LEVEL_ERR, "aslmanager finished%s\n", dryrun ? " dryrun" : "");
	debug_close();

	return 0;
}

/* dispatched on server_queue, dispatches to work_queue */
void
main_task(void)
{
	/* if main task is already running or queued, do nothing */
	if (main_task_enqueued) return;

	main_task_enqueued = true;

	os_transaction_t transaction = os_transaction_create("com.apple.aslmanager");

	if (initial_main_task)
	{
		initial_main_task = false;
		dispatch_time_t delay = dispatch_walltime(NULL, MAIN_TASK_INITIAL_DELAY * NSEC_PER_SEC);

		dispatch_after(delay, work_queue, ^{
			cli_main(0, NULL);
			main_task_enqueued = false;
			os_release(transaction);
		});
	}
	else
	{
		dispatch_async(work_queue, ^{
			cli_main(0, NULL);
			main_task_enqueued = false;
			os_release(transaction);
		});
	}
}

static void
accept_connection(xpc_connection_t peer)
{
	xpc_connection_set_event_handler(peer, ^(xpc_object_t request) {
		if (xpc_get_type(request) == XPC_TYPE_DICTIONARY)
		{
			uid_t uid = xpc_connection_get_euid(peer);

			/* send a reply immediately */
			xpc_object_t reply = xpc_dictionary_create_reply(request);
			if (reply != NULL)
			{
				xpc_connection_send_message(peer, reply);
				xpc_release(reply);
			}

			/*
			 * Some day, we may use the dictionary to pass parameters
			 * to aslmanager, but for now, we ignore the input.
			 */

			if (uid == geteuid())
			{
				main_task();
			}
		}
		else if (xpc_get_type(request) == XPC_TYPE_ERROR)
		{
			/* disconnect */
		}
	});

	xpc_connection_resume(peer);
}

int
main(int argc, char *argv[])
{
	int64_t is_managed = 0;

	vproc_swap_integer(NULL, VPROC_GSK_IS_MANAGED, NULL, &is_managed);

	if (is_managed == 0) return cli_main(argc, argv);

	/* Set I/O policy */
	setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS, IOPOL_THROTTLE);

	/* XPC server */
	server_queue = dispatch_queue_create("aslmanager", NULL);

	work_queue = dispatch_queue_create("work queue", NULL);

	/* Exit on SIGTERM */
	signal(SIGTERM, SIG_IGN);
	sig_term_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, (uintptr_t)SIGTERM, 0, dispatch_get_main_queue());
	dispatch_source_set_event_handler(sig_term_src, ^{
		debug_log(ASL_LEVEL_NOTICE, "SIGTERM exit\n");
		exit(0);
	});

	dispatch_resume(sig_term_src);

	/* Handle incoming messages. */
	listener = xpc_connection_create_mach_service("com.apple.aslmanager", server_queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);
	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) == XPC_TYPE_CONNECTION) accept_connection(peer);
	});
	xpc_connection_resume(listener);

	dispatch_main();
}
