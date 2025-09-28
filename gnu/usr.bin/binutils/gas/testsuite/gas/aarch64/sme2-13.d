#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c1000000 	smlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1006000 	smlall	za\.s\[w11, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1000003 	smlall	za\.s\[w8, 12:15\], z0\.b, z0\.b\[0\]
[^:]+:	c10003e0 	smlall	za\.s\[w8, 0:3\], z31\.b, z0\.b\[0\]
[^:]+:	c10f0000 	smlall	za\.s\[w8, 0:3\], z0\.b, z15\.b\[0\]
[^:]+:	c1009c00 	smlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[15\]
[^:]+:	c109a6a2 	smlall	za\.s\[w9, 8:11\], z21\.b, z9\.b\[9\]
[^:]+:	c1100000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1106000 	smlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100001 	smlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c11003c0 	smlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f0000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b\[0\]
[^:]+:	c1100c06 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[15\]
[^:]+:	c1192e41 	smlall	za\.s\[w9, 4:7, vgx2\], {z18\.b-z19\.b}, z9\.b\[12\]
[^:]+:	c1108000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c110e000 	smlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108001 	smlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108380 	smlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f8000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b\[0\]
[^:]+:	c1108c06 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[15\]
[^:]+:	c11ec704 	smlall	za\.s\[w10, 0:3, vgx4\], {z24\.b-z27\.b}, z14\.b\[6\]
[^:]+:	c1200400 	smlall	za\.s\[w8, 0:3\], z0\.b, z0\.b
[^:]+:	c1206400 	smlall	za\.s\[w11, 0:3\], z0\.b, z0\.b
[^:]+:	c1200403 	smlall	za\.s\[w8, 12:15\], z0\.b, z0\.b
[^:]+:	c12007e0 	smlall	za\.s\[w8, 0:3\], z31\.b, z0\.b
[^:]+:	c12f0400 	smlall	za\.s\[w8, 0:3\], z0\.b, z15\.b
[^:]+:	c1274721 	smlall	za\.s\[w10, 4:7\], z25\.b, z7\.b
[^:]+:	c1200000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1206000 	smlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200001 	smlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c12003c0 	smlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b
[^:]+:	c12003e0 	smlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12003e0 	smlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12f0000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b
[^:]+:	c12d2261 	smlall	za\.s\[w9, 4:7, vgx2\], {z19\.b-z20\.b}, z13\.b
[^:]+:	c1300000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1306000 	smlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300001 	smlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300380 	smlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b
[^:]+:	c13003a0 	smlall	za\.s\[w8, 0:3, vgx4\], {z29\.b-z0\.b}, z0\.b
[^:]+:	c13003c0 	smlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003c0 	smlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003e0 	smlall	za\.s\[w8, 0:3, vgx4\], {z31\.b-z2\.b}, z0\.b
[^:]+:	c13f0000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b
[^:]+:	c13e2320 	smlall	za\.s\[w9, 0:3, vgx4\], {z25\.b-z28\.b}, z14\.b
[^:]+:	c1a00000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a06000 	smlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00001 	smlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a003c0 	smlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, {z0\.b-z1\.b}
[^:]+:	c1be0000 	smlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z30\.b-z31\.b}
[^:]+:	c1b242c1 	smlall	za\.s\[w10, 4:7, vgx2\], {z22\.b-z23\.b}, {z18\.b-z19\.b}
[^:]+:	c1a10000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a16000 	smlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10001 	smlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10380 	smlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c1bd0000 	smlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c1b96200 	smlall	za\.s\[w11, 0:3, vgx4\], {z16\.b-z19\.b}, {z24\.b-z27\.b}
[^:]+:	c1000008 	smlsll	za\.s\[w8, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1006008 	smlsll	za\.s\[w11, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c100000b 	smlsll	za\.s\[w8, 12:15\], z0\.b, z0\.b\[0\]
[^:]+:	c10003e8 	smlsll	za\.s\[w8, 0:3\], z31\.b, z0\.b\[0\]
[^:]+:	c10f0008 	smlsll	za\.s\[w8, 0:3\], z0\.b, z15\.b\[0\]
[^:]+:	c1009c08 	smlsll	za\.s\[w8, 0:3\], z0\.b, z0\.b\[15\]
[^:]+:	c109a6aa 	smlsll	za\.s\[w9, 8:11\], z21\.b, z9\.b\[9\]
[^:]+:	c1100008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1106008 	smlsll	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100009 	smlsll	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c11003c8 	smlsll	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f0008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b\[0\]
[^:]+:	c1100c0e 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[15\]
[^:]+:	c1192e49 	smlsll	za\.s\[w9, 4:7, vgx2\], {z18\.b-z19\.b}, z9\.b\[12\]
[^:]+:	c1108008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c110e008 	smlsll	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108009 	smlsll	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108388 	smlsll	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f8008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b\[0\]
[^:]+:	c1108c0e 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[15\]
[^:]+:	c11ec70c 	smlsll	za\.s\[w10, 0:3, vgx4\], {z24\.b-z27\.b}, z14\.b\[6\]
[^:]+:	c1200408 	smlsll	za\.s\[w8, 0:3\], z0\.b, z0\.b
[^:]+:	c1206408 	smlsll	za\.s\[w11, 0:3\], z0\.b, z0\.b
[^:]+:	c120040b 	smlsll	za\.s\[w8, 12:15\], z0\.b, z0\.b
[^:]+:	c12007e8 	smlsll	za\.s\[w8, 0:3\], z31\.b, z0\.b
[^:]+:	c12f0408 	smlsll	za\.s\[w8, 0:3\], z0\.b, z15\.b
[^:]+:	c1274729 	smlsll	za\.s\[w10, 4:7\], z25\.b, z7\.b
[^:]+:	c1200008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1206008 	smlsll	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200009 	smlsll	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c12003c8 	smlsll	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b
[^:]+:	c12003e8 	smlsll	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12003e8 	smlsll	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12f0008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b
[^:]+:	c12d2269 	smlsll	za\.s\[w9, 4:7, vgx2\], {z19\.b-z20\.b}, z13\.b
[^:]+:	c1300008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1306008 	smlsll	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300009 	smlsll	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300388 	smlsll	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b
[^:]+:	c13003a8 	smlsll	za\.s\[w8, 0:3, vgx4\], {z29\.b-z0\.b}, z0\.b
[^:]+:	c13003c8 	smlsll	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003c8 	smlsll	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003e8 	smlsll	za\.s\[w8, 0:3, vgx4\], {z31\.b-z2\.b}, z0\.b
[^:]+:	c13f0008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b
[^:]+:	c13e2328 	smlsll	za\.s\[w9, 0:3, vgx4\], {z25\.b-z28\.b}, z14\.b
[^:]+:	c1a00008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a06008 	smlsll	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00009 	smlsll	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a003c8 	smlsll	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, {z0\.b-z1\.b}
[^:]+:	c1be0008 	smlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z30\.b-z31\.b}
[^:]+:	c1b242c9 	smlsll	za\.s\[w10, 4:7, vgx2\], {z22\.b-z23\.b}, {z18\.b-z19\.b}
[^:]+:	c1a10008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a16008 	smlsll	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10009 	smlsll	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10388 	smlsll	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c1bd0008 	smlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c1b96208 	smlsll	za\.s\[w11, 0:3, vgx4\], {z16\.b-z19\.b}, {z24\.b-z27\.b}
[^:]+:	c1000010 	umlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1006010 	umlall	za\.s\[w11, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1000013 	umlall	za\.s\[w8, 12:15\], z0\.b, z0\.b\[0\]
[^:]+:	c10003f0 	umlall	za\.s\[w8, 0:3\], z31\.b, z0\.b\[0\]
[^:]+:	c10f0010 	umlall	za\.s\[w8, 0:3\], z0\.b, z15\.b\[0\]
[^:]+:	c1009c10 	umlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[15\]
[^:]+:	c109a6b2 	umlall	za\.s\[w9, 8:11\], z21\.b, z9\.b\[9\]
[^:]+:	c1100010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1106010 	umlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100011 	umlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c11003d0 	umlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f0010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b\[0\]
[^:]+:	c1100c16 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[15\]
[^:]+:	c1192e51 	umlall	za\.s\[w9, 4:7, vgx2\], {z18\.b-z19\.b}, z9\.b\[12\]
[^:]+:	c1108010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c110e010 	umlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108011 	umlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108390 	umlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f8010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b\[0\]
[^:]+:	c1108c16 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[15\]
[^:]+:	c11ec714 	umlall	za\.s\[w10, 0:3, vgx4\], {z24\.b-z27\.b}, z14\.b\[6\]
[^:]+:	c1200410 	umlall	za\.s\[w8, 0:3\], z0\.b, z0\.b
[^:]+:	c1206410 	umlall	za\.s\[w11, 0:3\], z0\.b, z0\.b
[^:]+:	c1200413 	umlall	za\.s\[w8, 12:15\], z0\.b, z0\.b
[^:]+:	c12007f0 	umlall	za\.s\[w8, 0:3\], z31\.b, z0\.b
[^:]+:	c12f0410 	umlall	za\.s\[w8, 0:3\], z0\.b, z15\.b
[^:]+:	c1274731 	umlall	za\.s\[w10, 4:7\], z25\.b, z7\.b
[^:]+:	c1200010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1206010 	umlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200011 	umlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c12003d0 	umlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b
[^:]+:	c12003f0 	umlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12003f0 	umlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12f0010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b
[^:]+:	c12d2271 	umlall	za\.s\[w9, 4:7, vgx2\], {z19\.b-z20\.b}, z13\.b
[^:]+:	c1300010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1306010 	umlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300011 	umlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300390 	umlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b
[^:]+:	c13003b0 	umlall	za\.s\[w8, 0:3, vgx4\], {z29\.b-z0\.b}, z0\.b
[^:]+:	c13003d0 	umlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003d0 	umlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003f0 	umlall	za\.s\[w8, 0:3, vgx4\], {z31\.b-z2\.b}, z0\.b
[^:]+:	c13f0010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b
[^:]+:	c13e2330 	umlall	za\.s\[w9, 0:3, vgx4\], {z25\.b-z28\.b}, z14\.b
[^:]+:	c1a00010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a06010 	umlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00011 	umlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a003d0 	umlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, {z0\.b-z1\.b}
[^:]+:	c1be0010 	umlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z30\.b-z31\.b}
[^:]+:	c1b242d1 	umlall	za\.s\[w10, 4:7, vgx2\], {z22\.b-z23\.b}, {z18\.b-z19\.b}
[^:]+:	c1a10010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a16010 	umlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10011 	umlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10390 	umlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c1bd0010 	umlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c1b96210 	umlall	za\.s\[w11, 0:3, vgx4\], {z16\.b-z19\.b}, {z24\.b-z27\.b}
[^:]+:	c1000018 	umlsll	za\.s\[w8, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1006018 	umlsll	za\.s\[w11, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c100001b 	umlsll	za\.s\[w8, 12:15\], z0\.b, z0\.b\[0\]
[^:]+:	c10003f8 	umlsll	za\.s\[w8, 0:3\], z31\.b, z0\.b\[0\]
[^:]+:	c10f0018 	umlsll	za\.s\[w8, 0:3\], z0\.b, z15\.b\[0\]
[^:]+:	c1009c18 	umlsll	za\.s\[w8, 0:3\], z0\.b, z0\.b\[15\]
[^:]+:	c109a6ba 	umlsll	za\.s\[w9, 8:11\], z21\.b, z9\.b\[9\]
[^:]+:	c1100018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1106018 	umlsll	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100019 	umlsll	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c11003d8 	umlsll	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f0018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b\[0\]
[^:]+:	c1100c1e 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[15\]
[^:]+:	c1192e59 	umlsll	za\.s\[w9, 4:7, vgx2\], {z18\.b-z19\.b}, z9\.b\[12\]
[^:]+:	c1108018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c110e018 	umlsll	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108019 	umlsll	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108398 	umlsll	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f8018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b\[0\]
[^:]+:	c1108c1e 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[15\]
[^:]+:	c11ec71c 	umlsll	za\.s\[w10, 0:3, vgx4\], {z24\.b-z27\.b}, z14\.b\[6\]
[^:]+:	c1200418 	umlsll	za\.s\[w8, 0:3\], z0\.b, z0\.b
[^:]+:	c1206418 	umlsll	za\.s\[w11, 0:3\], z0\.b, z0\.b
[^:]+:	c120041b 	umlsll	za\.s\[w8, 12:15\], z0\.b, z0\.b
[^:]+:	c12007f8 	umlsll	za\.s\[w8, 0:3\], z31\.b, z0\.b
[^:]+:	c12f0418 	umlsll	za\.s\[w8, 0:3\], z0\.b, z15\.b
[^:]+:	c1274739 	umlsll	za\.s\[w10, 4:7\], z25\.b, z7\.b
[^:]+:	c1200018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1206018 	umlsll	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200019 	umlsll	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c12003d8 	umlsll	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b
[^:]+:	c12003f8 	umlsll	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12003f8 	umlsll	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12f0018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b
[^:]+:	c12d2279 	umlsll	za\.s\[w9, 4:7, vgx2\], {z19\.b-z20\.b}, z13\.b
[^:]+:	c1300018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1306018 	umlsll	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300019 	umlsll	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300398 	umlsll	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b
[^:]+:	c13003b8 	umlsll	za\.s\[w8, 0:3, vgx4\], {z29\.b-z0\.b}, z0\.b
[^:]+:	c13003d8 	umlsll	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003d8 	umlsll	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003f8 	umlsll	za\.s\[w8, 0:3, vgx4\], {z31\.b-z2\.b}, z0\.b
[^:]+:	c13f0018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b
[^:]+:	c13e2338 	umlsll	za\.s\[w9, 0:3, vgx4\], {z25\.b-z28\.b}, z14\.b
[^:]+:	c1a00018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a06018 	umlsll	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00019 	umlsll	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a003d8 	umlsll	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, {z0\.b-z1\.b}
[^:]+:	c1be0018 	umlsll	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z30\.b-z31\.b}
[^:]+:	c1b242d9 	umlsll	za\.s\[w10, 4:7, vgx2\], {z22\.b-z23\.b}, {z18\.b-z19\.b}
[^:]+:	c1a10018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a16018 	umlsll	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10019 	umlsll	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10398 	umlsll	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c1bd0018 	umlsll	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c1b96218 	umlsll	za\.s\[w11, 0:3, vgx4\], {z16\.b-z19\.b}, {z24\.b-z27\.b}
