
.*/tls-lib2-got.so:     file format elf32-.*arm.*
architecture: arm.*, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x[0-9a-f]+


Disassembly of section .got:

0+10(2e8|320) <.*>:
   10(2e8|320):	00010(260|298) 	.*
	...
   10(2f4|32c):	00000008 	.*
			10(2f4|32c): R_ARM_TLS_DESC	\*ABS\*
   10(2f8|330):	00000000 	.*
   10(2fc|334):	0000000c 	.*
			10(2fc|334): R_ARM_TLS_DESC	\*ABS\*
   10(300|338):	00000000 	.*
   10(304|33c):	80000002 	.*
			10(304|33c): R_ARM_TLS_DESC	glob1
   10(308|340):	00000000 	.*
   10(30c|344):	80000004 	.*
			10(30c|344): R_ARM_TLS_DESC	ext2
   10(310|348):	00000000 	.*
   10(314|34c):	80000005 	.*
			10(314|34c): R_ARM_TLS_DESC	ext1
   10(318|350):	00000000 	.*
   10(31c|354):	80000007 	.*
			10(31c|354): R_ARM_TLS_DESC	glob2
	...
