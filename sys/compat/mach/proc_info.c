/*-
 * Copyright (c) 2014-2015, Matthew Macy <mmacy@nextbsd.org>
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/procctl.h>
#include <sys/ptrace.h>
#include <sys/rwlock.h>
#include <sys/syscallsubr.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/vnode.h>
#include <sys/sx.h>
#include <sys/signalvar.h>
#include <sys/tty.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_param.h>

#include <sys/mach/mach_types.h>
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/thread.h>
#define PRIVATE
#include <sys/proc_info.h>


int set_security_token(task_t);


static int
proc_terminate(int pid)
{
	int err;
	struct proc *p;

	err = 0;
	if (pid <= 0)
		return (EINVAL);
	if (pid == curproc->p_pid)
		return (EPERM);
	if ((p = pfind(pid)) == NULL)
		return (ESRCH);
	if ((err = p_cansignal(curthread, p, SIGKILL)) == 0) {
		/* p_cansignal returns 0 if you can, or an error if you can't. */
		kern_psignal(p, SIGTERM);
	}
	PROC_UNLOCK(p);
	return (err);
}

static int
proc_listpids(uint32_t type, uint32_t flavor, void *ubuffer, uint32_t buffersize)
{
	struct proc *p;
	struct proclist *list;
	struct pgrp *pgrpp;	
	int err, count, incr, maxpidcount, numpids;
	pid_t *buf, *ptr;
	uid_t uid;
	struct thread *td;

	if (type == PROC_TTY_ONLY)
		return (EINVAL);

	td = curthread;
	if (ubuffer == NULL) {
		td->td_retval[0] = (nprocs + 30)*sizeof(pid_t);
		return (0);
	}	
	if (buffersize < sizeof(pid_t))
		return (ENOMEM);

	maxpidcount = buffersize/sizeof(pid_t);
	numpids = nprocs + 30;
	err = count = 0;
	if (numpids > maxpidcount)
		numpids = maxpidcount;

	buf = malloc(numpids *sizeof(pid_t), M_DEVBUF, M_WAITOK);

	ptr = buf;
	if (type == PROC_PGRP_ONLY) {
		count = 0;
		pgrpp = pgfind(flavor);
		if (pgrpp == NULL)
			return (ENOENT);
		list = (struct proclist *)&pgrpp->pg_members;
		LIST_FOREACH(p, list, p_pglist) {
			if (count == numpids)
				break;
			*ptr++ = p->p_pid;
			count++;
		}
		PGRP_UNLOCK(pgrpp);
		goto done;
	}

	list = &allproc;
	sx_slock(&allproc_lock);
scan_next:	
	LIST_FOREACH(p, list, p_list) {
		incr = 0;
		switch (type) {
		case PROC_ALL_PIDS:
			incr = 1;
			break;
		case PROC_PPID_ONLY:
			if (((p->p_pptr && (p->p_pptr->p_pid == flavor)) &&
				 ((p->p_flag & P_TRACED) == 0)) ||
				p->p_oppid == flavor)
				incr = 1;
			break;
		case PROC_UID_ONLY:
			if (p->p_ucred != NULL) {
				uid = p->p_ucred->cr_uid;
				if (uid == flavor)
					incr = 1;
			}
			break;
		case PROC_RUID_ONLY:
			if (p->p_ucred != NULL) {
				uid = p->p_ucred->cr_ruid;
				if (uid == flavor)
					incr = 1;
			}
			break;
		default:
			err = EINVAL;
			goto done;
		}
		if (incr) {
			*ptr++ = p->p_pid;
			count++;
		}
		if (count >= numpids)
			break;
	}
	if ((count < numpids) && (list == &allproc)) {
		list = &zombproc;
		goto scan_next;
	}
done:
	sx_sunlock(&allproc_lock);
	if (err)
		return (err);

	err = copyout(buf, ubuffer, count * sizeof(pid_t));
	if (err == 0)
		td->td_retval[0] = count * sizeof(pid_t);
	free(buf, M_DEVBUF);

	return (err);
}

