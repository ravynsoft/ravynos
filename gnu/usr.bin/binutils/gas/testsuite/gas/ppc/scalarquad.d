#as: -mpower10
#objdump: -dr -Mpower10
#name: scalar min/max/compare quad precision

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(fc 01 10 88|88 10 01 fc) 	xscmpeqqp v0,v1,v2
.*:	(fc 64 29 88|88 29 64 fc) 	xscmpgeqp v3,v4,v5
.*:	(fc c7 41 c8|c8 41 c7 fc) 	xscmpgtqp v6,v7,v8
.*:	(fd 2a 5d 48|48 5d 2a fd) 	xsmaxcqp v9,v10,v11
.*:	(fd 8d 75 c8|c8 75 8d fd) 	xsmincqp v12,v13,v14
