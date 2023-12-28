#PROG: objcopy
#readelf: --notes --wide
#objcopy: --merge-notes
#name: v2 gnu build attribute notes (64-bit)
#source: note-3-64.s

#...
Displaying notes found in: .gnu.build.attributes
[ 	]+Owner[ 	]+Data size[ 	]+Description
[ 	]+GA\$<version>3p1[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122 \(note_1.s\)
[ 	]+GA\*<stack prot>off[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
[ 	]+GA\$<tool>gcc 6.3.1 20161221[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
[ 	]+GA\*<ABI>0x[0-9a-f]+[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
[ 	]+GA\*<PIC>PIC[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
[ 	]+GA\!<short enum>false[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
[ 	]+GA\*FORTIFY:0xff[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
[ 	]+GA\*GOW:0x700[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x122
#...
