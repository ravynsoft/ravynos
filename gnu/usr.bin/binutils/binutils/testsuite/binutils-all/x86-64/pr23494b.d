#source: pr23494a.s
#PROG: objcopy
#as: --64 -defsym __64_bit__=1 -mx86-used-note=yes
#objcopy: -O elf32-x86-64 -R .note.gnu.property
#readelf: -n
