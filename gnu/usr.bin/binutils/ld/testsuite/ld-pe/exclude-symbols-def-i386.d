#source: exclude-symbols-def-i386.s
#target: i?86-*-cygwin* i?86-*-pe i?86-*-mingw*
#ld: -shared ${srcdir}/${subdir}/exclude-symbols-def.def
#objdump: -p

#...
.*\[[ 	]*0\] sym1
.*\[[ 	]*1\] sym3
.*\[[ 	]*2\] sym5
#pass
