/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ucred.h>
#include <sys/socket.h>
#include <pthread.h>

#include "common/font.h"
#include "common/spawn.h"
#include "config/session.h"
#include "labwc.h"
#include "theme.h"
#include "xbm/xbm.h"
#include "menu/menu.h"

#define SA_RESTART      0x0002  /* restart system call on signal return */

struct rcxml rc = { 0 };

enum ShellType {
    NONE, LOGINWINDOW, DESKTOP
};

void launchShell(void *arg) {
    enum ShellType shell = *(enum ShellType *)arg;
    if(fork() == 0) {
        setsid();
        execl("/usr/bin/foot", "foot", NULL);
        exit(-1);
    }
    while(shell != NONE) {
        // FIXME: launch loginwindow or systemuiserver here
    }
    pthread_exit(NULL);
}

int main(int argc, const char *argv[]) {
    enum wlr_log_importance debuglevel = WLR_ERROR;
    enum ShellType shell = LOGINWINDOW;
    char *config_file = NULL;
    pthread_t shellThread;

    if(getenv("XDG_RUNTIME_DIR") == NULL) {
        char *buf = 0;
        asprintf(&buf, "/tmp/runtime.%u", getuid());
        setenv("XDG_RUNTIME_DIR", buf, 0);
        if(access(buf, R_OK|W_OK|X_OK) != 0) {
            switch(errno) {
                case ENOENT: mkdir(buf, 0700); break;
                default: perror("WindowServer"); exit(-1);
            }
        }
        free(buf);
    }

    NSString *confPath = [[NSBundle mainBundle] pathForResource:@"ws" ofType:@"conf"];
    config_file = [confPath UTF8String];

    while(getopt(argc, argv, "Lxv") != -1) {
        switch(optopt) {
            case 'L': // bypass loginwindow, run desktop for current user
                shell = DESKTOP;
                break;
            case 'x': // just run the compositor
                shell = NONE;
                break;
            case 'v':
                debuglevel = WLR_INFO;
                break;
        }
    }

    while(access("/var/run/windowserver", F_OK) != 0)
        sleep(1);

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGTHR, SIG_IGN);
    signal(SIGLIBRT, SIG_IGN);

    wlr_log_init(debuglevel, NULL);

    session_environment_init();
    rcxml_read(config_file);

    struct server server = { 0 };
    server_init(&server);
    server_start(&server);

    struct theme theme = { 0 };
    theme_init(&theme, server.renderer, rc.theme_name);
    server.theme = &theme;

    menu_init_rootmenu(&server);
    menu_init_windowmenu(&server);

    pthread_create(&shellThread, NULL, launchShell, &shell);

    close(0);
    close(1);
    close(2);

    wl_display_run(server.wl_display);

    shell = NONE;
    pthread_cancel(shellThread);

    server_finish(&server);
    menu_finish();
    theme_finish(&theme);
    rcxml_finish();
    font_finish();
}
