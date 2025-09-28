	.syntax unified
	.thumb
	.align 2
@ Wide instruction in IT block is deprecated.
it eq
ldrdeq r0, [r1]

@ This IT block is not deprecated.
it eq
moveq r2, r3

@ IT block of more than one instruction is deprecated.
itt eq
moveq r0, r1
moveq r2, r3

@ Even for auto IT blocks
moveq r2, r3
movne r2, r3

adds r0, r1

@ This automatic IT block is valid
moveq r2,r3

add r0, r1, r2

@ This one is too wide.
ldrdeq r0, [r1]

add r0, r1, r2

@ Test automatic IT block generation at end of a file.
movne r0, r1
moveq r1, r0

@ Test the various classes of 16-bit instructions that are deprecated.
it eq
svceq 0

it eq
uxtheq r0, r1

it eq
addeq r0, pc, #0

it eq
ldreq r0, [pc, #4]

it eq
bxeq pc

it eq
addeq r0, pc, pc

it eq
addeq pc, r0, r0

it eq
addeq sp, sp, #12

@ Misaligned immediate.
it eq
addeq sp, sp, #3

it eq
subeq sp, sp, #12

@ Misaligned immediate.
it eq
subeq sp, sp, #3
