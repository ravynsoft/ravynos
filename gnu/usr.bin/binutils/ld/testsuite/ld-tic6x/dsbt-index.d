#name: C6X DSBT_INDEX reloc
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tsbr.ld --dsbt-index 5
#source: dsbt-index.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

10000000 <[^>]*>:
10000000:[ \t]+0700056e[ \t]+ldw \.D2T2 \*\+b14\(20\),b14
[ \t]*\.\.\.
