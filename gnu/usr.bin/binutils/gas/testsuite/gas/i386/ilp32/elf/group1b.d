#source: ../../../elf/group1.s
#readelf: -g
#name: group section with multiple sections of same name

#...
COMDAT group section \[    1\] `\.group' \[\.foo_group\] contains 1 sections:
[ 	]+\[Index\][ 	]+Name
[ 	]+\[.*\][ 	]+.text
#pass