static int
proc_pidbsdinfo(struct proc *p, struct proc_bsdinfo * pbsd, int zombie)
{
	register struct tty *tp;
	struct  session *sessionp = NULL;
	struct pgrp * pg;
	struct ucred *cred;

	pg = p->p_pgrp;
	if (pg)
		sessionp = pg->pg_session;

	bzero(pbsd, sizeof(struct proc_bsdinfo));
	pbsd->pbi_status = p->p_state;
	pbsd->pbi_xstatus = KW_EXITCODE(p->p_xexit, p->p_xsig);
	pbsd->pbi_pid = p->p_pid;
	if (p->p_pptr)
		pbsd->pbi_ppid = p->p_pptr->p_pid;
	crhold(p->p_ucred);
	cred = p->p_ucred;
	pbsd->pbi_uid = cred->cr_uid;
	pbsd->pbi_gid = cred->cr_gid;
	pbsd->pbi_ruid =  cred->cr_ruid;
	pbsd->pbi_rgid = cred->cr_rgid;
	pbsd->pbi_svuid =  cred->cr_svuid;
	pbsd->pbi_svgid = cred->cr_svgid;
	crfree(cred);
	
	pbsd->pbi_nice = p->p_nice;
#ifdef notyet	
	pbsd->pbi_start_tvsec = p->p_start.tv_sec;
	pbsd->pbi_start_tvusec = p->p_start.tv_usec;
#endif	
	bcopy(&p->p_comm, &pbsd->pbi_comm[0], MAXCOMLEN);
	pbsd->pbi_comm[MAXCOMLEN - 1] = '\0';
	pbsd->pbi_flags = 0;	
	if ((p->p_flag & P_SYSTEM) == P_SYSTEM) 
		pbsd->pbi_flags |= PROC_FLAG_SYSTEM;
	if ((p->p_flag & P_TRACED) == P_TRACED)
		pbsd->pbi_flags |= PROC_FLAG_TRACED;
	if ((p->p_flag & P_WEXIT) == P_WEXIT) 
		pbsd->pbi_flags |= PROC_FLAG_INEXIT;
	if ((p->p_flag & P_PPWAIT) == P_PPWAIT) 
		pbsd->pbi_flags |= PROC_FLAG_PPWAIT;
#ifdef notyet	
	if ((p->p_flag & P_LP64) == P_LP64)
#endif
#ifdef __LP64__		
		pbsd->pbi_flags |= PROC_FLAG_LP64;
#endif	
	if ((p->p_flag & P_CONTROLT) == P_CONTROLT) 
		pbsd->pbi_flags |= PROC_FLAG_CONTROLT;
	if ((p->p_flag & P_SUGID) == P_SUGID) 
		pbsd->pbi_flags |= PROC_FLAG_PSUGID;
	if ((p->p_flag & P_EXEC) == P_EXEC) 
		pbsd->pbi_flags |= PROC_FLAG_EXEC;

	if (sessionp != NULL) {
		if (p == sessionp->s_leader)
			pbsd->pbi_flags |= PROC_FLAG_SLEADER;
		if (sessionp->s_ttyvp)
			pbsd->pbi_flags |= PROC_FLAG_CTTY;
	}
		
	if (zombie == 0)
		pbsd->pbi_nfiles = p->p_fd->fd_nfiles;
	
	pbsd->e_tdev = NODEV;
	if (pg != NULL) {
		pbsd->pbi_pgid = pg->pg_id;
		pbsd->pbi_pjobc = pg->pg_jobc;
		if ((p->p_flag & P_CONTROLT) && (sessionp != NULL) && (tp = sessionp->s_ttyp)) {
#ifdef notyet
			pbsd->e_tdev = tp->t_dev;
#endif
			if (tp->t_pgrp)
				pbsd->e_tpgid = tp->t_pgrp->pg_id;
		}
	} 
	return(0);
}

