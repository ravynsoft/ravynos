#ld: --gc-sections -e _start
#target: [check_gc_sections_available]
#readelf: -SW

#...
 +\[ *[0-9]+\] \.stack_sizes +PROGBITS +0+ +[0-9a-f]+ 0+1 +00 +L +[0-9] .*
#pass
