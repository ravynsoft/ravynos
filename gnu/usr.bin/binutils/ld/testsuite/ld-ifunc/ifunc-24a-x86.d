#source: ifunc-24-x86.s
#ld:
#readelf: -r --wide
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

Relocation section '.rel(a|).plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym.* Value +Symbol's Name.*
[0-9a-f]+[ ]+[0-9a-f]+[ ]+R_(386|X86_64)+_IRELATIVE[ ]*[0-9a-f]*
