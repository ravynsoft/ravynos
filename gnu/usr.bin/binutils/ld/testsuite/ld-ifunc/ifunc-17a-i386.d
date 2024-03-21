#source: ifunc-17a.s
#source: ifunc-17b.s
#ld: -static -m elf_i386
#as: --32
#readelf: -s --wide
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

#...
 +[0-9]+: +[0-9a-f]+ +4 +OBJECT +GLOBAL +DEFAULT +[1-9] foo
#pass
