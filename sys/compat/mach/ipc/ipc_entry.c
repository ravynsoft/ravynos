/*-
 * Copyright (c) 2014-2015, Matthew Macy <mmacy@nextbsd.org>
 * Copyright (c) 2022, Zoe Knox <zoe@pixin.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Neither the name of Matthew Macy nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/* CMU_HIST */
/*
 * Revision 2.7  91/10/09  16:08:15  af
 * 	 Revision 2.6.2.1  91/09/16  10:15:30  rpd
 * 	 	Added <ipc/ipc_hash.h>.
 * 	 	[91/09/02            rpd]
 * 
 * Revision 2.6.2.1  91/09/16  10:15:30  rpd
 * 	Added <ipc/ipc_hash.h>.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.6  91/05/14  16:31:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/03/16  14:47:45  rpd
 * 	Fixed ipc_entry_grow_table to use it_entries_realloc.
 * 	[91/03/05            rpd]
 * 
 * Revision 2.4  91/02/05  17:21:17  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:44:19  mrt]
 * 
 * Revision 2.3  91/01/08  15:12:58  rpd
 * 	Removed MACH_IPC_GENNOS.
 * 	[90/11/08            rpd]
 * 
 * Revision 2.2  90/06/02  14:49:36  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:54:27  rpd]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	ipc/ipc_entry.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Primitive functions to manipulate translation entries.
 */


#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_capsicum.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/eventhandler.h>
#include <sys/capsicum.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/fcntl.h>
#include <sys/kernel.h>
#include <sys/syscallsubr.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/user.h>
#include <sys/limits.h>

#include <sys/mach/mach_types.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/port.h>
#include <sys/mach/message.h>
#include <sys/mach/mach_port_server.h>
#include <vm/uma.h>
#include <sys/mach/ipc/port.h>
#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_hash.h>
#include <sys/mach/ipc/ipc_table.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_pset.h>
#include <sys/mach/thread.h>

#include <security/audit/audit.h>

static void fdunused(struct filedesc *fdp, int fd);
static int kern_fdalloc(struct thread *td, int minfd, int *result);
static void kern_fddealloc(struct thread *td, int fd);
static inline void kern_fdfree(struct filedesc *fdp, int fd);
static int kern_finstall(struct thread *td, struct file *fp, int *fd, int flags,
			 struct filecaps *fcaps);

#define MODERN (__FreeBSD_version >= 1100000)
#define FNOFDALLOC    0x80000000
#define        fde_change_size (offsetof(struct filedescent, fde_seqc))

static void
ipc_entry_hash_delete(
	ipc_space_t space,
	ipc_entry_t entry)
{
	mach_port_index_t idx;
	ipc_entry_t entryp;

	if ((idx = entry->ie_index) == UINT_MAX)
		return;
	if ((entry->ie_bits & (MACH_PORT_TYPE_DEAD_NAME | MACH_PORT_TYPE_SEND)) !=
		MACH_PORT_TYPE_SEND)
		return;
	if (idx >= space->is_table_size)
		return;

	entryp = space->is_table[idx];
	assert(entryp);

	if (entryp == entry) {
		space->is_table[idx] = entry->ie_link;
		entry->ie_link = NULL;
		entry->ie_index = UINT_MAX;
	} else {
		while (entryp->ie_link != NULL) {
			if (entryp->ie_link == entry) {
				entryp->ie_link = entry->ie_link;
				entry->ie_link = NULL;
				entry->ie_index = UINT_MAX;
				break;
			}
			entryp = entryp->ie_link;
		}
	}
	/* assert that it was found */
	MPASS(entry->ie_index == UINT_MAX);
}

static fo_close_t mach_port_close;
static fo_stat_t mach_port_stat;
#if MODERN
static fo_fill_kinfo_t mach_port_fill_kinfo;
#endif

