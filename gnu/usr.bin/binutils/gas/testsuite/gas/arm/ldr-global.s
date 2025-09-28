@ Test pc-relative loads from global objects defined in the same text segment.
@ See tc-arm.c:arm_force_relocation.
.arch armv7-a
.fpu vfp
.syntax unified
.text
foo_arm:
  ldr r0, bar
  ldrsh r0, bar
  vldr s0, bar
.thumb
foo_thumb:
  ldr r0, bar
  ldr.n r0, bar
  vldr s0, bar
  ldr.w r0, bar
  
.align 2
.globl bar
bar:
  .word 42

