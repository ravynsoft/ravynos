#as: -mfuture
#objdump: -dr -Mfuture
#name: RFC02658 tests

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(d8 12 00 ec|ec 00 12 d8) 	dmxvbf16gerx2 dm0,vs0,vs2
.*:	(50 67 8a ec|ec 8a 67 50) 	dmxvbf16gerx2nn dm1,vs10,vs12
.*:	(98 b3 14 ed|ed 14 b3 98) 	dmxvbf16gerx2np dm2,vs20,vs22
.*:	(9a 05 9e ed|ed 9e 05 9a) 	dmxvbf16gerx2pn dm3,vs30,vs32
.*:	(56 52 08 ee|ee 08 52 56) 	dmxvbf16gerx2pp dm4,vs40,vs42
.*:	(1e a2 92 ee|ee 92 a2 1e) 	dmxvf16gerx2 dm5,vs50,vs52
.*:	(56 f6 1c ef|ef 1c f6 56) 	dmxvf16gerx2nn dm6,vs60,vs62
.*:	(98 72 8c ef|ef 8c 72 98) 	dmxvf16gerx2np dm7,vs12,vs14
.*:	(98 84 0e ec|ec 0e 84 98) 	dmxvf16gerx2pn dm0,vs14,vs16
.*:	(10 92 90 ec|ec 90 92 10) 	dmxvf16gerx2pp dm1,vs16,vs18
.*:	(58 a0 12 ed|ed 12 a0 58) 	dmxvi8gerx4 dm2,vs18,vs20
.*:	(50 c0 96 ed|ed 96 c0 50) 	dmxvi8gerx4pp dm3,vs22,vs24
.*:	(10 d3 18 ee|ee 18 d3 10) 	dmxvi8gerx4spp dm4,vs24,vs26
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvbf16gerx2nn dm0,vs0,vs2,255,15,3
.*:	(50 17 00 ec|ec 00 17 50) 
.*:	(00 00 00 60|60 00 00 00) 	nop
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvbf16gerx2np dm1,vs10,vs12,255,15,3
.*:	(98 63 8a ec|ec 8a 63 98) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvbf16gerx2 dm2,vs20,vs22,255,15,3
.*:	(d8 b2 14 ed|ed 14 b2 d8) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvbf16gerx2pn dm3,vs30,vs32,255,15,3
.*:	(9a 05 9e ed|ed 9e 05 9a) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvbf16gerx2pp dm4,vs40,vs42,255,15,3
.*:	(56 52 08 ee|ee 08 52 56) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvf16gerx2nn dm5,vs50,vs52,255,15,3
.*:	(56 a6 92 ee|ee 92 a6 56) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvf16gerx2np dm6,vs60,vs62,255,15,3
.*:	(9e f2 1c ef|ef 1c f2 9e) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvf16gerx2 dm7,vs12,vs14,255,15,3
.*:	(18 72 8c ef|ef 8c 72 18) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvf16gerx2pn dm0,vs14,vs16,255,15,3
.*:	(98 84 0e ec|ec 0e 84 98) 
.*:	(ff cf 90 07|07 90 cf ff) 	pmdmxvf16gerx2pp dm1,vs16,vs18,255,15,3
.*:	(10 92 90 ec|ec 90 92 10) 
.*:	(ff ff 90 07|07 90 ff ff) 	pmdmxvi8gerx4 dm2,vs18,vs20,255,15,15
.*:	(58 a0 12 ed|ed 12 a0 58) 
.*:	(ff ff 90 07|07 90 ff ff) 	pmdmxvi8gerx4pp dm3,vs22,vs24,255,15,15
.*:	(50 c0 96 ed|ed 96 c0 50) 
.*:	(ff ff 90 07|07 90 ff ff) 	pmdmxvi8gerx4spp dm4,vs24,vs26,255,15,15
.*:	(10 d3 18 ee|ee 18 d3 10) 
#pass
