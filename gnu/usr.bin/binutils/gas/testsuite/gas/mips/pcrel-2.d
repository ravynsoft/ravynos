#objdump: -s
#name: Locally-resolvable PC-relative data references
#as: -EB

#...
Contents of section \.data:
 0000 ff0f000e 0000000c 00000000 00000008  .*
#pass
