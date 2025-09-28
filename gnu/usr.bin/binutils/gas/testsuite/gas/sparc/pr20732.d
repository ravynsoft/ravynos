#as: 
#objdump: -dr --prefix-addresses
#name: PR20732 - make sure that the L and LL suffix is accepted on constant values

.*: +file format .*

Disassembly of section .text:
[0x]+000.*sethi[ 	]+%hi\(0x3b9ac800\), %l5
[0x]+004.*or[ 	]+%l5, 0x200, %l5.*
[0x]+008.*sethi[ 	]+%hi\(0x3b9ac800\), %l5
[0x]+00c.*or[ 	]+%l5, 0x200, %l5.*
[0x]+010.*sethi[ 	]+%hi\(0x3b9ac800\), %l5
[0x]+014.*or[ 	]+%l5, 0x200, %l5.*
#pass
