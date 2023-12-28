#as: -mpower10
#objdump: -dr -Mpower10
#name: PCV generate operations

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(f2 03 7f 29|29 7f 03 f2) 	xxgenpcvbm vs48,v15,3
.*:	(f0 02 3f 2b|2b 3f 02 f0) 	xxgenpcvhm vs32,v7,2
.*:	(f2 01 1f 68|68 1f 01 f2) 	xxgenpcvwm vs16,v3,1
.*:	(f1 00 0f 6a|6a 0f 00 f1) 	xxgenpcvdm vs8,v1,0
