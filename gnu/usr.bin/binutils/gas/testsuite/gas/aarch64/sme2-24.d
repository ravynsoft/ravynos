#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c160e000 	bfcvt	z0\.h, {z0\.s-z1\.s}
[^:]+:	c160e01f 	bfcvt	z31\.h, {z0\.s-z1\.s}
[^:]+:	c160e3c0 	bfcvt	z0\.h, {z30\.s-z31\.s}
[^:]+:	c160e28e 	bfcvt	z14\.h, {z20\.s-z21\.s}
[^:]+:	c160e020 	bfcvtn	z0\.h, {z0\.s-z1\.s}
[^:]+:	c160e03f 	bfcvtn	z31\.h, {z0\.s-z1\.s}
[^:]+:	c160e3e0 	bfcvtn	z0\.h, {z30\.s-z31\.s}
[^:]+:	c160e1fa 	bfcvtn	z26\.h, {z14\.s-z15\.s}
[^:]+:	c120e000 	fcvt	z0\.h, {z0\.s-z1\.s}
[^:]+:	c120e01f 	fcvt	z31\.h, {z0\.s-z1\.s}
[^:]+:	c120e3c0 	fcvt	z0\.h, {z30\.s-z31\.s}
[^:]+:	c120e0dd 	fcvt	z29\.h, {z6\.s-z7\.s}
[^:]+:	c120e020 	fcvtn	z0\.h, {z0\.s-z1\.s}
[^:]+:	c120e03f 	fcvtn	z31\.h, {z0\.s-z1\.s}
[^:]+:	c120e3e0 	fcvtn	z0\.h, {z30\.s-z31\.s}
[^:]+:	c120e0fd 	fcvtn	z29\.h, {z6\.s-z7\.s}