struct fileops mach_fileops  = {
	.fo_close = mach_port_close,
	.fo_stat = mach_port_stat,
#if MODERN
	.fo_fill_kinfo = mach_port_fill_kinfo,
#endif
	.fo_flags = 0,
};

static int
mach_port_close(struct file *fp, struct thread *td)
{
	ipc_entry_t entry;
	ipc_object_t object;
	ipc_pset_t pset;
	ipc_port_t port;

	MACH_VERIFY(fp->f_data != NULL, ("expected fp->f_data != NULL - got NULL\n"));
	if ((entry = fp->f_data) == NULL)
		return (0);
	if ((entry->ie_bits & MACH_PORT_TYPE_PORT_SET)  == 0)
		ipc_entry_hash_delete(entry->ie_space, entry);
	MPASS(entry->ie_link == NULL);
	PROC_LOCK(td->td_proc);
	LIST_REMOVE(entry, ie_space_link);
	PROC_UNLOCK(td->td_proc);
	if ((object = entry->ie_object) != NULL) {
		if (entry->ie_bits & MACH_PORT_TYPE_PORT_SET) {
			pset = (ipc_pset_t)object;
			ips_lock(pset);
			ipc_pset_destroy(pset);
		} else {
			port = (ipc_port_t)object;
			if (port->ip_receiver == current_space()) {
				ip_lock(port);
				ipc_port_clear_receiver(port);
				ipc_port_destroy(port);
			} else {
				ip_release(port);
			}
		}
		entry->ie_object = NULL;
	}
	free(entry, M_MACH_IPC_ENTRY);
	fp->f_data = NULL;

	return (0);
}

static int
mach_port_stat(struct file *fp __unused, struct stat *sb,
			   struct ucred *active_cred __unused)
{
	ipc_entry_t entry;

	bzero((caddr_t)sb, sizeof(*sb));

	entry = fp->f_data;
	if (entry->ie_bits & MACH_PORT_TYPE_PORT_SET) {
		sb->st_mode = S_IFPSET;
	} else {
		sb->st_mode = S_IFPORT;
	}
	return (0);
}

#if MODERN
static int
mach_port_fill_kinfo(struct file *fp, struct kinfo_file *kif,
					 struct filedesc *fdp __unused)
{
	ipc_entry_t entry;

	/* assume it's a port first */
	kif->kf_type = KF_TYPE_PORT;

	if ((entry = fp->f_data) == NULL)
		return (0);
	/* What else do we want from it? */
	if (entry->ie_bits & MACH_PORT_TYPE_PORT_SET) {
		kif->kf_type = KF_TYPE_PORTSET;
	}

	return (0);
}
#endif

/*
 *	Routine:	ipc_entry_release
 *	Purpose:
 *		Drops a reference to an entry.
 *	Conditions:
 *		The space must be locked.
 */

void
ipc_entry_release(ipc_entry_t entry)
{

	fdrop(entry->ie_fp, curthread);
}

/*
 *	Routine:	ipc_entry_lookup
 *	Purpose:
 *		Searches for an entry, given its name.
 *	Conditions:
 *		The space must be active.
 */

extern void kdb_backtrace(void);
ipc_entry_t
ipc_entry_lookup(ipc_space_t space, mach_port_name_t name)
{
	struct file *fp;
	ipc_entry_t entry;
	cap_rights_t rights;

	assert(space->is_active);

	if (curthread->td_proc->p_fd == NULL)
		return (NULL);

	if (fget(curthread, name, cap_rights_init(&rights, CAP_KQUEUE_EVENT|CAP_KQUEUE_CHANGE), &fp) != 0) {
		if (mach_debug_enable)
			log(LOG_DEBUG, "%s:%d entry for port name: %d not found\n", curproc->p_comm, curproc->p_pid, name);
		return (NULL);
	}
	if (fp->f_type != DTYPE_MACH_IPC) {
		if (mach_debug_enable) {
			kdb_backtrace();
			log(LOG_DEBUG, "%s:%d port name: %d is not MACH\n", curproc->p_comm, curproc->p_pid, name);
		}
		fdrop(fp, curthread);
		return (NULL);
	}
	entry = fp->f_data;
	fdrop(fp, curthread);
	return (entry);
}

