/*
 * Copyright (c) 2004, Apple Computer, Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/* byte_sex.c */

#define __cr cr
#define __ctr ctr
#define __dar dar
#define __dsisr dsisr
#define __exception exception
#define __fpregs fpregs
#define __fpscr fpscr
#define __fpscr_pad fpscr_pad
#define __lr lr
#define __mq mq
#define __pad0 pad0
#define __pad1 pad1
#define __r0 r0
#define __r1 r1
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __r16 r16
#define __r17 r17
#define __r18 r18
#define __r19 r19
#define __r2 r2
#define __r20 r20
#define __r21 r21
#define __r22 r22
#define __r23 r23
#define __r24 r24
#define __r25 r25
#define __r26 r26
#define __r27 r27
#define __r28 r28
#define __r29 r29
#define __r3 r3
#define __r30 r30
#define __r31 r31
#define __r4 r4
#define __r5 r5
#define __r6 r6
#define __r7 r7
#define __r8 r8
#define __r9 r9
#define __srr0 srr0
#define __srr1 srr1
#define __vrsave vrsave
#define __xer xer

#define __darwin_i386_exception_state i386_exception_state
#define __darwin_i386_float_state i386_float_state
#define __darwin_i386_thread_state i386_thread_state
#define __busy busy
#define __c0 c0
#define __c1 c1
#define __c2 c2
#define __c3 c3
#define __cs cs
#define __darwin_fp_control fp_control
#define __darwin_fp_status fp_status
#define __darwin_mmst_reg mmst_reg
#define __darwin_xmm_reg xmm_reg
#define __denorm denorm
#define __ds ds
#define __eax eax
#define __ebp ebp
#define __ebx ebx
#define __ecx ecx
#define __edi edi
#define __edx edx
#define __eflags eflags
#define __eip eip
#define __err err
#define __errsumm errsumm
#define __es es
#define __esi esi
#define __esp esp
#define __faultvaddr faultvaddr
#define __fpu_cs fpu_cs
#define __fpu_dp fpu_dp
#define __fpu_ds fpu_ds
#define __fpu_fcw fpu_fcw
#define __fpu_fop fpu_fop
#define __fpu_fsw fpu_fsw
#define __fpu_ftw fpu_ftw
#define __fpu_ip fpu_ip
#define __fpu_mxcsr fpu_mxcsr
#define __fpu_mxcsrmask fpu_mxcsrmask
#define __fpu_reserved fpu_reserved
#define __fpu_reserved1 fpu_reserved1
#define __fpu_rsrv1 fpu_rsrv1
#define __fpu_rsrv2 fpu_rsrv2
#define __fpu_rsrv3 fpu_rsrv3
#define __fpu_rsrv4 fpu_rsrv4
#define __fpu_stmm0 fpu_stmm0
#define __fpu_stmm1 fpu_stmm1
#define __fpu_stmm2 fpu_stmm2
#define __fpu_stmm3 fpu_stmm3
#define __fpu_stmm4 fpu_stmm4
#define __fpu_stmm5 fpu_stmm5
#define __fpu_stmm6 fpu_stmm6
#define __fpu_stmm7 fpu_stmm7
#define __fpu_xmm0 fpu_xmm0
#define __fpu_xmm1 fpu_xmm1
#define __fpu_xmm2 fpu_xmm2
#define __fpu_xmm3 fpu_xmm3
#define __fpu_xmm4 fpu_xmm4
#define __fpu_xmm5 fpu_xmm5
#define __fpu_xmm6 fpu_xmm6
#define __fpu_xmm7 fpu_xmm7
#define __fs fs
#define __gs gs
#define __invalid invalid
#define __mmst_reg mmst_reg
#define __mmst_rsrv mmst_rsrv
#define __ovrfl ovrfl
#define __pc pc
#define __precis precis
#define __rc rc
#define __ss ss
#define __stkflt stkflt
#define __tos tos
#define __trapno trapno
#define __undfl undfl
#define __xmm_reg xmm_reg
#define __zdiv zdiv

#define __rax rax
#define __rbx rbx
#define __rcx rcx
#define __rdx rdx
#define __rdi rdi
#define __rsi rsi
#define __rbp rbp
#define __rsp rsp
#define __r8 r8
#define __r9 r9
#define __r10 r10
#define __r11 r11
#define __r12 r12
#define __r13 r13
#define __r14 r14
#define __r15 r15
#define __rip rip
#define __rflags rflags

#define __dr0 dr0
#define __dr1 dr1
#define __dr2 dr2
#define __dr3 dr3
#define __dr4 dr4
#define __dr5 dr5
#define __dr6 dr6
#define __dr7 dr7

#include <string.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/i386/thread_status.h>
#undef MACHINE_THREAD_STATE
#undef MACHINE_THREAD_STATE_COUNT
#include <mach/arm/thread_status.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/ranlib.h>
#include "xar/xar.h" /* cctools-port: 
				   force the use of the bundled xar header */
#include "stuff/bool.h"
#include "stuff/bytesex.h"

__private_extern__
long long
SWAP_LONG_LONG(
long long ll)
{
	union {
	    char c[8];
	    long long ll;
	} in, out;
	in.ll = ll;
	out.c[0] = in.c[7];
	out.c[1] = in.c[6];
	out.c[2] = in.c[5];
	out.c[3] = in.c[4];
	out.c[4] = in.c[3];
	out.c[5] = in.c[2];
	out.c[6] = in.c[1];
	out.c[7] = in.c[0];
	return(out.ll);
}

__private_extern__
double
SWAP_DOUBLE(
double d)
{
	union {
	    char c[8];
	    double d;
	} in, out;
	in.d = d;
	out.c[0] = in.c[7];
	out.c[1] = in.c[6];
	out.c[2] = in.c[5];
	out.c[3] = in.c[4];
	out.c[4] = in.c[3];
	out.c[5] = in.c[2];
	out.c[6] = in.c[1];
	out.c[7] = in.c[0];
	return(out.d);
}

__private_extern__
float
SWAP_FLOAT(
float f)
{
	union {
	    char c[7];
	    float f;
	} in, out;
	in.f = f;
	out.c[0] = in.c[3];
	out.c[1] = in.c[2];
	out.c[2] = in.c[1];
	out.c[3] = in.c[0];
	return(out.f);
}

/*
 * get_host_byte_sex() returns the enum constant for the byte sex of the host
 * it is running on.
 */
__private_extern__
enum byte_sex
get_host_byte_sex(
void)
{
    uint32_t s;

	s = (BIG_ENDIAN_BYTE_SEX << 24) | LITTLE_ENDIAN_BYTE_SEX;
	return((enum byte_sex)*((char *)&s));
}

__private_extern__
void
swap_fat_header(
struct fat_header *fat_header,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	fat_header->magic     = SWAP_INT(fat_header->magic);
	fat_header->nfat_arch = SWAP_INT(fat_header->nfat_arch);
}

__private_extern__
void
swap_fat_arch(
struct fat_arch *fat_archs,
uint32_t nfat_arch,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nfat_arch; i++){
	    fat_archs[i].cputype    = SWAP_INT(fat_archs[i].cputype);
	    fat_archs[i].cpusubtype = SWAP_INT(fat_archs[i].cpusubtype);
	    fat_archs[i].offset     = SWAP_INT(fat_archs[i].offset);
	    fat_archs[i].size       = SWAP_INT(fat_archs[i].size);
	    fat_archs[i].align      = SWAP_INT(fat_archs[i].align);
	}
}

__private_extern__
void
swap_fat_arch_64(
struct fat_arch_64 *fat_archs64,
uint32_t nfat_arch,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nfat_arch; i++){
	    fat_archs64[i].cputype    = SWAP_INT(fat_archs64[i].cputype);
	    fat_archs64[i].cpusubtype = SWAP_INT(fat_archs64[i].cpusubtype);
	    fat_archs64[i].offset     = SWAP_LONG_LONG(fat_archs64[i].offset);
	    fat_archs64[i].size       = SWAP_LONG_LONG(fat_archs64[i].size);
	    fat_archs64[i].align      = SWAP_INT(fat_archs64[i].align);
	    fat_archs64[i].reserved   = SWAP_INT(fat_archs64[i].reserved);
	}
}

__private_extern__
void
swap_mach_header(
struct mach_header *mh,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	mh->magic = SWAP_INT(mh->magic);
	mh->cputype = SWAP_INT(mh->cputype);
	mh->cpusubtype = SWAP_INT(mh->cpusubtype);
	mh->filetype = SWAP_INT(mh->filetype);
	mh->ncmds = SWAP_INT(mh->ncmds);
	mh->sizeofcmds = SWAP_INT(mh->sizeofcmds);
	mh->flags = SWAP_INT(mh->flags);
}

