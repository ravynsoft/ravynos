#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	64204000 	fdot	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	64204000 	fdot	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	6420401f 	fdot	z31\.s, z0\.h, z0\.h\[0\]
[^:]+:	642043e0 	fdot	z0\.s, z31\.h, z0\.h\[0\]
[^:]+:	64274000 	fdot	z0\.s, z0\.h, z7\.h\[0\]
[^:]+:	64384000 	fdot	z0\.s, z0\.h, z0\.h\[3\]
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	64214020 	fdot	z0\.s, z1\.h, z1\.h\[0\]
[^:]+:	64208000 	fdot	z0\.s, z0\.h, z0\.h
[^:]+:	6420801f 	fdot	z31\.s, z0\.h, z0\.h
[^:]+:	642083e0 	fdot	z0\.s, z31\.h, z0\.h
[^:]+:	643f8000 	fdot	z0\.s, z0\.h, z31\.h
[^:]+:	6429834e 	fdot	z14\.s, z26\.h, z9\.h
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	64228020 	fdot	z0\.s, z1\.h, z2\.h
[^:]+:	4480c800 	sdot	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	4480c800 	sdot	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	4480c81f 	sdot	z31\.s, z0\.h, z0\.h\[0\]
[^:]+:	4480cbe0 	sdot	z0\.s, z31\.h, z0\.h\[0\]
[^:]+:	4487c800 	sdot	z0\.s, z0\.h, z7\.h\[0\]
[^:]+:	4498c800 	sdot	z0\.s, z0\.h, z0\.h\[3\]
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	4481c820 	sdot	z0\.s, z1\.h, z1\.h\[0\]
[^:]+:	4400c800 	sdot	z0\.s, z0\.h, z0\.h
[^:]+:	4400c81f 	sdot	z31\.s, z0\.h, z0\.h
[^:]+:	4400cbe0 	sdot	z0\.s, z31\.h, z0\.h
[^:]+:	441fc800 	sdot	z0\.s, z0\.h, z31\.h
[^:]+:	4409cb4e 	sdot	z14\.s, z26\.h, z9\.h
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	4402c820 	sdot	z0\.s, z1\.h, z2\.h
[^:]+:	4480cc00 	udot	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	4480cc00 	udot	z0\.s, z0\.h, z0\.h\[0\]
[^:]+:	4480cc1f 	udot	z31\.s, z0\.h, z0\.h\[0\]
[^:]+:	4480cfe0 	udot	z0\.s, z31\.h, z0\.h\[0\]
[^:]+:	4487cc00 	udot	z0\.s, z0\.h, z7\.h\[0\]
[^:]+:	4498cc00 	udot	z0\.s, z0\.h, z0\.h\[3\]
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	4481cc20 	udot	z0\.s, z1\.h, z1\.h\[0\]
[^:]+:	4400cc00 	udot	z0\.s, z0\.h, z0\.h
[^:]+:	4400cc1f 	udot	z31\.s, z0\.h, z0\.h
[^:]+:	4400cfe0 	udot	z0\.s, z31\.h, z0\.h
[^:]+:	441fcc00 	udot	z0\.s, z0\.h, z31\.h
[^:]+:	4409cf4e 	udot	z14\.s, z26\.h, z9\.h
[^:]+:	0420bc20 	movprfx	z0, z1
[^:]+:	4402cc20 	udot	z0\.s, z1\.h, z2\.h