kern_return_t
ipc_entry_file_to_port(ipc_space_t space, mach_port_name_t name, ipc_object_t *objectp)
{
	struct file *fp;
	ipc_port_t port;
	cap_rights_t rights;

	assert(space->is_active);

	if (curthread->td_proc->p_fd == NULL)
		return (KERN_INVALID_ARGUMENT);

	if (fget(curthread, name, cap_rights_init(&rights, CAP_ALL1), &fp) != 0) {
		log(LOG_DEBUG, "%s:%d entry for port name: %d not found\n", curproc->p_comm, curproc->p_pid, name);
		return (KERN_INVALID_ARGUMENT);
	}
	if (fp->f_type == DTYPE_MACH_IPC) {
		fdrop(fp, curthread);
		return (KERN_INVALID_ARGUMENT);
	}
	if ((port = ipc_port_alloc_special(space)) == NULL)
		return (KERN_RESOURCE_SHORTAGE);

	port->ip_context = (mach_vm_address_t) fp;
	port->ip_flags = IP_CONTEXT_FILE;
	port->ip_receiver = space;
	port->ip_receiver_name = name;

	*objectp = (ipc_object_t)port;
	return (KERN_SUCCESS);
}

void
ipc_entry_file_destroy(ipc_object_t objectp)
{
	ipc_port_t port;
	struct file *fp;

	if (curthread->td_proc->p_fd == NULL)
		return;

	port = (ipc_port_t)objectp;
	fp = (void *)port->ip_context;
	ipc_port_dealloc_special(port, current_space());
	fdrop(fp, curthread);
}

kern_return_t
ipc_entry_port_to_file(ipc_space_t space, mach_port_name_t *namep, ipc_object_t object)
{
	ipc_port_t port;
	struct file *fp;

	port = (ipc_port_t)object;
	MPASS(object != NULL);
	MPASS(port->ip_flags & IP_CONTEXT_FILE);
	fp = (void *)port->ip_context;
	/* the receiver will have been set by the sender of the port */
	port->ip_receiver = space;
	ipc_port_dealloc_special(port, space);

	/* Are sent file O_CLOEXEC? */
	if (kern_finstall(curthread, fp, namep, 0, NULL) != 0) {
		fdrop(fp, curthread);
		if (mach_debug_enable)
			printf("finstall failed\n");
		return (KERN_RESOURCE_SHORTAGE);
	}
	if (mach_debug_enable)
		printf(" installing received file *fp=%p at %d\n", fp, *namep);
	return (KERN_SUCCESS);
}

/*
 *	Routine:	ipc_entry_get
 *	Purpose:
 *		Tries to allocate an entry out of the space.
 *	Conditions:
 *		The space is active throughout.
 *		An object may be locked.  Will try to allocate memory.
 *	Returns:
 *		KERN_SUCCESS		A free entry was found.
 *		KERN_NO_SPACE		No entry allocated.
 */

