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
#include <sys/wait.h>
#include <pthread.h>
#include <pwd.h>
#include <grp.h>
#include <login_cap.h>

#include "common/font.h"
#include "common/spawn.h"
#include "config/session.h"
#include "labwc.h"
#include "theme.h"
#include "xbm/xbm.h"
#include "menu/menu.h"

#define SA_RESTART      0x0002  /* restart system call on signal return */
#define XDG_DIR_PATTERN "/tmp/runtime.%u"

struct rcxml rc = { 0 };
BOOL ready = NO;
unsigned int nobodyUID, videoGID;
char *xdgDir = 0;

enum ShellType {
    NONE, LOGINWINDOW, DESKTOP
};

static inline void giveXdgDir(unsigned int uid, unsigned int gid, const char *path) {
    char *buf = 0;

    chown(path, uid, gid);

    asprintf(&buf, "%s/wayland-0", path);
    chown(buf, uid, gid);
    free(buf);

    asprintf(&buf, "%s/wayland-0.lock", path);
    chown(buf, uid, gid);
    free(buf);

    chown("/tmp/com.ravynos.WindowServer", uid, gid); // menu socket
}

static inline void createSymlinks(const char *path) {
    char *buf = 0;
    char *buf2 = 0;
    asprintf(&buf, "%s/wayland-0", xdgDir);
    asprintf(&buf2, "%s/wayland-0", path);
    unlink(buf2);
    symlink(buf, buf2);
    free(buf);
    free(buf2);

    asprintf(&buf, "%s/wayland-0.lock", xdgDir);
    asprintf(&buf2, "%s/wayland-0.lock", path);
    unlink(buf2);
    symlink(buf, buf2);
    free(buf);
    free(buf2);
}

