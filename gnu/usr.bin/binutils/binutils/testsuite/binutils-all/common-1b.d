#source: common-1.s
#as: --elf-stt-common=no
#PROG: objcopy
#readelf: -s -W

#...
 +[0-9]+: +0+4 +30 +OBJECT +GLOBAL +DEFAULT +COM +foobar
#pass
