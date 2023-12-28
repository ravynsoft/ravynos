#name: --gc-sections with note section
#ld: --gc-sections -e _start
#readelf: -S --wide

#...
.* .note.ABI-tag[ 	]+NOTE.*
#...
