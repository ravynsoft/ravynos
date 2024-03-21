#ld: --shared -melf_i386 -z notext
#readelf: -S --wide
#as: --32

#...
[ 	]*\[.*\][ 	]+\.dynamic[ 	]+DYNAMIC.*
#...
[ 	]*\[.*\][ 	]+.*STRTAB.*
#pass
