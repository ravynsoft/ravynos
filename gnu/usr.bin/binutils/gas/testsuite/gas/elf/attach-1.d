#readelf: --section-groups
#name: Attaching a section to a group
#source: attach-1.s

#...
group section \[    1\] `\.group' \[foo\.group\] contains . sections:
   \[Index\]    Name
   \[    .\]   .*
   \[    .\]   foo
#pass

