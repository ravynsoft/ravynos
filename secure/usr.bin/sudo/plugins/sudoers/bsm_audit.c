/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2015 Todd C. Miller <Todd.Miller@sudo.ws>
 * Copyright (c) 2009 Christian S.J. Peron
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#ifdef HAVE_BSM_AUDIT

#include <sys/types.h>		/* for pid_t */

#include <bsm/audit.h>
#include <bsm/libbsm.h>
#include <bsm/audit_uevents.h>

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include <sudoers.h>
#include <bsm_audit.h>

/*
 * Solaris auditon() returns EINVAL if BSM audit not configured.
 * OpenBSM returns ENOSYS for unimplemented options.
 */
#ifdef __sun
# define AUDIT_NOT_CONFIGURED	EINVAL
#else
# define AUDIT_NOT_CONFIGURED	ENOSYS
#endif

#ifdef __FreeBSD__
# define BSM_AUDIT_COMPAT
#endif

static au_event_t sudo_audit_event = AUE_sudo;

static int
audit_sudo_selected(int sorf)
{
	auditinfo_addr_t ainfo_addr;
	struct au_mask *mask;
	int rc;
	debug_decl(audit_sudo_selected, SUDOERS_DEBUG_AUDIT);

	if (getaudit_addr(&ainfo_addr, sizeof(ainfo_addr)) < 0) {
#ifdef BSM_AUDIT_COMPAT
		if (errno == ENOSYS) {
			auditinfo_t ainfo;

			/* Fall back to older BSM API. */
			if (getaudit(&ainfo) < 0) {
				sudo_warn("getaudit");
				debug_return_int(-1);
			}
			mask = &ainfo.ai_mask;
		} else
#endif /* BSM_AUDIT_COMPAT */
		{
			sudo_warn("getaudit_addr");
			debug_return_int(-1);
		}
        } else {
		mask = &ainfo_addr.ai_mask;
	}
	rc = au_preselect(sudo_audit_event, mask, sorf, AU_PRS_REREAD);
	if (rc == -1) {
#if defined(__APPLE__) && defined(AUE_DARWIN_sudo)
	    /*
	     * Mac OS X 10.10 au_preselect() only accepts AUE_DARWIN_sudo.
	     */
	    sudo_audit_event = AUE_DARWIN_sudo;
	    rc = au_preselect(sudo_audit_event, mask, sorf, AU_PRS_REREAD);
	    if (rc == -1)
#endif

		sudo_warn("au_preselect");
	}
        debug_return_int(rc);
}

/*
 * Returns 0 on success or -1 on error.
 */
