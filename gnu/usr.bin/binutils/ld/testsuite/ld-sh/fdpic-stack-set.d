#source: fdpic-stack.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -z stack-size=0x40000
#readelf: -l
#target: sh*-*-uclinux*

Elf file type is EXEC \(Executable file\)
Entry point 0x400074
There are 2 program headers, starting at offset 52

Program Headers:
[ \t]+Type[ \t]+Offset[ \t]+VirtAddr[ \t]+PhysAddr[ \t]+FileSiz MemSiz[ \t]+Flg Align
[ \t]+LOAD[ \t]+0x000000 0x00400000 0x00400000 0x00076 0x00076 R E 0x10000
[ \t]+GNU_STACK[ \t]+0x000000 0x00000000 0x00000000 0x00000 0x40000 RWE 0x8

 Section to Segment mapping:
[ \t]+Segment Sections\.\.\.
[ \t]+00[ \t]+\.text 
[ \t]+01[ \t]+
