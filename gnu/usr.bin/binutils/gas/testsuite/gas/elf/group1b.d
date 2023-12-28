#readelf: -g
#name: group section with multiple sections of same name (using readelf -g)
#source: group1.s

#...
COMDAT group section \[    1\] `\.group' \[.foo_group\] contains . sections:
[ 	]+\[Index\][ 	]+Name
#...
[ 	]+\[.*\][ 	]+.text
#pass
