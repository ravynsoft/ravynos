#name: Z8002 forward jr just in range
#source: jr-opcode.s -z8002
#source: filler.s -z8002 --defsym NOPS=127
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#objdump: -dr

.*:     file format coff-z8k


Disassembly of section \.text:

00001000 <\.text>:
    1000:	e87f           	jr	t,0x1100

00001002 <\.text>:
    1002:	8d07           	nop	
    1004:	8d07           	nop	
    1006:	8d07           	nop	
    1008:	8d07           	nop	
    100a:	8d07           	nop	
    100c:	8d07           	nop	
    100e:	8d07           	nop	
    1010:	8d07           	nop	
    1012:	8d07           	nop	
    1014:	8d07           	nop	
    1016:	8d07           	nop	
    1018:	8d07           	nop	
    101a:	8d07           	nop	
    101c:	8d07           	nop	
    101e:	8d07           	nop	
    1020:	8d07           	nop	
    1022:	8d07           	nop	
    1024:	8d07           	nop	
    1026:	8d07           	nop	
    1028:	8d07           	nop	
    102a:	8d07           	nop	
    102c:	8d07           	nop	
    102e:	8d07           	nop	
    1030:	8d07           	nop	
    1032:	8d07           	nop	
    1034:	8d07           	nop	
    1036:	8d07           	nop	
    1038:	8d07           	nop	
    103a:	8d07           	nop	
    103c:	8d07           	nop	
    103e:	8d07           	nop	
    1040:	8d07           	nop	
    1042:	8d07           	nop	
    1044:	8d07           	nop	
    1046:	8d07           	nop	
    1048:	8d07           	nop	
    104a:	8d07           	nop	
    104c:	8d07           	nop	
    104e:	8d07           	nop	
    1050:	8d07           	nop	
    1052:	8d07           	nop	
    1054:	8d07           	nop	
    1056:	8d07           	nop	
    1058:	8d07           	nop	
    105a:	8d07           	nop	
    105c:	8d07           	nop	
    105e:	8d07           	nop	
    1060:	8d07           	nop	
    1062:	8d07           	nop	
    1064:	8d07           	nop	
    1066:	8d07           	nop	
    1068:	8d07           	nop	
    106a:	8d07           	nop	
    106c:	8d07           	nop	
    106e:	8d07           	nop	
    1070:	8d07           	nop	
    1072:	8d07           	nop	
    1074:	8d07           	nop	
    1076:	8d07           	nop	
    1078:	8d07           	nop	
    107a:	8d07           	nop	
    107c:	8d07           	nop	
    107e:	8d07           	nop	
    1080:	8d07           	nop	
    1082:	8d07           	nop	
    1084:	8d07           	nop	
    1086:	8d07           	nop	
    1088:	8d07           	nop	
    108a:	8d07           	nop	
    108c:	8d07           	nop	
    108e:	8d07           	nop	
    1090:	8d07           	nop	
    1092:	8d07           	nop	
    1094:	8d07           	nop	
    1096:	8d07           	nop	
    1098:	8d07           	nop	
    109a:	8d07           	nop	
    109c:	8d07           	nop	
    109e:	8d07           	nop	
    10a0:	8d07           	nop	
    10a2:	8d07           	nop	
    10a4:	8d07           	nop	
    10a6:	8d07           	nop	
    10a8:	8d07           	nop	
    10aa:	8d07           	nop	
    10ac:	8d07           	nop	
    10ae:	8d07           	nop	
    10b0:	8d07           	nop	
    10b2:	8d07           	nop	
    10b4:	8d07           	nop	
    10b6:	8d07           	nop	
    10b8:	8d07           	nop	
    10ba:	8d07           	nop	
    10bc:	8d07           	nop	
    10be:	8d07           	nop	
    10c0:	8d07           	nop	
    10c2:	8d07           	nop	
    10c4:	8d07           	nop	
    10c6:	8d07           	nop	
    10c8:	8d07           	nop	
    10ca:	8d07           	nop	
    10cc:	8d07           	nop	
    10ce:	8d07           	nop	
    10d0:	8d07           	nop	
    10d2:	8d07           	nop	
    10d4:	8d07           	nop	
    10d6:	8d07           	nop	
    10d8:	8d07           	nop	
    10da:	8d07           	nop	
    10dc:	8d07           	nop	
    10de:	8d07           	nop	
    10e0:	8d07           	nop	
    10e2:	8d07           	nop	
    10e4:	8d07           	nop	
    10e6:	8d07           	nop	
    10e8:	8d07           	nop	
    10ea:	8d07           	nop	
    10ec:	8d07           	nop	
    10ee:	8d07           	nop	
    10f0:	8d07           	nop	
    10f2:	8d07           	nop	
    10f4:	8d07           	nop	
    10f6:	8d07           	nop	
    10f8:	8d07           	nop	
    10fa:	8d07           	nop	
    10fc:	8d07           	nop	
    10fe:	8d07           	nop	

00001100 <target>:
    1100:	bd04           	ldk	r0,#0x4