kern_return_t
ipc_entry_get(
	ipc_space_t	space,
	boolean_t	is_send_once,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp)
{	
	ipc_entry_t free_entry;
	int fd;
	struct file *fp;
	struct thread *td;

	assert(space->is_active);

	td  = curthread;
	if ((free_entry = malloc(sizeof(*free_entry), M_MACH_IPC_ENTRY, M_WAITOK|M_ZERO)) == NULL)
		return KERN_RESOURCE_SHORTAGE;

	if (kern_fdalloc(td, 16, &fd)) {
		log(LOG_WARNING, "%s:%d failed to allocate fd\n", __FILE__, __LINE__);
		return (KERN_RESOURCE_SHORTAGE);
	}
	if (falloc_noinstall(td, &fp)) {
		kern_fddealloc(td, fd);
		log(LOG_WARNING, "%s:%d failed to allocate fp\n", __FILE__, __LINE__);
		return (KERN_RESOURCE_SHORTAGE);
	}
	if (kern_finstall(td, fp, &fd, FNOFDALLOC, NULL)) {
		log(LOG_WARNING, "%s:%d failed to allocate fp:%p at fd:%d \n", __FILE__, __LINE__, fp, fd);
		kern_fddealloc(td, fd);
		fdrop(fp, td);
		return (KERN_RESOURCE_SHORTAGE);
	}

	free_entry->ie_bits = 0;
	free_entry->ie_request = 0;
	free_entry->ie_name = fd;
	free_entry->ie_fp = fp;
	free_entry->ie_index = UINT_MAX;
	free_entry->ie_link = NULL;
	free_entry->ie_space = space;
	PROC_LOCK(curproc);
	LIST_INSERT_HEAD(&space->is_entry_list, free_entry, ie_space_link);
	PROC_UNLOCK(curproc);
	finit(fp, 0, DTYPE_MACH_IPC, free_entry, &mach_fileops);
	fdrop(fp, td);
	assert(fp->f_count == 1);
	*namep = fd;
	*entryp = free_entry;

	return KERN_SUCCESS;
}

/*
 *	Routine:	ipc_entry_alloc
 *	Purpose:
 *		Allocate an entry out of the space.
 *	Conditions:
 *		The space is not locked before, but it is write-locked after
 *		if the call is successful.  May allocate memory.
 *	Returns:
 *		KERN_SUCCESS		An entry was allocated.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_NO_SPACE		No room for an entry in the space.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory for an entry.
 */

kern_return_t
ipc_entry_alloc(
	ipc_space_t	space,
	boolean_t	is_send_once,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp)
{
	kern_return_t kr;

	if (!space->is_active)
		return (KERN_INVALID_TASK);

	*namep = MACH_PORT_NAME_NULL;
	if ((kr = ipc_entry_get(space, is_send_once, namep, entryp)) != KERN_SUCCESS)
		return (kr);

	is_write_lock(space);
	return (0);
}

/*
 *	Routine:	ipc_entry_alloc_name
 *	Purpose:
 *		Allocates/finds an entry with a specific name.
 *		If an existing entry is returned, its type will be nonzero.
 *	Conditions:
 *		The space is not locked before, but it is write-locked after
 *		if the call is successful.  May allocate memory.
 *	Returns:
 *		KERN_SUCCESS		Found existing entry with same name.
 *		KERN_SUCCESS		Allocated a new entry.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
ipc_entry_alloc_name(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	*entryp)
{
	mach_port_name_t newname;
	struct file *fp;
	kern_return_t kr;
	struct thread *td = curthread;

	if (!space->is_active) {
		return (KERN_INVALID_TASK);
	}
	assert(MACH_PORT_NAME_VALID(name));
	is_write_lock(space);
	if ((*entryp = ipc_entry_lookup(space, name)) != NULL)
		return (KERN_SUCCESS);

	is_write_unlock(space);

	/* name could technically be a ridiculously large value */
	if (kern_fdalloc(td, name, &newname)) {
		log(LOG_WARNING, "%s:%d failed to allocate %d\n", __FILE__, __LINE__, name);
		return (KERN_RESOURCE_SHORTAGE);
	}
	if (newname != name) {
		kern_fddealloc(td, newname);
		return (KERN_NAME_EXISTS);
	}
	if (falloc_noinstall(td, &fp)) {
		kern_fddealloc(td, newname);
		return (KERN_RESOURCE_SHORTAGE);
	}
	if (kern_finstall(td, fp, &name, FNOFDALLOC, NULL)) {
		kern_fddealloc(td, newname);
		fdrop(fp, td);
		return (KERN_RESOURCE_SHORTAGE);
	}
	kr = ipc_entry_get(space, 0, &name, entryp);
	if (kr != KERN_SUCCESS) {
		kern_fddealloc(td, newname);
		return (KERN_INVALID_TASK);
	}
	is_write_lock(space);
	return (kr);
}

