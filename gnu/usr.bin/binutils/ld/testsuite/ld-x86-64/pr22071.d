#as: --64
#ld: -melf_x86_64 -shared
#readelf: -d --wide

#...
.*\(TLSDESC_PLT\).*
.*\(TLSDESC_GOT\).*
#pass
