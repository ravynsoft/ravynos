#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	64602400 	fclamp	z0\.h, z0\.h, z0\.h
[^:]+:	6460241f 	fclamp	z31\.h, z0\.h, z0\.h
[^:]+:	646027e0 	fclamp	z0\.h, z31\.h, z0\.h
[^:]+:	647f2400 	fclamp	z0\.h, z0\.h, z31\.h
[^:]+:	647526c9 	fclamp	z9\.h, z22\.h, z21\.h
[^:]+:	64a02400 	fclamp	z0\.s, z0\.s, z0\.s
[^:]+:	64a0241f 	fclamp	z31\.s, z0\.s, z0\.s
[^:]+:	64a027e0 	fclamp	z0\.s, z31\.s, z0\.s
[^:]+:	64bf2400 	fclamp	z0\.s, z0\.s, z31\.s
[^:]+:	64a124b9 	fclamp	z25\.s, z5\.s, z1\.s
[^:]+:	64e02400 	fclamp	z0\.d, z0\.d, z0\.d
[^:]+:	64e0241f 	fclamp	z31\.d, z0\.d, z0\.d
[^:]+:	64e027e0 	fclamp	z0\.d, z31\.d, z0\.d
[^:]+:	64ff2400 	fclamp	z0\.d, z0\.d, z31\.d
[^:]+:	64fc27c3 	fclamp	z3\.d, z30\.d, z28\.d
