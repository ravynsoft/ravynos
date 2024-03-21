#as: --EL
#source: reloc-data.s
#objdump: -s
#ld: -Tdata=0x20 -EL
#name: data relocs LE

.*:     file format .*-bpfle

Contents of section \.data:
 0020 666f6f00 62617200 20000000 00000000  .*
 0030 28000000                             .*
