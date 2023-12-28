#source: ifunc-14b.s
#source: ifunc-14a.s
#ld: -shared -m elf_i386 -z nocombreloc
#as: --32
#readelf: -d --wide
#target: x86_64-*-* i?86-*-*
#notarget: *-*-lynxos *-*-nto*

#failif
#...
.*\(TEXTREL\).*
#...