void
ipc_entry_close(
	ipc_space_t space,
	mach_port_name_t fd)
{
	struct filedesc *fdp;
	struct file *fp;
	struct thread *td;

	td = curthread;
	fdp = td->td_proc->p_fd;

	FILEDESC_XLOCK(fdp);
	if ((fp = fget_noref(fdp, fd)) == NULL) {
		FILEDESC_XUNLOCK(fdp);
		audit_sysclose(td, fd, NULL);
		return;
	}

	audit_sysclose(td, fd, fp);

	/* we deliberately skip closing the knote so that it will
	 * have the last reference to the fp
	 */
	kern_fdfree(fdp, fd);
	FILEDESC_XUNLOCK(fdp);
	fdrop(fp, td);
}

int
ipc_entry_refs(
	ipc_entry_t entry)
{

	return (entry->ie_fp->f_count);
}

void
ipc_entry_add_refs(
	ipc_entry_t entry,
	int delta)
{

	atomic_add_acq_int(&entry->ie_fp->f_count, delta);
}

void
ipc_entry_hold(ipc_entry_t entry)
{
	// FIXME: We should handle errors here
	if (fhold(entry->ie_fp))
		/*log(LOG_WARNING, "%s:%d ipc_entry_hold failed\n", __FILE__, __LINE__)*/ ;
}

/*
 *	Routine:	ipc_entry_dealloc
 *	Purpose:
 *		Deallocates an entry from a space.
 *	Conditions:
 *		The space must be write-locked.
 *		The space is unlocked on return.
 *		The space must be active.
 */

void
ipc_entry_dealloc(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry)
{
	assert(space->is_active);
	assert(entry->ie_object == IO_NULL);
	assert(entry->ie_request == 0);

	if (space != entry->ie_space) {
		is_write_unlock(space);
		is_write_lock(entry->ie_space);
	}
	ipc_entry_hash_delete(space, entry);
	if (space != entry->ie_space) {
		is_write_unlock(entry->ie_space);
	} else {
		is_write_unlock(space);
	}
	MPASS(entry->ie_link == NULL);

	ipc_entry_close(space, name);
}

static void
kern_last_close(struct thread *td, struct file *fp, struct filedesc *fdp, int fd)
{

	FILEDESC_XLOCK(fdp);
	knote_fdclose(td, fd);
	kern_fdfree(fdp, fd);
	FILEDESC_XUNLOCK(fdp);
	fdrop(fp, td);
}

