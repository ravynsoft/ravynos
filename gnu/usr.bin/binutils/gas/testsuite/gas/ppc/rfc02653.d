#as: -mfuture
#objdump: -dr -Mfuture
#name: RFC02653 tests

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(62 01 02 7c|7c 02 01 62) 	dmsetdmrz dm0
.*:	(62 41 86 7c|7c 86 41 62) 	dmmr    dm1,dm2
.*:	(62 61 07 7d|7d 07 61 62) 	dmxor   dm2,dm3
.*:	(10 17 00 f2|f2 00 17 10) 	dmxxextfdmr512 vs0,vs2,dm4,0
.*:	(10 37 85 f2|f2 85 37 10) 	dmxxextfdmr512 vs4,vs6,dm5,1
.*:	(50 57 08 f3|f3 08 57 50) 	dmxxinstdmr512 dm6,vs8,vs10,0
.*:	(50 57 89 f3|f3 89 57 50) 	dmxxinstdmr512 dm7,vs8,vs10,1
.*:	(90 67 00 f0|f0 00 67 90) 	dmxxextfdmr256 vs12,dm0,0
.*:	(90 7f 80 f0|f0 80 7f 90) 	dmxxextfdmr256 vs14,dm1,1
.*:	(90 87 01 f1|f1 01 87 90) 	dmxxextfdmr256 vs16,dm2,2
.*:	(90 9f 81 f1|f1 81 9f 90) 	dmxxextfdmr256 vs18,dm3,3
.*:	(94 a7 00 f2|f2 00 a7 94) 	dmxxinstdmr256 dm4,vs20,0
.*:	(94 bf 80 f2|f2 80 bf 94) 	dmxxinstdmr256 dm5,vs22,1
.*:	(94 c7 01 f3|f3 01 c7 94) 	dmxxinstdmr256 dm6,vs24,2
.*:	(94 df 81 f3|f3 81 df 94) 	dmxxinstdmr256 dm7,vs26,3
.*:	(18 09 00 ec|ec 00 09 18) 	dmxvi4ger8 a0,vs0,vs1
#pass
