#readelf: --section-groups
#name: Attaching a section to a non-existant group
#source: attach-2.s

#...
group section \[    1\] `\.group' \[foo\.group\] contains 2 sections:
   \[Index\]    Name
   \[    .\]   bar
   \[    .\]   foo
#pass