__private_extern__
void
swap_mach_header_64(
struct mach_header_64 *mh,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	mh->magic = SWAP_INT(mh->magic);
	mh->cputype = SWAP_INT(mh->cputype);
	mh->cpusubtype = SWAP_INT(mh->cpusubtype);
	mh->filetype = SWAP_INT(mh->filetype);
	mh->ncmds = SWAP_INT(mh->ncmds);
	mh->sizeofcmds = SWAP_INT(mh->sizeofcmds);
	mh->flags = SWAP_INT(mh->flags);
	mh->reserved = SWAP_INT(mh->reserved);
}

__private_extern__
void
swap_load_command(
struct load_command *lc,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	lc->cmd = SWAP_INT(lc->cmd);
	lc->cmdsize = SWAP_INT(lc->cmdsize);
}

__private_extern__
void
swap_segment_command(
struct segment_command *sg,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	/* segname[16] */
	sg->cmd = SWAP_INT(sg->cmd);
	sg->cmdsize = SWAP_INT(sg->cmdsize);
	sg->vmaddr = SWAP_INT(sg->vmaddr);
	sg->vmsize = SWAP_INT(sg->vmsize);
	sg->fileoff = SWAP_INT(sg->fileoff);
	sg->filesize = SWAP_INT(sg->filesize);
	sg->maxprot = SWAP_INT(sg->maxprot);
	sg->initprot = SWAP_INT(sg->initprot);
	sg->nsects = SWAP_INT(sg->nsects);
	sg->flags = SWAP_INT(sg->flags);
}

__private_extern__
void
swap_segment_command_64(
struct segment_command_64 *sg,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	/* segname[16] */
	sg->cmd = SWAP_INT(sg->cmd);
	sg->cmdsize = SWAP_INT(sg->cmdsize);
	sg->vmaddr = SWAP_LONG_LONG(sg->vmaddr);
	sg->vmsize = SWAP_LONG_LONG(sg->vmsize);
	sg->fileoff = SWAP_LONG_LONG(sg->fileoff);
	sg->filesize = SWAP_LONG_LONG(sg->filesize);
	sg->maxprot = SWAP_INT(sg->maxprot);
	sg->initprot = SWAP_INT(sg->initprot);
	sg->nsects = SWAP_INT(sg->nsects);
	sg->flags = SWAP_INT(sg->flags);
}

__private_extern__
void
swap_section(
struct section *s,
uint32_t nsects,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nsects; i++){
	    /* sectname[16] */
	    /* segname[16] */
	    s[i].addr = SWAP_INT(s[i].addr);
	    s[i].size = SWAP_INT(s[i].size);
	    s[i].offset = SWAP_INT(s[i].offset);
	    s[i].align = SWAP_INT(s[i].align);
	    s[i].reloff = SWAP_INT(s[i].reloff);
	    s[i].nreloc = SWAP_INT(s[i].nreloc);
	    s[i].flags = SWAP_INT(s[i].flags);
	    s[i].reserved1 = SWAP_INT(s[i].reserved1);
	    s[i].reserved2 = SWAP_INT(s[i].reserved2);
	}
}

__private_extern__
void
swap_section_64(
struct section_64 *s,
uint32_t nsects,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nsects; i++){
	    /* sectname[16] */
	    /* segname[16] */
	    s[i].addr = SWAP_LONG_LONG(s[i].addr);
	    s[i].size = SWAP_LONG_LONG(s[i].size);
	    s[i].offset = SWAP_INT(s[i].offset);
	    s[i].align = SWAP_INT(s[i].align);
	    s[i].reloff = SWAP_INT(s[i].reloff);
	    s[i].nreloc = SWAP_INT(s[i].nreloc);
	    s[i].flags = SWAP_INT(s[i].flags);
	    s[i].reserved1 = SWAP_INT(s[i].reserved1);
	    s[i].reserved2 = SWAP_INT(s[i].reserved2);
	}
}

__private_extern__
void
swap_symtab_command(
struct symtab_command *st,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	st->cmd = SWAP_INT(st->cmd);
	st->cmdsize = SWAP_INT(st->cmdsize);
	st->symoff = SWAP_INT(st->symoff);
	st->nsyms = SWAP_INT(st->nsyms);
	st->stroff = SWAP_INT(st->stroff);
	st->strsize = SWAP_INT(st->strsize);
}

__private_extern__
void
swap_dysymtab_command(
struct dysymtab_command *dyst,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	dyst->cmd = SWAP_INT(dyst->cmd);
	dyst->cmdsize = SWAP_INT(dyst->cmdsize);
	dyst->ilocalsym = SWAP_INT(dyst->ilocalsym);
	dyst->nlocalsym = SWAP_INT(dyst->nlocalsym);
	dyst->iextdefsym = SWAP_INT(dyst->iextdefsym);
	dyst->nextdefsym = SWAP_INT(dyst->nextdefsym);
	dyst->iundefsym = SWAP_INT(dyst->iundefsym);
	dyst->nundefsym = SWAP_INT(dyst->nundefsym);
	dyst->tocoff = SWAP_INT(dyst->tocoff);
	dyst->ntoc = SWAP_INT(dyst->ntoc);
	dyst->modtaboff = SWAP_INT(dyst->modtaboff);
	dyst->nmodtab = SWAP_INT(dyst->nmodtab);
	dyst->extrefsymoff = SWAP_INT(dyst->extrefsymoff);
	dyst->nextrefsyms = SWAP_INT(dyst->nextrefsyms);
	dyst->indirectsymoff = SWAP_INT(dyst->indirectsymoff);
	dyst->nindirectsyms = SWAP_INT(dyst->nindirectsyms);
	dyst->extreloff = SWAP_INT(dyst->extreloff);
	dyst->nextrel = SWAP_INT(dyst->nextrel);
	dyst->locreloff = SWAP_INT(dyst->locreloff);
	dyst->nlocrel = SWAP_INT(dyst->nlocrel);
}

__private_extern__
void
swap_symseg_command(
struct symseg_command *ss,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	ss->cmd = SWAP_INT(ss->cmd);
	ss->cmdsize = SWAP_INT(ss->cmdsize);
	ss->offset = SWAP_INT(ss->offset);
	ss->size = SWAP_INT(ss->size);
}

__private_extern__
void
swap_fvmlib_command(
struct fvmlib_command *fl,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	fl->cmd = SWAP_INT(fl->cmd);
	fl->cmdsize = SWAP_INT(fl->cmdsize);
	fl->fvmlib.name.offset = SWAP_INT(fl->fvmlib.name.offset);
	fl->fvmlib.minor_version = SWAP_INT(fl->fvmlib.minor_version);
	fl->fvmlib.header_addr = SWAP_INT(fl->fvmlib.header_addr);
}

__private_extern__
void
swap_dylib_command(
struct dylib_command *dl,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	dl->cmd = SWAP_INT(dl->cmd);
	dl->cmdsize = SWAP_INT(dl->cmdsize);
	dl->dylib.name.offset = SWAP_INT(dl->dylib.name.offset);
	dl->dylib.timestamp = SWAP_INT(dl->dylib.timestamp);
	dl->dylib.current_version = SWAP_INT(dl->dylib.current_version);
	dl->dylib.compatibility_version =
				SWAP_INT(dl->dylib.compatibility_version);
}

__private_extern__
void
swap_sub_framework_command(
struct sub_framework_command *sub,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	sub->cmd = SWAP_INT(sub->cmd);
	sub->cmdsize = SWAP_INT(sub->cmdsize);
	sub->umbrella.offset = SWAP_INT(sub->umbrella.offset);
}

__private_extern__
void
swap_sub_umbrella_command(
struct sub_umbrella_command *usub,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	usub->cmd = SWAP_INT(usub->cmd);
	usub->cmdsize = SWAP_INT(usub->cmdsize);
	usub->sub_umbrella.offset = SWAP_INT(usub->sub_umbrella.offset);
}

__private_extern__
void
swap_sub_library_command(
struct sub_library_command *lsub,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	lsub->cmd = SWAP_INT(lsub->cmd);
	lsub->cmdsize = SWAP_INT(lsub->cmdsize);
	lsub->sub_library.offset = SWAP_INT(lsub->sub_library.offset);
}

__private_extern__
void
swap_sub_client_command(
struct sub_client_command *csub,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	csub->cmd = SWAP_INT(csub->cmd);
	csub->cmdsize = SWAP_INT(csub->cmdsize);
	csub->client.offset = SWAP_INT(csub->client.offset);
}

__private_extern__
void
swap_prebound_dylib_command(
struct prebound_dylib_command *pbdylib,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	pbdylib->cmd = SWAP_INT(pbdylib->cmd);
	pbdylib->cmdsize = SWAP_INT(pbdylib->cmdsize);
	pbdylib->name.offset = SWAP_INT(pbdylib->name.offset);
	pbdylib->nmodules = SWAP_INT(pbdylib->nmodules);
	pbdylib->linked_modules.offset =
		SWAP_INT(pbdylib->linked_modules.offset);
}

