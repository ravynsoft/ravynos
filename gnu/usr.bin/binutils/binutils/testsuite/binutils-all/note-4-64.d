#PROG: objcopy
#readelf: --notes --wide
#objcopy: --merge-notes
#name: v3 gnu build attribute notes (64-bit)
#source: note-4-64.s

#...
Displaying notes found in: .gnu.build.attributes
[ 	]+Owner[ 	]+Data size[ 	]+Description
[ 	]+GA\$<version>3p3[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120 \(note_4.s\)
[ 	]+GA\*<stack prot>off[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\$<tool>gcc 7.2.1 20170915[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\*<ABI>0x[0-9a-f]+[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\*<PIC>PIC[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\!<short enum>false[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\*FORTIFY:0xff[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\*GOW:0x700[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x120
[ 	]+GA\*<stack prot>strong[ 	]+0x00000010[ 	]+func[ 	]+Applies to region from 0x110 to 0x120.*
#...
