/*
 * Copyright © 2014 Ran Benita <ran234@gmail.com>
 * Copyright © 2023 Pierre Le Marre <dev@wismill.eu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <stdio.h>
#include <spawn.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"
#include "xvfb-wrapper.h"
#include "xkbcommon/xkbcommon-x11.h"

static bool xvfb_is_ready;

static void
sigusr1_handler(int signal)
{
    xvfb_is_ready = true;
}

int
xvfb_wrapper(int (*test_func)(char* display))
{
    int ret = 0;
    FILE * display_fd;
    char display_fd_string[32];
    sigset_t mask;
    struct sigaction sa;
    char *xvfb_argv[] = {
        (char *) "Xvfb", (char *) "-displayfd", display_fd_string, NULL
    };
    char *envp[] = { NULL };
    pid_t xvfb_pid = 0;
    size_t counter = 0;
    char display[32] = ":";
    size_t length;

    /* File descriptor to retrieve the display number */
    display_fd = tmpfile();
    if (display_fd == NULL){
        fprintf(stderr, "Unable to create temporary file.\n");
        goto err_display_fd;
    }
    snprintf(display_fd_string, sizeof(display_fd_string), "%d", fileno(display_fd));

    /* Set SIGUSR1 to SIG_IGN so Xvfb will send us that signal
     * when it's ready to accept connections */
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    xvfb_is_ready = false;

    /*
     * Xvfb command: let the server find an available display.
     *
     * Note that it may generate multiple times the following output in stderr:
     *    _XSERVTransSocketUNIXCreateListener: ...SocketCreateListener() failed
     * It is expected: this is the server trying the ports until it finds one
     * that works.
     */
    ret = posix_spawnp(&xvfb_pid, "Xvfb", NULL, NULL, xvfb_argv, envp);
    if (ret != 0) {
        ret = SKIP_TEST;
        goto err_xvfd;
    }

    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    signal(SIGUSR1, sigusr1_handler);
    sigprocmask (SIG_UNBLOCK, &mask, NULL);

    /* Now wait for the SIGUSR1 signal that Xvfb is ready */
    while (!xvfb_is_ready) {
        usleep(1000);
        if (++counter >= 3000) /* 3 seconds max wait */
            break;
    }

    signal(SIGUSR1, SIG_DFL);

    /* Retrieve the display number: Xvfd writes the display number as a newline-
     * terminated string; copy this number to form a proper display string. */
    rewind(display_fd);
    length = fread(&display[1], 1, sizeof(display) - 1, display_fd);
    if (length <= 0) {
        ret = SKIP_TEST;
        goto err_xvfd;
    } else {
        /* Drop the newline character */
        display[length] = '\0';
    }

    /* Run the function requiring a running X server */
    ret = test_func(display);

err_xvfd:
    if (xvfb_pid > 0)
        kill(xvfb_pid, SIGTERM);
    fclose(display_fd);
err_display_fd:
    return ret;
}

/* All X11_TEST functions are in the test_functions_section ELF section.
 * __start and __stop point to the start and end of that section. See the
 * __attribute__(section) documentation.
 */
extern const struct test_function __start_test_functions_section, __stop_test_functions_section;

int
x11_tests_run()
{
    size_t count = 1; /* For NULL-terminated entry */

    for (const struct test_function *t = &__start_test_functions_section;
         t < &__stop_test_functions_section;
         t++)
        count++;

    int rc;
    for (const struct test_function *t = &__start_test_functions_section;
         t < &__stop_test_functions_section;
         t++) {
        fprintf(stderr, "Running test: %s from %s\n", t->name, t->file);
        rc = xvfb_wrapper(t->func);
        if (rc != 0) {
            break;
        }
    }

    return rc;
}
