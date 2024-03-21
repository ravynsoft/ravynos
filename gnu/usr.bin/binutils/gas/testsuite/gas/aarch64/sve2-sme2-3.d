#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	64e06000 	bfmlslb	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	64e06000 	bfmlslb	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	64e0601f 	bfmlslb	z31\.s, z0\.h, z0\.h\[0\]
[^:]+:	64e063e0 	bfmlslb	z0\.s, z31\.h, z0\.h\[0\]
[^:]+:	64e76000 	bfmlslb	z0\.s, z0\.h, z7\.h\[0\]
[^:]+:	64f86800 	bfmlslb	z0\.s, z0\.h, z0\.h\[7\]
[^:]+:	64ec6ac5 	bfmlslb	z5\.s, z22\.h, z4\.h\[3\]
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	64e16020 	bfmlslb	z0\.s, z1\.h, z1\.h\[0\]
[^:]+:	64e0a000 	bfmlslb	z0\.s, z0\.h, z0\.h
[^:]+:	64e0a01f 	bfmlslb	z31\.s, z0\.h, z0\.h
[^:]+:	64e0a3e0 	bfmlslb	z0\.s, z31\.h, z0\.h
[^:]+:	64ffa000 	bfmlslb	z0\.s, z0\.h, z31\.h
[^:]+:	64e6a1b9 	bfmlslb	z25\.s, z13\.h, z6\.h
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	64e2a020 	bfmlslb	z0\.s, z1\.h, z2\.h
[^:]+:	64e06400 	bfmlslt	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	64e06400 	bfmlslt	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	64e0641f 	bfmlslt	z31\.s, z0\.h, z0\.h\[0\]
[^:]+:	64e067e0 	bfmlslt	z0\.s, z31\.h, z0\.h\[0\]
[^:]+:	64e76400 	bfmlslt	z0\.s, z0\.h, z7\.h\[0\]
[^:]+:	64f86c00 	bfmlslt	z0\.s, z0\.h, z0\.h\[7\]
[^:]+:	64ec6ec5 	bfmlslt	z5\.s, z22\.h, z4\.h\[3\]
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	64e16420 	bfmlslt	z0\.s, z1\.h, z1\.h\[0\]
[^:]+:	64e0a400 	bfmlslt	z0\.s, z0\.h, z0\.h
[^:]+:	64e0a41f 	bfmlslt	z31\.s, z0\.h, z0\.h
[^:]+:	64e0a7e0 	bfmlslt	z0\.s, z31\.h, z0\.h
[^:]+:	64ffa400 	bfmlslt	z0\.s, z0\.h, z31\.h
[^:]+:	64e6a5b9 	bfmlslt	z25\.s, z13\.h, z6\.h
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	64e2a420 	bfmlslt	z0\.s, z1\.h, z2\.h
