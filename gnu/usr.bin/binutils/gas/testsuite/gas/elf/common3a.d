#source: common3.s
#as: --elf-stt-common=yes
#readelf: -s -W
# MIPS'es IRIX emulation puts lbuf (STB_LOCAL) after the globals in the
# symbol table, and that mode is hard to check for (see irixemul in
# binutils/testsuite/binutils-all/mips/mips.exp)
#notarget: mips*-*-*

#...
 +[0-9]+: +0+ +8 +(OBJECT|NOTYPE) +LOCAL +DEFAULT +[1-9] +lbuf
#...
 +[0-9]+: +0+4 +30 +COMMON +GLOBAL +DEFAULT +COM +foobar
#...
 +[0-9]+: +0+8 +4 +COMMON +GLOBAL +DEFAULT +COM +buf1
 +[0-9]+: +0+8 +4 +COMMON +GLOBAL +DEFAULT +COM +buf2
#pass
