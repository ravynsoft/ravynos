#source: pr24721a.s
#source: pr24721b.s
#as: --x32 -mx86-used-note=no
#ld: -r -m elf32_x86_64 -Map tmpdir/pr24721.map
#readelf: -n
#map: pr24721.map
