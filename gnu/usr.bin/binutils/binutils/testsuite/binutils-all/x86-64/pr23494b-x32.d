#source: pr23494a.s
#PROG: objcopy
#as: --x32 -mx86-used-note=yes
#objcopy: -O elf64-x86-64 -R .note.gnu.property
#readelf: -n
