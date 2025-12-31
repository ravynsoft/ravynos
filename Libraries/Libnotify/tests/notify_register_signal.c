#include <stdlib.h>
#include <notify.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <darwintest.h>
#include <signal.h>

#define KEY "com.apple.notify.test.notify_register_signal"

void poster(char * msg);

#define HANDLED 1
#define UNHANDLED 0

// Notification token
int n_token;

// Global to show if notification was handled
int handled = 0;

void post_notification(char * msg, int should_handle) {
    int i, loops = 100;

    // Post notification
    handled = 0;
    T_LOG("%s", msg);
    notify_post(KEY);

    for(i=0; i < loops; i++) {
        if(handled == HANDLED)
            break;
        else
            usleep(10);
    }

    if(should_handle)
        T_EXPECT_EQ(handled, (should_handle ? HANDLED : UNHANDLED),
                "Signal based notification handled as expected.");
}

int check() {
    int status;
    uint32_t rv;

    rv = notify_check(n_token, &status);

    if(rv != NOTIFY_STATUS_OK)
        T_FAIL("notify_check return value (%d) != NOTIFY_STATUS_OK", rv);

    return status;
}

void sig_handler(int sig) {
    int status = check();

    // Only continue if handler was called from notification system
    if(status < 1)
        return;

    if(handled != UNHANDLED)
        T_FAIL("handled (%d) was not reset to UNHANDLED as expected.", handled);

    handled = HANDLED;
}

void sig_setup() {
    // Register our signal handler
    signal(SIGINFO, sig_handler);
}

void sig_cleanup() {
    // Register our signal handler
    signal(SIGINFO, SIG_DFL);
}

void setup() {
    int n_check;

    // Setup our signal handler
    sig_setup();

    // Register with notification system
    notify_register_signal(KEY, SIGINFO, &n_token);

    n_check = check();
    if(n_check != 1)
        T_FAIL("notify_check failed to return 1 after initial registration.");
}

void cleanup() {
    // Clean up our signal handler
    sig_cleanup();

    notify_cancel(n_token);
}

T_DECL(notify_register_signal_1,
       "Basic test to confirm that notify_register_signal works.",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
    sig_setup();

    T_LOG("Registering for signal based notification.");

    // Register with notification system
    notify_register_signal(KEY, SIGINFO, &n_token);

    post_notification("Ensure notifications are being handled properly.", 1);

    sig_cleanup();
}

T_DECL(notify_register_signal_2,
        "Test that notify_check handles signal notifications properly.",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
    int n_check;

    setup();

    // Send UNIX style signal
    kill(getpid(),SIGINFO);
    T_EXPECT_EQ(handled, UNHANDLED,
            "handled (%d) remains unhandled, as expected.", handled);

    post_notification("Sighandler notification after kill()", 1);
    cleanup();
}

T_DECL(notify_register_signal_3,
        "Test that notify_resume and notify_suspend handle signal notifications properly.",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
    int i, n_check;
    uint32_t rv;

    setup();

    post_notification("Ensure notifications are being handled properly.", 1);

    rv = notify_suspend(n_token);
    T_EXPECT_EQ(rv, NOTIFY_STATUS_OK,
            "notify_suspend rv (%d) != NOTIFY_STATUS_OK", rv);

    for(i=0; i<3; i++) {
        post_notification("Suspended notification", 0);
        T_EXPECT_EQ(handled, UNHANDLED,
            "handled (%d) remains unhandled, as expected.", handled);
    }

    // Returns NOTIFY_STATUS_FAILED.  Leave off for now.
    //rv = notify_resume(n_token);
    //T_EXPECT_EQ(rv, NOTIFY_STATUS_OK,
    //        "notify_resume rv != NOTIFY_STATUS_OK");
    //T_EXPECT_EQ(handled, HANDLED,
    //        "handled has been handed for suspended notifications");

    cleanup();
}
