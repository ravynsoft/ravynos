#as: -mcpu=metac21
#objdump: -dr
#name: metacpu21ext

.*: +file format .*

Disassembly of section \.text:

00000000 <.text>:
.*:	60004640 	          MULD      D0Re0,D0Ar6,D0Ar2
.*:	c7c041ad 	          MOVL      RABZ,\[D0Ar6\+\+\]
.*:	c7c841cd 	          MOVL      RAWZ,\[D1Ar5\+\+\]
.*:	c7d081ad 	          MOVL      RADZ,\[D0Ar4\+\+\]
.*:	c7e081cd 	          MOVL      RABX,\[D1Ar3\+\+\]
.*:	c7e8c1ad 	          MOVL      RAWX,\[D0Ar2\+\+\]
.*:	c7f0c1cd 	          MOVL      RADX,\[D1Ar1\+\+\]
.*:	c7f901ad 	          MOVL      RAMX,\[D0FrT\+\+\]
.*:	c7f881ed 	          MOVL      RAMX,\[A0\.2\+\+\]
