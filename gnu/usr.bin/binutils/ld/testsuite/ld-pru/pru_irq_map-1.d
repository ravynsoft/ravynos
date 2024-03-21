#name: pru_irq_map special section for host 1
#source: pru_irq_map.s
#ld: --defsym=__HEAP_SIZE=0 --defsym=__STACK_SIZE=0
#readelf: -l --wide

# Ensure that .pru_irq_map section is not loaded into target memory.

#...
Program Headers:
 +Type +Offset +VirtAddr +PhysAddr +FileSiz +MemSiz +Flg +Align
 +LOAD +0x[0-9a-f]+ 0x0+ 0x0+ 0x0+8 0x0+8 RW  0x1
 +LOAD +0x[0-9a-f]+ 0x20+ 0x20+ 0x0+4 0x0+4 R E 0x4

 Section to Segment mapping:
 +Segment Sections...
 +00 +.data[ ]*
 +01 +.text[ ]*
