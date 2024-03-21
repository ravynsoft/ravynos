#name: copy removing reloc and non-reloc group member
#source: group-7.s
#PROG: objcopy
#objcopy: -R .data.foo -R .data2.foo
#readelf: -g --wide

There are no section groups in this file\.
