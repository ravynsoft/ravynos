        .text
_start:
	dmxvbf16gerx2	0,0,2
	dmxvbf16gerx2nn	1,10,12
	dmxvbf16gerx2np	2,20,22
	dmxvbf16gerx2pn	3,30,32
	dmxvbf16gerx2pp	4,40,42
	dmxvf16gerx2	5,50,52
	dmxvf16gerx2nn	6,60,62
	dmxvf16gerx2np	7,12,14
	dmxvf16gerx2pn	0,14,16
	dmxvf16gerx2pp	1,16,18
	dmxvi8gerx4	2,18,20
	dmxvi8gerx4pp	3,22,24
	dmxvi8gerx4spp	4,24,26
	pmdmxvbf16gerx2nn	0,0,2,0xff,0xf,0x3
	pmdmxvbf16gerx2np	1,10,12,0xff,0xf,0x3
	pmdmxvbf16gerx2		2,20,22,0xff,0xf,0x3
	pmdmxvbf16gerx2pn	3,30,32,0xff,0xf,0x3
	pmdmxvbf16gerx2pp	4,40,42,0xff,0xf,0x3
	pmdmxvf16gerx2nn	5,50,52,0xff,0xf,0x3
	pmdmxvf16gerx2np	6,60,62,0xff,0xf,0x3
	pmdmxvf16gerx2		7,12,14,0xff,0xf,0x3
	pmdmxvf16gerx2pn	0,14,16,0xff,0xf,0x3
	pmdmxvf16gerx2pp	1,16,18,0xff,0xf,0x3
	pmdmxvi8gerx4		2,18,20,0xff,0xf,0xf
	pmdmxvi8gerx4pp		3,22,24,0xff,0xf,0xf
	pmdmxvi8gerx4spp	4,24,26,0xff,0xf,0xf
