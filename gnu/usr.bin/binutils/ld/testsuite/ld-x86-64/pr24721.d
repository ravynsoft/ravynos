#source: pr24721a.s
#source: pr24721b.s
#as: --64 -defsym __64_bit__=1 -mx86-used-note=no
#ld: -r -melf_x86_64 -Map tmpdir/pr24721.map
#readelf: -n
#map: pr24721.map
