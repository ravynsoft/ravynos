#source: multibss1.s
#ld: -e 0
#readelf: -l --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: hppa64-*-*
# hppa64 default script add 16 bytes at start of .data giving 0x500010 p_memsz

#...
 +LOAD +0x[^ ]+ +0x[^ ]+ +0x[^ ]+ +0x[^ ]+ +0x500000 .*
#       p_offset p_vaddr  p_paddr  p_filesz
#pass
