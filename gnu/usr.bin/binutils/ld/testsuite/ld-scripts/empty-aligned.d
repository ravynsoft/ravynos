#source: empty-aligned.s
#ld: -T empty-aligned.t
#readelf: -S --wide

#...
.* .text .*
!.* .text[234] .*
#pass
