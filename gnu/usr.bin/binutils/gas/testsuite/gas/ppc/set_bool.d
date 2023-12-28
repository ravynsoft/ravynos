#as: -mpower10
#objdump: -dr -Mpower10
#name: set bool

.*


Disassembly of section \.text:

0+00 <_start>:
.*:	(7d 41 03 00|00 03 41 7d) 	setbc   r10,gt
.*:	(7d 62 03 40|40 03 62 7d) 	setbcr  r11,eq
.*:	(7d 83 03 80|80 03 83 7d) 	setnbc  r12,so
.*:	(7d a0 03 c0|c0 03 a0 7d) 	setnbcr r13,lt