__private_extern__
void
swap_dylinker_command(
struct dylinker_command *dyld,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	dyld->cmd = SWAP_INT(dyld->cmd);
	dyld->cmdsize = SWAP_INT(dyld->cmdsize);
	dyld->name.offset = SWAP_INT(dyld->name.offset);
}

__private_extern__
void
swap_fvmfile_command(
struct fvmfile_command *ff,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	ff->cmd = SWAP_INT(ff->cmd);
	ff->cmdsize = SWAP_INT(ff->cmdsize);
	ff->name.offset = SWAP_INT(ff->name.offset);
	ff->header_addr = SWAP_INT(ff->header_addr);
}


__private_extern__
void
swap_thread_command(
struct thread_command *ut,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	ut->cmd = SWAP_INT(ut->cmd);
	ut->cmdsize = SWAP_INT(ut->cmdsize);
}

__private_extern__
void
swap_i386_thread_state(
i386_thread_state_t *cpu,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	cpu->eax = SWAP_INT(cpu->eax);
	cpu->ebx = SWAP_INT(cpu->ebx);
	cpu->ecx = SWAP_INT(cpu->ecx);
	cpu->edx = SWAP_INT(cpu->edx);
	cpu->edi = SWAP_INT(cpu->edi);
	cpu->esi = SWAP_INT(cpu->esi);
	cpu->ebp = SWAP_INT(cpu->ebp);
	cpu->esp = SWAP_INT(cpu->esp);
	cpu->ss = SWAP_INT(cpu->ss);
	cpu->eflags = SWAP_INT(cpu->eflags);
	cpu->eip = SWAP_INT(cpu->eip);
	cpu->cs = SWAP_INT(cpu->cs);
	cpu->ds = SWAP_INT(cpu->ds);
	cpu->es = SWAP_INT(cpu->es);
	cpu->fs = SWAP_INT(cpu->fs);
	cpu->gs = SWAP_INT(cpu->gs);
}

#ifdef x86_THREAD_STATE64
__private_extern__
void
swap_x86_thread_state64(
x86_thread_state64_t *cpu,
enum byte_sex target_byte_sex)
{
	cpu->rax = SWAP_LONG_LONG(cpu->rax);
	cpu->rbx = SWAP_LONG_LONG(cpu->rbx);
	cpu->rcx = SWAP_LONG_LONG(cpu->rcx);
	cpu->rdx = SWAP_LONG_LONG(cpu->rdx);
	cpu->rdi = SWAP_LONG_LONG(cpu->rdi);
	cpu->rsi = SWAP_LONG_LONG(cpu->rsi);
	cpu->rbp = SWAP_LONG_LONG(cpu->rbp);
	cpu->rsp = SWAP_LONG_LONG(cpu->rsp);
	cpu->rflags = SWAP_LONG_LONG(cpu->rflags);
	cpu->rip = SWAP_LONG_LONG(cpu->rip);
	cpu->r8 = SWAP_LONG_LONG(cpu->r8);
	cpu->r9 = SWAP_LONG_LONG(cpu->r9);
	cpu->r10 = SWAP_LONG_LONG(cpu->r10);
	cpu->r11 = SWAP_LONG_LONG(cpu->r11);
	cpu->r12 = SWAP_LONG_LONG(cpu->r12);
	cpu->r13 = SWAP_LONG_LONG(cpu->r13);
	cpu->r14 = SWAP_LONG_LONG(cpu->r14);
	cpu->r15 = SWAP_LONG_LONG(cpu->r15);
	cpu->cs = SWAP_LONG_LONG(cpu->cs);
	cpu->fs = SWAP_LONG_LONG(cpu->fs);
	cpu->gs = SWAP_LONG_LONG(cpu->gs);
}
#endif /* x86_THREAD_STATE64 */

/* current i386 thread states */
#if i386_THREAD_STATE == 1
__private_extern__
void
swap_i386_float_state(
struct i386_float_state *fpu,
enum byte_sex target_byte_sex)
{
#ifndef i386_EXCEPTION_STATE_COUNT
    /* this routine does nothing as their are currently no non-byte fields */
#else /* defined(i386_EXCEPTION_STATE_COUNT) */
    struct swapped_fp_control {
	union {
	    struct {
		unsigned short
			    :3,
		    /*inf*/ :1,
		    rc	    :2,
		    pc	    :2,
			    :2,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfpc;

    struct swapped_fp_status {
	union {
	    struct {
		unsigned short
		    busy    :1,
		    c3	    :1,
		    tos	    :3,
		    c2	    :1,
		    c1	    :1,
		    c0	    :1,
		    errsumm :1,
		    stkflt  :1,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfps;

    enum byte_sex host_byte_sex;

	host_byte_sex = get_host_byte_sex();

	fpu->fpu_reserved[0] = SWAP_INT(fpu->fpu_reserved[0]);
	fpu->fpu_reserved[1] = SWAP_INT(fpu->fpu_reserved[1]);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfpc, &(fpu->fpu_fcw),
		   sizeof(struct swapped_fp_control));
	    sfpc.u.half = SWAP_SHORT(sfpc.u.half);
	    fpu->fpu_fcw.rc = sfpc.u.fields.rc;
	    fpu->fpu_fcw.pc = sfpc.u.fields.pc;
	    fpu->fpu_fcw.precis = sfpc.u.fields.precis;
	    fpu->fpu_fcw.undfl = sfpc.u.fields.undfl;
	    fpu->fpu_fcw.ovrfl = sfpc.u.fields.ovrfl;
	    fpu->fpu_fcw.zdiv = sfpc.u.fields.zdiv;
	    fpu->fpu_fcw.denorm = sfpc.u.fields.denorm;
	    fpu->fpu_fcw.invalid = sfpc.u.fields.invalid;

	    memcpy(&sfps, &(fpu->fpu_fsw),
		   sizeof(struct swapped_fp_status));
	    sfps.u.half = SWAP_SHORT(sfps.u.half);
	    fpu->fpu_fsw.busy = sfps.u.fields.busy;
	    fpu->fpu_fsw.c3 = sfps.u.fields.c3;
	    fpu->fpu_fsw.tos = sfps.u.fields.tos;
	    fpu->fpu_fsw.c2 = sfps.u.fields.c2;
	    fpu->fpu_fsw.c1 = sfps.u.fields.c1;
	    fpu->fpu_fsw.c0 = sfps.u.fields.c0;
	    fpu->fpu_fsw.errsumm = sfps.u.fields.errsumm;
	    fpu->fpu_fsw.stkflt = sfps.u.fields.stkflt;
	    fpu->fpu_fsw.precis = sfps.u.fields.precis;
	    fpu->fpu_fsw.undfl = sfps.u.fields.undfl;
	    fpu->fpu_fsw.ovrfl = sfps.u.fields.ovrfl;
	    fpu->fpu_fsw.zdiv = sfps.u.fields.zdiv;
	    fpu->fpu_fsw.denorm = sfps.u.fields.denorm;
	    fpu->fpu_fsw.invalid = sfps.u.fields.invalid;
	}
	else{
	    sfpc.u.fields.rc = fpu->fpu_fcw.rc;
	    sfpc.u.fields.pc = fpu->fpu_fcw.pc;
	    sfpc.u.fields.precis = fpu->fpu_fcw.precis;
	    sfpc.u.fields.undfl = fpu->fpu_fcw.undfl;
	    sfpc.u.fields.ovrfl = fpu->fpu_fcw.ovrfl;
	    sfpc.u.fields.zdiv = fpu->fpu_fcw.zdiv;
	    sfpc.u.fields.denorm = fpu->fpu_fcw.denorm;
	    sfpc.u.fields.invalid = fpu->fpu_fcw.invalid;
	    sfpc.u.half = SWAP_SHORT(sfpc.u.half);
	    memcpy(&(fpu->fpu_fcw), &sfpc,
		   sizeof(struct swapped_fp_control));

	    sfps.u.fields.busy = fpu->fpu_fsw.busy;
	    sfps.u.fields.c3 = fpu->fpu_fsw.c3;
	    sfps.u.fields.tos = fpu->fpu_fsw.tos;
	    sfps.u.fields.c2 = fpu->fpu_fsw.c2;
	    sfps.u.fields.c1 = fpu->fpu_fsw.c1;
	    sfps.u.fields.c0 = fpu->fpu_fsw.c0;
	    sfps.u.fields.errsumm = fpu->fpu_fsw.errsumm;
	    sfps.u.fields.stkflt = fpu->fpu_fsw.stkflt;
	    sfps.u.fields.precis = fpu->fpu_fsw.precis;
	    sfps.u.fields.undfl = fpu->fpu_fsw.undfl;
	    sfps.u.fields.ovrfl = fpu->fpu_fsw.ovrfl;
	    sfps.u.fields.zdiv = fpu->fpu_fsw.zdiv;
	    sfps.u.fields.denorm = fpu->fpu_fsw.denorm;
	    sfps.u.fields.invalid = fpu->fpu_fsw.invalid;
	    sfps.u.half = SWAP_SHORT(sfps.u.half);
	    memcpy(&(fpu->fpu_fsw), &sfps,
		   sizeof(struct swapped_fp_status));
	}
	fpu->fpu_fop = SWAP_SHORT(fpu->fpu_fop);
	fpu->fpu_ip = SWAP_INT(fpu->fpu_ip);
	fpu->fpu_cs = SWAP_SHORT(fpu->fpu_cs);
	fpu->fpu_rsrv2 = SWAP_SHORT(fpu->fpu_rsrv2);
	fpu->fpu_dp = SWAP_INT(fpu->fpu_dp);
	fpu->fpu_ds = SWAP_SHORT(fpu->fpu_ds);
	fpu->fpu_rsrv3 = SWAP_SHORT(fpu->fpu_rsrv3);
	fpu->fpu_mxcsr = SWAP_INT(fpu->fpu_mxcsr);
	fpu->fpu_mxcsrmask = SWAP_INT(fpu->fpu_mxcsrmask);
	fpu->fpu_reserved1 = SWAP_INT(fpu->fpu_reserved1);

#endif /* defined(i386_EXCEPTION_STATE_COUNT) */
}

__private_extern__
void
swap_i386_exception_state(
i386_exception_state_t *exc,
enum byte_sex target_byte_sex)
{
	exc->trapno = SWAP_INT(exc->trapno);
	exc->err = SWAP_INT(exc->err);
    	exc->faultvaddr = SWAP_INT(exc->faultvaddr);
}

#ifdef x86_THREAD_STATE64

__private_extern__
void
swap_x86_float_state64(
x86_float_state64_t *fpu,
enum byte_sex target_byte_sex)
{
    struct swapped_fp_control {
	union {
	    struct {
		unsigned short
			    :3,
		    /*inf*/ :1,
		    rc	    :2,
		    pc	    :2,
			    :2,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfpc;

