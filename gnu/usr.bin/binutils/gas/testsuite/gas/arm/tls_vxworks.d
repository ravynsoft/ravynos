#objdump: -dr
#name: TLS
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince
# This is the VxWorks variant of this file.
#source: tls.s
#noskip: *-*-vxworks*

# Test generation of TLS relocations

.*: +file format .*arm.*

Disassembly of section .text:

00+0 <arm_fn>:
   0:	e1a00000 	nop			\@ \(mov r0, r0\)
			0: R_ARM_TLS_DESCSEQ	af
   4:	e59f0014 	ldr	r0, \[pc, \#20\]	@ 20 <\.arm_pool\+0x10>
   8:	fa000000 	blx	8 <ae\+0x8>
			8: R_ARM_TLS_CALL	ae
# ??? The addend is appearing in both the RELA field and the
# contents.  Shouldn't it be just one?  bfd_install_relocation
# appears to write the addend into the contents unconditionally,
# yet somehow this does not happen for the majority of relocations.
   c:	e1a00000 	nop			\@ \(mov r0, r0\)
00000010 <.arm_pool>:
  10:	00000008 	.word	0x00000008
			10: R_ARM_TLS_GD32	aa\+0x8
  14:	0000000c 	.word	0x0000000c
			14: R_ARM_TLS_LDM32	ab\+0xc
  18:	00000010 	.word	0x00000010
#pass
