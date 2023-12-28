#objdump: -b binary -s
#name: MIPS link ELF into binary output format
#ld: -r --oformat=binary -T binary.ld

.*: +file format binary

Contents of section \.data:
 0000 61626364 65666768 696a6b6c 6d6e6f70  abcdefghijklmnop