    struct swapped_fp_status {
	union {
	    struct {
		unsigned short
		    busy    :1,
		    c3	    :1,
		    tos	    :3,
		    c2	    :1,
		    c1	    :1,
		    c0	    :1,
		    errsumm :1,
		    stkflt  :1,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfps;

    enum byte_sex host_byte_sex;

	host_byte_sex = get_host_byte_sex();

	fpu->fpu_reserved[0] = SWAP_INT(fpu->fpu_reserved[0]);
	fpu->fpu_reserved[1] = SWAP_INT(fpu->fpu_reserved[1]);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfpc, &(fpu->fpu_fcw),
		   sizeof(struct swapped_fp_control));
	    sfpc.u.half = SWAP_SHORT(sfpc.u.half);
	    fpu->fpu_fcw.rc = sfpc.u.fields.rc;
	    fpu->fpu_fcw.pc = sfpc.u.fields.pc;
	    fpu->fpu_fcw.precis = sfpc.u.fields.precis;
	    fpu->fpu_fcw.undfl = sfpc.u.fields.undfl;
	    fpu->fpu_fcw.ovrfl = sfpc.u.fields.ovrfl;
	    fpu->fpu_fcw.zdiv = sfpc.u.fields.zdiv;
	    fpu->fpu_fcw.denorm = sfpc.u.fields.denorm;
	    fpu->fpu_fcw.invalid = sfpc.u.fields.invalid;

	    memcpy(&sfps, &(fpu->fpu_fsw),
		   sizeof(struct swapped_fp_status));
	    sfps.u.half = SWAP_SHORT(sfps.u.half);
	    fpu->fpu_fsw.busy = sfps.u.fields.busy;
	    fpu->fpu_fsw.c3 = sfps.u.fields.c3;
	    fpu->fpu_fsw.tos = sfps.u.fields.tos;
	    fpu->fpu_fsw.c2 = sfps.u.fields.c2;
	    fpu->fpu_fsw.c1 = sfps.u.fields.c1;
	    fpu->fpu_fsw.c0 = sfps.u.fields.c0;
	    fpu->fpu_fsw.errsumm = sfps.u.fields.errsumm;
	    fpu->fpu_fsw.stkflt = sfps.u.fields.stkflt;
	    fpu->fpu_fsw.precis = sfps.u.fields.precis;
	    fpu->fpu_fsw.undfl = sfps.u.fields.undfl;
	    fpu->fpu_fsw.ovrfl = sfps.u.fields.ovrfl;
	    fpu->fpu_fsw.zdiv = sfps.u.fields.zdiv;
	    fpu->fpu_fsw.denorm = sfps.u.fields.denorm;
	    fpu->fpu_fsw.invalid = sfps.u.fields.invalid;
	}
	else{
	    sfpc.u.fields.rc = fpu->fpu_fcw.rc;
	    sfpc.u.fields.pc = fpu->fpu_fcw.pc;
	    sfpc.u.fields.precis = fpu->fpu_fcw.precis;
	    sfpc.u.fields.undfl = fpu->fpu_fcw.undfl;
	    sfpc.u.fields.ovrfl = fpu->fpu_fcw.ovrfl;
	    sfpc.u.fields.zdiv = fpu->fpu_fcw.zdiv;
	    sfpc.u.fields.denorm = fpu->fpu_fcw.denorm;
	    sfpc.u.fields.invalid = fpu->fpu_fcw.invalid;
	    sfpc.u.half = SWAP_SHORT(sfpc.u.half);
	    memcpy(&(fpu->fpu_fcw), &sfpc,
		   sizeof(struct swapped_fp_control));

	    sfps.u.fields.busy = fpu->fpu_fsw.busy;
	    sfps.u.fields.c3 = fpu->fpu_fsw.c3;
	    sfps.u.fields.tos = fpu->fpu_fsw.tos;
	    sfps.u.fields.c2 = fpu->fpu_fsw.c2;
	    sfps.u.fields.c1 = fpu->fpu_fsw.c1;
	    sfps.u.fields.c0 = fpu->fpu_fsw.c0;
	    sfps.u.fields.errsumm = fpu->fpu_fsw.errsumm;
	    sfps.u.fields.stkflt = fpu->fpu_fsw.stkflt;
	    sfps.u.fields.precis = fpu->fpu_fsw.precis;
	    sfps.u.fields.undfl = fpu->fpu_fsw.undfl;
	    sfps.u.fields.ovrfl = fpu->fpu_fsw.ovrfl;
	    sfps.u.fields.zdiv = fpu->fpu_fsw.zdiv;
	    sfps.u.fields.denorm = fpu->fpu_fsw.denorm;
	    sfps.u.fields.invalid = fpu->fpu_fsw.invalid;
	    sfps.u.half = SWAP_SHORT(sfps.u.half);
	    memcpy(&(fpu->fpu_fsw), &sfps,
		   sizeof(struct swapped_fp_status));
	}
	fpu->fpu_fop = SWAP_SHORT(fpu->fpu_fop);
	fpu->fpu_ip = SWAP_INT(fpu->fpu_ip);
	fpu->fpu_cs = SWAP_SHORT(fpu->fpu_cs);
	fpu->fpu_rsrv2 = SWAP_SHORT(fpu->fpu_rsrv2);
	fpu->fpu_dp = SWAP_INT(fpu->fpu_dp);
	fpu->fpu_ds = SWAP_SHORT(fpu->fpu_ds);
	fpu->fpu_rsrv3 = SWAP_SHORT(fpu->fpu_rsrv3);
	fpu->fpu_mxcsr = SWAP_INT(fpu->fpu_mxcsr);
	fpu->fpu_mxcsrmask = SWAP_INT(fpu->fpu_mxcsrmask);
	fpu->fpu_reserved1 = SWAP_INT(fpu->fpu_reserved1);
}

__private_extern__
void
swap_x86_exception_state64(
x86_exception_state64_t *exc,
enum byte_sex target_byte_sex)
{
	exc->trapno = SWAP_INT(exc->trapno);
	exc->err = SWAP_INT(exc->err);
    	exc->faultvaddr = SWAP_LONG_LONG(exc->faultvaddr);
}

__private_extern__
void
swap_x86_debug_state32(
x86_debug_state32_t *debug,
enum byte_sex target_byte_sex)
{
	debug->dr0 = SWAP_INT(debug->dr0);
	debug->dr1 = SWAP_INT(debug->dr1);
	debug->dr2 = SWAP_INT(debug->dr2);
	debug->dr3 = SWAP_INT(debug->dr3);
	debug->dr4 = SWAP_INT(debug->dr4);
	debug->dr5 = SWAP_INT(debug->dr5);
	debug->dr6 = SWAP_INT(debug->dr6);
	debug->dr7 = SWAP_INT(debug->dr7);
}

__private_extern__
void
swap_x86_debug_state64(
x86_debug_state64_t *debug,
enum byte_sex target_byte_sex)
{
	debug->dr0 = SWAP_LONG_LONG(debug->dr0);
	debug->dr1 = SWAP_LONG_LONG(debug->dr1);
	debug->dr2 = SWAP_LONG_LONG(debug->dr2);
	debug->dr3 = SWAP_LONG_LONG(debug->dr3);
	debug->dr4 = SWAP_LONG_LONG(debug->dr4);
	debug->dr5 = SWAP_LONG_LONG(debug->dr5);
	debug->dr6 = SWAP_LONG_LONG(debug->dr6);
	debug->dr7 = SWAP_LONG_LONG(debug->dr7);
}

__private_extern__
void
swap_x86_state_hdr(
struct x86_state_hdr *hdr,
enum byte_sex target_byte_sex)
{
	hdr->flavor = SWAP_INT(hdr->flavor);
	hdr->count = SWAP_INT(hdr->count);
}
#endif /* x86_THREAD_STATE64 */

#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
__private_extern__
void
swap_i386_thread_fpstate(
i386_thread_fpstate_t *fpu,
enum byte_sex target_byte_sex)
{
    struct swapped_fp_control {
	union {
	    struct {
		unsigned short
			    :3,
		    /*inf*/ :1,
		    rc	    :2,
		    pc	    :2,
			    :2,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfpc;

