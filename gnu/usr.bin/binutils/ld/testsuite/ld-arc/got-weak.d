#source: got-weak.s
#as:
#ld: -Bstatic
#objdump: -d

[^:]*:\s+file format elf32-.*arc


Disassembly of section \.text:

[0-9a-f]+ <.*>:
^\s+[0-9a-f]+:\s+2730\s7f80\s[0-9a-f]+\s[0-9a-f]+\s+ld\s+r\d+,\[pcl,.*