static int
proc_pidshortbsdinfo(struct proc *p, struct proc_bsdshortinfo * pbsd_shortp, int zombie)
{
	struct ucred *cred;
	
	bzero(pbsd_shortp, sizeof(struct proc_bsdshortinfo));
	pbsd_shortp->pbsi_pid = p->p_pid;
	if (p->p_pptr)
		pbsd_shortp->pbsi_ppid = p->p_pptr->p_pid;
	if (p->p_pgrp)
	pbsd_shortp->pbsi_pgid = p->p_pgrp->pg_id;
	pbsd_shortp->pbsi_status = p->p_state;
	bcopy(&p->p_comm, &pbsd_shortp->pbsi_comm[0], MAXCOMLEN);
	pbsd_shortp->pbsi_comm[MAXCOMLEN - 1] = '\0';

	pbsd_shortp->pbsi_flags = 0;	
	if ((p->p_flag & P_SYSTEM) == P_SYSTEM) 
		pbsd_shortp->pbsi_flags |= PROC_FLAG_SYSTEM;
	if ((p->p_flag & P_TRACED) == P_TRACED)
		pbsd_shortp->pbsi_flags |= PROC_FLAG_TRACED;
	if ((p->p_flag & P_WEXIT) == P_WEXIT) 
		pbsd_shortp->pbsi_flags |= PROC_FLAG_INEXIT;
	if ((p->p_flag & P_PPWAIT) == P_PPWAIT) 
		pbsd_shortp->pbsi_flags |= PROC_FLAG_PPWAIT;
#ifdef notyet
	if ((p->p_flag & P_LP64) == P_LP64)
#endif		
#ifdef __LP64__
		pbsd_shortp->pbsi_flags |= PROC_FLAG_LP64;
#endif	
	if ((p->p_flag & P_CONTROLT) == P_CONTROLT) 
		pbsd_shortp->pbsi_flags |= PROC_FLAG_CONTROLT;
	if ((p->p_flag & P_SUGID) == P_SUGID) 
		pbsd_shortp->pbsi_flags |= PROC_FLAG_PSUGID;
	if ((p->p_flag & P_EXEC) == P_EXEC) 
		pbsd_shortp->pbsi_flags |= PROC_FLAG_EXEC;

	crhold(p->p_ucred);
	cred = p->p_ucred;
	pbsd_shortp->pbsi_uid = cred->cr_uid;
	pbsd_shortp->pbsi_gid = cred->cr_gid;
	pbsd_shortp->pbsi_ruid =  cred->cr_ruid;
	pbsd_shortp->pbsi_rgid = cred->cr_rgid;
	pbsd_shortp->pbsi_svuid =  cred->cr_svuid;
	pbsd_shortp->pbsi_svgid = cred->cr_svgid;
	crfree(cred);
	
	return(0);
}

static uint64_t
proc_puniqueid(struct proc *p)
{
	task_t task;
	
	task = p->p_machdata;
	return (task->itk_puniqueid);
}

static uint64_t
proc_uniqueid(struct proc *p)
{
	task_t task;

	task = p->p_machdata;
	return (task->itk_uniqueid);
}

static void
proc_getexecutableuuid(struct proc *p __unused, unsigned char *uuid, int size)
{

	bzero(uuid, size);
}

static void
proc_piduniqidentifierinfo(struct proc *p, struct proc_uniqidentifierinfo *p_uniqidinfo)
{
	p_uniqidinfo->p_uniqueid = proc_uniqueid(p);
	proc_getexecutableuuid(p, (unsigned char *)&p_uniqidinfo->p_uuid, sizeof(p_uniqidinfo->p_uuid));
	p_uniqidinfo->p_puniqueid = proc_puniqueid(p);
	p_uniqidinfo->p_reserve2 = 0;
	p_uniqidinfo->p_reserve3 = 0;
	p_uniqidinfo->p_reserve4 = 0;
}

