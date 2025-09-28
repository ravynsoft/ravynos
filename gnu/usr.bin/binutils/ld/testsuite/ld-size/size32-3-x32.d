#source: size32-3a.s
#source: size32-3b.s
#as: --x32
#ld: -shared -melf32_x86_64 -z nocombreloc
#readelf: -r --wide
#target: x86_64-*-*

#failif
#...
.* +R_X86_64_NONE +.*
#...
