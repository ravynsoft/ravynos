#source: symtocbase-1.s
#source: symtocbase-2.s
#as: -a64
#ld: -shared -melf64ppc
#objdump: -dj.data -z
#target: powerpc64*-*-*

.*

Disassembly of section \.data:

.* <i>:
#...
.*	\.long 0x28000
.*	\.long 0x0
.*	\.long 0x28000
.*	\.long 0x0
.*	\.long 0x38000
#...
.*	\.long 0x38000
.*	\.long 0x0
.*	\.long 0x28000
.*	\.long 0x0
.*	\.long 0x38000
#pass
