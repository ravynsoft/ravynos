#name: i386 R_386_PC16 reloc in 16-bit mode
#as: --32 -mx86-used-note=no --generate-missing-build-notes=no
#source: ${srcdir}/../../../gas/testsuite/gas/i386/code16-2.s
#ld: -T code16.t
#objdump: -dw -Mi8086

.*: +file format .*


Disassembly of section .text.default_process_op.isra.0:

0+737c <default_process_op.isra.0>:
 +[a-f0-9]+:	66 c3                	retl

Disassembly of section .text.mpt_scsi_process_op:

0+f869 <mpt_scsi_process_op>:
 +[a-f0-9]+:	e9 10 7b             	jmp    737c <default_process_op.isra.0>
#pass