static void
ipc_entry_list_close(void *arg __unused, struct proc *p)
{
	struct filedesc *fdp;
	struct filedescent *fde;
	struct file *fp;
	struct thread *td;
#if 0
	ipc_port_t port;
	ipc_pset_t pset;
	ipc_entry_t entry_tmp;
#endif
	ipc_entry_t entry;
	ipc_space_t space;
	int i;

	fdp = p->p_fd;
	td = curthread;
	space = current_space();

	/* do we want to just return if the refcount is > 1 or should we
	 * bar this from happening in the first place?
	 **/
	KASSERT(fdp->fd_refcnt == 1, ("the fdtable should not be shared"));

	for (i = 0; i <= fdlastfile(fdp); i++) {
		fde = &fdp->fd_ofiles[i];
		fp = fde->fde_file;
		if (fp == NULL || (fp->f_type != DTYPE_MACH_IPC))
			continue;
		MPASS(fp->f_count > 0);

		if (fp->f_data == NULL) {
			log(LOG_WARNING, "%s:%d fd: %d has NULL f_data\n", p->p_comm, p->p_pid, i);
			kern_last_close(td, fp, fdp, i);
			continue;
		}
		entry = fp->f_data;
		MPASS(entry->ie_bits != 0xdeadc0de);
		if ((entry->ie_bits & MACH_PORT_TYPE_PORT_SET) == 0)
			continue;
		kern_last_close(td, fp, fdp, i);
	}

	for (i = 0; i <= fdlastfile(fdp); i++) {

		fde = &fdp->fd_ofiles[i];
		fp = fde->fde_file;
		if (fp == NULL || (fp->f_type != DTYPE_MACH_IPC))
			continue;
		MPASS(fp->f_count > 0);

		entry = fp->f_data;
#if 0
		if (fp->f_count > 1) {
			int ispset = (entry->ie_bits & MACH_PORT_TYPE_PORT_SET);
			log(LOG_WARNING, "%s:%d fd: %d %s refcount: %d\n", p->p_comm, p->p_pid, i,
				ispset ? "pset" : "port", fp->f_count);
		}
#endif
		kern_last_close(td, fp, fdp, i);
	}

#ifdef INVARIANTS
	for (i = 0; i <= fdlastfile(fdp); i++) {
		fde = &fdp->fd_ofiles[i];
		fp = fde->fde_file;
		if (fp != NULL)
			MPASS(fp->f_type != DTYPE_MACH_IPC);
	}
#endif
	/* free unreferenced ipc_entrys */
	i = 0;
	while(!LIST_EMPTY(&space->is_entry_list)) {
		entry = LIST_FIRST(&space->is_entry_list);
		/* mach_port_close removes the entry */
		fp = entry->ie_fp;
		MPASS(fp->f_count > 0);
		fp->f_count = 1;
		fdrop(fp, td);
		/* ensure no infinite loop */
		MPASS(i++ < 10000);
	}
}


