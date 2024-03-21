.arch armv7-r
.syntax unified
.text
.thumb
	.global foo
foo:
	.align 4
@ Section A6.1.3 "Use of 0b1101 as a register specifier".

@ R13 as the source or destination register of a mov instruction.
@ only register to register transfers without shifts are supported,
@ with no flag setting

mov	sp,r0
mov	r0,sp

@ Using the following instructions to adjust r13 up or down by a
@ multiple of 4:

add	sp,sp,#0
addw	sp,sp,#0
sub	sp,sp,#0
subw	sp,sp,#0
add	sp,sp,r0
add	sp,sp,r0,lsl #1
sub	sp,sp,r0
sub	sp,sp,r0,lsl #1

@ R13 as a base register <Rn> of any load/store instruction.

ldr	r0, [sp]
ldr	r0, [pc]
ldr	pc, [r0]
ldr	sp, [r0]
ldr	pc, [pc]
ldr	sp, [sp]
ldr	pc, [sp]
ldr	sp, [pc]

str	r0, [sp]
str	sp, [r0]
str	sp, [sp]

@ R13 as the first operand <Rn> in any add{s}, cmn, cmp, or sub{s} instruction.

add	r0, sp, r0
adds	r0, sp, r0
add	r0, sp, r0, lsl #1
adds	r0, sp, r0, lsl #1

cmn	sp, #0
cmn	sp, r0
cmn	sp, r0, lsl #1
cmp	sp, #0
cmp	sp, r0
cmp	sp, r0, lsl #1

sub	sp, #0
subs	sp, #0
sub	r0, sp, #0
subs	r0, sp, #0

@ ADD (sp plus immediate).

add	sp, #4
add	r0, sp, #4
adds	sp, #4
adds	r0, sp, #4
addw	r0, sp, #4

add	sp, sp, #4
adds	sp, sp, #4
addw	sp, sp, #4

@ ADD (sp plus register).

add	sp, r0
add	r0, sp, r0
add	r0, sp, r0, lsl #1
adds	sp, r0
adds	r0, sp, r0
adds	r0, sp, r0, lsl #1

add	sp, sp, r0
add	sp, sp, r0, lsl #1
adds	sp, sp, r0
adds	sp, sp, r0, lsl #1

add	sp, sp, sp

@ SUB (sp minus immediate).

sub	r0, sp , #0
subs	r0, sp , #0
subw	r0, sp , #0

sub	sp, sp , #0
subs	sp, sp , #0
subw	sp, sp , #0

@ SUB (sp minus register).

sub	sp, #0
subs	sp, #0
sub	r0, sp, r0, lsl #1
subs	r0, sp, r0, lsl #1

sub	sp, sp, r0, lsl #1
subs	sp, sp, r0, lsl #1

@ PC-related insns (equivalent to adr).

add	r0, pc, #4
sub	r0, pc, #4
adds	r0, pc, #4
subs	r0, pc, #4
addw	r0, pc, #4
subw	r0, pc, #4

@ nops to pad the section out to an alignment boundary.

nop
nop
nop
