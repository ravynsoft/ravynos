#source: largecomm-1.s
#as: --elf-stt-common=no
#ld: -r -z common
#readelf: -s -W

#...
 +[0-9]+: +0+4 +30 +COMMON +GLOBAL +DEFAULT +LARGE_COM +foobar
#pass
