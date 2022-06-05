/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2021-2022 Zoe Knox <zoe@ravynsoft.com> 
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

static int  macho_fixup(uintptr_t *stack_base, struct image_params *imgp);
static int	exec_macho_imgact(struct image_params *imgp);

extern const char _binary_elf_vdso_so_1_start[];
extern const char _binary_elf_vdso_so_1_end[];
extern char _binary_elf_vdso_so_1_size;
extern const char *syscallnames[];

static int macho_szsigcode;

#if defined(__amd64__)

#define __MAXUSER              VM_MAXUSER_ADDRESS_LA48
#define __USRSTACK             USRSTACK_LA48
#define __FORK_RETVAL          x86_set_fork_retval

#elif defined(__arm64__) || defined(__aarch64__)

// from sys/arm64/arm64/elf32_machdep.c
#define FREEBSD32_MAXUSER      ((1ul << 32) - PAGE_SIZE)

#define __MAXUSER              VM_MAXUSER_ADDRESS
#define __USRSTACK             (FREEBSD32_MAXUSER - PAGE_SIZE)
#define __FORK_RETVAL          null_set_fork_retval
extern void null_set_fork_retval(struct thread *td);

#else
#error "unhandled architecture"
#endif

struct sysentvec macho_sysvec = {
	.sv_size	= SYS_MAXSYSCALL,
	.sv_table	= sysent,
	.sv_fixup	= macho_fixup,
	.sv_sendsig	= sendsig,
	.sv_sigcode	= _binary_elf_vdso_so_1_start,
	.sv_szsigcode	= &macho_szsigcode,
	.sv_name	= "FreeBSD MachO64",
	.sv_coredump	= NULL,
	.sv_imgact_try	= NULL,
	.sv_minsigstksz	= MINSIGSTKSZ,
	.sv_minuser	= VM_MIN_ADDRESS,
	.sv_maxuser	= __MAXUSER,
	.sv_usrstack	= __USRSTACK,
	.sv_psstrings	= 0, //PS_STRINGS_LA48,
	.sv_psstringssz	= 0, //sizeof(struct ps_strings),
	.sv_stackprot	= VM_PROT_ALL,
	.sv_copyout_strings	= exec_copyout_strings,
	.sv_setregs	= exec_setregs,
	.sv_fixlimit	= NULL,
	.sv_maxssiz	= NULL,
	.sv_flags	= SV_ABI_FREEBSD | SV_LP64,
	.sv_set_syscall_retval = cpu_set_syscall_retval,
	.sv_fetch_syscall_args = cpu_fetch_syscall_args,
	.sv_syscallnames = syscallnames,
	.sv_onexec_old	= exec_onexec_old,
	.sv_onexit	= exit_onexit,
	.sv_set_fork_retval = __FORK_RETVAL,
};

static void
macho_sysent(void *arg __unused)
{
	macho_szsigcode = (int)(uintptr_t)&_binary_elf_vdso_so_1_size;
}
SYSINIT(macho_sysent, SI_SUB_EXEC, SI_ORDER_ANY, macho_sysent, NULL);

static int
macho_fixup(uintptr_t *stack_base, struct image_params *imgp)
{
        *stack_base -= sizeof(uintptr_t);
        if (suword((void *)stack_base, imgp->args->argc) != 0)
                return (EFAULT);
        *stack_base += sizeof(uintptr_t);
        return (0);
}

