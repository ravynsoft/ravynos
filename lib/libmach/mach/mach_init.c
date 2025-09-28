#include <sys/cdefs.h>
#include <sys/types.h>

#include <pthread.h>

#include <mach/mach.h>
#include <mach/boolean.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>
#include <mach/mach_port.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


#include <syslog.h>
#include <stdarg.h>

#include <sys/mach/ndr_def.h>
void mach_init(void) __attribute__((constructor));
mach_port_t mach_reply_port(void);
mach_port_t task_self_trap(void);

extern mach_port_t _task_reply_port;
mach_port_t bootstrap_port = 0;
mach_port_t mach_task_self_ = 0;

 __attribute__((visibility("hidden"))) mach_port_t _task_reply_port;
extern void mig_init(void);

void
mach_init(void)
{
	pid_t pid;
	char *root_flag;
	int root_bootstrap;
	kern_return_t kr;
	static int mach_inited_pid = 0;

	/* we may need to call this again after fork */
	if (mach_inited_pid != (pid = getpid())) {
		mig_init();
		root_flag = getenv("ROOT_BOOTSTRAP");
		root_bootstrap = (root_flag != NULL) && (strcmp(root_flag, "T") == 0);

		/* Only call pthread_atfork when not in the fork handler */
		if (mach_inited_pid == 0)
			pthread_atfork(NULL, NULL, mach_init);

		mach_task_self_ = task_self_trap();
		_task_reply_port = mach_reply_port();
		if (pid != 1 && root_bootstrap == false) {
			kr = task_get_special_port(mach_task_self_, TASK_BOOTSTRAP_PORT, &bootstrap_port);
			if (kr != KERN_SUCCESS) {
				syslog(LOG_EMERG, "get_special_port failed - mach_task_self_: %d", mach_task_self_);
			  return;
			}
		}
		if (root_bootstrap == true) {
			syslog(LOG_ERR, "skip bootstrap port fetch");
			unsetenv("ROOT_BOOTSTRAP");
		}
		mach_inited_pid = pid;
	}
}
