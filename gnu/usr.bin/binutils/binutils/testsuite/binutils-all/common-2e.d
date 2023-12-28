#source: common-2.s
#as: --elf-stt-common=no
#PROG: objcopy
#objcopy: --elf-stt-common=yes
#readelf: -s -W

#...
 +[0-9]+: +0+4 +30 +TLS +GLOBAL +DEFAULT +COM +foobar
#pass
