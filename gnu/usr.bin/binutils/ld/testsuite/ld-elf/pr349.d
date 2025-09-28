#source: pr349-1.s
#source: pr349-2.s
#ld: -r
#readelf: -S
#xfail: [uses_genelf]
# if not using elf.em, you don't get fancy section handling

#...
.* .abcxyz .*
#...
.* .abcxyz .*
#pass
