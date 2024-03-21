#source: vle-multiseg-6a.s -mregnames -mvle
#source: vle-multiseg-6b.s
#source: vle-multiseg-6c.s
#source: vle-multiseg-6d.s -mregnames -mvle
#ld: -T vle-multiseg-6.ld
#target: powerpc-*-*
#readelf: -l

Elf file type is EXEC.*
Entry point 0x[0-9a-f]+
There are 4 program headers, starting at offset [0-9]+

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD ( +0x[0-9a-f]+){5} RW  0x[0-f]+
  LOAD ( +0x[0-9a-f]+){5} R E 0x[0-9a-f]+
  LOAD ( +0x[0-9a-f]+){5} R E 0x[0-9a-f]+
  LOAD ( +0x[0-9a-f]+){5} R E 0x[0-9a-f]+

 Section to Segment mapping:
  Segment Sections...
   00     \.data 
   01     \.text_vle 
   02     \.text_iv 
   03     \.text 
