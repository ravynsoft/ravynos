#ld: -T noload-3.t
#objdump: -s -j .foo1

#...
Contents of section .foo1:
 [0-9a-f]+ [0-9a-f]+ [0-9a-f]+ [0-9a-f]+ [0-9a-f]+[ \t]+This is a test.*
#pass
