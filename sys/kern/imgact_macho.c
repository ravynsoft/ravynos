/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2021 Zoe Knox <zoe@ravynsoft.com> 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/exec.h>
#include <sys/imgact.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/racct.h>
#include <sys/resourcevar.h>
#include <sys/signalvar.h>
#include <sys/syscall.h>
#include <sys/sysent.h>
#include <sys/systm.h>
#include <sys/vnode.h>

#include <machine/frame.h>
#include <machine/md_var.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_param.h>

#include <mach-o/loader.h>

static int	exec_macho_imgact(struct image_params *imgp);
int		exec_elf64_imgact(struct image_params *imgp);

static int
exec_macho_imgact(struct image_params *imgp)
{
    const struct mach_header_64 *mh = (const struct mach_header_64 *)imgp->image_header;
    const char *path = NULL; 
    uint32_t pathlen = 0;
    char *d;

    /* We currently only handle thin 64-bit executables */
    if (mh->magic != MH_MAGIC_64 ||
	mh->filetype != MH_EXECUTE ||
	(mh->cputype != CPU_TYPE_X86_64 && 
	mh->cputype != (uint32_t)CPU_TYPE_ANY))
	return -1;

    /* For now, MachO is really a special case of binfmt_misc. When
     * a suitable binary is exec'd, look for its DYLD and actually
     * run that with the original path in argv[0].
     */
    uint32_t offset = sizeof(struct mach_header_64);
    for(int i = 0; i < mh->ncmds; ++i) {
	const struct load_command *lc = (const struct
	    load_command *)((uint64_t)mh + offset);
	switch(lc->cmd) {
	    case LC_LOAD_DYLINKER: {
		const struct dylinker_command *dl = (const struct dylinker_command *)lc;
		path = (const char *)((uint64_t)lc + dl->name.offset);
		break;
	    }
	    default:
		break;
	}
	offset += lc->cmdsize;
	if(offset > PAGE_SIZE)
	    return -1;
    }

    if (!path)
	return ENOEXEC;

    pathlen = strlen(path);
    offset = pathlen + 1;
    if (offset > imgp->args->stringspace)
	return E2BIG;

    bcopy(imgp->args->begin_argv, imgp->args->begin_argv + offset,
	imgp->args->endp - imgp->args->begin_argv);

    imgp->args->begin_envv += offset;
    imgp->args->endp += offset;
    imgp->args->stringspace -= offset;
    imgp->args->argc++;
    d = imgp->args->begin_argv;
    memcpy(d, path, pathlen);
    d += pathlen;
    *d++ = '\0';

    imgp->interpreter_name = imgp->args->begin_argv;
    imgp->interpreted |= IMGACT_BINMISC;

    return 0;
}

/*
* Tell kern_execve.c about it, with a little help from the linker.
*/
static struct execsw macho_execsw = {
    .ex_imgact = exec_macho_imgact,
    .ex_name = "macho"
};
EXEC_SET(macho, macho_execsw);
