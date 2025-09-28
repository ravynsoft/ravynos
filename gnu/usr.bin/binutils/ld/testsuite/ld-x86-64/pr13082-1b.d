#source: pr13082-1.s
#name: PR ld/13082-1 (b)
#as: --x32
#ld: -pie -melf32_x86_64
#readelf: -d -r --wide

Dynamic section at offset 0x[0-9a-f]+ contains [0-9]+ entries:
#...
 0x[0-9a-f]+ +\(RELACOUNT\) +1
#...
Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset     Info    Type                Sym. Value  Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_RELATIVE64 +[0-9a-f]+
