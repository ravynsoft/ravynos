#name: s390x opcode
#objdump: -drw

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	eb d6 65 b3 01 6a [	 ]*asi	5555\(%r6\),-42
.*:	eb d6 65 b3 01 7a [	 ]*agsi	5555\(%r6\),-42
.*:	eb d6 65 b3 01 6e [	 ]*alsi	5555\(%r6\),-42
.*:	eb d6 65 b3 01 7e [	 ]*algsi	5555\(%r6\),-42
.*:	c6 6d 00 00 00 00 [	 ]*crl	%r6,18 <foo\+0x18>
.*:	c6 68 00 00 00 00 [	 ]*cgrl	%r6,1e <foo\+0x1e>
.*:	c6 6c 00 00 00 00 [	 ]*cgfrl	%r6,24 <foo\+0x24>
.*:	ec 67 84 57 a0 f6 [	 ]*crbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 f6 [	 ]*crbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 f6 [	 ]*crbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 f6 [	 ]*crbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 f6 [	 ]*crbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 f6 [	 ]*crbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 f6 [	 ]*crbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 f6 [	 ]*crbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 f6 [	 ]*crbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 f6 [	 ]*crbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 f6 [	 ]*crbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 f6 [	 ]*crbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 f6 [	 ]*crbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 e4 [	 ]*cgrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 e4 [	 ]*cgrbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 e4 [	 ]*cgrbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 e4 [	 ]*cgrbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 e4 [	 ]*cgrbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 e4 [	 ]*cgrbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 e4 [	 ]*cgrbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 e4 [	 ]*cgrbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 e4 [	 ]*cgrbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 e4 [	 ]*cgrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 e4 [	 ]*cgrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 e4 [	 ]*cgrbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 e4 [	 ]*cgrbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 00 00 a0 76 [	 ]*crjnl	%r6,%r7,c6 <foo\+0xc6>
.*:	ec 67 00 00 20 76 [	 ]*crjh	%r6,%r7,cc <foo\+0xcc>
.*:	ec 67 00 00 20 76 [	 ]*crjh	%r6,%r7,d2 <foo\+0xd2>
.*:	ec 67 00 00 40 76 [	 ]*crjl	%r6,%r7,d8 <foo\+0xd8>
.*:	ec 67 00 00 40 76 [	 ]*crjl	%r6,%r7,de <foo\+0xde>
.*:	ec 67 00 00 60 76 [	 ]*crjne	%r6,%r7,e4 <foo\+0xe4>
.*:	ec 67 00 00 60 76 [	 ]*crjne	%r6,%r7,ea <foo\+0xea>
.*:	ec 67 00 00 80 76 [	 ]*crje	%r6,%r7,f0 <foo\+0xf0>
.*:	ec 67 00 00 80 76 [	 ]*crje	%r6,%r7,f6 <foo\+0xf6>
.*:	ec 67 00 00 a0 76 [	 ]*crjnl	%r6,%r7,fc <foo\+0xfc>
.*:	ec 67 00 00 a0 76 [	 ]*crjnl	%r6,%r7,102 <foo\+0x102>
.*:	ec 67 00 00 c0 76 [	 ]*crjnh	%r6,%r7,108 <foo\+0x108>
.*:	ec 67 00 00 c0 76 [	 ]*crjnh	%r6,%r7,10e <foo\+0x10e>
.*:	ec 67 00 00 a0 64 [	 ]*cgrjnl	%r6,%r7,114 <foo\+0x114>
.*:	ec 67 00 00 20 64 [	 ]*cgrjh	%r6,%r7,11a <foo\+0x11a>
.*:	ec 67 00 00 20 64 [	 ]*cgrjh	%r6,%r7,120 <foo\+0x120>
.*:	ec 67 00 00 40 64 [	 ]*cgrjl	%r6,%r7,126 <foo\+0x126>
.*:	ec 67 00 00 40 64 [	 ]*cgrjl	%r6,%r7,12c <foo\+0x12c>
.*:	ec 67 00 00 60 64 [	 ]*cgrjne	%r6,%r7,132 <foo\+0x132>
.*:	ec 67 00 00 60 64 [	 ]*cgrjne	%r6,%r7,138 <foo\+0x138>
.*:	ec 67 00 00 80 64 [	 ]*cgrje	%r6,%r7,13e <foo\+0x13e>
.*:	ec 67 00 00 80 64 [	 ]*cgrje	%r6,%r7,144 <foo\+0x144>
.*:	ec 67 00 00 a0 64 [	 ]*cgrjnl	%r6,%r7,14a <foo\+0x14a>
.*:	ec 67 00 00 a0 64 [	 ]*cgrjnl	%r6,%r7,150 <foo\+0x150>
.*:	ec 67 00 00 c0 64 [	 ]*cgrjnh	%r6,%r7,156 <foo\+0x156>
.*:	ec 67 00 00 c0 64 [	 ]*cgrjnh	%r6,%r7,15c <foo\+0x15c>
.*:	ec 6a 74 57 d6 fe [	 ]*cibnl	%r6,-42,1111\(%r7\)
.*:	ec 62 74 57 d6 fe [	 ]*cibh	%r6,-42,1111\(%r7\)
.*:	ec 62 74 57 d6 fe [	 ]*cibh	%r6,-42,1111\(%r7\)
.*:	ec 64 74 57 d6 fe [	 ]*cibl	%r6,-42,1111\(%r7\)
.*:	ec 64 74 57 d6 fe [	 ]*cibl	%r6,-42,1111\(%r7\)
.*:	ec 66 74 57 d6 fe [	 ]*cibne	%r6,-42,1111\(%r7\)
.*:	ec 66 74 57 d6 fe [	 ]*cibne	%r6,-42,1111\(%r7\)
.*:	ec 68 74 57 d6 fe [	 ]*cibe	%r6,-42,1111\(%r7\)
.*:	ec 68 74 57 d6 fe [	 ]*cibe	%r6,-42,1111\(%r7\)
.*:	ec 6a 74 57 d6 fe [	 ]*cibnl	%r6,-42,1111\(%r7\)
.*:	ec 6a 74 57 d6 fe [	 ]*cibnl	%r6,-42,1111\(%r7\)
.*:	ec 6c 74 57 d6 fe [	 ]*cibnh	%r6,-42,1111\(%r7\)
.*:	ec 6c 74 57 d6 fe [	 ]*cibnh	%r6,-42,1111\(%r7\)
.*:	ec 6a 74 57 d6 fc [	 ]*cgibnl	%r6,-42,1111\(%r7\)
.*:	ec 62 74 57 d6 fc [	 ]*cgibh	%r6,-42,1111\(%r7\)
.*:	ec 62 74 57 d6 fc [	 ]*cgibh	%r6,-42,1111\(%r7\)
.*:	ec 64 74 57 d6 fc [	 ]*cgibl	%r6,-42,1111\(%r7\)
.*:	ec 64 74 57 d6 fc [	 ]*cgibl	%r6,-42,1111\(%r7\)
.*:	ec 66 74 57 d6 fc [	 ]*cgibne	%r6,-42,1111\(%r7\)
.*:	ec 66 74 57 d6 fc [	 ]*cgibne	%r6,-42,1111\(%r7\)
.*:	ec 68 74 57 d6 fc [	 ]*cgibe	%r6,-42,1111\(%r7\)
.*:	ec 68 74 57 d6 fc [	 ]*cgibe	%r6,-42,1111\(%r7\)
.*:	ec 6a 74 57 d6 fc [	 ]*cgibnl	%r6,-42,1111\(%r7\)
.*:	ec 6a 74 57 d6 fc [	 ]*cgibnl	%r6,-42,1111\(%r7\)
.*:	ec 6c 74 57 d6 fc [	 ]*cgibnh	%r6,-42,1111\(%r7\)
.*:	ec 6c 74 57 d6 fc [	 ]*cgibnh	%r6,-42,1111\(%r7\)
.*:	ec 6a 00 00 d6 7e [	 ]*cijnl	%r6,-42,1fe <foo\+0x1fe>
.*:	ec 62 00 00 d6 7e [	 ]*cijh	%r6,-42,204 <foo\+0x204>
.*:	ec 62 00 00 d6 7e [	 ]*cijh	%r6,-42,20a <foo\+0x20a>
.*:	ec 64 00 00 d6 7e [	 ]*cijl	%r6,-42,210 <foo\+0x210>
.*:	ec 64 00 00 d6 7e [	 ]*cijl	%r6,-42,216 <foo\+0x216>
.*:	ec 66 00 00 d6 7e [	 ]*cijne	%r6,-42,21c <foo\+0x21c>
.*:	ec 66 00 00 d6 7e [	 ]*cijne	%r6,-42,222 <foo\+0x222>
.*:	ec 68 00 00 d6 7e [	 ]*cije	%r6,-42,228 <foo\+0x228>
.*:	ec 68 00 00 d6 7e [	 ]*cije	%r6,-42,22e <foo\+0x22e>
.*:	ec 6a 00 00 d6 7e [	 ]*cijnl	%r6,-42,234 <foo\+0x234>
.*:	ec 6a 00 00 d6 7e [	 ]*cijnl	%r6,-42,23a <foo\+0x23a>
.*:	ec 6c 00 00 d6 7e [	 ]*cijnh	%r6,-42,240 <foo\+0x240>
.*:	ec 6c 00 00 d6 7e [	 ]*cijnh	%r6,-42,246 <foo\+0x246>
.*:	ec 6a 00 00 d6 7c [	 ]*cgijnl	%r6,-42,24c <foo\+0x24c>
.*:	ec 62 00 00 d6 7c [	 ]*cgijh	%r6,-42,252 <foo\+0x252>
.*:	ec 62 00 00 d6 7c [	 ]*cgijh	%r6,-42,258 <foo\+0x258>
.*:	ec 64 00 00 d6 7c [	 ]*cgijl	%r6,-42,25e <foo\+0x25e>
.*:	ec 64 00 00 d6 7c [	 ]*cgijl	%r6,-42,264 <foo\+0x264>
.*:	ec 66 00 00 d6 7c [	 ]*cgijne	%r6,-42,26a <foo\+0x26a>
.*:	ec 66 00 00 d6 7c [	 ]*cgijne	%r6,-42,270 <foo\+0x270>
.*:	ec 68 00 00 d6 7c [	 ]*cgije	%r6,-42,276 <foo\+0x276>
.*:	ec 68 00 00 d6 7c [	 ]*cgije	%r6,-42,27c <foo\+0x27c>
.*:	ec 6a 00 00 d6 7c [	 ]*cgijnl	%r6,-42,282 <foo\+0x282>
.*:	ec 6a 00 00 d6 7c [	 ]*cgijnl	%r6,-42,288 <foo\+0x288>
.*:	ec 6c 00 00 d6 7c [	 ]*cgijnh	%r6,-42,28e <foo\+0x28e>
.*:	ec 6c 00 00 d6 7c [	 ]*cgijnh	%r6,-42,294 <foo\+0x294>
.*:	b9 72 a0 67 [	 ]*crtnl	%r6,%r7
.*:	b9 72 20 67 [	 ]*crth	%r6,%r7
.*:	b9 72 20 67 [	 ]*crth	%r6,%r7
.*:	b9 72 40 67 [	 ]*crtl	%r6,%r7
.*:	b9 72 40 67 [	 ]*crtl	%r6,%r7
.*:	b9 72 60 67 [	 ]*crtne	%r6,%r7
.*:	b9 72 60 67 [	 ]*crtne	%r6,%r7
.*:	b9 72 80 67 [	 ]*crte	%r6,%r7
.*:	b9 72 80 67 [	 ]*crte	%r6,%r7
.*:	b9 72 a0 67 [	 ]*crtnl	%r6,%r7
.*:	b9 72 a0 67 [	 ]*crtnl	%r6,%r7
.*:	b9 72 c0 67 [	 ]*crtnh	%r6,%r7
.*:	b9 72 c0 67 [	 ]*crtnh	%r6,%r7
.*:	b9 60 a0 67 [	 ]*cgrtnl	%r6,%r7
.*:	b9 60 20 67 [	 ]*cgrth	%r6,%r7
.*:	b9 60 20 67 [	 ]*cgrth	%r6,%r7
.*:	b9 60 40 67 [	 ]*cgrtl	%r6,%r7
.*:	b9 60 40 67 [	 ]*cgrtl	%r6,%r7
.*:	b9 60 60 67 [	 ]*cgrtne	%r6,%r7
.*:	b9 60 60 67 [	 ]*cgrtne	%r6,%r7
.*:	b9 60 80 67 [	 ]*cgrte	%r6,%r7
.*:	b9 60 80 67 [	 ]*cgrte	%r6,%r7
.*:	b9 60 a0 67 [	 ]*cgrtnl	%r6,%r7
.*:	b9 60 a0 67 [	 ]*cgrtnl	%r6,%r7
.*:	b9 60 c0 67 [	 ]*cgrtnh	%r6,%r7
.*:	b9 60 c0 67 [	 ]*cgrtnh	%r6,%r7
.*:	ec 60 8a d0 a0 72 [	 ]*citnl	%r6,-30000
.*:	ec 60 8a d0 20 72 [	 ]*cith	%r6,-30000
.*:	ec 60 8a d0 20 72 [	 ]*cith	%r6,-30000
.*:	ec 60 8a d0 40 72 [	 ]*citl	%r6,-30000
.*:	ec 60 8a d0 40 72 [	 ]*citl	%r6,-30000
.*:	ec 60 8a d0 60 72 [	 ]*citne	%r6,-30000
.*:	ec 60 8a d0 60 72 [	 ]*citne	%r6,-30000
.*:	ec 60 8a d0 80 72 [	 ]*cite	%r6,-30000
.*:	ec 60 8a d0 80 72 [	 ]*cite	%r6,-30000
.*:	ec 60 8a d0 a0 72 [	 ]*citnl	%r6,-30000
.*:	ec 60 8a d0 a0 72 [	 ]*citnl	%r6,-30000
.*:	ec 60 8a d0 c0 72 [	 ]*citnh	%r6,-30000
.*:	ec 60 8a d0 c0 72 [	 ]*citnh	%r6,-30000
.*:	ec 60 8a d0 a0 70 [	 ]*cgitnl	%r6,-30000
.*:	ec 60 8a d0 20 70 [	 ]*cgith	%r6,-30000
.*:	ec 60 8a d0 20 70 [	 ]*cgith	%r6,-30000
.*:	ec 60 8a d0 40 70 [	 ]*cgitl	%r6,-30000
.*:	ec 60 8a d0 40 70 [	 ]*cgitl	%r6,-30000
.*:	ec 60 8a d0 60 70 [	 ]*cgitne	%r6,-30000
.*:	ec 60 8a d0 60 70 [	 ]*cgitne	%r6,-30000
.*:	ec 60 8a d0 80 70 [	 ]*cgite	%r6,-30000
.*:	ec 60 8a d0 80 70 [	 ]*cgite	%r6,-30000
.*:	ec 60 8a d0 a0 70 [	 ]*cgitnl	%r6,-30000
.*:	ec 60 8a d0 a0 70 [	 ]*cgitnl	%r6,-30000
.*:	ec 60 8a d0 c0 70 [	 ]*cgitnh	%r6,-30000
.*:	ec 60 8a d0 c0 70 [	 ]*cgitnh	%r6,-30000
.*:	e3 67 85 b3 01 34 [	 ]*cgh	%r6,5555\(%r7,%r8\)
.*:	e5 54 64 57 8a d0 [	 ]*chhsi	1111\(%r6\),-30000
.*:	e5 5c 64 57 8a d0 [	 ]*chsi	1111\(%r6\),-30000
.*:	e5 58 64 57 8a d0 [	 ]*cghsi	1111\(%r6\),-30000
.*:	c6 65 00 00 00 00 [	 ]*chrl	%r6,3b6 <foo\+0x3b6>
.*:	c6 64 00 00 00 00 [	 ]*cghrl	%r6,3bc <foo\+0x3bc>
.*:	e5 55 64 57 9c 40 [	 ]*clhhsi	1111\(%r6\),40000
.*:	e5 5d 64 57 9c 40 [	 ]*clfhsi	1111\(%r6\),40000
.*:	e5 59 64 57 9c 40 [	 ]*clghsi	1111\(%r6\),40000
.*:	c6 6f 00 00 00 00 [	 ]*clrl	%r6,3d4 <foo\+0x3d4>
.*:	c6 6a 00 00 00 00 [	 ]*clgrl	%r6,3da <foo\+0x3da>
.*:	c6 6e 00 00 00 00 [	 ]*clgfrl	%r6,3e0 <foo\+0x3e0>
.*:	c6 67 00 00 00 00 [	 ]*clhrl	%r6,3e6 <foo\+0x3e6>
.*:	c6 66 00 00 00 00 [	 ]*clghrl	%r6,3ec <foo\+0x3ec>
.*:	ec 67 84 57 a0 f7 [	 ]*clrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 f7 [	 ]*clrbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 f7 [	 ]*clrbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 f7 [	 ]*clrbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 f7 [	 ]*clrbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 f7 [	 ]*clrbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 f7 [	 ]*clrbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 f7 [	 ]*clrbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 f7 [	 ]*clrbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 f7 [	 ]*clrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 f7 [	 ]*clrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 f7 [	 ]*clrbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 f7 [	 ]*clrbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 e5 [	 ]*clgrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 e5 [	 ]*clgrbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 20 e5 [	 ]*clgrbh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 e5 [	 ]*clgrbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 40 e5 [	 ]*clgrbl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 e5 [	 ]*clgrbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 60 e5 [	 ]*clgrbne	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 e5 [	 ]*clgrbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 80 e5 [	 ]*clgrbe	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 e5 [	 ]*clgrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 a0 e5 [	 ]*clgrbnl	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 e5 [	 ]*clgrbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 84 57 c0 e5 [	 ]*clgrbnh	%r6,%r7,1111\(%r8\)
.*:	ec 67 00 00 a0 77 [	 ]*clrjnl	%r6,%r7,48e <foo\+0x48e>
.*:	ec 67 00 00 20 77 [	 ]*clrjh	%r6,%r7,494 <foo\+0x494>
.*:	ec 67 00 00 20 77 [	 ]*clrjh	%r6,%r7,49a <foo\+0x49a>
.*:	ec 67 00 00 40 77 [	 ]*clrjl	%r6,%r7,4a0 <foo\+0x4a0>
.*:	ec 67 00 00 40 77 [	 ]*clrjl	%r6,%r7,4a6 <foo\+0x4a6>
.*:	ec 67 00 00 60 77 [	 ]*clrjne	%r6,%r7,4ac <foo\+0x4ac>
.*:	ec 67 00 00 60 77 [	 ]*clrjne	%r6,%r7,4b2 <foo\+0x4b2>
.*:	ec 67 00 00 80 77 [	 ]*clrje	%r6,%r7,4b8 <foo\+0x4b8>
.*:	ec 67 00 00 80 77 [	 ]*clrje	%r6,%r7,4be <foo\+0x4be>
.*:	ec 67 00 00 a0 77 [	 ]*clrjnl	%r6,%r7,4c4 <foo\+0x4c4>
.*:	ec 67 00 00 a0 77 [	 ]*clrjnl	%r6,%r7,4ca <foo\+0x4ca>
.*:	ec 67 00 00 c0 77 [	 ]*clrjnh	%r6,%r7,4d0 <foo\+0x4d0>
.*:	ec 67 00 00 c0 77 [	 ]*clrjnh	%r6,%r7,4d6 <foo\+0x4d6>
.*:	ec 67 00 00 a0 65 [	 ]*clgrjnl	%r6,%r7,4dc <foo\+0x4dc>
.*:	ec 67 00 00 20 65 [	 ]*clgrjh	%r6,%r7,4e2 <foo\+0x4e2>
.*:	ec 67 00 00 20 65 [	 ]*clgrjh	%r6,%r7,4e8 <foo\+0x4e8>
.*:	ec 67 00 00 40 65 [	 ]*clgrjl	%r6,%r7,4ee <foo\+0x4ee>
.*:	ec 67 00 00 40 65 [	 ]*clgrjl	%r6,%r7,4f4 <foo\+0x4f4>
.*:	ec 67 00 00 60 65 [	 ]*clgrjne	%r6,%r7,4fa <foo\+0x4fa>
.*:	ec 67 00 00 60 65 [	 ]*clgrjne	%r6,%r7,500 <foo\+0x500>
.*:	ec 67 00 00 80 65 [	 ]*clgrje	%r6,%r7,506 <foo\+0x506>
.*:	ec 67 00 00 80 65 [	 ]*clgrje	%r6,%r7,50c <foo\+0x50c>
.*:	ec 67 00 00 a0 65 [	 ]*clgrjnl	%r6,%r7,512 <foo\+0x512>
.*:	ec 67 00 00 a0 65 [	 ]*clgrjnl	%r6,%r7,518 <foo\+0x518>
.*:	ec 67 00 00 c0 65 [	 ]*clgrjnh	%r6,%r7,51e <foo\+0x51e>
.*:	ec 67 00 00 c0 65 [	 ]*clgrjnh	%r6,%r7,524 <foo\+0x524>
.*:	ec 6a 74 57 c8 ff [	 ]*clibnl	%r6,200,1111\(%r7\)
.*:	ec 62 74 57 c8 ff [	 ]*clibh	%r6,200,1111\(%r7\)
.*:	ec 62 74 57 c8 ff [	 ]*clibh	%r6,200,1111\(%r7\)
.*:	ec 64 74 57 c8 ff [	 ]*clibl	%r6,200,1111\(%r7\)
.*:	ec 64 74 57 c8 ff [	 ]*clibl	%r6,200,1111\(%r7\)
.*:	ec 66 74 57 c8 ff [	 ]*clibne	%r6,200,1111\(%r7\)
.*:	ec 66 74 57 c8 ff [	 ]*clibne	%r6,200,1111\(%r7\)
.*:	ec 68 74 57 c8 ff [	 ]*clibe	%r6,200,1111\(%r7\)
.*:	ec 68 74 57 c8 ff [	 ]*clibe	%r6,200,1111\(%r7\)
.*:	ec 6a 74 57 c8 ff [	 ]*clibnl	%r6,200,1111\(%r7\)
.*:	ec 6a 74 57 c8 ff [	 ]*clibnl	%r6,200,1111\(%r7\)
.*:	ec 6c 74 57 c8 ff [	 ]*clibnh	%r6,200,1111\(%r7\)
.*:	ec 6c 74 57 c8 ff [	 ]*clibnh	%r6,200,1111\(%r7\)
.*:	ec 6a 74 57 c8 fd [	 ]*clgibnl	%r6,200,1111\(%r7\)
.*:	ec 62 74 57 c8 fd [	 ]*clgibh	%r6,200,1111\(%r7\)
.*:	ec 62 74 57 c8 fd [	 ]*clgibh	%r6,200,1111\(%r7\)
.*:	ec 64 74 57 c8 fd [	 ]*clgibl	%r6,200,1111\(%r7\)
.*:	ec 64 74 57 c8 fd [	 ]*clgibl	%r6,200,1111\(%r7\)
.*:	ec 66 74 57 c8 fd [	 ]*clgibne	%r6,200,1111\(%r7\)
.*:	ec 66 74 57 c8 fd [	 ]*clgibne	%r6,200,1111\(%r7\)
.*:	ec 68 74 57 c8 fd [	 ]*clgibe	%r6,200,1111\(%r7\)
.*:	ec 68 74 57 c8 fd [	 ]*clgibe	%r6,200,1111\(%r7\)
.*:	ec 6a 74 57 c8 fd [	 ]*clgibnl	%r6,200,1111\(%r7\)
.*:	ec 6a 74 57 c8 fd [	 ]*clgibnl	%r6,200,1111\(%r7\)
.*:	ec 6c 74 57 c8 fd [	 ]*clgibnh	%r6,200,1111\(%r7\)
.*:	ec 6c 74 57 c8 fd [	 ]*clgibnh	%r6,200,1111\(%r7\)
.*:	ec 6a 00 00 c8 7f [	 ]*clijnl	%r6,200,5c6 <foo\+0x5c6>
.*:	ec 62 00 00 c8 7f [	 ]*clijh	%r6,200,5cc <foo\+0x5cc>
.*:	ec 62 00 00 c8 7f [	 ]*clijh	%r6,200,5d2 <foo\+0x5d2>
.*:	ec 64 00 00 c8 7f [	 ]*clijl	%r6,200,5d8 <foo\+0x5d8>
.*:	ec 64 00 00 c8 7f [	 ]*clijl	%r6,200,5de <foo\+0x5de>
.*:	ec 66 00 00 c8 7f [	 ]*clijne	%r6,200,5e4 <foo\+0x5e4>
.*:	ec 66 00 00 c8 7f [	 ]*clijne	%r6,200,5ea <foo\+0x5ea>
.*:	ec 68 00 00 c8 7f [	 ]*clije	%r6,200,5f0 <foo\+0x5f0>
.*:	ec 68 00 00 c8 7f [	 ]*clije	%r6,200,5f6 <foo\+0x5f6>
.*:	ec 6a 00 00 c8 7f [	 ]*clijnl	%r6,200,5fc <foo\+0x5fc>
.*:	ec 6a 00 00 c8 7f [	 ]*clijnl	%r6,200,602 <foo\+0x602>
.*:	ec 6c 00 00 c8 7f [	 ]*clijnh	%r6,200,608 <foo\+0x608>
.*:	ec 6c 00 00 c8 7f [	 ]*clijnh	%r6,200,60e <foo\+0x60e>
.*:	ec 6a 00 00 c8 7d [	 ]*clgijnl	%r6,200,614 <foo\+0x614>
.*:	ec 62 00 00 c8 7d [	 ]*clgijh	%r6,200,61a <foo\+0x61a>
.*:	ec 62 00 00 c8 7d [	 ]*clgijh	%r6,200,620 <foo\+0x620>
.*:	ec 64 00 00 c8 7d [	 ]*clgijl	%r6,200,626 <foo\+0x626>
.*:	ec 64 00 00 c8 7d [	 ]*clgijl	%r6,200,62c <foo\+0x62c>
.*:	ec 66 00 00 c8 7d [	 ]*clgijne	%r6,200,632 <foo\+0x632>
.*:	ec 66 00 00 c8 7d [	 ]*clgijne	%r6,200,638 <foo\+0x638>
.*:	ec 68 00 00 c8 7d [	 ]*clgije	%r6,200,63e <foo\+0x63e>
.*:	ec 68 00 00 c8 7d [	 ]*clgije	%r6,200,644 <foo\+0x644>
.*:	ec 6a 00 00 c8 7d [	 ]*clgijnl	%r6,200,64a <foo\+0x64a>
.*:	ec 6a 00 00 c8 7d [	 ]*clgijnl	%r6,200,650 <foo\+0x650>
.*:	ec 6c 00 00 c8 7d [	 ]*clgijnh	%r6,200,656 <foo\+0x656>
.*:	ec 6c 00 00 c8 7d [	 ]*clgijnh	%r6,200,65c <foo\+0x65c>
.*:	b9 73 a0 67 [	 ]*clrtnl	%r6,%r7
.*:	b9 73 20 67 [	 ]*clrth	%r6,%r7
.*:	b9 73 20 67 [	 ]*clrth	%r6,%r7
.*:	b9 73 40 67 [	 ]*clrtl	%r6,%r7
.*:	b9 73 40 67 [	 ]*clrtl	%r6,%r7
.*:	b9 73 60 67 [	 ]*clrtne	%r6,%r7
.*:	b9 73 60 67 [	 ]*clrtne	%r6,%r7
.*:	b9 73 80 67 [	 ]*clrte	%r6,%r7
.*:	b9 73 80 67 [	 ]*clrte	%r6,%r7
.*:	b9 73 a0 67 [	 ]*clrtnl	%r6,%r7
.*:	b9 73 a0 67 [	 ]*clrtnl	%r6,%r7
.*:	b9 73 c0 67 [	 ]*clrtnh	%r6,%r7
.*:	b9 73 c0 67 [	 ]*clrtnh	%r6,%r7
.*:	b9 61 a0 67 [	 ]*clgrtnl	%r6,%r7
.*:	b9 61 20 67 [	 ]*clgrth	%r6,%r7
.*:	b9 61 20 67 [	 ]*clgrth	%r6,%r7
.*:	b9 61 40 67 [	 ]*clgrtl	%r6,%r7
.*:	b9 61 40 67 [	 ]*clgrtl	%r6,%r7
.*:	b9 61 60 67 [	 ]*clgrtne	%r6,%r7
.*:	b9 61 60 67 [	 ]*clgrtne	%r6,%r7
.*:	b9 61 80 67 [	 ]*clgrte	%r6,%r7
.*:	b9 61 80 67 [	 ]*clgrte	%r6,%r7
.*:	b9 61 a0 67 [	 ]*clgrtnl	%r6,%r7
.*:	b9 61 a0 67 [	 ]*clgrtnl	%r6,%r7
.*:	b9 61 c0 67 [	 ]*clgrtnh	%r6,%r7
.*:	b9 61 c0 67 [	 ]*clgrtnh	%r6,%r7
.*:	ec 60 75 30 a0 73 [	 ]*clfitnl	%r6,30000
.*:	ec 60 75 30 20 73 [	 ]*clfith	%r6,30000
.*:	ec 60 75 30 20 73 [	 ]*clfith	%r6,30000
.*:	ec 60 75 30 40 73 [	 ]*clfitl	%r6,30000
.*:	ec 60 75 30 40 73 [	 ]*clfitl	%r6,30000
.*:	ec 60 75 30 60 73 [	 ]*clfitne	%r6,30000
.*:	ec 60 75 30 60 73 [	 ]*clfitne	%r6,30000
.*:	ec 60 75 30 80 73 [	 ]*clfite	%r6,30000
.*:	ec 60 75 30 80 73 [	 ]*clfite	%r6,30000
.*:	ec 60 75 30 a0 73 [	 ]*clfitnl	%r6,30000
.*:	ec 60 75 30 a0 73 [	 ]*clfitnl	%r6,30000
.*:	ec 60 75 30 c0 73 [	 ]*clfitnh	%r6,30000
.*:	ec 60 75 30 c0 73 [	 ]*clfitnh	%r6,30000
.*:	ec 60 75 30 a0 71 [	 ]*clgitnl	%r6,30000
.*:	ec 60 75 30 20 71 [	 ]*clgith	%r6,30000
.*:	ec 60 75 30 20 71 [	 ]*clgith	%r6,30000
.*:	ec 60 75 30 40 71 [	 ]*clgitl	%r6,30000
.*:	ec 60 75 30 40 71 [	 ]*clgitl	%r6,30000
.*:	ec 60 75 30 60 71 [	 ]*clgitne	%r6,30000
.*:	ec 60 75 30 60 71 [	 ]*clgitne	%r6,30000
.*:	ec 60 75 30 80 71 [	 ]*clgite	%r6,30000
.*:	ec 60 75 30 80 71 [	 ]*clgite	%r6,30000
.*:	ec 60 75 30 a0 71 [	 ]*clgitnl	%r6,30000
.*:	ec 60 75 30 a0 71 [	 ]*clgitnl	%r6,30000
.*:	ec 60 75 30 c0 71 [	 ]*clgitnh	%r6,30000
.*:	ec 60 75 30 c0 71 [	 ]*clgitnh	%r6,30000
.*:	eb 67 84 57 00 4c [	 ]*ecag	%r6,%r7,1111\(%r8\)
.*:	c4 6d 00 00 00 00 [	 ]*lrl	%r6,76c <foo\+0x76c>
.*:	c4 68 00 00 00 00 [	 ]*lgrl	%r6,772 <foo\+0x772>
.*:	c4 6c 00 00 00 00 [	 ]*lgfrl	%r6,778 <foo\+0x778>
.*:	e3 67 85 b3 01 75 [	 ]*laey	%r6,5555\(%r7,%r8\)
.*:	e3 67 85 b3 01 32 [	 ]*ltgf	%r6,5555\(%r7,%r8\)
.*:	c4 65 00 00 00 00 [	 ]*lhrl	%r6,78a <foo\+0x78a>
.*:	c4 64 00 00 00 00 [	 ]*lghrl	%r6,790 <foo\+0x790>
.*:	c4 6e 00 00 00 00 [	 ]*llgfrl	%r6,796 <foo\+0x796>
.*:	c4 62 00 00 00 00 [	 ]*llhrl	%r6,79c <foo\+0x79c>
.*:	c4 66 00 00 00 00 [	 ]*llghrl	%r6,7a2 <foo\+0x7a2>
.*:	e5 44 64 57 8a d0 [	 ]*mvhhi	1111\(%r6\),-30000
.*:	e5 4c 64 57 8a d0 [	 ]*mvhi	1111\(%r6\),-30000
.*:	e5 48 64 57 8a d0 [	 ]*mvghi	1111\(%r6\),-30000
.*:	e3 67 85 b3 01 5c [	 ]*mfy	%r6,5555\(%r7,%r8\)
.*:	e3 67 85 b3 01 7c [	 ]*mhy	%r6,5555\(%r7,%r8\)
.*:	c2 61 ff fe 79 60 [	 ]*msfi	%r6,-100000
.*:	c2 60 ff fe 79 60 [	 ]*msgfi	%r6,-100000
.*:	e3 a6 75 b3 01 36 [	 ]*pfd	10,5555\(%r6,%r7\)
.*:	c6 a2 00 00 00 00 [	 ]*pfdrl	10,7d8 <foo\+0x7d8>
.*:	ec 67 d2 dc e6 54 [	 ]*rnsbg	%r6,%r7,210,220,230
.*:	ec 67 d2 dc e6 57 [	 ]*rxsbg	%r6,%r7,210,220,230
.*:	ec 67 d2 dc e6 56 [	 ]*rosbg	%r6,%r7,210,220,230
.*:	ec 67 d2 14 e6 55 [	 ]*risbg	%r6,%r7,210,20,230
.*:	ec 67 d2 bc e6 55 [	 ]*risbgz	%r6,%r7,210,60,230
.*:	ec 67 d2 94 e6 55 [	 ]*risbgz	%r6,%r7,210,20,230
.*:	c4 6f 00 00 00 00 [	 ]*strl	%r6,802 <foo\+0x802>
.*:	c4 6b 00 00 00 00 [	 ]*stgrl	%r6,808 <foo\+0x808>
.*:	c4 67 00 00 00 00 [	 ]*sthrl	%r6,80e <foo\+0x80e>
.*:	c6 60 00 00 00 00 [	 ]*exrl	%r6,814 <foo\+0x814>
.*:	af ee 6d 05 [	 ]*mc	3333\(%r6\),238
.*:	b9 a2 00 60 [	 ]*ptf	%r6
.*:	b9 af 00 67 [	 ]*pfmf	%r6,%r7
.*:	b9 bf a0 67 [	 ]*trte	%r6,%r7,10
.*:	b9 bf 00 67 [	 ]*trte	%r6,%r7
.*:	b9 bd a0 67 [	 ]*trtre	%r6,%r7,10
.*:	b9 bd 00 67 [	 ]*trtre	%r6,%r7
.*:	b2 ed 00 67 [	 ]*ecpga	%r6,%r7
.*:	b2 e4 00 67 [	 ]*ecctr	%r6,%r7
.*:	b2 e5 00 67 [	 ]*epctr	%r6,%r7
.*:	b2 84 6d 05 [	 ]*lcctl	3333\(%r6\)
.*:	b2 85 6d 05 [	 ]*lpctl	3333\(%r6\)
.*:	b2 87 6d 05 [	 ]*lsctl	3333\(%r6\)
.*:	b2 8e 6d 05 [	 ]*qctri	3333\(%r6\)
.*:	b2 86 6d 05 [	 ]*qsi	3333\(%r6\)
.*:	b2 e0 00 67 [	 ]*scctr	%r6,%r7
.*:	b2 e1 00 67 [	 ]*spctr	%r6,%r7
.*:	b2 80 6d 05 [	 ]*lpp	3333\(%r6\)
.*:	07 07 [	 ]*nopr	%r7
