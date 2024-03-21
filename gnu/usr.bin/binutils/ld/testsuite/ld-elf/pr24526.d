#ld: --gc-sections -e _start
#target: [check_gc_sections_available]
#readelf: -SW

#...
 +\[ *[0-9]+\] \.bar +PROGBITS +[0-9a-f]+ +[0-9a-f]+ [0-9a-f]+ +00 +AL .*
#...
 +\[ *[0-9]+\] \.zed +PROGBITS +[0-9a-f]+ +[0-9a-f]+ [0-9a-f]+ +00 +AL .*
#pass