    struct swapped_fp_status {
	union {
	    struct {
		unsigned short
		    busy    :1,
		    c3	    :1,
		    tos	    :3,
		    c2	    :1,
		    c1	    :1,
		    c0	    :1,
		    errsumm :1,
		    stkflt  :1,
		    precis  :1,
		    undfl   :1,
		    ovrfl   :1,
		    zdiv    :1,
		    denorm  :1,
		    invalid :1;
	    } fields;
	    unsigned short half;
	} u;
    } sfps;

    struct swapped_fp_tag {
	union {
	    struct {
		unsigned short
		    tag7 :2,
		    tag6 :2,
		    tag5 :2,
		    tag4 :2,
		    tag3 :2,
		    tag2 :2,
		    tag1 :2,
		    tag0 :2;
	    } fields;
	    unsigned short half;
	} u;
    } sfpt;

    struct swapped_fp_data_reg {
	unsigned short mant;
	unsigned short mant1 :16,
		       mant2 :16,
		       mant3 :16;
	union {
	    struct {
		unsigned short sign :1,
			       exp  :15;
	    } fields;
	    unsigned short half;
	} u;
    } sfpd;

    struct swapped_sel {
	union {
	    struct {
    		unsigned short
		    index :13,
		    ti    :1,
		    rpl   :2;
	    } fields;
	    unsigned short half;
	} u;
    } ss;

    enum byte_sex host_byte_sex;
    uint32_t i;

	host_byte_sex = get_host_byte_sex();

	fpu->environ.ip = SWAP_INT(fpu->environ.ip);
	fpu->environ.opcode = SWAP_SHORT(fpu->environ.opcode);
	fpu->environ.dp = SWAP_INT(fpu->environ.dp);

	if(target_byte_sex == host_byte_sex){
	    memcpy(&sfpc, &(fpu->environ.control),
		   sizeof(struct swapped_fp_control));
	    sfpc.u.half = SWAP_SHORT(sfpc.u.half);
	    fpu->environ.control.rc = sfpc.u.fields.rc;
	    fpu->environ.control.pc = sfpc.u.fields.pc;
	    fpu->environ.control.precis = sfpc.u.fields.precis;
	    fpu->environ.control.undfl = sfpc.u.fields.undfl;
	    fpu->environ.control.ovrfl = sfpc.u.fields.ovrfl;
	    fpu->environ.control.zdiv = sfpc.u.fields.zdiv;
	    fpu->environ.control.denorm = sfpc.u.fields.denorm;
	    fpu->environ.control.invalid = sfpc.u.fields.invalid;

	    memcpy(&sfps, &(fpu->environ.status),
		   sizeof(struct swapped_fp_status));
	    sfps.u.half = SWAP_SHORT(sfps.u.half);
	    fpu->environ.status.busy = sfps.u.fields.busy;
	    fpu->environ.status.c3 = sfps.u.fields.c3;
	    fpu->environ.status.tos = sfps.u.fields.tos;
	    fpu->environ.status.c2 = sfps.u.fields.c2;
	    fpu->environ.status.c1 = sfps.u.fields.c1;
	    fpu->environ.status.c0 = sfps.u.fields.c0;
	    fpu->environ.status.errsumm = sfps.u.fields.errsumm;
	    fpu->environ.status.stkflt = sfps.u.fields.stkflt;
	    fpu->environ.status.precis = sfps.u.fields.precis;
	    fpu->environ.status.undfl = sfps.u.fields.undfl;
	    fpu->environ.status.ovrfl = sfps.u.fields.ovrfl;
	    fpu->environ.status.zdiv = sfps.u.fields.zdiv;
	    fpu->environ.status.denorm = sfps.u.fields.denorm;
	    fpu->environ.status.invalid = sfps.u.fields.invalid;

	    memcpy(&sfpt, &(fpu->environ.tag),
		   sizeof(struct swapped_fp_tag));
	    sfpt.u.half = SWAP_SHORT(sfpt.u.half);
	    fpu->environ.tag.tag7 = sfpt.u.fields.tag7;
	    fpu->environ.tag.tag6 = sfpt.u.fields.tag6;
	    fpu->environ.tag.tag5 = sfpt.u.fields.tag5;
	    fpu->environ.tag.tag4 = sfpt.u.fields.tag4;
	    fpu->environ.tag.tag3 = sfpt.u.fields.tag3;
	    fpu->environ.tag.tag2 = sfpt.u.fields.tag2;
	    fpu->environ.tag.tag1 = sfpt.u.fields.tag1;
	    fpu->environ.tag.tag0 = sfpt.u.fields.tag0;

	    memcpy(&ss, &(fpu->environ.cs),
		   sizeof(struct swapped_sel));
	    ss.u.half = SWAP_SHORT(ss.u.half);
	    fpu->environ.cs.index = ss.u.fields.index;
	    fpu->environ.cs.ti = ss.u.fields.ti;
	    fpu->environ.cs.rpl = ss.u.fields.rpl;

	    memcpy(&ss, &(fpu->environ.ds),
		   sizeof(struct swapped_sel));
	    ss.u.half = SWAP_SHORT(ss.u.half);
	    fpu->environ.ds.index = ss.u.fields.index;
	    fpu->environ.ds.ti = ss.u.fields.ti;
	    fpu->environ.ds.rpl = ss.u.fields.rpl;
	
	    for(i = 0; i < 8; i++){
		memcpy(&sfpd, &(fpu->stack.ST[i]),
		       sizeof(struct swapped_fp_data_reg));
		fpu->stack.ST[i].mant = SWAP_SHORT(sfpd.mant);
		fpu->stack.ST[i].mant1 = SWAP_SHORT(sfpd.mant1);
		fpu->stack.ST[i].mant2 = SWAP_SHORT(sfpd.mant2);
		fpu->stack.ST[i].mant3 = SWAP_SHORT(sfpd.mant3);
		sfpd.u.half = SWAP_SHORT(sfpd.u.half);
		fpu->stack.ST[i].exp = sfpd.u.fields.exp;
		fpu->stack.ST[i].sign = sfpd.u.fields.sign;
	    }
	}
	else{
	    sfpc.u.fields.rc = fpu->environ.control.rc;
	    sfpc.u.fields.pc = fpu->environ.control.pc;
	    sfpc.u.fields.precis = fpu->environ.control.precis;
	    sfpc.u.fields.undfl = fpu->environ.control.undfl;
	    sfpc.u.fields.ovrfl = fpu->environ.control.ovrfl;
	    sfpc.u.fields.zdiv = fpu->environ.control.zdiv;
	    sfpc.u.fields.denorm = fpu->environ.control.denorm;
	    sfpc.u.fields.invalid = fpu->environ.control.invalid;
	    sfpc.u.half = SWAP_SHORT(sfpc.u.half);
	    memcpy(&(fpu->environ.control), &sfpc,
		   sizeof(struct swapped_fp_control));

	    sfps.u.fields.busy = fpu->environ.status.busy;
	    sfps.u.fields.c3 = fpu->environ.status.c3;
	    sfps.u.fields.tos = fpu->environ.status.tos;
	    sfps.u.fields.c2 = fpu->environ.status.c2;
	    sfps.u.fields.c1 = fpu->environ.status.c1;
	    sfps.u.fields.c0 = fpu->environ.status.c0;
	    sfps.u.fields.errsumm = fpu->environ.status.errsumm;
	    sfps.u.fields.stkflt = fpu->environ.status.stkflt;
	    sfps.u.fields.precis = fpu->environ.status.precis;
	    sfps.u.fields.undfl = fpu->environ.status.undfl;
	    sfps.u.fields.ovrfl = fpu->environ.status.ovrfl;
	    sfps.u.fields.zdiv = fpu->environ.status.zdiv;
	    sfps.u.fields.denorm = fpu->environ.status.denorm;
	    sfps.u.fields.invalid = fpu->environ.status.invalid;
	    sfps.u.half = SWAP_SHORT(sfps.u.half);
	    memcpy(&(fpu->environ.status), &sfps,
		   sizeof(struct swapped_fp_status));

	    sfpt.u.fields.tag7 = fpu->environ.tag.tag7;
	    sfpt.u.fields.tag6 = fpu->environ.tag.tag6;
	    sfpt.u.fields.tag5 = fpu->environ.tag.tag5;
	    sfpt.u.fields.tag4 = fpu->environ.tag.tag4;
	    sfpt.u.fields.tag3 = fpu->environ.tag.tag3;
	    sfpt.u.fields.tag2 = fpu->environ.tag.tag2;
	    sfpt.u.fields.tag1 = fpu->environ.tag.tag1;
	    sfpt.u.fields.tag0 = fpu->environ.tag.tag0;
	    sfpt.u.half = SWAP_SHORT(sfpt.u.half);
	    memcpy(&(fpu->environ.tag), &sfpt,
		   sizeof(struct swapped_fp_tag));

	    ss.u.fields.index = fpu->environ.cs.index;
	    ss.u.fields.ti = fpu->environ.cs.ti;
	    ss.u.fields.rpl = fpu->environ.cs.rpl;
	    ss.u.half = SWAP_SHORT(ss.u.half);
	    memcpy(&(fpu->environ.cs), &ss,
		   sizeof(struct swapped_sel));

	    ss.u.fields.index = fpu->environ.ds.index;
	    ss.u.fields.ti = fpu->environ.ds.ti;
	    ss.u.fields.rpl = fpu->environ.ds.rpl;
	    ss.u.half = SWAP_SHORT(ss.u.half);
	    memcpy(&(fpu->environ.cs), &ss,
		   sizeof(struct swapped_sel));

	    for(i = 0; i < 8; i++){
		sfpd.mant = SWAP_SHORT(fpu->stack.ST[i].mant);
		sfpd.mant1 = SWAP_SHORT(fpu->stack.ST[i].mant1);
		sfpd.mant2 = SWAP_SHORT(fpu->stack.ST[i].mant2);
		sfpd.mant3 = SWAP_SHORT(fpu->stack.ST[i].mant3);
		sfpd.u.fields.exp = fpu->stack.ST[i].exp;
		sfpd.u.fields.sign = fpu->stack.ST[i].sign;
		sfpd.u.half = SWAP_SHORT(sfpd.u.half);
		memcpy(&(fpu->stack.ST[i]), &sfpd,
		       sizeof(struct swapped_fp_data_reg));
	    }
	}
}

__private_extern__
void
swap_i386_thread_exceptstate(
i386_thread_exceptstate_t *exc,
enum byte_sex target_byte_sex)
{
    struct swapped_err_code {
	union {
	    struct err_code_normal {
		unsigned int		:16,
				index	:13,
				tbl	:2,
				ext	:1;
	    } normal;
	    struct err_code_pgfault {
		unsigned int		:29,
				user	:1,
				wrtflt	:1,
				prot	:1;
	    } pgfault;
	    uint32_t word;
	} u;
    } sec;
    uint32_t word;
    enum byte_sex host_byte_sex;

