#ld: -Tpr19005.t
#objcopy_linked_file: -O binary -j .foo -j .bar --gap-fill=0xff
#objdump: -b binary -s

#...
Contents of section .data:
 0000 10ffffff ffffffff ffffffff ffffffff  ................
 0010 ffffffff ffffffff ffffffff ffffffff  ................
 0020 20.*
#pass
