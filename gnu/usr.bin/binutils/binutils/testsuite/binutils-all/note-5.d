#PROG: objcopy
#as: --generate-missing-build-notes=yes
#readelf: --notes --wide
#name: assembler generated build notes
#source: note-5.s

#...
Displaying notes found in: .gnu.build.attributes
[ 	]+Owner[ 	]+Data size[ 	]+Description
[ 	]+GA\$<version>3a1[ 	]+0x000000(08|10)[ 	]+OPEN[ 	]+Applies to region from 0 to 0x.. \(note_5.s\)
#...
