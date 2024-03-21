#as: -mpower10
#objdump: -dr -Mpower10

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(f0 02 ff 6e|6e ff 02 f0) 	xvtlsbb cr0,vs63
.*:	(f0 82 07 6c|6c 07 82 f0) 	xvtlsbb cr1,vs0
.*:	(f1 02 f7 6e|6e f7 02 f1) 	xvtlsbb cr2,vs62
.*:	(f1 82 0f 6c|6c 0f 82 f1) 	xvtlsbb cr3,vs1
.*:	(f2 02 ef 6e|6e ef 02 f2) 	xvtlsbb cr4,vs61
.*:	(f2 82 17 6c|6c 17 82 f2) 	xvtlsbb cr5,vs2
.*:	(f3 02 e7 6e|6e e7 02 f3) 	xvtlsbb cr6,vs60
.*:	(f3 82 1f 6c|6c 1f 82 f3) 	xvtlsbb cr7,vs3
