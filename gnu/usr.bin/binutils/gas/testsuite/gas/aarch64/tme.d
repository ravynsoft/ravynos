#as: -march=armv8-a+tme
#objdump: -dr

.* file format .*

Disassembly of section .*:

.* <.*>:
.*:	d5233060 	tstart	x0
.*:	d5233060 	tstart	x0
.*:	d523306f 	tstart	x15
.*:	d523306f 	tstart	x15
.*:	d523307e 	tstart	x30
.*:	d523307e 	tstart	x30
.*:	d503307f 	tcommit
.*:	d503307f 	tcommit
.*:	d5233160 	ttest	x0
.*:	d523317e 	ttest	x30
.*:	d4600000 	tcancel	#0
.*:	d47fffe0 	tcancel	#65535
.*:	d47fffe0 	tcancel	#65535
.*:	d4600140 	tcancel	#10
