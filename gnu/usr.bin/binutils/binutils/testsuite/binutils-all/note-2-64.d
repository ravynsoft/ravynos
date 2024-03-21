#PROG: objcopy
#readelf: --notes --wide
#objcopy: --merge-notes
#name: merge notes section (64-bits)
#source: note-2-64.s
#notarget: sparc64-*-*
# Internally the Sparc64 backend uses two relocs for every one reloc visible externally.
# Unfortunately the BFD library does not provide a target specific way to delete individual
# relocs, so the note merging feature fails.

#...
[ 	]+Owner[ 	]+Data size[ 	]+Description
[ 	]+GA\$<version>3p1[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x10b \(note1.s\)
[ 	]+GA\$<tool>gcc 7.0.1[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x10b
[ 	]+GA\*<ABI>0x0[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x108 \(note1.s\)
[ 	]+GA\+<stack prot>true[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x104 \(note1.s\)
[ 	]+GA\*<PIC>static[ 	]+0x00000000[ 	]+OPEN[ 	]+Applies to region from 0x100 to 0x104
[ 	]+GA!<stack prot>false[ 	]+0x00000010[ 	]+OPEN[ 	]+Applies to region from 0x104 to 0x108 \(note2.s\)
[ 	]+GA\*<PIC>pic[ 	]+0x00000010[ 	]+func[ 	]+Applies to region from 0x104 to 0x106 \(func1\)
#...
