#as: -march=armv8-a+sme2+sme-i16i64
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c1800000 	smlall	za\.d\[w8, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c1806000 	smlall	za\.d\[w11, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c1800003 	smlall	za\.d\[w8, 12:15\], z0\.h, z0\.h\[0\]
[^:]+:	c18003e0 	smlall	za\.d\[w8, 0:3\], z31\.h, z0\.h\[0\]
[^:]+:	c18f0000 	smlall	za\.d\[w8, 0:3\], z0\.h, z15\.h\[0\]
[^:]+:	c1808c00 	smlall	za\.d\[w8, 0:3\], z0\.h, z0\.h\[7\]
[^:]+:	c1892ea2 	smlall	za\.d\[w9, 8:11\], z21\.h, z9\.h\[3\]
[^:]+:	c1900000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1906000 	smlall	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900001 	smlall	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c19003c0 	smlall	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f0000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h\[0\]
[^:]+:	c1900406 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[7\]
[^:]+:	c1992245 	smlall	za\.d\[w9, 4:7, vgx2\], {z18\.h-z19\.h}, z9\.h\[2\]
[^:]+:	c1908000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c190e000 	smlall	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908001 	smlall	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908380 	smlall	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f8000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h\[0\]
[^:]+:	c1908406 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[7\]
[^:]+:	c19ec704 	smlall	za\.d\[w10, 0:3, vgx4\], {z24\.h-z27\.h}, z14\.h\[6\]
[^:]+:	c1600400 	smlall	za\.d\[w8, 0:3\], z0\.h, z0\.h
[^:]+:	c1606400 	smlall	za\.d\[w11, 0:3\], z0\.h, z0\.h
[^:]+:	c1600403 	smlall	za\.d\[w8, 12:15\], z0\.h, z0\.h
[^:]+:	c16007e0 	smlall	za\.d\[w8, 0:3\], z31\.h, z0\.h
[^:]+:	c16f0400 	smlall	za\.d\[w8, 0:3\], z0\.h, z15\.h
[^:]+:	c1674721 	smlall	za\.d\[w10, 4:7\], z25\.h, z7\.h
[^:]+:	c1600000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1606000 	smlall	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600001 	smlall	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c16003c0 	smlall	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h
[^:]+:	c16003e0 	smlall	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16003e0 	smlall	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16f0000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h
[^:]+:	c16d2261 	smlall	za\.d\[w9, 4:7, vgx2\], {z19\.h-z20\.h}, z13\.h
[^:]+:	c1700000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1706000 	smlall	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700001 	smlall	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700380 	smlall	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h
[^:]+:	c17003a0 	smlall	za\.d\[w8, 0:3, vgx4\], {z29\.h-z0\.h}, z0\.h
[^:]+:	c17003c0 	smlall	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003c0 	smlall	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003e0 	smlall	za\.d\[w8, 0:3, vgx4\], {z31\.h-z2\.h}, z0\.h
[^:]+:	c17f0000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h
[^:]+:	c17e2320 	smlall	za\.d\[w9, 0:3, vgx4\], {z25\.h-z28\.h}, z14\.h
[^:]+:	c1e00000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e06000 	smlall	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00001 	smlall	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e003c0 	smlall	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, {z0\.h-z1\.h}
[^:]+:	c1fe0000 	smlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z30\.h-z31\.h}
[^:]+:	c1f242c1 	smlall	za\.d\[w10, 4:7, vgx2\], {z22\.h-z23\.h}, {z18\.h-z19\.h}
[^:]+:	c1e10000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e16000 	smlall	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10001 	smlall	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10380 	smlall	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c1fd0000 	smlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c1f96200 	smlall	za\.d\[w11, 0:3, vgx4\], {z16\.h-z19\.h}, {z24\.h-z27\.h}
[^:]+:	c1800008 	smlsll	za\.d\[w8, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c1806008 	smlsll	za\.d\[w11, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c180000b 	smlsll	za\.d\[w8, 12:15\], z0\.h, z0\.h\[0\]
[^:]+:	c18003e8 	smlsll	za\.d\[w8, 0:3\], z31\.h, z0\.h\[0\]
[^:]+:	c18f0008 	smlsll	za\.d\[w8, 0:3\], z0\.h, z15\.h\[0\]
[^:]+:	c1808c08 	smlsll	za\.d\[w8, 0:3\], z0\.h, z0\.h\[7\]
[^:]+:	c1892eaa 	smlsll	za\.d\[w9, 8:11\], z21\.h, z9\.h\[3\]
[^:]+:	c1900008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1906008 	smlsll	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900009 	smlsll	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c19003c8 	smlsll	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f0008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h\[0\]
[^:]+:	c190040e 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[7\]
[^:]+:	c199224d 	smlsll	za\.d\[w9, 4:7, vgx2\], {z18\.h-z19\.h}, z9\.h\[2\]
[^:]+:	c1908008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c190e008 	smlsll	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908009 	smlsll	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908388 	smlsll	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f8008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h\[0\]
[^:]+:	c190840e 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[7\]
[^:]+:	c19ec70c 	smlsll	za\.d\[w10, 0:3, vgx4\], {z24\.h-z27\.h}, z14\.h\[6\]
[^:]+:	c1600408 	smlsll	za\.d\[w8, 0:3\], z0\.h, z0\.h
[^:]+:	c1606408 	smlsll	za\.d\[w11, 0:3\], z0\.h, z0\.h
[^:]+:	c160040b 	smlsll	za\.d\[w8, 12:15\], z0\.h, z0\.h
[^:]+:	c16007e8 	smlsll	za\.d\[w8, 0:3\], z31\.h, z0\.h
[^:]+:	c16f0408 	smlsll	za\.d\[w8, 0:3\], z0\.h, z15\.h
[^:]+:	c1674729 	smlsll	za\.d\[w10, 4:7\], z25\.h, z7\.h
[^:]+:	c1600008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1606008 	smlsll	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600009 	smlsll	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c16003c8 	smlsll	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h
[^:]+:	c16003e8 	smlsll	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16003e8 	smlsll	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16f0008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h
[^:]+:	c16d2269 	smlsll	za\.d\[w9, 4:7, vgx2\], {z19\.h-z20\.h}, z13\.h
[^:]+:	c1700008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1706008 	smlsll	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700009 	smlsll	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700388 	smlsll	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h
[^:]+:	c17003a8 	smlsll	za\.d\[w8, 0:3, vgx4\], {z29\.h-z0\.h}, z0\.h
[^:]+:	c17003c8 	smlsll	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003c8 	smlsll	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003e8 	smlsll	za\.d\[w8, 0:3, vgx4\], {z31\.h-z2\.h}, z0\.h
[^:]+:	c17f0008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h
[^:]+:	c17e2328 	smlsll	za\.d\[w9, 0:3, vgx4\], {z25\.h-z28\.h}, z14\.h
[^:]+:	c1e00008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e06008 	smlsll	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00009 	smlsll	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e003c8 	smlsll	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, {z0\.h-z1\.h}
[^:]+:	c1fe0008 	smlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z30\.h-z31\.h}
[^:]+:	c1f242c9 	smlsll	za\.d\[w10, 4:7, vgx2\], {z22\.h-z23\.h}, {z18\.h-z19\.h}
[^:]+:	c1e10008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e16008 	smlsll	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10009 	smlsll	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10388 	smlsll	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c1fd0008 	smlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c1f96208 	smlsll	za\.d\[w11, 0:3, vgx4\], {z16\.h-z19\.h}, {z24\.h-z27\.h}
[^:]+:	c1800010 	umlall	za\.d\[w8, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c1806010 	umlall	za\.d\[w11, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c1800013 	umlall	za\.d\[w8, 12:15\], z0\.h, z0\.h\[0\]
[^:]+:	c18003f0 	umlall	za\.d\[w8, 0:3\], z31\.h, z0\.h\[0\]
[^:]+:	c18f0010 	umlall	za\.d\[w8, 0:3\], z0\.h, z15\.h\[0\]
[^:]+:	c1808c10 	umlall	za\.d\[w8, 0:3\], z0\.h, z0\.h\[7\]
[^:]+:	c1892eb2 	umlall	za\.d\[w9, 8:11\], z21\.h, z9\.h\[3\]
[^:]+:	c1900010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1906010 	umlall	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900011 	umlall	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c19003d0 	umlall	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f0010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h\[0\]
[^:]+:	c1900416 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[7\]
[^:]+:	c1992653 	umlall	za\.d\[w9, 4:7, vgx2\], {z18\.h-z19\.h}, z9\.h\[5\]
[^:]+:	c1908010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c190e010 	umlall	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908011 	umlall	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908390 	umlall	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f8010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h\[0\]
[^:]+:	c1908416 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[7\]
[^:]+:	c19ec312 	umlall	za\.d\[w10, 0:3, vgx4\], {z24\.h-z27\.h}, z14\.h\[1\]
[^:]+:	c1600410 	umlall	za\.d\[w8, 0:3\], z0\.h, z0\.h
[^:]+:	c1606410 	umlall	za\.d\[w11, 0:3\], z0\.h, z0\.h
[^:]+:	c1600413 	umlall	za\.d\[w8, 12:15\], z0\.h, z0\.h
[^:]+:	c16007f0 	umlall	za\.d\[w8, 0:3\], z31\.h, z0\.h
[^:]+:	c16f0410 	umlall	za\.d\[w8, 0:3\], z0\.h, z15\.h
[^:]+:	c1674731 	umlall	za\.d\[w10, 4:7\], z25\.h, z7\.h
[^:]+:	c1600010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1606010 	umlall	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600011 	umlall	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c16003d0 	umlall	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h
[^:]+:	c16003f0 	umlall	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16003f0 	umlall	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16f0010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h
[^:]+:	c16d2271 	umlall	za\.d\[w9, 4:7, vgx2\], {z19\.h-z20\.h}, z13\.h
[^:]+:	c1700010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1706010 	umlall	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700011 	umlall	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700390 	umlall	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h
[^:]+:	c17003b0 	umlall	za\.d\[w8, 0:3, vgx4\], {z29\.h-z0\.h}, z0\.h
[^:]+:	c17003d0 	umlall	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003d0 	umlall	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003f0 	umlall	za\.d\[w8, 0:3, vgx4\], {z31\.h-z2\.h}, z0\.h
[^:]+:	c17f0010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h
[^:]+:	c17e2330 	umlall	za\.d\[w9, 0:3, vgx4\], {z25\.h-z28\.h}, z14\.h
[^:]+:	c1e00010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e06010 	umlall	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00011 	umlall	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e003d0 	umlall	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, {z0\.h-z1\.h}
[^:]+:	c1fe0010 	umlall	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z30\.h-z31\.h}
[^:]+:	c1f242d1 	umlall	za\.d\[w10, 4:7, vgx2\], {z22\.h-z23\.h}, {z18\.h-z19\.h}
[^:]+:	c1e10010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e16010 	umlall	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10011 	umlall	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10390 	umlall	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c1fd0010 	umlall	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c1f96210 	umlall	za\.d\[w11, 0:3, vgx4\], {z16\.h-z19\.h}, {z24\.h-z27\.h}
[^:]+:	c1800018 	umlsll	za\.d\[w8, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c1806018 	umlsll	za\.d\[w11, 0:3\], z0\.h, z0\.h\[0\]
[^:]+:	c180001b 	umlsll	za\.d\[w8, 12:15\], z0\.h, z0\.h\[0\]
[^:]+:	c18003f8 	umlsll	za\.d\[w8, 0:3\], z31\.h, z0\.h\[0\]
[^:]+:	c18f0018 	umlsll	za\.d\[w8, 0:3\], z0\.h, z15\.h\[0\]
[^:]+:	c1808c18 	umlsll	za\.d\[w8, 0:3\], z0\.h, z0\.h\[7\]
[^:]+:	c1892eba 	umlsll	za\.d\[w9, 8:11\], z21\.h, z9\.h\[3\]
[^:]+:	c1900018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1906018 	umlsll	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c1900019 	umlsll	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h\[0\]
[^:]+:	c19003d8 	umlsll	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f0018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h\[0\]
[^:]+:	c190041e 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h\[7\]
[^:]+:	c199225d 	umlsll	za\.d\[w9, 4:7, vgx2\], {z18\.h-z19\.h}, z9\.h\[2\]
[^:]+:	c1908018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c190e018 	umlsll	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908019 	umlsll	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h\[0\]
[^:]+:	c1908398 	umlsll	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h\[0\]
[^:]+:	c19f8018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h\[0\]
[^:]+:	c190841e 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h\[7\]
[^:]+:	c19ec71c 	umlsll	za\.d\[w10, 0:3, vgx4\], {z24\.h-z27\.h}, z14\.h\[6\]
[^:]+:	c1600418 	umlsll	za\.d\[w8, 0:3\], z0\.h, z0\.h
[^:]+:	c1606418 	umlsll	za\.d\[w11, 0:3\], z0\.h, z0\.h
[^:]+:	c160041b 	umlsll	za\.d\[w8, 12:15\], z0\.h, z0\.h
[^:]+:	c16007f8 	umlsll	za\.d\[w8, 0:3\], z31\.h, z0\.h
[^:]+:	c16f0418 	umlsll	za\.d\[w8, 0:3\], z0\.h, z15\.h
[^:]+:	c1674739 	umlsll	za\.d\[w10, 4:7\], z25\.h, z7\.h
[^:]+:	c1600018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1606018 	umlsll	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c1600019 	umlsll	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, z0\.h
[^:]+:	c16003d8 	umlsll	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, z0\.h
[^:]+:	c16003f8 	umlsll	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16003f8 	umlsll	za\.d\[w8, 0:3, vgx2\], {z31\.h-z0\.h}, z0\.h
[^:]+:	c16f0018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, z15\.h
[^:]+:	c16d2279 	umlsll	za\.d\[w9, 4:7, vgx2\], {z19\.h-z20\.h}, z13\.h
[^:]+:	c1700018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1706018 	umlsll	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700019 	umlsll	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, z0\.h
[^:]+:	c1700398 	umlsll	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, z0\.h
[^:]+:	c17003b8 	umlsll	za\.d\[w8, 0:3, vgx4\], {z29\.h-z0\.h}, z0\.h
[^:]+:	c17003d8 	umlsll	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003d8 	umlsll	za\.d\[w8, 0:3, vgx4\], {z30\.h-z1\.h}, z0\.h
[^:]+:	c17003f8 	umlsll	za\.d\[w8, 0:3, vgx4\], {z31\.h-z2\.h}, z0\.h
[^:]+:	c17f0018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, z15\.h
[^:]+:	c17e2338 	umlsll	za\.d\[w9, 0:3, vgx4\], {z25\.h-z28\.h}, z14\.h
[^:]+:	c1e00018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e06018 	umlsll	za\.d\[w11, 0:3, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e00019 	umlsll	za\.d\[w8, 4:7, vgx2\], {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1e003d8 	umlsll	za\.d\[w8, 0:3, vgx2\], {z30\.h-z31\.h}, {z0\.h-z1\.h}
[^:]+:	c1fe0018 	umlsll	za\.d\[w8, 0:3, vgx2\], {z0\.h-z1\.h}, {z30\.h-z31\.h}
[^:]+:	c1f242d9 	umlsll	za\.d\[w10, 4:7, vgx2\], {z22\.h-z23\.h}, {z18\.h-z19\.h}
[^:]+:	c1e10018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e16018 	umlsll	za\.d\[w11, 0:3, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10019 	umlsll	za\.d\[w8, 4:7, vgx4\], {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1e10398 	umlsll	za\.d\[w8, 0:3, vgx4\], {z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c1fd0018 	umlsll	za\.d\[w8, 0:3, vgx4\], {z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c1f96218 	umlsll	za\.d\[w11, 0:3, vgx4\], {z16\.h-z19\.h}, {z24\.h-z27\.h}
