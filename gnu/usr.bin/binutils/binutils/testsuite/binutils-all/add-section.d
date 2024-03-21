#PROG: objcopy
#name: objcopy add-section
#source: empty.s
#objcopy: --add-section NEW=$srcdir/$subdir/empty.s
#objdump: -s -j NEW

.*: +file format .*

Contents of section NEW:
 0000 2320416e 20656d70 74792066 696c652e  # An empty file.
 0010 0a                                   .               
