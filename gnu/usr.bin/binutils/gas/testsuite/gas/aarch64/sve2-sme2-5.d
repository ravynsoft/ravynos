#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	45314000 	sqcvtn	z0\.h, {z0\.s-z1\.s}
[^:]+:	4531401f 	sqcvtn	z31\.h, {z0\.s-z1\.s}
[^:]+:	453143c0 	sqcvtn	z0\.h, {z30\.s-z31\.s}
[^:]+:	4531428e 	sqcvtn	z14\.h, {z20\.s-z21\.s}
[^:]+:	45315000 	sqcvtun	z0\.h, {z0\.s-z1\.s}
[^:]+:	4531501f 	sqcvtun	z31\.h, {z0\.s-z1\.s}
[^:]+:	453153c0 	sqcvtun	z0\.h, {z30\.s-z31\.s}
[^:]+:	453151da 	sqcvtun	z26\.h, {z14\.s-z15\.s}
[^:]+:	45314800 	uqcvtn	z0\.h, {z0\.s-z1\.s}
[^:]+:	4531481f 	uqcvtn	z31\.h, {z0\.s-z1\.s}
[^:]+:	45314bc0 	uqcvtn	z0\.h, {z30\.s-z31\.s}
[^:]+:	453148dd 	uqcvtn	z29\.h, {z6\.s-z7\.s}
