#name: s390x opcode
#objdump: -drw

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	b9 c8 80 67 [	 ]*ahhhr	%r6,%r7,%r8
.*:	b9 d8 80 67 [	 ]*ahhlr	%r6,%r7,%r8
.*:	cc 68 ff ff 02 18 [	 ]*aih	%r6,-65000
.*:	b9 ca 80 67 [	 ]*alhhhr	%r6,%r7,%r8
.*:	b9 da 80 67 [	 ]*alhhlr	%r6,%r7,%r8
.*:	cc 6a 00 00 fd e8 [	 ]*alsih	%r6,65000
.*:	cc 6b 00 00 fd e8 [	 ]*alsihn	%r6,65000
.*:	cc 66 00 00 00 00 [	 ]*brcth	%r6,22 <foo\+0x22>
.*:	b9 cd 00 67 [	 ]*chhr	%r6,%r7
.*:	b9 dd 00 67 [	 ]*chlr	%r6,%r7
.*:	e3 67 85 b3 01 cd [	 ]*chf	%r6,5555\(%r7,%r8\)
.*:	cc 6d 00 00 fd e8 [	 ]*cih	%r6,65000
.*:	b9 cf 00 67 [	 ]*clhhr	%r6,%r7
.*:	b9 df 00 67 [	 ]*clhlr	%r6,%r7
.*:	e3 67 85 b3 01 cf [	 ]*clhf	%r6,5555\(%r7,%r8\)
.*:	cc 6f 00 09 eb 10 [	 ]*clih	%r6,650000
.*:	cc 9f ee 6b 28 00 [	 ]*clih	%r9,4000000000
.*:	e3 67 8a 4d fe c0 [	 ]*lbh	%r6,-5555\(%r7,%r8\)
.*:	e3 67 8a 4d fe c4 [	 ]*lhh	%r6,-5555\(%r7,%r8\)
.*:	e3 67 8a 4d fe ca [	 ]*lfh	%r6,-5555\(%r7,%r8\)
.*:	e3 67 8a 4d fe c2 [	 ]*llch	%r6,-5555\(%r7,%r8\)
.*:	e3 67 8a 4d fe c6 [	 ]*llhh	%r6,-5555\(%r7,%r8\)
.*:	ec 67 0c 0d 0e 5d [	 ]*risbhg	%r6,%r7,12,13,14
.*:	ec 67 0c 0d 0e 51 [	 ]*risblg	%r6,%r7,12,13,14
.*:	e3 67 8a 4d fe c3 [	 ]*stch	%r6,-5555\(%r7,%r8\)
.*:	e3 67 8a 4d fe c7 [	 ]*sthh	%r6,-5555\(%r7,%r8\)
.*:	e3 67 8a 4d fe cb [	 ]*stfh	%r6,-5555\(%r7,%r8\)
.*:	b9 c9 80 67 [	 ]*shhhr	%r6,%r7,%r8
.*:	b9 d9 80 67 [	 ]*shhlr	%r6,%r7,%r8
.*:	b9 cb 80 67 [	 ]*slhhhr	%r6,%r7,%r8
.*:	b9 db 80 67 [	 ]*slhhlr	%r6,%r7,%r8
.*:	eb 67 8a 4d fe f8 [	 ]*laa	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe e8 [	 ]*laag	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe fa [	 ]*laal	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe ea [	 ]*laalg	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe f4 [	 ]*lan	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe e4 [	 ]*lang	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe f7 [	 ]*lax	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe e7 [	 ]*laxg	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe f6 [	 ]*lao	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe e6 [	 ]*laog	%r6,%r7,-5555\(%r8\)
.*:	c8 64 78 ae 84 57 [	 ]*lpd	%r6,2222\(%r7\),1111\(%r8\)
.*:	c8 65 78 ae 84 57 [	 ]*lpdg	%r6,2222\(%r7\),1111\(%r8\)
.*:	b9 f2 10 67 [	 ]*locro	%r6,%r7
.*:	b9 f2 20 67 [	 ]*locrh	%r6,%r7
.*:	b9 f2 20 67 [	 ]*locrh	%r6,%r7
.*:	b9 f2 30 67 [	 ]*locrnle	%r6,%r7
.*:	b9 f2 40 67 [	 ]*locrl	%r6,%r7
.*:	b9 f2 40 67 [	 ]*locrl	%r6,%r7
.*:	b9 f2 50 67 [	 ]*locrnhe	%r6,%r7
.*:	b9 f2 60 67 [	 ]*locrlh	%r6,%r7
.*:	b9 f2 70 67 [	 ]*locrne	%r6,%r7
.*:	b9 f2 70 67 [	 ]*locrne	%r6,%r7
.*:	b9 f2 80 67 [	 ]*locre	%r6,%r7
.*:	b9 f2 80 67 [	 ]*locre	%r6,%r7
.*:	b9 f2 90 67 [	 ]*locrnlh	%r6,%r7
.*:	b9 f2 a0 67 [	 ]*locrhe	%r6,%r7
.*:	b9 f2 b0 67 [	 ]*locrnl	%r6,%r7
.*:	b9 f2 b0 67 [	 ]*locrnl	%r6,%r7
.*:	b9 f2 c0 67 [	 ]*locrle	%r6,%r7
.*:	b9 f2 d0 67 [	 ]*locrnh	%r6,%r7
.*:	b9 f2 d0 67 [	 ]*locrnh	%r6,%r7
.*:	b9 f2 e0 67 [	 ]*locrno	%r6,%r7
.*:	b9 f2 80 67 [	 ]*locre	%r6,%r7
.*:	b9 e2 10 67 [	 ]*locgro	%r6,%r7
.*:	b9 e2 20 67 [	 ]*locgrh	%r6,%r7
.*:	b9 e2 20 67 [	 ]*locgrh	%r6,%r7
.*:	b9 e2 30 67 [	 ]*locgrnle	%r6,%r7
.*:	b9 e2 40 67 [	 ]*locgrl	%r6,%r7
.*:	b9 e2 40 67 [	 ]*locgrl	%r6,%r7
.*:	b9 e2 50 67 [	 ]*locgrnhe	%r6,%r7
.*:	b9 e2 60 67 [	 ]*locgrlh	%r6,%r7
.*:	b9 e2 70 67 [	 ]*locgrne	%r6,%r7
.*:	b9 e2 70 67 [	 ]*locgrne	%r6,%r7
.*:	b9 e2 80 67 [	 ]*locgre	%r6,%r7
.*:	b9 e2 80 67 [	 ]*locgre	%r6,%r7
.*:	b9 e2 90 67 [	 ]*locgrnlh	%r6,%r7
.*:	b9 e2 a0 67 [	 ]*locgrhe	%r6,%r7
.*:	b9 e2 b0 67 [	 ]*locgrnl	%r6,%r7
.*:	b9 e2 b0 67 [	 ]*locgrnl	%r6,%r7
.*:	b9 e2 c0 67 [	 ]*locgrle	%r6,%r7
.*:	b9 e2 d0 67 [	 ]*locgrnh	%r6,%r7
.*:	b9 e2 d0 67 [	 ]*locgrnh	%r6,%r7
.*:	b9 e2 e0 67 [	 ]*locgrno	%r6,%r7
.*:	b9 e2 80 67 [	 ]*locgre	%r6,%r7
.*:	eb 61 7a 4d fe f2 [	 ]*loco	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe f2 [	 ]*loch	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe f2 [	 ]*loch	%r6,-5555\(%r7\)
.*:	eb 63 7a 4d fe f2 [	 ]*locnle	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe f2 [	 ]*locl	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe f2 [	 ]*locl	%r6,-5555\(%r7\)
.*:	eb 65 7a 4d fe f2 [	 ]*locnhe	%r6,-5555\(%r7\)
.*:	eb 66 7a 4d fe f2 [	 ]*loclh	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe f2 [	 ]*locne	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe f2 [	 ]*locne	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe f2 [	 ]*loce	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe f2 [	 ]*loce	%r6,-5555\(%r7\)
.*:	eb 69 7a 4d fe f2 [	 ]*locnlh	%r6,-5555\(%r7\)
.*:	eb 6a 7a 4d fe f2 [	 ]*loche	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe f2 [	 ]*locnl	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe f2 [	 ]*locnl	%r6,-5555\(%r7\)
.*:	eb 6c 7a 4d fe f2 [	 ]*locle	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe f2 [	 ]*locnh	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe f2 [	 ]*locnh	%r6,-5555\(%r7\)
.*:	eb 6e 7a 4d fe f2 [	 ]*locno	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe f2 [	 ]*loce	%r6,-5555\(%r7\)
.*:	eb 61 7a 4d fe e2 [	 ]*locgo	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe e2 [	 ]*locgh	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe e2 [	 ]*locgh	%r6,-5555\(%r7\)
.*:	eb 63 7a 4d fe e2 [	 ]*locgnle	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe e2 [	 ]*locgl	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe e2 [	 ]*locgl	%r6,-5555\(%r7\)
.*:	eb 65 7a 4d fe e2 [	 ]*locgnhe	%r6,-5555\(%r7\)
.*:	eb 66 7a 4d fe e2 [	 ]*locglh	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe e2 [	 ]*locgne	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe e2 [	 ]*locgne	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe e2 [	 ]*locge	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe e2 [	 ]*locge	%r6,-5555\(%r7\)
.*:	eb 69 7a 4d fe e2 [	 ]*locgnlh	%r6,-5555\(%r7\)
.*:	eb 6a 7a 4d fe e2 [	 ]*locghe	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe e2 [	 ]*locgnl	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe e2 [	 ]*locgnl	%r6,-5555\(%r7\)
.*:	eb 6c 7a 4d fe e2 [	 ]*locgle	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe e2 [	 ]*locgnh	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe e2 [	 ]*locgnh	%r6,-5555\(%r7\)
.*:	eb 6e 7a 4d fe e2 [	 ]*locgno	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe e2 [	 ]*locge	%r6,-5555\(%r7\)
.*:	eb 61 7a 4d fe f3 [	 ]*stoco	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe f3 [	 ]*stoch	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe f3 [	 ]*stoch	%r6,-5555\(%r7\)
.*:	eb 63 7a 4d fe f3 [	 ]*stocnle	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe f3 [	 ]*stocl	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe f3 [	 ]*stocl	%r6,-5555\(%r7\)
.*:	eb 65 7a 4d fe f3 [	 ]*stocnhe	%r6,-5555\(%r7\)
.*:	eb 66 7a 4d fe f3 [	 ]*stoclh	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe f3 [	 ]*stocne	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe f3 [	 ]*stocne	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe f3 [	 ]*stoce	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe f3 [	 ]*stoce	%r6,-5555\(%r7\)
.*:	eb 69 7a 4d fe f3 [	 ]*stocnlh	%r6,-5555\(%r7\)
.*:	eb 6a 7a 4d fe f3 [	 ]*stoche	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe f3 [	 ]*stocnl	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe f3 [	 ]*stocnl	%r6,-5555\(%r7\)
.*:	eb 6c 7a 4d fe f3 [	 ]*stocle	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe f3 [	 ]*stocnh	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe f3 [	 ]*stocnh	%r6,-5555\(%r7\)
.*:	eb 6e 7a 4d fe f3 [	 ]*stocno	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe f3 [	 ]*stoce	%r6,-5555\(%r7\)
.*:	eb 61 7a 4d fe e3 [	 ]*stocgo	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe e3 [	 ]*stocgh	%r6,-5555\(%r7\)
.*:	eb 62 7a 4d fe e3 [	 ]*stocgh	%r6,-5555\(%r7\)
.*:	eb 63 7a 4d fe e3 [	 ]*stocgnle	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe e3 [	 ]*stocgl	%r6,-5555\(%r7\)
.*:	eb 64 7a 4d fe e3 [	 ]*stocgl	%r6,-5555\(%r7\)
.*:	eb 65 7a 4d fe e3 [	 ]*stocgnhe	%r6,-5555\(%r7\)
.*:	eb 66 7a 4d fe e3 [	 ]*stocglh	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe e3 [	 ]*stocgne	%r6,-5555\(%r7\)
.*:	eb 67 7a 4d fe e3 [	 ]*stocgne	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe e3 [	 ]*stocge	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe e3 [	 ]*stocge	%r6,-5555\(%r7\)
.*:	eb 69 7a 4d fe e3 [	 ]*stocgnlh	%r6,-5555\(%r7\)
.*:	eb 6a 7a 4d fe e3 [	 ]*stocghe	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe e3 [	 ]*stocgnl	%r6,-5555\(%r7\)
.*:	eb 6b 7a 4d fe e3 [	 ]*stocgnl	%r6,-5555\(%r7\)
.*:	eb 6c 7a 4d fe e3 [	 ]*stocgle	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe e3 [	 ]*stocgnh	%r6,-5555\(%r7\)
.*:	eb 6d 7a 4d fe e3 [	 ]*stocgnh	%r6,-5555\(%r7\)
.*:	eb 6e 7a 4d fe e3 [	 ]*stocgno	%r6,-5555\(%r7\)
.*:	eb 68 7a 4d fe e3 [	 ]*stocge	%r6,-5555\(%r7\)
.*:	b9 f8 80 67 [	 ]*ark	%r6,%r7,%r8
.*:	b9 e8 80 67 [	 ]*agrk	%r6,%r7,%r8
.*:	ec 67 83 00 00 d8 [	 ]*ahik	%r6,%r7,-32000
.*:	ec 67 83 00 00 d9 [	 ]*aghik	%r6,%r7,-32000
.*:	b9 fa 80 67 [	 ]*alrk	%r6,%r7,%r8
.*:	b9 ea 80 67 [	 ]*algrk	%r6,%r7,%r8
.*:	ec 67 83 00 00 da [	 ]*alhsik	%r6,%r7,-32000
.*:	ec 67 83 00 00 db [	 ]*alghsik	%r6,%r7,-32000
.*:	b9 f4 80 67 [	 ]*nrk	%r6,%r7,%r8
.*:	b9 e4 80 67 [	 ]*ngrk	%r6,%r7,%r8
.*:	b9 f7 80 67 [	 ]*xrk	%r6,%r7,%r8
.*:	b9 e7 80 67 [	 ]*xgrk	%r6,%r7,%r8
.*:	b9 f6 80 67 [	 ]*ork	%r6,%r7,%r8
.*:	b9 e6 80 67 [	 ]*ogrk	%r6,%r7,%r8
.*:	eb 67 8a 4d fe dd [	 ]*slak	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe df [	 ]*sllk	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe dc [	 ]*srak	%r6,%r7,-5555\(%r8\)
.*:	eb 67 8a 4d fe de [	 ]*srlk	%r6,%r7,-5555\(%r8\)
.*:	b9 f9 80 67 [	 ]*srk	%r6,%r7,%r8
.*:	b9 e9 80 67 [	 ]*sgrk	%r6,%r7,%r8
.*:	b9 fb 80 67 [	 ]*slrk	%r6,%r7,%r8
.*:	b9 eb 80 67 [	 ]*slgrk	%r6,%r7,%r8
.*:	b9 e1 00 67 [	 ]*popcnt	%r6,%r7
.*:	b9 ae 00 67 [	 ]*rrbm	%r6,%r7
.*:	b3 94 37 59 [	 ]*cefbra	%f5,3,%r9,7
.*:	b3 95 37 59 [	 ]*cdfbra	%f5,3,%r9,7
.*:	b3 96 37 59 [	 ]*cxfbra	%f5,3,%r9,7
.*:	b3 a4 37 59 [	 ]*cegbra	%f5,3,%r9,7
.*:	b3 a5 37 59 [	 ]*cdgbra	%f5,3,%r9,7
.*:	b3 a6 37 59 [	 ]*cxgbra	%f5,3,%r9,7
.*:	b3 90 37 59 [	 ]*celfbr	%f5,3,%r9,7
.*:	b3 91 37 59 [	 ]*cdlfbr	%f5,3,%r9,7
.*:	b3 92 37 59 [	 ]*cxlfbr	%f5,3,%r9,7
.*:	b3 a0 37 59 [	 ]*celgbr	%f5,3,%r9,7
.*:	b3 a1 37 59 [	 ]*cdlgbr	%f5,3,%r9,7
.*:	b3 a2 37 59 [	 ]*cxlgbr	%f5,3,%r9,7
.*:	b3 98 37 59 [	 ]*cfebra	%r5,3,%f9,7
.*:	b3 99 37 59 [	 ]*cfdbra	%r5,3,%f9,7
.*:	b3 9a 37 58 [	 ]*cfxbra	%r5,3,%f8,7
.*:	b3 a8 37 59 [	 ]*cgebra	%r5,3,%f9,7
.*:	b3 a9 37 59 [	 ]*cgdbra	%r5,3,%f9,7
.*:	b3 aa 37 58 [	 ]*cgxbra	%r5,3,%f8,7
.*:	b3 9c 37 59 [	 ]*clfebr	%r5,3,%f9,7
.*:	b3 9d 37 59 [	 ]*clfdbr	%r5,3,%f9,7
.*:	b3 9e 37 58 [	 ]*clfxbr	%r5,3,%f8,7
.*:	b3 ac 37 59 [	 ]*clgebr	%r5,3,%f9,7
.*:	b3 ad 37 59 [	 ]*clgdbr	%r5,3,%f9,7
.*:	b3 ae 37 58 [	 ]*clgxbr	%r5,3,%f8,7
.*:	b3 57 37 59 [	 ]*fiebra	%f5,3,%f9,7
.*:	b3 5f 37 59 [	 ]*fidbra	%f5,3,%f9,7
.*:	b3 47 37 58 [	 ]*fixbra	%f5,3,%f8,7
.*:	b3 44 37 59 [	 ]*ledbra	%f5,3,%f9,7
.*:	b3 45 37 58 [	 ]*ldxbra	%f5,3,%f8,7
.*:	b3 46 37 58 [	 ]*lexbra	%f5,3,%f8,7
.*:	b3 d2 97 35 [	 ]*adtra	%f3,%f5,%f9,7
.*:	b3 da 57 14 [	 ]*axtra	%f1,%f4,%f5,7
.*:	b3 f1 37 59 [	 ]*cdgtra	%f5,3,%r9,7
.*:	b9 51 37 59 [	 ]*cdftr	%f5,3,%r9,7
.*:	b9 59 37 59 [	 ]*cxftr	%f5,3,%r9,7
.*:	b3 f9 37 59 [	 ]*cxgtra	%f5,3,%r9,7
.*:	b9 52 37 59 [	 ]*cdlgtr	%f5,3,%r9,7
.*:	b9 5a 37 59 [	 ]*cxlgtr	%f5,3,%r9,7
.*:	b9 53 37 59 [	 ]*cdlftr	%f5,3,%r9,7
.*:	b9 5b 37 59 [	 ]*cxlftr	%f5,3,%r9,7
.*:	b3 e1 37 59 [	 ]*cgdtra	%r5,3,%f9,7
.*:	b3 e9 37 58 [	 ]*cgxtra	%r5,3,%f8,7
.*:	b9 41 37 59 [	 ]*cfdtr	%r5,3,%f9,7
.*:	b9 49 37 59 [	 ]*cfxtr	%r5,3,%f9,7
.*:	b9 42 37 59 [	 ]*clgdtr	%r5,3,%f9,7
.*:	b9 4a 37 58 [	 ]*clgxtr	%r5,3,%f8,7
.*:	b9 43 37 59 [	 ]*clfdtr	%r5,3,%f9,7
.*:	b9 4b 37 58 [	 ]*clfxtr	%r5,3,%f8,7
.*:	b3 d1 97 35 [	 ]*ddtra	%f3,%f5,%f9,7
.*:	b3 d9 57 14 [	 ]*dxtra	%f1,%f4,%f5,7
.*:	b3 d0 97 35 [	 ]*mdtra	%f3,%f5,%f9,7
.*:	b3 d8 57 14 [	 ]*mxtra	%f1,%f4,%f5,7
.*:	b3 d3 97 35 [	 ]*sdtra	%f3,%f5,%f9,7
.*:	b3 db 57 14 [	 ]*sxtra	%f1,%f4,%f5,7
.*:	b2 b8 7f a0 [	 ]*srnmb	4000\(%r7\)
.*:	b9 2a 00 56 [	 ]*kmf	%r5,%r6
.*:	b9 2b 00 56 [	 ]*kmo	%r5,%r6
.*:	b9 2c 00 00 [	 ]*pcc
.*:	b9 2d 60 59 [	 ]*kmctr	%r5,%r6,%r9
.*:	b9 28 00 00 [	 ]*pckmo
.*:	07 07 [	 ]*nopr	%r7
