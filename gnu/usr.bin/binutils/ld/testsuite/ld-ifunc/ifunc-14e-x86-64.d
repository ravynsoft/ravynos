#source: ifunc-14a.s
#source: ifunc-14c.s
#source: ifunc-14b.s
#ld: -shared -m elf_x86_64 -z nocombreloc
#as: --64
#readelf: -r --wide
#target: x86_64-*-*

#failif
#...
.* +R_X86_64_NONE +.*
#...