static int
exec_macho_imgact(struct image_params *imgp)
{
    const struct mach_header_64 *mh = (const struct mach_header_64 *)imgp->image_header;
    const char *path = NULL;
    struct vmspace *vmspace;
    vm_map_t map;
    vm_object_t object;
    vm_offset_t text_end, entry_addr;
    unsigned int found_offset = 0;
    unsigned long virtual_offset = 0;
    unsigned long file_offset = 0;
    //unsigned long bss_size = 0;
    unsigned long total_size = 0;
    int error;


    /* We currently only handle thin 64-bit executables */
    if (mh->magic != MH_MAGIC_64 ||	mh->filetype != MH_EXECUTE ||
        (mh->cputype != CPU_TYPE_X86_64 && mh->cputype != (uint32_t)CPU_TYPE_ANY))
        return -1;

    uint32_t offset = sizeof(struct mach_header_64);
    for(int i = 0; i < mh->ncmds; ++i) {
        const struct load_command *lc = (const struct load_command *)((uint64_t)mh + offset);
        switch(lc->cmd) {
            case LC_LOAD_DYLINKER: {
                const struct dylinker_command *dl = (const struct dylinker_command *)lc;
                path = (const char *)((uint64_t)lc + dl->name.offset);
                break;
            }

            case LC_SEGMENT_64: {
                const struct segment_command_64 *ls = (const struct segment_command_64 *)lc;
                printf("LC_SEGMENT_64 %s at %lx (%d) sz %lx\n", ls->segname, ls->vmaddr, offset, ls->vmsize);
                if(strcmp(SEG_PAGEZERO, ls->segname)) {
                    if(!found_offset) {
                        virtual_offset = trunc_page(ls->vmaddr);
                        file_offset = offset;
                        found_offset = 1;
                        printf("virtual_offset = 0x%lx\n", virtual_offset);
                    }
                    total_size += ls->vmsize;
                }
                break;
            }

            case LC_MAIN: {
                const struct entry_point_command *lm = (const struct entry_point_command *)lc;
                printf("LC_MAIN entry point %lx\n",lm->entryoff);
                entry_addr = lm->entryoff;
                break;
            }

            default:
                break;
        }
        offset += lc->cmdsize;
        if(offset > 4*PAGE_SIZE)
            return -1;
    }

    if (!path) {
        printf("No DYLINKER - loading directly\n");
    } else {
        printf("Loading %s from DYLINKER section\n", path);
    }

    VOP_UNLOCK(imgp->vp);
    error = exec_new_vmspace(imgp, &macho_sysvec);
    vn_lock(imgp->vp, LK_SHARED | LK_RETRY);
    printf("exec_new_vmspace = %d, vmspace = %p\n", error, imgp->proc->p_vmspace);

    if(error)
        return error;

    vmspace = imgp->proc->p_vmspace;
    object = imgp->object;
    map = &vmspace->vm_map;
    vm_map_lock(map);
    vm_object_reference(object);
    printf("referenced object\n");

    text_end = virtual_offset + total_size;
    error = vm_map_insert(map, object,
        file_offset,
        virtual_offset, text_end,
        VM_PROT_READ | VM_PROT_EXECUTE, VM_PROT_ALL,
        MAP_COPY_ON_WRITE | MAP_PREFAULT | MAP_VN_EXEC);
    printf("vm_map_insert = %d\n", error);
    if (error) {
        vm_map_unlock(map);
        vm_object_deallocate(object);
        return (error);
    }
    VOP_SET_TEXT_CHECKED(imgp->vp);

    vm_map_unlock(map);
    printf("mapped object foff %lx vmaddr %lx end %lx entry %lx\n",
        file_offset, virtual_offset, text_end, entry_addr);

    /* Fill in process VM information */
    vmspace->vm_tsize = total_size >> PAGE_SHIFT;
    vmspace->vm_dsize = 0 >> PAGE_SHIFT;
    vmspace->vm_taddr = (caddr_t) (uintptr_t) virtual_offset;
    vmspace->vm_daddr = (caddr_t) (uintptr_t) text_end;

    imgp->stack_sz = PAGE_SIZE*64;

    error = exec_map_stack(imgp);
    printf("exec_map_stack = %d sz %ld\n", error, imgp->stack_sz);
    if (error != 0)
        return (error);

    /* Fill in image_params */
    imgp->interpreted = 0;
    imgp->entry_addr = entry_addr;
    imgp->proc->p_sysent = &macho_sysvec;
    printf("returning 0\n");

    return 0;
}

/*
* Tell kern_execve.c about it, with a little help from the linker.
*/
static struct execsw macho_execsw = {
    .ex_imgact = exec_macho_imgact,
    .ex_name = "mach-o"
};
EXEC_SET(macho, macho_execsw);
