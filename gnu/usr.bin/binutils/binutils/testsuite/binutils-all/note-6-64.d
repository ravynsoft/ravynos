#PROG: objcopy
#readelf: --notes --wide
#objcopy: --merge-notes
#name: v3 gnu build attribute note merging (64-bit)
#source: note-6-64.s

#...
Displaying notes found in: .gnu.build.attributes
[ 	]+Owner[ 	]+Data size[ 	]+Description
[ 	]+GA\$<version>3p1[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106 \(note_test\)
[ 	]+GA\*<stack prot>off[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\$<tool>gcc 8.3.1 20190507[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\*<ABI>0x[0-9a-f]+[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\*<PIC>PIC[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\!<short enum>false[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\*FORTIFY:0xff[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\*GOW:0x700[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x106
[ 	]+GA\$<version>3p1[ 	]+0x00000010[ 	]+func[ 	]+Applies to region from 0x102 to 0x106
[ 	]+GA\$<tool>hello world[ 	]+0x00000000[ 	]+func[ 	]+Applies to region from 0x102 to 0x106
#...
