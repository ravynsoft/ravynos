#as: -mpower10
#objdump: -dr -Mpower10
#name: VSX ISA power10 instructions

.*

Disassembly of section \.text:

0+0 <vsx4>:
.*:	(f0 50 6f 6f|6f 6f 50 f0) 	xvcvbf16spn vs34,vs45
.*:	(f1 f1 27 6f|6f 27 f1 f1) 	xvcvspbf16 vs47,vs36
#pass
