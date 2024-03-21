#as: -mpower10
#objdump: -dr -Mpower10
#name: POWER10 tests

.*

Disassembly of section \.text:

0+0 <_start>:
.*:	(7d 40 06 a4|a4 06 40 7d) 	slbiag  r10
.*:	(7d 40 06 a4|a4 06 40 7d) 	slbiag  r10
.*:	(7d 41 06 a4|a4 06 41 7d) 	slbiag  r10,1
.*:	(7c 2a 5f 0d|0d 5f 2a 7c) 	paste\.  r10,r11
.*:	(7c 2a 5f 0d|0d 5f 2a 7c) 	paste\.  r10,r11
.*:	(7c 0a 5f 0d|0d 5f 0a 7c) 	paste\.  r10,r11,0
.*:	(7c 80 18 ac|ac 18 80 7c) 	dcbfps  0,r3
.*:	(7c 80 18 ac|ac 18 80 7c) 	dcbfps  0,r3
.*:	(7c c0 18 ac|ac 18 c0 7c) 	dcbstps 0,r3
.*:	(7c c0 18 ac|ac 18 c0 7c) 	dcbstps 0,r3
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
.*:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
.*:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
.*:	(7c 40 04 ac|ac 04 40 7c) 	ptesync
.*:	(7c 40 04 ac|ac 04 40 7c) 	ptesync
.*:	(7c 40 04 ac|ac 04 40 7c) 	ptesync
.*:	(7c 80 04 ac|ac 04 80 7c) 	phwsync
.*:	(7c 80 04 ac|ac 04 80 7c) 	phwsync
.*:	(7c 80 04 ac|ac 04 80 7c) 	phwsync
.*:	(7c a0 04 ac|ac 04 a0 7c) 	plwsync
.*:	(7c a0 04 ac|ac 04 a0 7c) 	plwsync
.*:	(7c a0 04 ac|ac 04 a0 7c) 	plwsync
.*:	(7c 21 04 ac|ac 04 21 7c) 	stncisync
.*:	(7c 21 04 ac|ac 04 21 7c) 	stncisync
.*:	(7c 02 04 ac|ac 04 02 7c) 	stcisync
.*:	(7c 02 04 ac|ac 04 02 7c) 	stcisync
.*:	(7c 03 04 ac|ac 04 03 7c) 	stsync
.*:	(7c 03 04 ac|ac 04 03 7c) 	stsync
.*:	(7c 00 00 3c|3c 00 00 7c) 	wait
.*:	(7c 00 00 3c|3c 00 00 7c) 	wait
.*:	(7c 00 00 3c|3c 00 00 7c) 	wait
.*:	(7c 20 00 3c|3c 00 20 7c) 	waitrsv
.*:	(7c 20 00 3c|3c 00 20 7c) 	waitrsv
.*:	(7c 20 00 3c|3c 00 20 7c) 	waitrsv
.*:	(7c 40 00 3c|3c 00 40 7c) 	pause_short
.*:	(7c 40 00 3c|3c 00 40 7c) 	pause_short
.*:	(7c 40 00 3c|3c 00 40 7c) 	pause_short
#pass
