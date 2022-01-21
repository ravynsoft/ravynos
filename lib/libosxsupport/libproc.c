#include <sys/types.h>
#include <sys/errno.h>
#include <mach/boolean.h>
#include <libproc.h>


int __proc_info(int callnum, int pid, int flavor, uint64_t arg, void * buffer, int buffersize);

int 
proc_listpids(uint32_t type, uint32_t typeinfo, void *buffer, int buffersize)
{
	int rv;
	
	if ((type >= PROC_ALL_PIDS) || (type <= PROC_PPID_ONLY)) {
		if ((rv = __proc_info(PROC_INFO_CALL_LISTPIDS, type, typeinfo, 0, buffer, buffersize)) == -1)
			return (0);
	} else {
		errno = EINVAL;
		rv = 0;
	}
	return (rv);
}

int
proc_listallpids(void * buffer, int buffersize)
{
	int numpids;

	numpids = proc_listpids(PROC_ALL_PIDS, 0, buffer, buffersize);
	if (numpids == -1)
		return (-1);
	else
		return (numpids/sizeof(int));
}

int 
proc_listpgrppids(pid_t pgrpid, void * buffer, int buffersize)
{
	int numpids;

	numpids = proc_listpids(PROC_PGRP_ONLY, (uint32_t)pgrpid, buffer, buffersize);
	if (numpids == -1)
		return (-1);
	else
		return (numpids/sizeof(int));
}

int 
proc_listchildpids(pid_t ppid, void * buffer, int buffersize)
{
	int numpids;

	numpids = proc_listpids(PROC_PPID_ONLY, (uint32_t)ppid, buffer, buffersize);
	if (numpids == -1)
		return (-1);
	else
		return (numpids/sizeof(int));
}

int 
proc_pidinfo(int pid, int flavor, uint64_t arg,  void *buffer, int buffersize)
{
	int rv;

	if ((rv = __proc_info(PROC_INFO_CALL_PIDINFO, pid, flavor,  arg,  buffer, buffersize)) == -1)
		return (0);

	return (rv);
}

int
proc_setpcontrol(const int control __unused)
{

	return (ENOTSUP);
}

int
proc_track_dirty(pid_t pid __unused, uint32_t flags __unused)
{

	return (ENOTSUP);
}

int
proc_set_dirty(pid_t pid __unused, bool dirty __unused)
{

	return (ENOTSUP);
}

int
proc_get_dirty(pid_t pid __unused, uint32_t *flags __unused)
{

	return (ENOTSUP);
}

int
proc_terminate(pid_t pid, int *sig)
{
	int rv;

	if (sig == NULL)
		return (EINVAL);

	if ((rv = __proc_info(PROC_INFO_CALL_TERMINATE, pid, 0, 0, NULL, 0)) == -1)
		return (errno);

	*sig = rv;
	return (0);
}
