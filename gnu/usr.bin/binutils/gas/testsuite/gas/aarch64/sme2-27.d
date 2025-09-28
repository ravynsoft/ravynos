#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c1efd400 	sqrshr	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1efd41f 	sqrshr	z31\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1efd7c0 	sqrshr	z0\.h, {z30\.s-z31\.s}, #1
[^:]+:	c1e0d400 	sqrshr	z0\.h, {z0\.s-z1\.s}, #16
[^:]+:	c1e9d6ce 	sqrshr	z14\.h, {z22\.s-z23\.s}, #7
[^:]+:	c1efd400 	sqrshr	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1eed400 	sqrshr	z0\.h, {z0\.s-z1\.s}, #2
[^:]+:	c1edd400 	sqrshr	z0\.h, {z0\.s-z1\.s}, #3
[^:]+:	c1ecd400 	sqrshr	z0\.h, {z0\.s-z1\.s}, #4
[^:]+:	c17fd800 	sqrshr	z0\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fd81f 	sqrshr	z31\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdb80 	sqrshr	z0\.b, {z28\.s-z31\.s}, #1
[^:]+:	c160d800 	sqrshr	z0\.b, {z0\.s-z3\.s}, #32
[^:]+:	c167d986 	sqrshr	z6\.b, {z12\.s-z15\.s}, #25
[^:]+:	c1ffd800 	sqrshr	z0\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffd81f 	sqrshr	z31\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdb80 	sqrshr	z0\.h, {z28\.d-z31\.d}, #1
[^:]+:	c1a0d800 	sqrshr	z0\.h, {z0\.d-z3\.d}, #64
[^:]+:	c1aeda99 	sqrshr	z25\.h, {z20\.d-z23\.d}, #50
[^:]+:	c13fd800 	\.inst	0xc13fd800 ; undefined
[^:]+:	c120d800 	\.inst	0xc120d800 ; undefined
[^:]+:	c1ffd400 	sqrshru	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1ffd41f 	sqrshru	z31\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1ffd7c0 	sqrshru	z0\.h, {z30\.s-z31\.s}, #1
[^:]+:	c1f0d400 	sqrshru	z0\.h, {z0\.s-z1\.s}, #16
[^:]+:	c1f9d6ce 	sqrshru	z14\.h, {z22\.s-z23\.s}, #7
[^:]+:	c17fd840 	sqrshru	z0\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fd85f 	sqrshru	z31\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdbc0 	sqrshru	z0\.b, {z28\.s-z31\.s}, #1
[^:]+:	c160d840 	sqrshru	z0\.b, {z0\.s-z3\.s}, #32
[^:]+:	c167d9c6 	sqrshru	z6\.b, {z12\.s-z15\.s}, #25
[^:]+:	c1ffd840 	sqrshru	z0\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffd85f 	sqrshru	z31\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdbc0 	sqrshru	z0\.h, {z28\.d-z31\.d}, #1
[^:]+:	c1a0d840 	sqrshru	z0\.h, {z0\.d-z3\.d}, #64
[^:]+:	c1aedad9 	sqrshru	z25\.h, {z20\.d-z23\.d}, #50
[^:]+:	c1efd420 	uqrshr	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1efd43f 	uqrshr	z31\.h, {z0\.s-z1\.s}, #1
[^:]+:	c1efd7e0 	uqrshr	z0\.h, {z30\.s-z31\.s}, #1
[^:]+:	c1e0d420 	uqrshr	z0\.h, {z0\.s-z1\.s}, #16
[^:]+:	c1e9d6ee 	uqrshr	z14\.h, {z22\.s-z23\.s}, #7
[^:]+:	c17fd820 	uqrshr	z0\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fd83f 	uqrshr	z31\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdba0 	uqrshr	z0\.b, {z28\.s-z31\.s}, #1
[^:]+:	c160d820 	uqrshr	z0\.b, {z0\.s-z3\.s}, #32
[^:]+:	c167d9a6 	uqrshr	z6\.b, {z12\.s-z15\.s}, #25
[^:]+:	c1ffd820 	uqrshr	z0\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffd83f 	uqrshr	z31\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdba0 	uqrshr	z0\.h, {z28\.d-z31\.d}, #1
[^:]+:	c1a0d820 	uqrshr	z0\.h, {z0\.d-z3\.d}, #64
[^:]+:	c1aedab9 	uqrshr	z25\.h, {z20\.d-z23\.d}, #50
[^:]+:	c13fd820 	\.inst	0xc13fd820 ; undefined
[^:]+:	c120d820 	\.inst	0xc120d820 ; undefined
