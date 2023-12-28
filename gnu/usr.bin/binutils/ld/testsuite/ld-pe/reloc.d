#name: PE base relocations
#ld: --enable-reloc-section
#objdump: -p
#xfail: mcore-*-*

.*:     file format .*

#...
PE File Base Relocations.*
Virtual Address: .* Number of fixups 4
[ 	]*reloc    0 offset    0 .* (LOW|HIGHLOW|DIR64)
[ 	]*reloc    1 offset    [248] .* (LOW|HIGHLOW|DIR64)
[ 	]*reloc    2 offset   [124]0 .* (LOW|HIGHLOW|DIR64)
[ 	]*reloc    3 offset    0 .* ABSOLUTE
#pass
