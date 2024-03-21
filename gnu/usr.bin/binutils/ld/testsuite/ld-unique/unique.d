#ld: -r
#readelf: -sh
#name: Linker setting GNU OSABI on STB_GNU_UNIQUE symbol (PR 10549)

#...
 *OS/ABI: +UNIX - GNU
#...
 *[0-9]+: +[0-9a-f]+ +[0-9]+ +OBJECT +UNIQUE +DEFAULT +[0-9]+ a_val
#pass
