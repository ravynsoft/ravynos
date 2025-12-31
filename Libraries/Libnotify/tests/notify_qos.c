#include <stdlib.h>
#include <notify.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <darwintest.h>
#include <signal.h>

#define KEY "com.apple.notify.test.notify_register_signal"

#define HANDLED 1
#define UNHANDLED 0

// Notification token
int n_token;

// Global to show if notification was handled
int handled = 0;

void post_notification(char * msg, int should_handle) {
    int i, loops = 5000;   //5000*10 = 50,000 = 50ms

    // Post notification
    handled = UNHANDLED;
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

void setup() {

    // Register with notification system
    dispatch_queue_t dq = dispatch_queue_create(NULL, DISPATCH_QUEUE_SERIAL);
    int rv = notify_register_dispatch(KEY, &n_token, dq, ^(int token){
        handled = HANDLED;

        qos_class_t block_qos = qos_class_self();
        if (block_qos != QOS_CLASS_DEFAULT){
            T_FAIL("Block is running at QoS %#x instead of DEF", block_qos);
        } else {
        }
    });
    if (rv)
        T_FAIL("Unable to notify_register_dispatch");
}

void cleanup() {
    notify_cancel(n_token); /* Releases the queue - block must finish executing */
}

T_DECL(notify_qos,
        "Test that work for notification runs at qos of target queue",
       T_META("owner", "Core Darwin Daemons & Tools"),
       T_META("as_root", "false"))
{
    setup();

    post_notification("Ensure notifications are being handled properly.", 1);

    cleanup();
}
