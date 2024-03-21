#source: ../../../elf/group0.s
#readelf: -g
#name: group section

#...
COMDAT group section \[    1\] `\.group' \[\.foo_group\] contains 2 sections:
[ 	]+\[Index\][ 	]+Name
[ 	]+\[.*\][ 	]+.foo
[ 	]+\[.*\][ 	]+.bar
#pass