static char **setUpEnviron(int uid) {
    struct passwd *pw = getpwuid(uid);
    if(!pw)
        return NULL;
    int entries = 8;
    char **envp = malloc(sizeof(char *) * entries);
    asprintf(&envp[0], "HOME=%s", pw->pw_dir);
    asprintf(&envp[1], "SHELL=%s", pw->pw_shell);
    asprintf(&envp[2], "USER=%s", pw->pw_name);
    asprintf(&envp[3], "LOGNAME=%s", pw->pw_name);
    asprintf(&envp[4], "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
    asprintf(&envp[5], "XDG_RUNTIME_DIR=" XDG_DIR_PATTERN, uid);
    asprintf(&envp[6], "TERM=xterm");
    envp[entries - 1] = NULL;
    return envp;
}

static void freeEnviron(char **envp) {
    if(envp == NULL)
        return;

    while(*envp != NULL)
        free(*envp++);
}

void launchShell(void *arg) {
    enum ShellType shell = *(enum ShellType *)arg;
    int spawned = 0, status;
    NSString *lwPath = nil;

    while(shell != NONE) {
        if(ready == NO) {
            sleep(1);
            continue;
        }

        switch(shell) {
            case LOGINWINDOW:
                if(seteuid(0) != 0) { // re-assert privileges
                    perror("seteuid");
                    exit(-1);
                }

                // take ownership of the socket, cutting off any previous user
                giveXdgDir(nobodyUID, videoGID, xdgDir);

                if(setresgid(videoGID, videoGID, 0) != 0) {
                    perror("setresgid");
                    exit(-1);
                }
                if(setresuid(nobodyUID, nobodyUID, 0) != 0) {
                    perror("setresuid");
                    exit(-1);
                }

                int uid = nobodyUID;
                int gid = videoGID;

                int fds[2];
                if(pipe(fds) != 0) {
                    perror("pipe");
                    exit(-1);
                }
                char fdbuf[8];
                sprintf(fdbuf, "%d", fds[1]);
                int status = -1;

                lwPath = [[NSBundle mainBundle] pathForResource:@"LoginWindow" ofType:@"app"];
                if(!lwPath) {
                    NSLog(@"missing LoginWindow.app!");
                    break;
                }
                lwPath = [[NSBundle bundleWithPath:lwPath] executablePath];
                if(!lwPath) {
                    NSLog(@"missing LoginWindow.app!");
                    break;
                }
                
                char **envp = setUpEnviron(nobodyUID);

                pid_t pid = fork();
                if(!pid) { // child
                    close(fds[0]);
                    seteuid(0);
                    execle([lwPath UTF8String], [[lwPath lastPathComponent] UTF8String], fdbuf, NULL, envp);
                    exit(-1);
                } else {
                    close(fds[1]);
                    read(fds[0], &uid, sizeof(int));
                    waitpid(pid, &status, 0);
                }
                freeEnviron(envp);
                close(fds[0]);
                NSLog(@"received uid %d", uid);

                if(uid < 500) {
                    NSLog(@"UID below minimum");
                    break;
                }
                    
                struct passwd *pw = getpwuid(uid);
                if(!pw || pw->pw_uid != uid) {
                    NSLog(@"no such uid %d", uid);
                    break;
                }
                gid = pw->pw_gid;

                // give socket to the logged in user
                char *userXdgDir = 0;
                asprintf(&userXdgDir, XDG_DIR_PATTERN, uid);
                mkdir(userXdgDir, 0700);

                if(seteuid(0) != 0) { // re-assert privileges
                    perror("seteuid");
                    exit(-1);
                }
                giveXdgDir(uid, gid, xdgDir);
                createSymlinks(userXdgDir);
                giveXdgDir(uid, gid, userXdgDir);
                free(userXdgDir);

                // ensure our helper is owned correctly
                {
                    NSString *path = [[NSBundle mainBundle] pathForResource:@"SystemUIServer" ofType:@"app"];
                    if(path)
                        path = [[NSBundle bundleWithPath:path] pathForResource:@"shutdown" ofType:@""];
                    if(path) {
                        chown([path UTF8String], 0, videoGID);
                        chmod([path UTF8String], 04550);
                    }
                }

                shell = DESKTOP;
                break;
            case DESKTOP: {
                char **envp = setUpEnviron(uid);

                if(!spawned && fork() == 0) {
                    setlogin(pw->pw_name);
                    chdir(pw->pw_dir);

                    login_cap_t *lc = login_getpwclass(pw);
                    if (setusercontext(lc, pw, pw->pw_uid,
                        LOGIN_SETALL & ~(LOGIN_SETLOGIN)) != 0) {
                            perror("setusercontext");
                            exit(-1);
                    }
                    login_close(lc);
                    ++spawned;
                    execle("/usr/bin/foot", "foot", "-dnone", "-L", "-W", "80x25", NULL, envp);
                    perror("execl");
                    spawned = 0;
                    exit(-1);
                }
                pid_t pid = fork();
                if(pid == 0) {
                    setlogin(pw->pw_name);
                    chdir(pw->pw_dir);

                    login_cap_t *lc = login_getpwclass(pw);
                    if (setusercontext(lc, pw, pw->pw_uid,
                        LOGIN_SETALL & ~(LOGIN_SETLOGIN)) != 0) {
                            perror("setusercontext");
                            exit(-1);
                    }
                    login_close(lc);

                    NSString *path = [[NSBundle mainBundle] pathForResource:@"SystemUIServer" ofType:@"app"];
                    if(path)
                        path = [[NSBundle bundleWithPath:path] executablePath];
                    
                    if(path)
                        execle([path UTF8String], [[path lastPathComponent] UTF8String], NULL, envp);

                    perror("execl");
                    exit(-1);
                } else if(pid < 0) {
                    perror("fork");
                    sleep(3);
                    shell = LOGINWINDOW;
                    break;
                }
                freeEnviron(envp);
                waitpid(pid, &status, 0);
                shell = LOGINWINDOW;
                execl("/bin/launchctl", "launchctl", "remove", "com.ravynos.WindowServer", NULL);
                break;
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, const char *argv[]) {
    enum wlr_log_importance debuglevel = WLR_ERROR;
    enum ShellType shell = LOGINWINDOW;
    char *config_file = NULL;
    pthread_t shellThread;

    struct passwd *passwd = getpwnam("nobody");
    if(!passwd) {
        perror("getpwnam(nobody)");
        exit(-1);
    }
    nobodyUID = passwd->pw_uid;

    struct group *group = getgrnam("video");
    if(!group) {
        perror("getgrnam(video)");
        exit(-1);
    }
    videoGID = group->gr_gid;

    asprintf(&xdgDir, XDG_DIR_PATTERN, nobodyUID);
    setenv("XDG_RUNTIME_DIR", xdgDir, 1);
    if(access(xdgDir, R_OK|W_OK|X_OK) != 0) {
        switch(errno) {
            case ENOENT:
                mkdir(xdgDir, 0700);
                break;
            default: perror("WindowServer"); exit(-1);
        }
    }
    giveXdgDir(nobodyUID, videoGID, xdgDir);

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

    pthread_create(&shellThread, NULL, launchShell, &shell);

    setresgid(videoGID, videoGID, 0);
    setresuid(nobodyUID, nobodyUID, 0);
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

    ready = YES;
    wl_display_run(server.wl_display);

    shell = NONE;
    pthread_cancel(shellThread);

    server_finish(&server);
    menu_finish();
    theme_finish(&theme);
    rcxml_finish();
    font_finish();
}
