#as: --EB
#source: reloc-data.s
#objdump: -s
#ld: -Tdata=0x20 -EB
#name: data relocs BE

.*:     file format .*-bpfbe

Contents of section \.data:
 0020 666f6f00 62617200 00000000 00000020  .*
 0030 00000028                             .*
