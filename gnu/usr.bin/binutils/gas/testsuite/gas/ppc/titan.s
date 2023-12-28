# AppliedMicro Titan tests 
	.text
start:
	blr
	tweqi		1, 0
	macchw		2, 1, 0
	macchw.		2, 1, 0
	macchwo		2, 1, 0
	macchwo.	2, 1, 0
	macchws		2, 1, 0
	macchws.	2, 1, 0
	macchwso	2, 1, 0
	macchwso.	2, 1, 0
	macchwsu	2, 1, 0
	macchwsu.	2, 1, 0
	macchwsuo	2, 1, 0
	macchwsuo.	2, 1, 0
	macchwu		2, 1, 0
	macchwu.	2, 1, 0
	macchwuo	2, 1, 0
	macchwuo.	2, 1, 0
	machhw		2, 1, 0
	machhw.		2, 1, 0
	machhwo		2, 1, 0
	machhwo.	2, 1, 0
	machhws		2, 1, 0
	machhws.	2, 1, 0
	machhwso	2, 1, 0
	machhwso.	2, 1, 0
	machhwsu	2, 1, 0
	machhwsu.	2, 1, 0
	machhwsuo	2, 1, 0
	machhwsuo.	2, 1, 0
	machhwu		2, 1, 0
	machhwu.	2, 1, 0
	machhwuo	2, 1, 0
	machhwuo.	2, 1, 0
	maclhw		2, 1, 0
	maclhw.		2, 1, 0
	maclhwo		2, 1, 0
	maclhwo.	2, 1, 0
	maclhws		2, 1, 0
	maclhws.	2, 1, 0
	maclhwso	2, 1, 0
	maclhwso.	2, 1, 0
	maclhwsu	2, 1, 0
	maclhwsu.	2, 1, 0
	maclhwsuo	2, 1, 0
	maclhwsuo.	2, 1, 0
	maclhwu		2, 1, 0
	maclhwu.	2, 1, 0
	maclhwuo	2, 1, 0
	maclhwuo.	2, 1, 0
	nmacchw		2, 1, 0
	nmacchw.	2, 1, 0
	nmacchwo	2, 1, 0
	nmacchwo.	2, 1, 0
	nmacchws	2, 1, 0
	nmacchws.	2, 1, 0
	nmacchwso	2, 1, 0
	nmacchwso.	2, 1, 0	
	nmachhw		2, 1, 0
	nmachhw.	2, 1, 0
	nmachhwo	2, 1, 0
	nmachhwo.	2, 1, 0
	nmachhws	2, 1, 0
	nmachhws.	2, 1, 0
	nmachhwso	2, 1, 0
	nmachhwso.	2, 1, 0
	nmaclhw		2, 1, 0
	nmaclhw.	2, 1, 0
	nmaclhwo	2, 1, 0
	nmaclhwo.	2, 1, 0
	nmaclhws	2, 1, 0
	nmaclhws.	2, 1, 0
	nmaclhwso	2, 1, 0
	nmaclhwso.	2, 1, 0
	mulchw		2, 1, 0
	mulchw.		2, 1, 0
	mulchwu		2, 1, 0
	mulchwu.	2, 1, 0
	mulhhw		2, 1, 0
	mulhhw.		2, 1, 0
	mulhhwu		2, 1, 0
	mulhhwu.	2, 1, 0
	mullhw		2, 1, 0
	mullhw.		2, 1, 0
	mullhwu		2, 1, 0
	mullhwu.	2, 1, 0
	dlmzb		2, 1, 0
	dlmzb.		2, 1, 0
	dccci		2, 1
	iccci		2, 1
	dcblc		0, 2, 1
	dcblc		2, 1
	dcblc		1, 2, 1
	dcbtls		0, 2, 1
	dcbtls		2, 1
	dcbtls		1, 2, 1
	dcbtstls	0, 2, 1
	dcbtstls	2, 1
	dcbtstls	1, 2, 1
	icblc		0, 2, 1
	icblc		2, 1
	icblc		1, 2, 1
	icbtls		0, 2, 1
	icbtls		2, 1
	icbtls		1, 2, 1
	dcread 		2, 1, 0
	icread 		2, 1
	mfpmr		2, 1
	mfpmr		1, 2
	mfspr		4, 0x001
	mfxer		4
	mfspr		4, 0x008
	mflr		4
	mfspr		4, 0x009
	mfctr		4
	mfspr		4, 0x016
	mfdec		4
	mfspr		4, 0x01a
	mfsrr0		4
	mfspr		4, 0x01b
	mfsrr1		4
	mfspr		4, 0x030
	mfpid		4
	mfspr		4, 0x03a
	mfcsrr0		4
	mfspr		4, 0x03b
	mfcsrr1		4
	mfspr		4, 0x03d
	mfdear		4
	mfspr		4, 0x03e
	mfesr		4
	mfspr		4, 0x03f
	mfivpr		4
	mfspr		4, 0x100
	mfusprg0	4
	mfspr		4, 0x104
	mfsprg4		4
	mfspr		4, 0x105
	mfsprg5		4
	mfspr		4, 0x106
	mfsprg6		4
	mfspr		4, 0x107
	mfsprg7		4
	mfspr		4, 0x10c
	mftbl		4
	mftb		4
	mfspr		4, 0x10d
	mftbu		4
	mfspr		4, 0x110
	mfsprg0		4
	mfspr		4, 0x111
	mfsprg1		4
	mfspr		4, 0x112
	mfsprg2		4
	mfspr		4, 0x113
	mfsprg3		4
	mfspr		4, 0x11e
	mfpir		4
	mfspr		4, 0x11f
	mfpvr		4
	mfspr		4, 0x130
	mfdbsr		4
	mfspr		4, 0x134
	mfdbcr0		4
	mfspr		4, 0x135
	mfdbcr1		4
	mfspr		4, 0x136
	mfdbcr2		4
	mfspr		4, 0x138
	mfiac1		4
	mfspr		4, 0x139
	mfiac2		4
	mfspr		4, 0x13a
	mfiac3		4
	mfspr		4, 0x13b
	mfiac4		4
	mfspr		4, 0x13c
	mfdac1		4
	mfspr		4, 0x13d
	mfdac2		4
	mfspr		4, 0x13e
	mfdvc1		4
	mfspr		4, 0x13f
	mfdvc2		4
	mfspr		4, 0x150
	mftsr		4
	mfspr		4, 0x154
	mftcr		4
	mfspr		4, 0x190
	mfivor0		4
	mfspr		4, 0x191
	mfivor1		4
	mfspr		4, 0x192
	mfivor2		4
	mfspr		4, 0x193
	mfivor3		4
	mfspr		4, 0x194
	mfivor4		4
	mfspr		4, 0x195
	mfivor5		4
	mfspr		4, 0x196
	mfivor6		4
	mfspr		4, 0x197
	mfivor7		4
	mfspr		4, 0x198
	mfivor8		4
	mfspr		4, 0x199
	mfivor9		4
	mfspr		4, 0x19a
	mfivor10	4
	mfspr		4, 0x19b
	mfivor11	4
	mfspr		4, 0x19c
	mfivor12	4
	mfspr		4, 0x19d
	mfivor13	4
	mfspr		4, 0x19e
	mfivor14	4
	mfspr		4, 0x19f
	mfivor15	4
	mfspr		4, 0x213
	mfivor35	4
	mfspr		4, 0x23a
	mfmcsrr0	4
	mfspr		4, 0x23b
	mfmcsrr1	4
	mfspr		4, 0x23c
	mfmcsr		4
	mfspr		4, 0x370
	mfivndx		4
	mfspr		4, 0x371
	mfdvndx		4
	mfspr		4, 0x372
	mfivlim		4
	mfspr		4, 0x373
	mfdvlim		4
	mfspr		4, 0x374
	mfclcsr		4
	mfspr		4, 0x378
	mfccr1		4
	mfspr		4, 0x39b
	mfrstcfg	4
	mfspr		4, 0x39c
	mfdcdbtrl	4
	mfspr		4, 0x39d
	mfdcdbtrh	4
	mfspr		4, 0x39f
	mficdbtr	4
	mfspr		4, 0x3b2
	mfmmucr		4
	mfspr		4, 0x3b3
	mfccr0		4
	mfspr		4, 0x3d3
	mficdbdr	4
	mfspr		4, 0x3f3
	mfdbdr		4
	mtspr		0x036, 4
	mtdecar		4
