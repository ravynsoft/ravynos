#source: start.s
#ld: -T pr14052.t
#readelf: -s

#failif
#...
 +[0-9]+: +[0-9a-f]+ +0 +(OBJECT|NOTYPE) +GLOBAL +DEFAULT +ABS _data_start
#...
