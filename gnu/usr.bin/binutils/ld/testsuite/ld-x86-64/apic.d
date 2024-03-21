#name: 32-bit relocs w/ index but no base
#ld: --defsym APIC_BASE=0xfee00000
#objdump: -dw

.*: +file format .*


Disassembly of section \.text:

#...
[0-9a-f]+[ 	]+<apic_read>:
[ 	]*[0-9a-f]+:[ 	]+67 8b 04 bd 00 00 e0 fe[ 	]+mov[ 	]+(0xfee|-0x12)00000\(,%edi,4\),%eax
[ 	]*[0-9a-f]+:[ 	]+c3[ 	]+retq?[ 	]*
#...
[0-9a-f]+[ 	]+<apic_write>:
[ 	]*[0-9a-f]+:[ 	]+67 89 34 bd 00 00 e0 fe[ 	]+mov[ 	]+%esi,(0xfee|-0x12)00000\(,%edi,4\)
[ 	]*[0-9a-f]+:[ 	]+c3[ 	]+retq?[ 	]*
#pass
