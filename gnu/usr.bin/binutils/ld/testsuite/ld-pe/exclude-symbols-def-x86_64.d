#source: exclude-symbols-def-x86_64.s
#target: x86_64-*-cygwin* x86_64-*-pe x86_64-*-mingw*
#ld: -shared ${srcdir}/${subdir}/exclude-symbols-def.def
#objdump: -p

#...
.*\[[ 	]*0\] sym1
.*\[[ 	]*1\] sym3
.*\[[ 	]*2\] sym5
#pass
