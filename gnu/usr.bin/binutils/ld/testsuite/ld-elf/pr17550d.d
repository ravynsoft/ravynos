#source: pr17550-2.s
#source: pr17550-4.s
#ld: -r
#readelf: -s --wide
# Disabled on alpha because alpha has a different .set directive.
# Generic linker targets don't support comdat group sections.
#xfail: alpha-*-* [is_generic]

#...
 +[0-9]+: +[0-9a-f]+ +0 +OBJECT +GLOBAL +DEFAULT +UND y
#pass
