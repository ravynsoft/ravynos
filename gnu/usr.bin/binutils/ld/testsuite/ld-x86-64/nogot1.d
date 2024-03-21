#ld: --shared -melf_x86_64
#readelf: -S --wide
#as: --64

#...
[ 	]*\[.*\][ 	]+\.dynamic[ 	]+DYNAMIC.*
#...
[ 	]*\[.*\][ 	]+.*STRTAB.*
#pass
