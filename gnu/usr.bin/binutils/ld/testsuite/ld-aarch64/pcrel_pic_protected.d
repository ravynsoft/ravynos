#name: PC-Rel relocation against protected
#source: pcrel-protected.s
#target: [check_shared_lib_support]
#ld: -shared -e0 --defsym protected_a=0x1000 --defsym protected_b=0x1010 --defsym protected_c=0x1020
#readelf: -r -W
#...
There are no relocations in this file.
