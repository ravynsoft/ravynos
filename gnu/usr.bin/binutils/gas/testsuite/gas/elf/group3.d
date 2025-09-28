#readelf: -g --wide

COMDAT group section \[[ 0-9]+\] `.group' \[foo\] contains . sections:
   \[Index\]    Name
   \[[ 0-9]+\]   \.foo
#...
   \[[ 0-9]+\]   \.bar
   \[[ 0-9]+\]   \.rela?\.bar
