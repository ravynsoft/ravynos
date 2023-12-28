#name: Zeda32 floating point numbers
#as: -fp-s=zeda32
#objdump: -s -j .data

.*:[     ]+file format (coff)|(elf32)\-z80

Contents of section \.data:
 0000 db0f4981 54f82d81 3baa3880 1872317f[  ]+.*
 0010 9b201a7e 789a5481 00000000 00000080[  ]+.*
 0020 00004000 0000c000 00002000 d95b5e7e[  ]+.*
 0030 db0f4982 83f9227d 0000007e 0000007f[  ]+.*
 0040 00000001 00008001 ffff7fff ffffffff[  ]+.*
#pass