static int
proc_pidinfo(int pid, int flavor, uint64_t arg, void *buffer, uint32_t  buffersize)
{
	struct proc *p;
	int err, size, findzomb, iszomb, shortversion, uniqidversion;
	void *kbuf;
	
	switch (flavor) {
	case PROC_PIDTBSDINFO:
		size = PROC_PIDTBSDINFO_SIZE;
		break;
	case PROC_PIDT_SHORTBSDINFO:
		size = PROC_PIDT_SHORTBSDINFO_SIZE;
		break;
	case PROC_PIDUNIQIDENTIFIERINFO:
		size = PROC_PIDUNIQIDENTIFIERINFO_SIZE;
		break;
	case PROC_PIDT_BSDINFOWITHUNIQID:
		size = PROC_PIDT_BSDINFOWITHUNIQID_SIZE;
		break;
	default:
		return (EINVAL);
	}

	if (buffersize < size)
		return (ENOMEM);

	kbuf = malloc(size, M_DEVBUF, M_WAITOK);
	uniqidversion = shortversion = err = findzomb = iszomb = 0;
	if ((flavor == PROC_PIDTBSDINFO) || (flavor == PROC_PIDT_SHORTBSDINFO) || (flavor == PROC_PIDT_BSDINFOWITHUNIQID) 
	    || (flavor == PROC_PIDUNIQIDENTIFIERINFO)) {
		if (arg)
			findzomb = 1;
	}
	if ((p = pfind(pid)) == NULL) {
		if (!findzomb || ((p = zpfind(pid)) == NULL)) {
			err = ESRCH;
			goto done;
		}
		iszomb = 1;
	}

	switch (flavor) {
	case PROC_PIDUNIQIDENTIFIERINFO: {
		struct proc_uniqidentifierinfo p_uniqidinfo;
		
		proc_piduniqidentifierinfo(p, &p_uniqidinfo);
		bcopy(&p_uniqidinfo, kbuf, sizeof(struct proc_uniqidentifierinfo));
	}
		break;
	case PROC_PIDT_SHORTBSDINFO:
		shortversion = 1;
	case PROC_PIDT_BSDINFOWITHUNIQID: 
	case PROC_PIDTBSDINFO: {
		struct proc_bsdinfo pbsd;
		struct proc_bsdshortinfo pbsd_short;
		struct proc_bsdinfowithuniqid pbsd_uniqid;
		
		if (flavor == PROC_PIDT_BSDINFOWITHUNIQID)
			uniqidversion = 1;
			
		if (shortversion != 0) {
			err = proc_pidshortbsdinfo(p, &pbsd_short, iszomb);
		} else {
			err = proc_pidbsdinfo(p, &pbsd, iszomb);
			if (uniqidversion != 0) { 
				proc_piduniqidentifierinfo(p, &pbsd_uniqid.p_uniqidentifier);
				pbsd_uniqid.pbsd = pbsd;
			}
		}
		
		if (err == 0) {
			if (shortversion != 0) {
				bcopy(&pbsd_short, kbuf, sizeof(struct proc_bsdshortinfo));
			} else if (uniqidversion != 0) {
				bcopy(&pbsd_uniqid, kbuf, sizeof(struct proc_bsdinfowithuniqid));
			} else {
				bcopy(&pbsd, kbuf, sizeof(struct proc_bsdinfo));
			}
		}	
	}
		break;
	default:
		err = ENOTSUP;
	}
	PROC_UNLOCK(p);
	if (err == 0)
		err = copyout(kbuf, buffer, size);
	if (err == 0)
		curthread->td_retval[0] = size;
done:
	free(kbuf, M_DEVBUF);
	return (err);
}

static int
proc_info(int op, pid_t pid, uint32_t flavor, uint64_t arg, void *addr,
		  uint32_t buffersize)
{

	switch (op) {
	case PROC_INFO_CALL_LISTPIDS:
		return (proc_listpids(pid, flavor, addr, buffersize));
		break;
	case PROC_INFO_CALL_TERMINATE:
		return (proc_terminate(pid));
		break;
	case PROC_INFO_CALL_PIDINFO:
		return (proc_pidinfo(pid, flavor, arg, addr, buffersize));
		break;
	default:
		return (EOPNOTSUPP);
	}
	/* NOT REACHED */
	return (0);
}

int
set_security_token(task_t task)
{
	struct proc *p;
	security_token_t sec_token;
	audit_token_t audit_token;

	p = task->itk_p;

	sec_token.val[0] = sec_token.val[1] = 0;
	audit_token.val[0] = 0; /* wat: p->p_ucred->cr_au.ai_auid; */
	audit_token.val[1] = p->p_ucred->cr_uid;
	audit_token.val[2] = p->p_ucred->cr_gid;
	audit_token.val[3] = p->p_ucred->cr_ruid;
	audit_token.val[4] = p->p_ucred->cr_rgid;
	audit_token.val[5] = p->p_pid;
	audit_token.val[6] = 0; /* wat: p->p_ucred->cr_au.ai_asid; */
	audit_token.val[7] = 0; /* wat: p->p_ucred->cr_au.ai_termid.port; */

	task->audit_token = audit_token;
	task->sec_token = sec_token;

	return (0);
}

int
sys___proc_info(struct thread *td __unused, struct __proc_info_args *uap)
{

	return (proc_info(uap->callnum, uap->pid, uap->flavor, uap->arg, uap->buffer,
					  uap->buffersize));
}


int
sys___iopolicysys(struct thread *td __unused, struct __iopolicysys_args *uap __unused)
{
 
	return (ENOSYS);
}
