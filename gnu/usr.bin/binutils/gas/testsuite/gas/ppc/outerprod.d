#as: -mpower10
#objdump: -dr -Mpower10
#name: outer product reduced precision

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(7e 80 01 62|62 01 80 7e) 	dmxxmfacc a5
.*:	(7e 80 01 62|62 01 80 7e) 	dmxxmfacc a5
.*:	(7f 01 01 62|62 01 01 7f) 	dmxxmtacc a6
.*:	(7f 01 01 62|62 01 01 7f) 	dmxxmtacc a6
.*:	(7f 83 01 62|62 01 83 7f) 	dmsetaccz a7
.*:	(7f 83 01 62|62 01 83 7f) 	dmsetaccz a7
.*:	(ec 1f f1 1e|1e f1 1f ec) 	dmxvi4ger8 a0,vs63,vs62
.*:	(ec 1f f1 1e|1e f1 1f ec) 	dmxvi4ger8 a0,vs63,vs62
.*:	(ec 9d e1 16|16 e1 9d ec) 	dmxvi4ger8pp a1,vs61,vs60
.*:	(ec 9d e1 16|16 e1 9d ec) 	dmxvi4ger8pp a1,vs61,vs60
.*:	(07 90 ff fe|fe ff 90 07) 	pmdmxvi4ger8 a2,vs59,vs58,15,14,255
.*:	(ed 1b d1 1e|1e d1 1b ed) 
.*:	(07 90 ff fe|fe ff 90 07) 	pmdmxvi4ger8 a2,vs59,vs58,15,14,255
.*:	(ed 1b d1 1e|1e d1 1b ed) 
.*:	(07 90 80 78|78 80 90 07) 	pmdmxvi4ger8pp a3,vs57,vs56,7,8,128
.*:	(ed 99 c1 16|16 c1 99 ed) 
.*:	(07 90 80 78|78 80 90 07) 	pmdmxvi4ger8pp a3,vs57,vs56,7,8,128
.*:	(ed 99 c1 16|16 c1 99 ed) 
.*:	(ee 17 b0 1e|1e b0 17 ee) 	dmxvi8ger4 a4,vs55,vs54
.*:	(ee 17 b0 1e|1e b0 17 ee) 	dmxvi8ger4 a4,vs55,vs54
.*:	(ee 95 a0 16|16 a0 95 ee) 	dmxvi8ger4pp a5,vs53,vs52
.*:	(ee 95 a0 16|16 a0 95 ee) 	dmxvi8ger4pp a5,vs53,vs52
.*:	(07 90 b0 dc|dc b0 90 07) 	pmdmxvi8ger4 a6,vs51,vs50,13,12,11
.*:	(ef 13 90 1e|1e 90 13 ef) 
.*:	(07 90 b0 dc|dc b0 90 07) 	pmdmxvi8ger4 a6,vs51,vs50,13,12,11
.*:	(ef 13 90 1e|1e 90 13 ef) 
.*:	(07 90 80 a9|a9 80 90 07) 	pmdmxvi8ger4pp a7,vs49,vs48,10,9,8
.*:	(ef 91 80 16|16 80 91 ef) 
.*:	(07 90 80 a9|a9 80 90 07) 	pmdmxvi8ger4pp a7,vs49,vs48,10,9,8
.*:	(ef 91 80 16|16 80 91 ef) 
.*:	(ec 0f 71 5e|5e 71 0f ec) 	dmxvi16ger2s a0,vs47,vs46
.*:	(ec 0f 71 5e|5e 71 0f ec) 	dmxvi16ger2s a0,vs47,vs46
.*:	(ec 8d 61 56|56 61 8d ec) 	dmxvi16ger2spp a1,vs45,vs44
.*:	(ec 8d 61 56|56 61 8d ec) 	dmxvi16ger2spp a1,vs45,vs44
.*:	(07 90 c0 76|76 c0 90 07) 	pmdmxvi16ger2s a2,vs43,vs42,7,6,3
.*:	(ed 0b 51 5e|5e 51 0b ed) 
.*:	(07 90 c0 76|76 c0 90 07) 	pmdmxvi16ger2s a2,vs43,vs42,7,6,3
.*:	(ed 0b 51 5e|5e 51 0b ed) 
.*:	(07 90 80 54|54 80 90 07) 	pmdmxvi16ger2spp a3,vs41,vs40,5,4,2
.*:	(ed 89 41 56|56 41 89 ed) 
.*:	(07 90 80 54|54 80 90 07) 	pmdmxvi16ger2spp a3,vs41,vs40,5,4,2
.*:	(ed 89 41 56|56 41 89 ed) 
.*:	(ee 07 30 9e|9e 30 07 ee) 	dmxvf16ger2 a4,vs39,vs38
.*:	(ee 07 30 9e|9e 30 07 ee) 	dmxvf16ger2 a4,vs39,vs38
.*:	(ee 85 20 96|96 20 85 ee) 	dmxvf16ger2pp a5,vs37,vs36
.*:	(ee 85 20 96|96 20 85 ee) 	dmxvf16ger2pp a5,vs37,vs36
.*:	(ef 03 14 96|96 14 03 ef) 	dmxvf16ger2pn a6,vs35,vs34
.*:	(ef 03 14 96|96 14 03 ef) 	dmxvf16ger2pn a6,vs35,vs34
.*:	(ef 81 02 96|96 02 81 ef) 	dmxvf16ger2np a7,vs33,vs32
.*:	(ef 81 02 96|96 02 81 ef) 	dmxvf16ger2np a7,vs33,vs32
.*:	(ec 04 2e 90|90 2e 04 ec) 	dmxvf16ger2nn a0,vs4,vs5
.*:	(ec 04 2e 90|90 2e 04 ec) 	dmxvf16ger2nn a0,vs4,vs5
.*:	(07 90 40 32|32 40 90 07) 	pmdmxvf16ger2 a1,vs2,vs3,3,2,1
.*:	(ec 82 18 98|98 18 82 ec) 
.*:	(07 90 40 32|32 40 90 07) 	pmdmxvf16ger2 a1,vs2,vs3,3,2,1
.*:	(ec 82 18 98|98 18 82 ec) 
.*:	(07 90 00 10|10 00 90 07) 	pmdmxvf16ger2pp a2,vs4,vs5,1,0,0
.*:	(ed 04 28 90|90 28 04 ed) 
.*:	(07 90 00 10|10 00 90 07) 	pmdmxvf16ger2pp a2,vs4,vs5,1,0,0
.*:	(ed 04 28 90|90 28 04 ed) 
.*:	(07 90 c0 fe|fe c0 90 07) 	pmdmxvf16ger2pn a3,vs6,vs7,15,14,3
.*:	(ed 86 3c 90|90 3c 86 ed) 
.*:	(07 90 c0 fe|fe c0 90 07) 	pmdmxvf16ger2pn a3,vs6,vs7,15,14,3
.*:	(ed 86 3c 90|90 3c 86 ed) 
.*:	(07 90 80 dc|dc 80 90 07) 	pmdmxvf16ger2np a4,vs8,vs9,13,12,2
.*:	(ee 08 4a 90|90 4a 08 ee) 
.*:	(07 90 80 dc|dc 80 90 07) 	pmdmxvf16ger2np a4,vs8,vs9,13,12,2
.*:	(ee 08 4a 90|90 4a 08 ee) 
.*:	(07 90 40 ba|ba 40 90 07) 	pmdmxvf16ger2nn a5,vs10,vs11,11,10,1
.*:	(ee 8a 5e 90|90 5e 8a ee) 
.*:	(07 90 40 ba|ba 40 90 07) 	pmdmxvf16ger2nn a5,vs10,vs11,11,10,1
.*:	(ee 8a 5e 90|90 5e 8a ee) 
.*:	(ef 0c 68 d8|d8 68 0c ef) 	dmxvf32ger a6,vs12,vs13
.*:	(ef 0c 68 d8|d8 68 0c ef) 	dmxvf32ger a6,vs12,vs13
.*:	(ef 8e 78 d0|d0 78 8e ef) 	dmxvf32gerpp a7,vs14,vs15
.*:	(ef 8e 78 d0|d0 78 8e ef) 	dmxvf32gerpp a7,vs14,vs15
.*:	(ec 10 8c d0|d0 8c 10 ec) 	dmxvf32gerpn a0,vs16,vs17
.*:	(ec 10 8c d0|d0 8c 10 ec) 	dmxvf32gerpn a0,vs16,vs17
.*:	(ec 92 9a d0|d0 9a 92 ec) 	dmxvf32gernp a1,vs18,vs19
.*:	(ec 92 9a d0|d0 9a 92 ec) 	dmxvf32gernp a1,vs18,vs19
.*:	(ed 14 ae d0|d0 ae 14 ed) 	dmxvf32gernn a2,vs20,vs21
.*:	(ed 14 ae d0|d0 ae 14 ed) 	dmxvf32gernn a2,vs20,vs21
.*:	(07 90 00 98|98 00 90 07) 	pmdmxvf32ger a3,vs22,vs23,9,8
.*:	(ed 96 b8 d8|d8 b8 96 ed) 
.*:	(07 90 00 98|98 00 90 07) 	pmdmxvf32ger a3,vs22,vs23,9,8
.*:	(ed 96 b8 d8|d8 b8 96 ed) 
.*:	(07 90 00 76|76 00 90 07) 	pmdmxvf32gerpp a4,vs24,vs25,7,6
.*:	(ee 18 c8 d0|d0 c8 18 ee) 
.*:	(07 90 00 76|76 00 90 07) 	pmdmxvf32gerpp a4,vs24,vs25,7,6
.*:	(ee 18 c8 d0|d0 c8 18 ee) 
.*:	(07 90 00 54|54 00 90 07) 	pmdmxvf32gerpn a5,vs26,vs27,5,4
.*:	(ee 9a dc d0|d0 dc 9a ee) 
.*:	(07 90 00 54|54 00 90 07) 	pmdmxvf32gerpn a5,vs26,vs27,5,4
.*:	(ee 9a dc d0|d0 dc 9a ee) 
.*:	(07 90 00 32|32 00 90 07) 	pmdmxvf32gernp a6,vs28,vs29,3,2
.*:	(ef 1c ea d0|d0 ea 1c ef) 
.*:	(07 90 00 32|32 00 90 07) 	pmdmxvf32gernp a6,vs28,vs29,3,2
.*:	(ef 1c ea d0|d0 ea 1c ef) 
.*:	(07 90 00 10|10 00 90 07) 	pmdmxvf32gernn a7,vs0,vs1,1,0
.*:	(ef 80 0e d0|d0 0e 80 ef) 
.*:	(07 90 00 10|10 00 90 07) 	pmdmxvf32gernn a7,vs0,vs1,1,0
.*:	(ef 80 0e d0|d0 0e 80 ef) 
.*:	(ec 04 29 d8|d8 29 04 ec) 	dmxvf64ger a0,vs4,vs5
.*:	(ec 04 29 d8|d8 29 04 ec) 	dmxvf64ger a0,vs4,vs5
.*:	(ec 88 49 d0|d0 49 88 ec) 	dmxvf64gerpp a1,vs8,vs9
.*:	(ec 88 49 d0|d0 49 88 ec) 	dmxvf64gerpp a1,vs8,vs9
.*:	(ed 02 15 d0|d0 15 02 ed) 	dmxvf64gerpn a2,vs2,vs2
.*:	(ed 02 15 d0|d0 15 02 ed) 	dmxvf64gerpn a2,vs2,vs2
.*:	(ed 84 1b d0|d0 1b 84 ed) 	dmxvf64gernp a3,vs4,vs3
.*:	(ed 84 1b d0|d0 1b 84 ed) 	dmxvf64gernp a3,vs4,vs3
.*:	(ee 04 27 d0|d0 27 04 ee) 	dmxvf64gernn a4,vs4,vs4
.*:	(ee 04 27 d0|d0 27 04 ee) 	dmxvf64gernn a4,vs4,vs4
.*:	(07 90 00 f0|f0 00 90 07) 	pmdmxvf64ger a5,vs6,vs5,15,0
.*:	(ee 86 29 d8|d8 29 86 ee) 
.*:	(07 90 00 f0|f0 00 90 07) 	pmdmxvf64ger a5,vs6,vs5,15,0
.*:	(ee 86 29 d8|d8 29 86 ee) 
.*:	(07 90 00 e4|e4 00 90 07) 	pmdmxvf64gerpp a6,vs6,vs6,14,1
.*:	(ef 06 31 d0|d0 31 06 ef) 
.*:	(07 90 00 e4|e4 00 90 07) 	pmdmxvf64gerpp a6,vs6,vs6,14,1
.*:	(ef 06 31 d0|d0 31 06 ef) 
.*:	(07 90 00 d8|d8 00 90 07) 	pmdmxvf64gerpn a7,vs8,vs7,13,2
.*:	(ef 88 3d d0|d0 3d 88 ef) 
.*:	(07 90 00 d8|d8 00 90 07) 	pmdmxvf64gerpn a7,vs8,vs7,13,2
.*:	(ef 88 3d d0|d0 3d 88 ef) 
.*:	(07 90 00 cc|cc 00 90 07) 	pmdmxvf64gernp a0,vs4,vs5,12,3
.*:	(ec 04 2b d0|d0 2b 04 ec) 
.*:	(07 90 00 cc|cc 00 90 07) 	pmdmxvf64gernp a0,vs4,vs5,12,3
.*:	(ec 04 2b d0|d0 2b 04 ec) 
.*:	(07 90 00 a0|a0 00 90 07) 	pmdmxvf64gernn a1,vs2,vs1,10,0
.*:	(ec 82 0f d0|d0 0f 82 ec) 
.*:	(07 90 00 a0|a0 00 90 07) 	pmdmxvf64gernn a1,vs2,vs1,10,0
.*:	(ec 82 0f d0|d0 0f 82 ec) 
.*:	(ed 03 21 90|90 21 03 ed) 	dmxvbf16ger2pp a2,vs3,vs4
.*:	(ed 03 21 90|90 21 03 ed) 	dmxvbf16ger2pp a2,vs3,vs4
.*:	(ed 84 29 98|98 29 84 ed) 	dmxvbf16ger2 a3,vs4,vs5
.*:	(ed 84 29 98|98 29 84 ed) 	dmxvbf16ger2 a3,vs4,vs5
.*:	(ee 05 33 90|90 33 05 ee) 	dmxvbf16ger2np a4,vs5,vs6
.*:	(ee 05 33 90|90 33 05 ee) 	dmxvbf16ger2np a4,vs5,vs6
.*:	(ee 86 3d 90|90 3d 86 ee) 	dmxvbf16ger2pn a5,vs6,vs7
.*:	(ee 86 3d 90|90 3d 86 ee) 	dmxvbf16ger2pn a5,vs6,vs7
.*:	(ef 07 47 90|90 47 07 ef) 	dmxvbf16ger2nn a6,vs7,vs8
.*:	(ef 07 47 90|90 47 07 ef) 	dmxvbf16ger2nn a6,vs7,vs8
.*:	(07 90 c0 ff|ff c0 90 07) 	pmdmxvbf16ger2pp a7,vs8,vs9,15,15,3
.*:	(ef 88 49 90|90 49 88 ef) 
.*:	(07 90 c0 ff|ff c0 90 07) 	pmdmxvbf16ger2pp a7,vs8,vs9,15,15,3
.*:	(ef 88 49 90|90 49 88 ef) 
.*:	(07 90 80 cc|cc 80 90 07) 	pmdmxvbf16ger2 a0,vs9,vs10,12,12,2
.*:	(ec 09 51 98|98 51 09 ec) 
.*:	(07 90 80 cc|cc 80 90 07) 	pmdmxvbf16ger2 a0,vs9,vs10,12,12,2
.*:	(ec 09 51 98|98 51 09 ec) 
.*:	(07 90 40 aa|aa 40 90 07) 	pmdmxvbf16ger2np a1,vs10,vs11,10,10,1
.*:	(ec 8a 5b 90|90 5b 8a ec) 
.*:	(07 90 40 aa|aa 40 90 07) 	pmdmxvbf16ger2np a1,vs10,vs11,10,10,1
.*:	(ec 8a 5b 90|90 5b 8a ec) 
.*:	(07 90 00 dd|dd 00 90 07) 	pmdmxvbf16ger2pn a2,vs12,vs13,13,13,0
.*:	(ed 0c 6d 90|90 6d 0c ed) 
.*:	(07 90 00 dd|dd 00 90 07) 	pmdmxvbf16ger2pn a2,vs12,vs13,13,13,0
.*:	(ed 0c 6d 90|90 6d 0c ed) 
.*:	(07 90 c0 ee|ee c0 90 07) 	pmdmxvbf16ger2nn a3,vs16,vs17,14,14,3
.*:	(ed 90 8f 90|90 8f 90 ed) 
.*:	(07 90 c0 ee|ee c0 90 07) 	pmdmxvbf16ger2nn a3,vs16,vs17,14,14,3
.*:	(ed 90 8f 90|90 8f 90 ed) 
.*:	(ee 00 0b 1e|1e 0b 00 ee) 	dmxvi8ger4spp a4,vs32,vs33
.*:	(ee 00 0b 1e|1e 0b 00 ee) 	dmxvi8ger4spp a4,vs32,vs33
.*:	(07 90 f0 ff|ff f0 90 07) 	pmdmxvi8ger4spp a5,vs34,vs35,15,15,15
.*:	(ee 82 1b 1e|1e 1b 82 ee) 
.*:	(07 90 f0 ff|ff f0 90 07) 	pmdmxvi8ger4spp a5,vs34,vs35,15,15,15
.*:	(ee 82 1b 1e|1e 1b 82 ee) 
.*:	(ef 04 2a 5e|5e 2a 04 ef) 	dmxvi16ger2 a6,vs36,vs37
.*:	(ef 04 2a 5e|5e 2a 04 ef) 	dmxvi16ger2 a6,vs36,vs37
.*:	(ef 86 3b 5e|5e 3b 86 ef) 	dmxvi16ger2pp a7,vs38,vs39
.*:	(ef 86 3b 5e|5e 3b 86 ef) 	dmxvi16ger2pp a7,vs38,vs39
.*:	(07 90 40 ff|ff 40 90 07) 	pmdmxvi16ger2 a0,vs38,vs39,15,15,1
.*:	(ec 06 3a 5e|5e 3a 06 ec) 
.*:	(07 90 40 ff|ff 40 90 07) 	pmdmxvi16ger2 a0,vs38,vs39,15,15,1
.*:	(ec 06 3a 5e|5e 3a 06 ec) 
.*:	(07 90 80 cc|cc 80 90 07) 	pmdmxvi16ger2pp a1,vs40,vs41,12,12,2
.*:	(ec 88 4b 5e|5e 4b 88 ec) 
.*:	(07 90 80 cc|cc 80 90 07) 	pmdmxvi16ger2pp a1,vs40,vs41,12,12,2
.*:	(ec 88 4b 5e|5e 4b 88 ec) 
#pass
