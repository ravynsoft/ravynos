#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <paths.h>
#include <fcntl.h>
#include <signal.h>
#include <util.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/uio.h>
#include <syslog.h>
#include <string.h>

void msg(const char *buf,...)
{
    va_list ap;
    va_start(ap, buf);

    vsyslog(LOG_ALERT, buf, ap);
    va_end(ap);
}

static void openConsole(void)
{
    int fd;

    /*
    * Try to open /dev/console.  Open the device with O_NONBLOCK to
    * prevent potential blocking on a carrier.
    */
    revoke(_PATH_CONSOLE);
    if ((fd = open(_PATH_CONSOLE, O_RDWR | O_NONBLOCK)) != -1) {
        (void)fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
        if (login_tty(fd) == 0)
            return;
        close(fd);
    }

    /* No luck.  Log output to file if possible. */
    if ((fd = open(_PATH_DEVNULL, O_RDWR)) == -1) {
        msg("cannot open null device.");
        _exit(1);
    }
    if (fd != STDIN_FILENO) {
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    fd = open("/tmp/init.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1)
        dup2(STDIN_FILENO, STDOUT_FILENO);
    else if (fd != STDOUT_FILENO) {
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    dup2(STDOUT_FILENO, STDERR_FILENO);
}

int main(int argc, char **argv)
{
    struct sigaction sa;
    pid_t child;
    int status, single = 0;
    int devfs = 0, verbose = 0;
    char c;

    openlog("init", LOG_CONS, LOG_AUTH);
    setsid();
    setlogin("root");

    close(0);
    close(1);
    close(2);

    bzero(&sa, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;

    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGWINCH, &sa, NULL);
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);

    sigprocmask(SIG_SETMASK, &sa.sa_mask, NULL);

    struct iovec iov[4];
    char *s;
    int i;

    char _fstype[]	= "fstype";
    char _devfs[]	= "devfs";
    char _fspath[]	= "fspath";
    char _path_dev[]= _PATH_DEV;

    iov[0].iov_base = _fstype;
    iov[0].iov_len = sizeof(_fstype);
    iov[1].iov_base = _devfs;
    iov[1].iov_len = sizeof(_devfs);
    iov[2].iov_base = _fspath;
    iov[2].iov_len = sizeof(_fspath);
    /*
    * Try to avoid the trailing slash in _PATH_DEV.
    * Be *very* defensive.
    */
    s = strdup(_PATH_DEV);
    if (s != NULL) {
        i = strlen(s);
        if (i > 0 && s[i - 1] == '/')
            s[i - 1] = '\0';
        iov[3].iov_base = s;
        iov[3].iov_len = strlen(s) + 1;
    } else {
        iov[3].iov_base = _path_dev;
        iov[3].iov_len = sizeof(_path_dev);
    }
    nmount(iov, 4, 0);
    if (s != NULL)
        free(s);


    while ((c = getopt(argc, argv, "dsv")) != -1)
        switch (c) {
            case 'd':
                devfs = 1;
                break;
            case 's':
                single = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                msg("unrecognized flag '-%c'", c);
                break;
        }

    msg("RAMDisk init starting\n");
    child = fork();
    if(child == 0) {
        openConsole();
        execl("/rescue/sh", "sh", "/init.sh", single ? "-s" : NULL, NULL);
    } else if(child > 0) {
        for(;;) {
            pid_t exited = wait(&status);
            if(WIFEXITED(status) && exited == child)
                execl("/sbin/launchd", "launchd", single ? "-s" : NULL, NULL);

            if(verbose)
                msg("pid %d exited with status %d\n", exited, status);
        }
    }
    msg("You're in the wrong place, friend\n");
    return -1;
}

