// TEST_CONFIG

#include "test.h"

void *flushthread(void *arg __unused)
{
    while (1) {
        _objc_flush_caches(nil);
    }
}

int main()
{
    pthread_t th;
    pthread_create(&th, nil, &flushthread, nil);

    alarm(120);
    
    [NSObject self];
    [NSObject self];

    int max = is_guardmalloc() ? 10: 100;
    
    for (int i = 0; i < max; i++) {
        pid_t child;
        switch ((child = fork())) {
          case -1:
            abort();
          case 0:
            // child
            alarm(10);
            [NSObject self];
            _exit(0);
          default: {
            // parent
            int result = 0;
            while (waitpid(child, &result, 0) < 0) {
                if (errno != EINTR) {
                    fail("waitpid failed (errno %d %s)", 
                         errno, strerror(errno));
                }
            }
            if (!WIFEXITED(result)) {
                fail("child crashed (waitpid result %d)", result);
            }

            [NSObject self];
            break;
          }
        }
    }

    succeed(__FILE__ " parent");
}