	host_byte_sex = get_host_byte_sex();

	exc->trapno = SWAP_INT(exc->trapno);
	if(exc->trapno == 14){
	    if(target_byte_sex == host_byte_sex){
		memcpy(&sec, &(exc->err), sizeof(struct swapped_err_code));
		sec.u.word = SWAP_INT(sec.u.word);
		exc->err.pgfault.user   = sec.u.pgfault.user;
		exc->err.pgfault.wrtflt = sec.u.pgfault.wrtflt;
		exc->err.pgfault.prot   = sec.u.pgfault.prot;
	    }
	    else{
		sec.u.pgfault.prot   = exc->err.pgfault.prot;
		sec.u.pgfault.wrtflt = exc->err.pgfault.wrtflt;
		sec.u.pgfault.user   = exc->err.pgfault.user;
		sec.u.word = SWAP_INT(sec.u.word);
		memcpy(&(exc->err), &sec, sizeof(struct swapped_err_code));
	    }
	}
	else{
	    if(target_byte_sex == host_byte_sex){
		memcpy(&sec, &(exc->err), sizeof(struct swapped_err_code));
		sec.u.word = SWAP_INT(sec.u.word);
		word = sec.u.normal.index;
		exc->err.normal.index = SWAP_INT(word);
		exc->err.normal.tbl   = sec.u.normal.tbl;
		exc->err.normal.ext   = sec.u.normal.ext;
	    }
	    else{
		sec.u.normal.ext   = exc->err.normal.ext;
		sec.u.normal.tbl   = exc->err.normal.tbl;
		word = exc->err.normal.index;
		sec.u.normal.index = SWAP_INT(word);
		sec.u.word = SWAP_INT(sec.u.word);
		memcpy(&(exc->err), &sec, sizeof(struct swapped_err_code));
	    }
	}
}

__private_extern__
void
swap_i386_thread_cthreadstate(
i386_thread_cthreadstate_t *user,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	user->self = SWAP_INT(user->self);
}
#endif /* i386_THREAD_STATE == -1 */


__private_extern__
void
swap_arm_thread_state_t(
arm_thread_state_t *cpu,
enum byte_sex target_byte_sex)
{
    int i;

	for(i = 0; i < 13; i++)
	    cpu->__r[i] = SWAP_INT(cpu->__r[i]);
	cpu->__sp = SWAP_INT(cpu->__sp);
	cpu->__lr = SWAP_INT(cpu->__lr);
	cpu->__pc = SWAP_INT(cpu->__pc);
	cpu->__cpsr = SWAP_INT(cpu->__cpsr);
}

void
swap_arm_thread_state64_t(
arm_thread_state64_t *cpu,
enum byte_sex target_byte_sex)
{
    int i;

	for(i = 0; i < 29; i++)
	    cpu->__x[i] = SWAP_LONG_LONG(cpu->__x[i]);
	cpu->__fp = SWAP_LONG_LONG(cpu->__fp);
	cpu->__lr = SWAP_LONG_LONG(cpu->__lr);
	cpu->__sp = SWAP_LONG_LONG(cpu->__sp);
	cpu->__pc = SWAP_LONG_LONG(cpu->__pc);
	cpu->__cpsr = SWAP_INT(cpu->__cpsr);
}

void
swap_arm_exception_state64_t(
arm_exception_state64_t *except,
enum byte_sex target_byte_sex)
{
	except->__far = SWAP_LONG_LONG(except->__far);
	except->__esr = SWAP_INT(except->__esr);
	except->__exception = SWAP_INT(except->__exception);
}

__private_extern__
void
swap_ident_command(
struct ident_command *id_cmd,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	id_cmd->cmd = SWAP_INT(id_cmd->cmd);
	id_cmd->cmdsize = SWAP_INT(id_cmd->cmdsize);
}

__private_extern__
void
swap_routines_command(
struct routines_command *r_cmd,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	r_cmd->cmd = SWAP_INT(r_cmd->cmd);
	r_cmd->cmdsize = SWAP_INT(r_cmd->cmdsize);
	r_cmd->init_address = SWAP_INT(r_cmd->init_address);
	r_cmd->init_module = SWAP_INT(r_cmd->init_module);
	r_cmd->reserved1 = SWAP_INT(r_cmd->reserved1);
	r_cmd->reserved2 = SWAP_INT(r_cmd->reserved2);
	r_cmd->reserved3 = SWAP_INT(r_cmd->reserved3);
	r_cmd->reserved4 = SWAP_INT(r_cmd->reserved4);
	r_cmd->reserved5 = SWAP_INT(r_cmd->reserved5);
	r_cmd->reserved6 = SWAP_INT(r_cmd->reserved6);
}

__private_extern__
void
swap_routines_command_64(
struct routines_command_64 *r_cmd,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	r_cmd->cmd = SWAP_INT(r_cmd->cmd);
	r_cmd->cmdsize = SWAP_INT(r_cmd->cmdsize);
	r_cmd->init_address = SWAP_LONG_LONG(r_cmd->init_address);
	r_cmd->init_module = SWAP_LONG_LONG(r_cmd->init_module);
	r_cmd->reserved1 = SWAP_LONG_LONG(r_cmd->reserved1);
	r_cmd->reserved2 = SWAP_LONG_LONG(r_cmd->reserved2);
	r_cmd->reserved3 = SWAP_LONG_LONG(r_cmd->reserved3);
	r_cmd->reserved4 = SWAP_LONG_LONG(r_cmd->reserved4);
	r_cmd->reserved5 = SWAP_LONG_LONG(r_cmd->reserved5);
	r_cmd->reserved6 = SWAP_LONG_LONG(r_cmd->reserved6);
}

__private_extern__
void
swap_twolevel_hints_command(
struct twolevel_hints_command *hints_cmd,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	hints_cmd->cmd = SWAP_INT(hints_cmd->cmd);
	hints_cmd->cmdsize = SWAP_INT(hints_cmd->cmdsize);
	hints_cmd->offset = SWAP_INT(hints_cmd->offset);
	hints_cmd->nhints = SWAP_INT(hints_cmd->nhints);
}

__private_extern__
void
swap_twolevel_hint(
struct twolevel_hint *hints,
uint32_t nhints,
enum byte_sex target_byte_sex)
{
    struct swapped_twolevel_hint {
	union {
	    struct {
		uint32_t
		    itoc:24,
		    isub_image:8;
	    } fields;
	    uint32_t word;
	} u;
    } shint;

