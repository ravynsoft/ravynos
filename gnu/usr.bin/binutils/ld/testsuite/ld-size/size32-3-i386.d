#source: size32-3a.s
#source: size32-3b.s
#as: --32
#ld: -shared -melf_i386 -z nocombreloc
#readelf: -r --wide
#target: x86_64-*-* i?86-*-*

#failif
#...
.* +R_386_NONE +.*
#...
