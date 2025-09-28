#source: pr22393-1.s
#ld: -shared -z separate-code
#readelf: -l --wide
#target: *-*-linux-gnu *-*-gnu* *-*-nacl*

#failif
#...
 +[0-9]+  +.*(\.note|\.gnu|\.hash|\.dyn|\.rel).*\.text.*
#...
