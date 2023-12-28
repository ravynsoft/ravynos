#source: weakhid.s
#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux -z nocombreloc --hash-style=sysv
#objdump: -s -R -T

# Check that .weak and .weak .hidden object references are handled
# correctly when generating a DSO.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
0+21b4 l    d  \.data	0+ \.data
0+21b4 g    DO \.data	0+c x
0+      D  \*UND\*	0+ xregobj
0+  w   D  \*UND\*	0+ xweakobj


DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0+21b8 R_CRIS_32         xweakobj
0+21bc R_CRIS_32         xregobj

Contents of section \.hash:
#...
Contents of section \.data:
 21b4 00000000 00000000 00000000           .*