static void
ipc_entry_sysinit(void *arg __unused)
{

	EVENTHANDLER_REGISTER(process_exit, ipc_entry_list_close, NULL, EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(process_exec, ipc_entry_list_close, NULL, EVENTHANDLER_PRI_ANY);
}

SYSINIT(ipc_entry, SI_SUB_KLD, SI_ORDER_ANY, ipc_entry_sysinit, NULL);


#define NDFILE		20
#define NDSLOTSIZE	sizeof(NDSLOTTYPE)
#define	NDENTRIES	(NDSLOTSIZE * __CHAR_BIT)
#define NDSLOT(x)	((x) / NDENTRIES)
#define NDBIT(x)	((NDSLOTTYPE)1 << ((x) % NDENTRIES))
#define	NDSLOTS(x)	(((x) + NDENTRIES - 1) / NDENTRIES)


/*
 * Find the highest non-zero bit in the given bitmap, starting at 0 and
 * not exceeding size - 1. Return -1 if not found.
 */
static int
fd_last_used(struct filedesc *fdp, int size)
{
	NDSLOTTYPE *map = fdp->fd_map;
	NDSLOTTYPE mask;
	int off, minoff;

	off = NDSLOT(size);
	if (size % NDENTRIES) {
		mask = ~(~(NDSLOTTYPE)0 << (size % NDENTRIES));
		if ((mask &= map[off]) != 0)
			return (off * NDENTRIES + flsl(mask) - 1);
		--off;
	}
	for (minoff = NDSLOT(0); off >= minoff; --off)
		if (map[off] != 0)
			return (off * NDENTRIES + flsl(map[off]) - 1);
	return (-1);
}

static int
fdisused(struct filedesc *fdp, int fd)
{

	FILEDESC_LOCK_ASSERT(fdp);

	KASSERT(fd >= 0 && fd < fdp->fd_nfiles,
	    ("file descriptor %d out of range (0, %d)", fd, fdp->fd_nfiles));

	return ((fdp->fd_map[NDSLOT(fd)] & NDBIT(fd)) != 0);
}

/*
 * Mark a file descriptor as unused.
 */
static void
fdunused(struct filedesc *fdp, int fd)
{

	FILEDESC_XLOCK_ASSERT(fdp);

	KASSERT(fdisused(fdp, fd), ("fd=%d is already unused", fd));
	KASSERT(fdp->fd_ofiles[fd].fde_file == NULL,
	    ("fd=%d is still in use", fd));

	fdp->fd_map[NDSLOT(fd)] &= ~NDBIT(fd);
	if (fd < fdp->fd_freefile)
		fdp->fd_freefile = fd;
}

static int
kern_fdalloc(struct thread *td, int minfd, int *result)
{
	struct proc *p = td->td_proc;
	struct filedesc *fdp = p->p_fd;
	int rc;
	FILEDESC_XLOCK(fdp);
	rc = fdalloc(td, minfd, result);
	FILEDESC_XUNLOCK(fdp);
	return (rc);
}

static void
kern_fddealloc(struct thread *td, int fd)
{
	struct proc *p = td->td_proc;
	struct filedesc *fdp = p->p_fd;
	FILEDESC_XLOCK(fdp);
	fdunused(fdp, fd);
	FILEDESC_XUNLOCK(fdp);
}

static inline void
kern_fdfree(struct filedesc *fdp, int fd)
{
	struct filedescent *fde;

	fde = &fdp->fd_ofiles[fd];
#ifdef CAPABILITIES
	seqc_write_begin(&fde->fde_seqc);
#endif
	bzero(fde, fde_change_size);
	fdunused(fdp, fd);
#ifdef CAPABILITIES
	seqc_write_end(&fde->fde_seqc);
#endif
}

static void
filecaps_fill(struct filecaps *fcaps)
{

	CAP_ALL(&fcaps->fc_rights);
	fcaps->fc_ioctls = NULL;
	fcaps->fc_nioctls = -1;
	fcaps->fc_fcntls = CAP_FCNTL_ALL;
}

/*
 * Install a file in a file descriptor table.
 */
static int
kern_finstall(struct thread *td, struct file *fp, int *fd, int flags,
    struct filecaps *fcaps)
{
	struct filedesc *fdp = td->td_proc->p_fd;
	struct filedescent *fde;
	int error, min;

	KASSERT(fd != NULL, ("%s: fd == NULL", __func__));
	KASSERT(fp != NULL, ("%s: fp == NULL", __func__));

	min = 16;

	FILEDESC_XLOCK(fdp);
	if (!(flags & FNOFDALLOC)) {
		if ((error = fdalloc(td, min, fd))) {
			FILEDESC_XUNLOCK(fdp);
			return (error);
		}
	}
	if (!fhold(fp)) {
		FILEDESC_XUNLOCK(fdp);
		return (1); // Caller will fddealloc and fdrop
	}
	fde = &fdp->fd_ofiles[*fd];
#ifdef CAPABILITIES
	seqc_write_begin(&fde->fde_seqc);
#endif
	fde->fde_file = fp;
	if ((flags & O_CLOEXEC) != 0)
		fde->fde_flags |= UF_EXCLOSE;
	filecaps_fill(&fde->fde_caps);
#ifdef CAPABILITIES
	seqc_write_end(&fde->fde_seqc);
#endif
	FILEDESC_XUNLOCK(fdp);
	return (0);
}
 
#if	MACH_KDB
#include <ddb/db_output.h>
#define	printf	kdbprintf

ipc_entry_t	db_ipc_object_by_name(
			task_t		task,
			mach_port_name_t	name);


ipc_entry_t
db_ipc_object_by_name(
	task_t		task,
	mach_port_name_t	name)
{
        ipc_space_t space = task->itk_space;
        ipc_entry_t entry;
 
 
        entry = ipc_entry_lookup(space, name);
        if(entry != IE_NULL) {
                iprintf("(task 0x%x, name 0x%x) ==> object 0x%x\n",
			task, name, entry->ie_object);
                return (ipc_entry_t) entry->ie_object;
        }
        return entry;
}
#endif	/* MACH_KDB */
