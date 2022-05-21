// SPDX-License-Identifier: GPL-2.0-only
#include "common/font.h"
#include "common/spawn.h"
#include "config/session.h"
#include "labwc.h"
#include "theme.h"
#include "xbm/xbm.h"
#include "menu/menu.h"

struct rcxml rc = { 0 };

static const char labwc_usage[] =
	"Usage: labwc [-h] [-s <command>] [-c <config-file>] [-d] [-V] [-v]\n";

static void
usage(void)
{
	printf("%s", labwc_usage);
	exit(0);
}

int
main(int argc, char *argv[])
{
	char *startup_cmd = NULL;
	char *config_file = NULL;
	enum wlr_log_importance verbosity = WLR_ERROR;

	int c;
	while ((c = getopt(argc, argv, "c:dhs:vV")) != -1) {
		switch (c) {
		case 'c':
			config_file = optarg;
			break;
		case 'd':
			verbosity = WLR_DEBUG;
			break;
		case 's':
			startup_cmd = optarg;
			break;
		case 'v':
			printf("labwc " LABWC_VERSION "\n");
			exit(0);
		case 'V':
			verbosity = WLR_INFO;
			break;
		case 'h':
		default:
			usage();
		}
	}
	if (optind < argc) {
		usage();
	}

	wlr_log_init(verbosity, NULL);

	session_environment_init();
	rcxml_read(config_file);

	if (!getenv("XDG_RUNTIME_DIR")) {
		wlr_log(WLR_ERROR, "XDG_RUNTIME_DIR is unset");
		exit(EXIT_FAILURE);
	}

	struct server server = { 0 };
	server_init(&server);
	server_start(&server);

	struct theme theme = { 0 };
	theme_init(&theme, server.renderer, rc.theme_name);
	server.theme = &theme;

	menu_init_rootmenu(&server);
	menu_init_windowmenu(&server);

	session_autostart_init();
	if (startup_cmd) {
		spawn_async_no_shell(startup_cmd);
	}

	wl_display_run(server.wl_display);

	server_finish(&server);

	menu_finish();
	theme_finish(&theme);
	rcxml_finish();
	font_finish();
	return 0;
}
