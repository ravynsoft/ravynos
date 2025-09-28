#as: --x32
#ld: -shared -melf32_x86_64
#readelf: -SW

#...
 +\[ [0-9]\] .debug_loclists .*
#pass
