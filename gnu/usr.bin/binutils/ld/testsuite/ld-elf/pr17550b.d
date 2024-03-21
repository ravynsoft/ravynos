#source: pr17550-2.s
#source: pr17550-1.s
#ld: -r
#readelf: -s --wide
# Disabled on alpha because alpha has a different .set directive.
#xfail: alpha-*-*

#failif
#...
 +[0-9]+: +[0-9a-f]+ +0 +OBJECT +GLOBAL +DEFAULT +UND x_alias
#...