    uint32_t i;
    enum byte_sex host_byte_sex;

	host_byte_sex = get_host_byte_sex();

	for(i = 0; i < nhints; i++){
	    if(target_byte_sex == host_byte_sex){
		memcpy(&shint, hints + i, sizeof(struct swapped_twolevel_hint));
		shint.u.word = SWAP_INT(shint.u.word);
		hints[i].itoc = shint.u.fields.itoc;
		hints[i].isub_image = shint.u.fields.isub_image;
	    }
	    else{
		shint.u.fields.isub_image = hints[i].isub_image;
		shint.u.fields.itoc = hints[i].itoc;
		shint.u.word = SWAP_INT(shint.u.word);
		memcpy(hints + i, &shint, sizeof(struct swapped_twolevel_hint));
	    }
	}
}

__private_extern__
void
swap_data_in_code_entry(
struct data_in_code_entry *dices,
uint32_t ndices,
enum byte_sex target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < ndices; i++){
	    dices[i].offset = SWAP_INT(dices[i].offset);
	    dices[i].length = SWAP_INT(dices[i].length);
	    dices[i].kind = SWAP_INT(dices[i].kind);
	}
}

__private_extern__
void
swap_prebind_cksum_command(
struct prebind_cksum_command *cksum_cmd,
enum byte_sex target_byte_sex)
{
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif
	cksum_cmd->cmd = SWAP_INT(cksum_cmd->cmd);
	cksum_cmd->cmdsize = SWAP_INT(cksum_cmd->cmdsize);
	cksum_cmd->cksum = SWAP_INT(cksum_cmd->cksum);
}

__private_extern__
void
swap_uuid_command(
struct uuid_command *uuid_cmd,
enum byte_sex target_byte_sex)
{
	uuid_cmd->cmd = SWAP_INT(uuid_cmd->cmd);
	uuid_cmd->cmdsize = SWAP_INT(uuid_cmd->cmdsize);
}

__private_extern__
void
swap_linkedit_data_command(
struct linkedit_data_command *ld,
enum byte_sex target_byte_sex)
{
	ld->cmd = SWAP_INT(ld->cmd);
	ld->cmdsize = SWAP_INT(ld->cmdsize);
	ld->dataoff = SWAP_INT(ld->dataoff);
	ld->datasize = SWAP_INT(ld->datasize);
}

__private_extern__
void
swap_version_min_command(
struct version_min_command *ver_cmd,
enum byte_sex target_byte_sex)
{
	ver_cmd->cmd = SWAP_INT(ver_cmd->cmd);
	ver_cmd->cmdsize = SWAP_INT(ver_cmd->cmdsize);
	ver_cmd->version = SWAP_INT(ver_cmd->version);
}

__private_extern__
void
swap_build_version_command(
struct build_version_command *bv,
enum byte_sex target_byte_sex)
{
	bv->cmd = SWAP_INT(bv->cmd);
	bv->cmdsize = SWAP_INT(bv->cmdsize);
	bv->platform = SWAP_INT(bv->platform);
	bv->minos = SWAP_INT(bv->minos);
	bv->sdk = SWAP_INT(bv->sdk);
	bv->ntools = SWAP_INT(bv->ntools);
}

__private_extern__
void
swap_build_tool_version(
struct build_tool_version *btv,
uint32_t ntools,
enum byte_sex target_byte_sex)
{
    uint32_t i;

	for(i = 0; i < ntools; i++){
	    btv[i].tool = SWAP_INT(btv[i].tool);
	    btv[i].version = SWAP_INT(btv[i].version);
	}
}

__private_extern__
void swap_rpath_command(
struct rpath_command *rpath_cmd,
enum byte_sex target_byte_sex)
{
	rpath_cmd->cmd = SWAP_INT(rpath_cmd->cmd);
	rpath_cmd->cmdsize = SWAP_INT(rpath_cmd->cmdsize);
	rpath_cmd->path.offset = SWAP_INT(rpath_cmd->path.offset);
}

__private_extern__
 void
swap_encryption_command(
struct encryption_info_command *ec,
enum byte_sex target_byte_sex)
{
	ec->cmd = SWAP_INT(ec->cmd);
	ec->cmdsize = SWAP_INT(ec->cmdsize);
	ec->cryptoff = SWAP_INT(ec->cryptoff);
	ec->cryptsize = SWAP_INT(ec->cryptsize);
	ec->cryptid = SWAP_INT(ec->cryptid);
}

__private_extern__
 void
swap_encryption_command_64(
struct encryption_info_command_64 *ec,
enum byte_sex target_byte_sex)
{
	ec->cmd = SWAP_INT(ec->cmd);
	ec->cmdsize = SWAP_INT(ec->cmdsize);
	ec->cryptoff = SWAP_INT(ec->cryptoff);
	ec->cryptsize = SWAP_INT(ec->cryptsize);
	ec->cryptid = SWAP_INT(ec->cryptid);
	ec->cryptid = SWAP_INT(ec->pad);
}

__private_extern__
 void
swap_linker_option_command(
struct linker_option_command *lo,
enum byte_sex target_byte_sex)
{
	lo->cmd = SWAP_INT(lo->cmd);
	lo->cmdsize = SWAP_INT(lo->cmdsize);
	lo->count = SWAP_INT(lo->count);
}

__private_extern__
 void
swap_dyld_info_command(
struct dyld_info_command *ed,
enum byte_sex target_byte_sex)
{
	ed->cmd = SWAP_INT(ed->cmd);
	ed->cmdsize = SWAP_INT(ed->cmdsize);
	ed->rebase_off = SWAP_INT(ed->rebase_off);
	ed->rebase_size = SWAP_INT(ed->rebase_size);
	ed->bind_off = SWAP_INT(ed->bind_off);
	ed->bind_size = SWAP_INT(ed->bind_size);
	ed->weak_bind_off = SWAP_INT(ed->weak_bind_off);
	ed->weak_bind_size = SWAP_INT(ed->weak_bind_size);
	ed->lazy_bind_off = SWAP_INT(ed->lazy_bind_off);
	ed->lazy_bind_size = SWAP_INT(ed->lazy_bind_size);
	ed->export_off = SWAP_INT(ed->export_off);
	ed->export_size = SWAP_INT(ed->export_size);
}

__private_extern__
void
swap_entry_point_command(
struct entry_point_command *ep,
enum byte_sex target_byte_sex)
{
	ep->cmd = SWAP_INT(ep->cmd);
	ep->cmdsize = SWAP_INT(ep->cmdsize);
	ep->entryoff = SWAP_LONG_LONG(ep->entryoff);
	ep->stacksize = SWAP_LONG_LONG(ep->stacksize);
}

__private_extern__
void
swap_source_version_command(
struct source_version_command *sv,
enum byte_sex target_byte_sex)
{
	sv->cmd = SWAP_INT(sv->cmd);
	sv->cmdsize = SWAP_INT(sv->cmdsize);
	sv->version = SWAP_LONG_LONG(sv->version);
}

__private_extern__
void
swap_note_command(
struct note_command *nc,
enum byte_sex target_byte_sex)
{
	nc->cmd = SWAP_INT(nc->cmd);
	nc->cmdsize = SWAP_INT(nc->cmdsize);
	nc->offset = SWAP_LONG_LONG(nc->offset);
	nc->size = SWAP_LONG_LONG(nc->size);
}

__private_extern__
void
swap_nlist(
struct nlist *symbols,
uint32_t nsymbols,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nsymbols; i++){
	    symbols[i].n_un.n_strx = SWAP_INT(symbols[i].n_un.n_strx);
	    /* n_type */
	    /* n_sect */
	    symbols[i].n_desc = SWAP_SHORT(symbols[i].n_desc);
	    symbols[i].n_value = SWAP_INT(symbols[i].n_value);
	}
}

__private_extern__
void
swap_nlist_64(
struct nlist_64 *symbols,
uint32_t nsymbols,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nsymbols; i++){
	    symbols[i].n_un.n_strx = SWAP_INT(symbols[i].n_un.n_strx);
	    /* n_type */
	    /* n_sect */
	    symbols[i].n_desc = SWAP_SHORT(symbols[i].n_desc);
	    symbols[i].n_value = SWAP_LONG_LONG(symbols[i].n_value);
	}
}

__private_extern__
void
swap_ranlib(
struct ranlib *ranlibs,
uint32_t nranlibs,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nranlibs; i++){
	    ranlibs[i].ran_un.ran_strx = SWAP_INT(ranlibs[i].ran_un.ran_strx);
	    ranlibs[i].ran_off = SWAP_INT(ranlibs[i].ran_off);
	}
}

__private_extern__
void
swap_ranlib_64(
struct ranlib_64 *ranlibs,
uint64_t nranlibs,
enum byte_sex target_byte_sex)
{
    uint64_t i;