int
bsm_audit_success(const struct sudoers_context *ctx, char *const exec_args[])
{
	auditinfo_addr_t ainfo_addr;
	token_t *tok;
	au_id_t auid;
	long au_cond;
	int aufd, selected;
	debug_decl(bsm_audit_success, SUDOERS_DEBUG_AUDIT);

	/*
	 * If we are not auditing, don't cut an audit record; just return.
	 */
	if (auditon(A_GETCOND, (caddr_t)&au_cond, sizeof(long)) < 0) {
		if (errno == AUDIT_NOT_CONFIGURED)
			debug_return_int(0);
		sudo_warn("%s", U_("Could not determine audit condition"));
		debug_return_int(-1);
	}
	if (au_cond == AUC_NOAUDIT)
		debug_return_int(0);
	/*
	 * Check to see if the preselection masks are interested in seeing
	 * this event.
	 */
	selected = audit_sudo_selected(AU_PRS_SUCCESS);
	if (selected != 1)
		debug_return_int(!selected ? 0 : -1);
	if (getauid(&auid) < 0) {
		sudo_warn("getauid");
		debug_return_int(-1);
	}
	if ((aufd = au_open()) == -1) {
		sudo_warn("au_open");
		debug_return_int(-1);
	}
	if (getaudit_addr(&ainfo_addr, sizeof(ainfo_addr)) == 0) {
		tok = au_to_subject_ex(auid, ctx->user.euid, ctx->user.egid,
		    ctx->user.uid, ctx->user.gid, ctx->user.pid, ctx->user.pid,
		    &ainfo_addr.ai_termid);
#ifdef BSM_AUDIT_COMPAT
	} else if (errno == ENOSYS) {
		auditinfo_t ainfo;

		/*
		 * NB: We should probably watch out for ERANGE here.
		 */
		if (getaudit(&ainfo) < 0) {
			sudo_warn("getaudit");
			debug_return_int(-1);
		}
		tok = au_to_subject(auid, ctx->user.euid, ctx->user.egid,
		    ctx->user.uid, ctx->user.gid, ctx->user.pid, ctx->user.pid,
		    &ainfo.ai_termid);
#endif /* BSM_AUDIT_COMPAT */
	} else {
		sudo_warn("getaudit_addr");
		debug_return_int(-1);
	}
	if (tok == NULL) {
		sudo_warn("au_to_subject");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
	tok = au_to_exec_args((char **)exec_args);
	if (tok == NULL) {
		sudo_warn("au_to_exec_args");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
	tok = au_to_return32(0, 0);
	if (tok == NULL) {
		sudo_warn("au_to_return32");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
#ifdef HAVE_AU_CLOSE_SOLARIS11
	if (au_close(aufd, 1, sudo_audit_event, 0) == -1)
#else
	if (au_close(aufd, 1, sudo_audit_event) == -1)
#endif
	{
		sudo_warn("%s", U_("unable to commit audit record"));
		debug_return_int(-1);
	}
	debug_return_int(0);
}

/*
 * Returns 0 on success or -1 on error.
 */
int
bsm_audit_failure(const struct sudoers_context *ctx, char *const exec_args[],
    const char *errmsg)
{
	auditinfo_addr_t ainfo_addr;
	token_t *tok;
	long au_cond;
	au_id_t auid;
	int aufd;
	debug_decl(bsm_audit_failure, SUDOERS_DEBUG_AUDIT);

	/*
	 * If we are not auditing, don't cut an audit record; just return.
	 */
	if (auditon(A_GETCOND, (caddr_t)&au_cond, sizeof(long)) < 0) {
		if (errno == AUDIT_NOT_CONFIGURED)
			debug_return_int(0);
		sudo_warn("%s", U_("Could not determine audit condition"));
		debug_return_int(-1);
	}
	if (au_cond == AUC_NOAUDIT)
		debug_return_int(0);
	if (!audit_sudo_selected(AU_PRS_FAILURE))
		debug_return_int(0);
	if (getauid(&auid) < 0) {
		sudo_warn("getauid");
		debug_return_int(-1);
	}
	if ((aufd = au_open()) == -1) {
		sudo_warn("au_open");
		debug_return_int(-1);
	}
	if (getaudit_addr(&ainfo_addr, sizeof(ainfo_addr)) == 0) { 
		tok = au_to_subject_ex(auid, ctx->user.euid, ctx->user.egid,
		    ctx->user.uid, ctx->user.gid, ctx->user.pid, ctx->user.pid,
		    &ainfo_addr.ai_termid);
#ifdef BSM_AUDIT_COMPAT
	} else if (errno == ENOSYS) {
		auditinfo_t ainfo;

		if (getaudit(&ainfo) < 0) {
			sudo_warn("getaudit");
			debug_return_int(-1);
		}
		tok = au_to_subject(auid, ctx->user.euid, ctx->user.egid,
		    ctx->user.uid, ctx->user.gid, ctx->user.pid, ctx->user.pid,
		    &ainfo.ai_termid);
#endif /* BSM_AUDIT_COMPAT */
	} else {
		sudo_warn("getaudit_addr");
		debug_return_int(-1);
	}
	if (tok == NULL) {
		sudo_warn("au_to_subject");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
	tok = au_to_exec_args((char **)exec_args);
	if (tok == NULL) {
		sudo_warn("au_to_exec_args");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
	tok = au_to_text((char *)errmsg);
	if (tok == NULL) {
		sudo_warn("au_to_text");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
	tok = au_to_return32(EPERM, 1);
	if (tok == NULL) {
		sudo_warn("au_to_return32");
		debug_return_int(-1);
	}
	au_write(aufd, tok);
#ifdef HAVE_AU_CLOSE_SOLARIS11
	if (au_close(aufd, 1, sudo_audit_event, PAD_FAILURE) == -1)
#else
	if (au_close(aufd, 1, sudo_audit_event) == -1)
#endif
	{
		sudo_warn("%s", U_("unable to commit audit record"));
		debug_return_int(-1);
	}
	debug_return_int(0);
}

#endif /* HAVE_BSM_AUDIT */
