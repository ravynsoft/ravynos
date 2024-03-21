#name: PHDRS headers 3a
#source: phdrs.s
#ld: -T phdrs3a.t
#readelf: -l --wide

#...
[ \t]+LOAD[ x0-9a-f]+ R [ x0-9a-f]+
[ \t]+LOAD[ x0-9a-f]+ E [ x0-9a-f]+
#pass