	for(i = 0; i < nranlibs; i++){
	    ranlibs[i].ran_un.ran_strx =
		SWAP_LONG_LONG(ranlibs[i].ran_un.ran_strx);
	    ranlibs[i].ran_off = SWAP_LONG_LONG(ranlibs[i].ran_off);
	}
}

__private_extern__
void
swap_relocation_info(
struct relocation_info *relocs,
uint32_t nrelocs,
enum byte_sex target_byte_sex)
{
    uint32_t i;
    enum byte_sex host_byte_sex;
    enum bool to_host_byte_sex, scattered;

    struct swapped_relocation_info {
	int32_t r_address;
	union {
	    struct {
		unsigned int
		    r_type:4,
		    r_extern:1,
		    r_length:2,
		    r_pcrel:1,
		    r_symbolnum:24;
	    } fields;
	    uint32_t word;
	} u;
    } sr;

    struct swapped_scattered_relocation_info {
	uint32_t word;
	int32_t	r_value;
    } *ssr;

	host_byte_sex = get_host_byte_sex();
	to_host_byte_sex = (enum bool)(target_byte_sex == host_byte_sex);

	for(i = 0; i < nrelocs; i++){
	    if(to_host_byte_sex)
		scattered = (enum bool)(
			(SWAP_INT(relocs[i].r_address) & R_SCATTERED) != 0);
	    else
		scattered = (enum bool)
			(((relocs[i].r_address) & R_SCATTERED) != 0);
	    if(scattered == FALSE){
		if(to_host_byte_sex){
		    memcpy(&sr, relocs + i, sizeof(struct relocation_info));
		    sr.r_address = SWAP_INT(sr.r_address);
		    sr.u.word = SWAP_INT(sr.u.word);
		    relocs[i].r_address = sr.r_address;
		    relocs[i].r_symbolnum = sr.u.fields.r_symbolnum;
		    relocs[i].r_pcrel = sr.u.fields.r_pcrel;
		    relocs[i].r_length = sr.u.fields.r_length;
		    relocs[i].r_extern = sr.u.fields.r_extern;
		    relocs[i].r_type = sr.u.fields.r_type;
		}
		else{
		    sr.r_address = relocs[i].r_address;
		    sr.u.fields.r_symbolnum = relocs[i].r_symbolnum;
		    sr.u.fields.r_length = relocs[i].r_length;
		    sr.u.fields.r_pcrel = relocs[i].r_pcrel;
		    sr.u.fields.r_extern = relocs[i].r_extern;
		    sr.u.fields.r_type = relocs[i].r_type;
		    sr.r_address = SWAP_INT(sr.r_address);
		    sr.u.word = SWAP_INT(sr.u.word);
		    memcpy(relocs + i, &sr, sizeof(struct relocation_info));
		}
	    }
	    else{
		ssr = (struct swapped_scattered_relocation_info *)(relocs + i);
		ssr->word = SWAP_INT(ssr->word);
		ssr->r_value = SWAP_INT(ssr->r_value);
	    }
	}
}

__private_extern__
void
swap_indirect_symbols(
uint32_t *indirect_symbols,
uint32_t nindirect_symbols,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nindirect_symbols; i++)
	    indirect_symbols[i] = SWAP_INT(indirect_symbols[i]);
}

__private_extern__
void
swap_dylib_reference(
struct dylib_reference *refs,
uint32_t nrefs,
enum byte_sex target_byte_sex)
{
    struct swapped_dylib_reference {
	union {
	    struct {
		uint32_t
		    flags:8,
		    isym:24;
	    } fields;
	    uint32_t word;
	} u;
    } sref;

    uint32_t i;
    enum byte_sex host_byte_sex;

	host_byte_sex = get_host_byte_sex();

	for(i = 0; i < nrefs; i++){
	    if(target_byte_sex == host_byte_sex){
		memcpy(&sref, refs + i, sizeof(struct swapped_dylib_reference));
		sref.u.word = SWAP_INT(sref.u.word);
		refs[i].flags = sref.u.fields.flags;
		refs[i].isym = sref.u.fields.isym;
	    }
	    else{
		sref.u.fields.isym = refs[i].isym;
		sref.u.fields.flags = refs[i].flags;
		sref.u.word = SWAP_INT(sref.u.word);
		memcpy(refs + i, &sref, sizeof(struct swapped_dylib_reference));
	    }
	}

}

__private_extern__
void
swap_dylib_module(
struct dylib_module *mods,
uint32_t nmods,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nmods; i++){
	    mods[i].module_name = SWAP_INT(mods[i].module_name);
	    mods[i].iextdefsym  = SWAP_INT(mods[i].iextdefsym);
	    mods[i].nextdefsym  = SWAP_INT(mods[i].nextdefsym);
	    mods[i].irefsym     = SWAP_INT(mods[i].irefsym);
	    mods[i].nrefsym     = SWAP_INT(mods[i].nrefsym);
	    mods[i].ilocalsym   = SWAP_INT(mods[i].ilocalsym);
	    mods[i].nlocalsym   = SWAP_INT(mods[i].nlocalsym);
	    mods[i].iextrel     = SWAP_INT(mods[i].iextrel);
	    mods[i].nextrel     = SWAP_INT(mods[i].nextrel);
	    mods[i].iinit_iterm = SWAP_INT(mods[i].iinit_iterm);
	    mods[i].ninit_nterm = SWAP_INT(mods[i].ninit_nterm);
	    mods[i].objc_module_info_addr =
				  SWAP_INT(mods[i].objc_module_info_addr);
	    mods[i].objc_module_info_size =
				  SWAP_INT(mods[i].objc_module_info_size);
	}
}

__private_extern__
void
swap_dylib_module_64(
struct dylib_module_64 *mods,
uint32_t nmods,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < nmods; i++){
	    mods[i].module_name = SWAP_INT(mods[i].module_name);
	    mods[i].iextdefsym  = SWAP_INT(mods[i].iextdefsym);
	    mods[i].nextdefsym  = SWAP_INT(mods[i].nextdefsym);
	    mods[i].irefsym     = SWAP_INT(mods[i].irefsym);
	    mods[i].nrefsym     = SWAP_INT(mods[i].nrefsym);
	    mods[i].ilocalsym   = SWAP_INT(mods[i].ilocalsym);
	    mods[i].nlocalsym   = SWAP_INT(mods[i].nlocalsym);
	    mods[i].iextrel     = SWAP_INT(mods[i].iextrel);
	    mods[i].nextrel     = SWAP_INT(mods[i].nextrel);
	    mods[i].iinit_iterm = SWAP_INT(mods[i].iinit_iterm);
	    mods[i].ninit_nterm = SWAP_INT(mods[i].ninit_nterm);
	    mods[i].objc_module_info_addr =
				  SWAP_LONG_LONG(mods[i].objc_module_info_addr);
	    mods[i].objc_module_info_size =
				  SWAP_INT(mods[i].objc_module_info_size);
	}
}

__private_extern__
void
swap_dylib_table_of_contents(
struct dylib_table_of_contents *tocs,
uint32_t ntocs,
enum byte_sex target_byte_sex)
{
    uint32_t i;
#ifdef __MWERKS__
    enum byte_sex dummy;
        dummy = target_byte_sex;
#endif

	for(i = 0; i < ntocs; i++){
	    tocs[i].symbol_index = SWAP_INT(tocs[i].symbol_index);
	    tocs[i].module_index = SWAP_INT(tocs[i].module_index);
	}
}

__private_extern__
void
swap_xar_header(
struct xar_header *xar,
enum byte_sex target_byte_sex)
{
	xar->magic = SWAP_INT(xar->magic);
	xar->size = SWAP_SHORT(xar->size);
	xar->version = SWAP_SHORT(xar->version);
	xar->toc_length_compressed = SWAP_LONG_LONG(xar->toc_length_compressed);
	xar->toc_length_uncompressed =
				SWAP_LONG_LONG(xar->toc_length_uncompressed);
	xar->cksum_alg = SWAP_INT(xar->cksum_alg);
}
