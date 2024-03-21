#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c1000014 	sumlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1006014 	sumlall	za\.s\[w11, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1000017 	sumlall	za\.s\[w8, 12:15\], z0\.b, z0\.b\[0\]
[^:]+:	c10003f4 	sumlall	za\.s\[w8, 0:3\], z31\.b, z0\.b\[0\]
[^:]+:	c10f0014 	sumlall	za\.s\[w8, 0:3\], z0\.b, z15\.b\[0\]
[^:]+:	c1009c14 	sumlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[15\]
[^:]+:	c109a6b6 	sumlall	za\.s\[w9, 8:11\], z21\.b, z9\.b\[9\]
[^:]+:	c1100030 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100030 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1106030 	sumlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100031 	sumlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c11003f0 	sumlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f0030 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b\[0\]
[^:]+:	c1100c36 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[15\]
[^:]+:	c1192e71 	sumlall	za\.s\[w9, 4:7, vgx2\], {z18\.b-z19\.b}, z9\.b\[12\]
[^:]+:	c1108030 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108030 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c110e030 	sumlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108031 	sumlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c11083b0 	sumlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f8030 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b\[0\]
[^:]+:	c1108c36 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[15\]
[^:]+:	c11ec734 	sumlall	za\.s\[w10, 0:3, vgx4\], {z24\.b-z27\.b}, z14\.b\[6\]
[^:]+:	c1200014 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200014 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1206014 	sumlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200015 	sumlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c12003d4 	sumlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b
[^:]+:	c12003f4 	sumlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12003f4 	sumlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12f0014 	sumlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b
[^:]+:	c12d2275 	sumlall	za\.s\[w9, 4:7, vgx2\], {z19\.b-z20\.b}, z13\.b
[^:]+:	c1300014 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300014 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1306014 	sumlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300015 	sumlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300394 	sumlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b
[^:]+:	c13003b4 	sumlall	za\.s\[w8, 0:3, vgx4\], {z29\.b-z0\.b}, z0\.b
[^:]+:	c13003d4 	sumlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003d4 	sumlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003f4 	sumlall	za\.s\[w8, 0:3, vgx4\], {z31\.b-z2\.b}, z0\.b
[^:]+:	c13f0014 	sumlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b
[^:]+:	c13e2334 	sumlall	za\.s\[w9, 0:3, vgx4\], {z25\.b-z28\.b}, z14\.b
[^:]+:	c1000004 	usmlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1006004 	usmlall	za\.s\[w11, 0:3\], z0\.b, z0\.b\[0\]
[^:]+:	c1000007 	usmlall	za\.s\[w8, 12:15\], z0\.b, z0\.b\[0\]
[^:]+:	c10003e4 	usmlall	za\.s\[w8, 0:3\], z31\.b, z0\.b\[0\]
[^:]+:	c10f0004 	usmlall	za\.s\[w8, 0:3\], z0\.b, z15\.b\[0\]
[^:]+:	c1009c04 	usmlall	za\.s\[w8, 0:3\], z0\.b, z0\.b\[15\]
[^:]+:	c109a6a6 	usmlall	za\.s\[w9, 8:11\], z21\.b, z9\.b\[9\]
[^:]+:	c1100020 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100020 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1106020 	usmlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c1100021 	usmlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b\[0\]
[^:]+:	c11003e0 	usmlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f0020 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b\[0\]
[^:]+:	c1100c26 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b\[15\]
[^:]+:	c1192e61 	usmlall	za\.s\[w9, 4:7, vgx2\], {z18\.b-z19\.b}, z9\.b\[12\]
[^:]+:	c1108020 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108020 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c110e020 	usmlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c1108021 	usmlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b\[0\]
[^:]+:	c11083a0 	usmlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b\[0\]
[^:]+:	c11f8020 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b\[0\]
[^:]+:	c1108c26 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b\[15\]
[^:]+:	c11ec724 	usmlall	za\.s\[w10, 0:3, vgx4\], {z24\.b-z27\.b}, z14\.b\[6\]
[^:]+:	c1200404 	usmlall	za\.s\[w8, 0:3\], z0\.b, z0\.b
[^:]+:	c1206404 	usmlall	za\.s\[w11, 0:3\], z0\.b, z0\.b
[^:]+:	c1200407 	usmlall	za\.s\[w8, 12:15\], z0\.b, z0\.b
[^:]+:	c12007e4 	usmlall	za\.s\[w8, 0:3\], z31\.b, z0\.b
[^:]+:	c12f0404 	usmlall	za\.s\[w8, 0:3\], z0\.b, z15\.b
[^:]+:	c1274725 	usmlall	za\.s\[w10, 4:7\], z25\.b, z7\.b
[^:]+:	c1200004 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200004 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1206004 	usmlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c1200005 	usmlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, z0\.b
[^:]+:	c12003c4 	usmlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, z0\.b
[^:]+:	c12003e4 	usmlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12003e4 	usmlall	za\.s\[w8, 0:3, vgx2\], {z31\.b-z0\.b}, z0\.b
[^:]+:	c12f0004 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, z15\.b
[^:]+:	c12d2265 	usmlall	za\.s\[w9, 4:7, vgx2\], {z19\.b-z20\.b}, z13\.b
[^:]+:	c1300004 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300004 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1306004 	usmlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300005 	usmlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, z0\.b
[^:]+:	c1300384 	usmlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, z0\.b
[^:]+:	c13003a4 	usmlall	za\.s\[w8, 0:3, vgx4\], {z29\.b-z0\.b}, z0\.b
[^:]+:	c13003c4 	usmlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003c4 	usmlall	za\.s\[w8, 0:3, vgx4\], {z30\.b-z1\.b}, z0\.b
[^:]+:	c13003e4 	usmlall	za\.s\[w8, 0:3, vgx4\], {z31\.b-z2\.b}, z0\.b
[^:]+:	c13f0004 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, z15\.b
[^:]+:	c13e2324 	usmlall	za\.s\[w9, 0:3, vgx4\], {z25\.b-z28\.b}, z14\.b
[^:]+:	c1a00004 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00004 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a06004 	usmlall	za\.s\[w11, 0:3, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a00005 	usmlall	za\.s\[w8, 4:7, vgx2\], {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1a003c4 	usmlall	za\.s\[w8, 0:3, vgx2\], {z30\.b-z31\.b}, {z0\.b-z1\.b}
[^:]+:	c1be0004 	usmlall	za\.s\[w8, 0:3, vgx2\], {z0\.b-z1\.b}, {z30\.b-z31\.b}
[^:]+:	c1b242c5 	usmlall	za\.s\[w10, 4:7, vgx2\], {z22\.b-z23\.b}, {z18\.b-z19\.b}
[^:]+:	c1a10004 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10004 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a16004 	usmlall	za\.s\[w11, 0:3, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10005 	usmlall	za\.s\[w8, 4:7, vgx4\], {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1a10384 	usmlall	za\.s\[w8, 0:3, vgx4\], {z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c1bd0004 	usmlall	za\.s\[w8, 0:3, vgx4\], {z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c1b96204 	usmlall	za\.s\[w11, 0:3, vgx4\], {z16\.b-z19\.b}, {z24\.b-z27\.b}
