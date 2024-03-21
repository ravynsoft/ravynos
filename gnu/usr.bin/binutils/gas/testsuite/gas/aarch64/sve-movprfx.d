#as: -march=armv8-a+sve -I$srcdir/$subdir -W
#objdump: -dr -M no-notes

.* file format .*

Disassembly of section .*:

0+ <.*>:
[^:]+:	0420bc00 	movprfx	z0, z0
[^:]+:	0420bc00 	movprfx	z0, z0
[^:]+:	0420bc01 	movprfx	z1, z0
[^:]+:	0420bc01 	movprfx	z1, z0
[^:]+:	0420bc1f 	movprfx	z31, z0
[^:]+:	0420bc1f 	movprfx	z31, z0
[^:]+:	0420bc40 	movprfx	z0, z2
[^:]+:	0420bc40 	movprfx	z0, z2
[^:]+:	0420bfe0 	movprfx	z0, z31
[^:]+:	0420bfe0 	movprfx	z0, z31
[^:]+:	04102000 	movprfx	z0.b, p0/z, z0.b
[^:]+:	04102000 	movprfx	z0.b, p0/z, z0.b
[^:]+:	04102001 	movprfx	z1.b, p0/z, z0.b
[^:]+:	04102001 	movprfx	z1.b, p0/z, z0.b
[^:]+:	0410201f 	movprfx	z31.b, p0/z, z0.b
[^:]+:	0410201f 	movprfx	z31.b, p0/z, z0.b
[^:]+:	04102800 	movprfx	z0.b, p2/z, z0.b
[^:]+:	04102800 	movprfx	z0.b, p2/z, z0.b
[^:]+:	04103c00 	movprfx	z0.b, p7/z, z0.b
[^:]+:	04103c00 	movprfx	z0.b, p7/z, z0.b
[^:]+:	04102060 	movprfx	z0.b, p0/z, z3.b
[^:]+:	04102060 	movprfx	z0.b, p0/z, z3.b
[^:]+:	041023e0 	movprfx	z0.b, p0/z, z31.b
[^:]+:	041023e0 	movprfx	z0.b, p0/z, z31.b
[^:]+:	04112000 	movprfx	z0.b, p0/m, z0.b
[^:]+:	04112000 	movprfx	z0.b, p0/m, z0.b
[^:]+:	04112001 	movprfx	z1.b, p0/m, z0.b
[^:]+:	04112001 	movprfx	z1.b, p0/m, z0.b
[^:]+:	0411201f 	movprfx	z31.b, p0/m, z0.b
[^:]+:	0411201f 	movprfx	z31.b, p0/m, z0.b
[^:]+:	04112800 	movprfx	z0.b, p2/m, z0.b
[^:]+:	04112800 	movprfx	z0.b, p2/m, z0.b
[^:]+:	04113c00 	movprfx	z0.b, p7/m, z0.b
[^:]+:	04113c00 	movprfx	z0.b, p7/m, z0.b
[^:]+:	04112060 	movprfx	z0.b, p0/m, z3.b
[^:]+:	04112060 	movprfx	z0.b, p0/m, z3.b
[^:]+:	041123e0 	movprfx	z0.b, p0/m, z31.b
[^:]+:	041123e0 	movprfx	z0.b, p0/m, z31.b
[^:]+:	04502000 	movprfx	z0.h, p0/z, z0.h
[^:]+:	04502000 	movprfx	z0.h, p0/z, z0.h
[^:]+:	04502001 	movprfx	z1.h, p0/z, z0.h
[^:]+:	04502001 	movprfx	z1.h, p0/z, z0.h
[^:]+:	0450201f 	movprfx	z31.h, p0/z, z0.h
[^:]+:	0450201f 	movprfx	z31.h, p0/z, z0.h
[^:]+:	04502800 	movprfx	z0.h, p2/z, z0.h
[^:]+:	04502800 	movprfx	z0.h, p2/z, z0.h
[^:]+:	04503c00 	movprfx	z0.h, p7/z, z0.h
[^:]+:	04503c00 	movprfx	z0.h, p7/z, z0.h
[^:]+:	04502060 	movprfx	z0.h, p0/z, z3.h
[^:]+:	04502060 	movprfx	z0.h, p0/z, z3.h
[^:]+:	045023e0 	movprfx	z0.h, p0/z, z31.h
[^:]+:	045023e0 	movprfx	z0.h, p0/z, z31.h
[^:]+:	04512000 	movprfx	z0.h, p0/m, z0.h
[^:]+:	04512000 	movprfx	z0.h, p0/m, z0.h
[^:]+:	04512001 	movprfx	z1.h, p0/m, z0.h
[^:]+:	04512001 	movprfx	z1.h, p0/m, z0.h
[^:]+:	0451201f 	movprfx	z31.h, p0/m, z0.h
[^:]+:	0451201f 	movprfx	z31.h, p0/m, z0.h
[^:]+:	04512800 	movprfx	z0.h, p2/m, z0.h
[^:]+:	04512800 	movprfx	z0.h, p2/m, z0.h
[^:]+:	04513c00 	movprfx	z0.h, p7/m, z0.h
[^:]+:	04513c00 	movprfx	z0.h, p7/m, z0.h
[^:]+:	04512060 	movprfx	z0.h, p0/m, z3.h
[^:]+:	04512060 	movprfx	z0.h, p0/m, z3.h
[^:]+:	045123e0 	movprfx	z0.h, p0/m, z31.h
[^:]+:	045123e0 	movprfx	z0.h, p0/m, z31.h
[^:]+:	04902000 	movprfx	z0.s, p0/z, z0.s
[^:]+:	04902000 	movprfx	z0.s, p0/z, z0.s
[^:]+:	04902001 	movprfx	z1.s, p0/z, z0.s
[^:]+:	04902001 	movprfx	z1.s, p0/z, z0.s
[^:]+:	0490201f 	movprfx	z31.s, p0/z, z0.s
[^:]+:	0490201f 	movprfx	z31.s, p0/z, z0.s
[^:]+:	04902800 	movprfx	z0.s, p2/z, z0.s
[^:]+:	04902800 	movprfx	z0.s, p2/z, z0.s
[^:]+:	04903c00 	movprfx	z0.s, p7/z, z0.s
[^:]+:	04903c00 	movprfx	z0.s, p7/z, z0.s
[^:]+:	04902060 	movprfx	z0.s, p0/z, z3.s
[^:]+:	04902060 	movprfx	z0.s, p0/z, z3.s
[^:]+:	049023e0 	movprfx	z0.s, p0/z, z31.s
[^:]+:	049023e0 	movprfx	z0.s, p0/z, z31.s
[^:]+:	04912000 	movprfx	z0.s, p0/m, z0.s
[^:]+:	04912000 	movprfx	z0.s, p0/m, z0.s
[^:]+:	04912001 	movprfx	z1.s, p0/m, z0.s
[^:]+:	04912001 	movprfx	z1.s, p0/m, z0.s
[^:]+:	0491201f 	movprfx	z31.s, p0/m, z0.s
[^:]+:	0491201f 	movprfx	z31.s, p0/m, z0.s
[^:]+:	04912800 	movprfx	z0.s, p2/m, z0.s
[^:]+:	04912800 	movprfx	z0.s, p2/m, z0.s
[^:]+:	04913c00 	movprfx	z0.s, p7/m, z0.s
[^:]+:	04913c00 	movprfx	z0.s, p7/m, z0.s
[^:]+:	04912060 	movprfx	z0.s, p0/m, z3.s
[^:]+:	04912060 	movprfx	z0.s, p0/m, z3.s
[^:]+:	049123e0 	movprfx	z0.s, p0/m, z31.s
[^:]+:	049123e0 	movprfx	z0.s, p0/m, z31.s
[^:]+:	04d02000 	movprfx	z0.d, p0/z, z0.d
[^:]+:	04d02000 	movprfx	z0.d, p0/z, z0.d
[^:]+:	04d02001 	movprfx	z1.d, p0/z, z0.d
[^:]+:	04d02001 	movprfx	z1.d, p0/z, z0.d
[^:]+:	04d0201f 	movprfx	z31.d, p0/z, z0.d
[^:]+:	04d0201f 	movprfx	z31.d, p0/z, z0.d
[^:]+:	04d02800 	movprfx	z0.d, p2/z, z0.d
[^:]+:	04d02800 	movprfx	z0.d, p2/z, z0.d
[^:]+:	04d03c00 	movprfx	z0.d, p7/z, z0.d
[^:]+:	04d03c00 	movprfx	z0.d, p7/z, z0.d
[^:]+:	04d02060 	movprfx	z0.d, p0/z, z3.d
[^:]+:	04d02060 	movprfx	z0.d, p0/z, z3.d
[^:]+:	04d023e0 	movprfx	z0.d, p0/z, z31.d
[^:]+:	04d023e0 	movprfx	z0.d, p0/z, z31.d
[^:]+:	04d12000 	movprfx	z0.d, p0/m, z0.d
[^:]+:	04d12000 	movprfx	z0.d, p0/m, z0.d
[^:]+:	04d12001 	movprfx	z1.d, p0/m, z0.d
[^:]+:	04d12001 	movprfx	z1.d, p0/m, z0.d
[^:]+:	04d1201f 	movprfx	z31.d, p0/m, z0.d
[^:]+:	04d1201f 	movprfx	z31.d, p0/m, z0.d
[^:]+:	04d12800 	movprfx	z0.d, p2/m, z0.d
[^:]+:	04d12800 	movprfx	z0.d, p2/m, z0.d
[^:]+:	04d13c00 	movprfx	z0.d, p7/m, z0.d
[^:]+:	04d13c00 	movprfx	z0.d, p7/m, z0.d
[^:]+:	04d12060 	movprfx	z0.d, p0/m, z3.d
[^:]+:	04d12060 	movprfx	z0.d, p0/m, z3.d
[^:]+:	04d123e0 	movprfx	z0.d, p0/m, z31.d
[^:]+:	04d123e0 	movprfx	z0.d, p0/m, z31.d
