#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	25208200 	cntp	x0, pn0\.b, vlx2
[^:]+:	25208200 	cntp	x0, pn0\.b, vlx2
[^:]+:	2520821e 	cntp	x30, pn0\.b, vlx2
[^:]+:	2520821f 	cntp	xzr, pn0\.b, vlx2
[^:]+:	252083e0 	cntp	x0, pn15\.b, vlx2
[^:]+:	25208600 	cntp	x0, pn0\.b, vlx4
[^:]+:	252087ab 	cntp	x11, pn13\.b, vlx4
[^:]+:	25608200 	cntp	x0, pn0\.h, vlx2
[^:]+:	25608200 	cntp	x0, pn0\.h, vlx2
[^:]+:	2560821e 	cntp	x30, pn0\.h, vlx2
[^:]+:	2560821f 	cntp	xzr, pn0\.h, vlx2
[^:]+:	256083e0 	cntp	x0, pn15\.h, vlx2
[^:]+:	25608600 	cntp	x0, pn0\.h, vlx4
[^:]+:	25608334 	cntp	x20, pn9\.h, vlx2
[^:]+:	25a08200 	cntp	x0, pn0\.s, vlx2
[^:]+:	25a08200 	cntp	x0, pn0\.s, vlx2
[^:]+:	25a0821e 	cntp	x30, pn0\.s, vlx2
[^:]+:	25a0821f 	cntp	xzr, pn0\.s, vlx2
[^:]+:	25a083e0 	cntp	x0, pn15\.s, vlx2
[^:]+:	25a08600 	cntp	x0, pn0\.s, vlx4
[^:]+:	25a0870f 	cntp	x15, pn8\.s, vlx4
[^:]+:	25e08200 	cntp	x0, pn0\.d, vlx2
[^:]+:	25e08200 	cntp	x0, pn0\.d, vlx2
[^:]+:	25e0821e 	cntp	x30, pn0\.d, vlx2
[^:]+:	25e0821f 	cntp	xzr, pn0\.d, vlx2
[^:]+:	25e083e0 	cntp	x0, pn15\.d, vlx2
[^:]+:	25e08600 	cntp	x0, pn0\.d, vlx4
[^:]+:	25e082a4 	cntp	x4, pn5\.d, vlx2
[^:]+:	25207010 	pext	p0\.b, pn8\[0\]
[^:]+:	25207010 	pext	p0\.b, pn8\[0\]
[^:]+:	2520701f 	pext	p15\.b, pn8\[0\]
[^:]+:	252070f0 	pext	p0\.b, pn15\[0\]
[^:]+:	25207310 	pext	p0\.b, pn8\[3\]
[^:]+:	25207274 	pext	p4\.b, pn11\[2\]
[^:]+:	25607010 	pext	p0\.h, pn8\[0\]
[^:]+:	25607010 	pext	p0\.h, pn8\[0\]
[^:]+:	2560701f 	pext	p15\.h, pn8\[0\]
[^:]+:	256070f0 	pext	p0\.h, pn15\[0\]
[^:]+:	25607310 	pext	p0\.h, pn8\[3\]
[^:]+:	256071d5 	pext	p5\.h, pn14\[1\]
[^:]+:	25a07010 	pext	p0\.s, pn8\[0\]
[^:]+:	25a07010 	pext	p0\.s, pn8\[0\]
[^:]+:	25a0701f 	pext	p15\.s, pn8\[0\]
[^:]+:	25a070f0 	pext	p0\.s, pn15\[0\]
[^:]+:	25a07310 	pext	p0\.s, pn8\[3\]
[^:]+:	25a07256 	pext	p6\.s, pn10\[2\]
[^:]+:	25e07010 	pext	p0\.d, pn8\[0\]
[^:]+:	25e07010 	pext	p0\.d, pn8\[0\]
[^:]+:	25e0701f 	pext	p15\.d, pn8\[0\]
[^:]+:	25e070f0 	pext	p0\.d, pn15\[0\]
[^:]+:	25e07310 	pext	p0\.d, pn8\[3\]
[^:]+:	25e07137 	pext	p7\.d, pn9\[1\]
[^:]+:	25207410 	pext	{p0\.b-p1\.b}, pn8\[0\]
[^:]+:	25207410 	pext	{p0\.b-p1\.b}, pn8\[0\]
[^:]+:	25207410 	pext	{p0\.b-p1\.b}, pn8\[0\]
[^:]+:	2520741e 	pext	{p14\.b-p15\.b}, pn8\[0\]
[^:]+:	2520741f 	pext	{p15\.b-p0\.b}, pn8\[0\]
[^:]+:	2520741f 	pext	{p15\.b-p0\.b}, pn8\[0\]
[^:]+:	252074f0 	pext	{p0\.b-p1\.b}, pn15\[0\]
[^:]+:	25207510 	pext	{p0\.b-p1\.b}, pn8\[1\]
[^:]+:	25207497 	pext	{p7\.b-p8\.b}, pn12\[0\]
[^:]+:	25607410 	pext	{p0\.h-p1\.h}, pn8\[0\]
[^:]+:	25607410 	pext	{p0\.h-p1\.h}, pn8\[0\]
[^:]+:	25607410 	pext	{p0\.h-p1\.h}, pn8\[0\]
[^:]+:	2560741e 	pext	{p14\.h-p15\.h}, pn8\[0\]
[^:]+:	2560741f 	pext	{p15\.h-p0\.h}, pn8\[0\]
[^:]+:	2560741f 	pext	{p15\.h-p0\.h}, pn8\[0\]
[^:]+:	256074f0 	pext	{p0\.h-p1\.h}, pn15\[0\]
[^:]+:	25607510 	pext	{p0\.h-p1\.h}, pn8\[1\]
[^:]+:	256074d2 	pext	{p2\.h-p3\.h}, pn14\[0\]
[^:]+:	25a07410 	pext	{p0\.s-p1\.s}, pn8\[0\]
[^:]+:	25a07410 	pext	{p0\.s-p1\.s}, pn8\[0\]
[^:]+:	25a07410 	pext	{p0\.s-p1\.s}, pn8\[0\]
[^:]+:	25a0741e 	pext	{p14\.s-p15\.s}, pn8\[0\]
[^:]+:	25a0741f 	pext	{p15\.s-p0\.s}, pn8\[0\]
[^:]+:	25a0741f 	pext	{p15\.s-p0\.s}, pn8\[0\]
[^:]+:	25a074f0 	pext	{p0\.s-p1\.s}, pn15\[0\]
[^:]+:	25a07510 	pext	{p0\.s-p1\.s}, pn8\[1\]
[^:]+:	25a074b5 	pext	{p5\.s-p6\.s}, pn13\[0\]
[^:]+:	25e07410 	pext	{p0\.d-p1\.d}, pn8\[0\]
[^:]+:	25e07410 	pext	{p0\.d-p1\.d}, pn8\[0\]
[^:]+:	25e07410 	pext	{p0\.d-p1\.d}, pn8\[0\]
[^:]+:	25e0741e 	pext	{p14\.d-p15\.d}, pn8\[0\]
[^:]+:	25e0741f 	pext	{p15\.d-p0\.d}, pn8\[0\]
[^:]+:	25e0741f 	pext	{p15\.d-p0\.d}, pn8\[0\]
[^:]+:	25e074f0 	pext	{p0\.d-p1\.d}, pn15\[0\]
[^:]+:	25e07510 	pext	{p0\.d-p1\.d}, pn8\[1\]
[^:]+:	25e0743c 	pext	{p12\.d-p13\.d}, pn9\[0\]
[^:]+:	25207810 	ptrue	pn8\.b
[^:]+:	25207813 	ptrue	pn11\.b
[^:]+:	25207817 	ptrue	pn15\.b
[^:]+:	25607810 	ptrue	pn8\.h
[^:]+:	25607811 	ptrue	pn9\.h
[^:]+:	25607817 	ptrue	pn15\.h
[^:]+:	25a07810 	ptrue	pn8\.s
[^:]+:	25a07816 	ptrue	pn14\.s
[^:]+:	25a07817 	ptrue	pn15\.s
[^:]+:	25e07810 	ptrue	pn8\.d
[^:]+:	25e07814 	ptrue	pn12\.d
[^:]+:	25e07817 	ptrue	pn15\.d
[^:]+:	c1208000 	sel	{z0\.b-z1\.b}, pn8, {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c120801e 	sel	{z30\.b-z31\.b}, pn8, {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c1209c00 	sel	{z0\.b-z1\.b}, pn15, {z0\.b-z1\.b}, {z0\.b-z1\.b}
[^:]+:	c12083c0 	sel	{z0\.b-z1\.b}, pn8, {z30\.b-z31\.b}, {z0\.b-z1\.b}
[^:]+:	c13e8000 	sel	{z0\.b-z1\.b}, pn8, {z0\.b-z1\.b}, {z30\.b-z31\.b}
[^:]+:	c12a90c2 	sel	{z2\.b-z3\.b}, pn12, {z6\.b-z7\.b}, {z10\.b-z11\.b}
[^:]+:	c1608000 	sel	{z0\.h-z1\.h}, pn8, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c160801e 	sel	{z30\.h-z31\.h}, pn8, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c1609c00 	sel	{z0\.h-z1\.h}, pn15, {z0\.h-z1\.h}, {z0\.h-z1\.h}
[^:]+:	c16083c0 	sel	{z0\.h-z1\.h}, pn8, {z30\.h-z31\.h}, {z0\.h-z1\.h}
[^:]+:	c17e8000 	sel	{z0\.h-z1\.h}, pn8, {z0\.h-z1\.h}, {z30\.h-z31\.h}
[^:]+:	c17085cc 	sel	{z12\.h-z13\.h}, pn9, {z14\.h-z15\.h}, {z16\.h-z17\.h}
[^:]+:	c1a08000 	sel	{z0\.s-z1\.s}, pn8, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^:]+:	c1a0801e 	sel	{z30\.s-z31\.s}, pn8, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^:]+:	c1a09c00 	sel	{z0\.s-z1\.s}, pn15, {z0\.s-z1\.s}, {z0\.s-z1\.s}
[^:]+:	c1a083c0 	sel	{z0\.s-z1\.s}, pn8, {z30\.s-z31\.s}, {z0\.s-z1\.s}
[^:]+:	c1be8000 	sel	{z0\.s-z1\.s}, pn8, {z0\.s-z1\.s}, {z30\.s-z31\.s}
[^:]+:	c1b88ed2 	sel	{z18\.s-z19\.s}, pn11, {z22\.s-z23\.s}, {z24\.s-z25\.s}
[^:]+:	c1e08000 	sel	{z0\.d-z1\.d}, pn8, {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^:]+:	c1e0801e 	sel	{z30\.d-z31\.d}, pn8, {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^:]+:	c1e09c00 	sel	{z0\.d-z1\.d}, pn15, {z0\.d-z1\.d}, {z0\.d-z1\.d}
[^:]+:	c1e083c0 	sel	{z0\.d-z1\.d}, pn8, {z30\.d-z31\.d}, {z0\.d-z1\.d}
[^:]+:	c1fe8000 	sel	{z0\.d-z1\.d}, pn8, {z0\.d-z1\.d}, {z30\.d-z31\.d}
[^:]+:	c1fc9b48 	sel	{z8\.d-z9\.d}, pn14, {z26\.d-z27\.d}, {z28\.d-z29\.d}
[^:]+:	c1218000 	sel	{z0\.b-z3\.b}, pn8, {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c121801c 	sel	{z28\.b-z31\.b}, pn8, {z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c1218380 	sel	{z0\.b-z3\.b}, pn8, {z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c13d8000 	sel	{z0\.b-z3\.b}, pn8, {z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c12d8904 	sel	{z4\.b-z7\.b}, pn10, {z8\.b-z11\.b}, {z12\.b-z15\.b}
[^:]+:	c1618000 	sel	{z0\.h-z3\.h}, pn8, {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c161801c 	sel	{z28\.h-z31\.h}, pn8, {z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c1618380 	sel	{z0\.h-z3\.h}, pn8, {z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c17d8000 	sel	{z0\.h-z3\.h}, pn8, {z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c1758a08 	sel	{z8\.h-z11\.h}, pn10, {z16\.h-z19\.h}, {z20\.h-z23\.h}
[^:]+:	c1a18000 	sel	{z0\.s-z3\.s}, pn8, {z0\.s-z3\.s}, {z0\.s-z3\.s}
[^:]+:	c1a1801c 	sel	{z28\.s-z31\.s}, pn8, {z0\.s-z3\.s}, {z0\.s-z3\.s}
[^:]+:	c1a18380 	sel	{z0\.s-z3\.s}, pn8, {z28\.s-z31\.s}, {z0\.s-z3\.s}
[^:]+:	c1bd8000 	sel	{z0\.s-z3\.s}, pn8, {z0\.s-z3\.s}, {z28\.s-z31\.s}
[^:]+:	c1b98a90 	sel	{z16\.s-z19\.s}, pn10, {z20\.s-z23\.s}, {z24\.s-z27\.s}
[^:]+:	c1e18000 	sel	{z0\.d-z3\.d}, pn8, {z0\.d-z3\.d}, {z0\.d-z3\.d}
[^:]+:	c1e1801c 	sel	{z28\.d-z31\.d}, pn8, {z0\.d-z3\.d}, {z0\.d-z3\.d}
[^:]+:	c1e18380 	sel	{z0\.d-z3\.d}, pn8, {z28\.d-z31\.d}, {z0\.d-z3\.d}
[^:]+:	c1fd8000 	sel	{z0\.d-z3\.d}, pn8, {z0\.d-z3\.d}, {z28\.d-z31\.d}
[^:]+:	c1e98894 	sel	{z20\.d-z23\.d}, pn10, {z4\.d-z7\.d}, {z8\.d-z11\.d}
