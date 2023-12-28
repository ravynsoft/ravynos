#name: i386 jump16
#objdump: -drw -mi8086
#warning_output: jump16.e

.*:     file format .*i386.*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	eb fe                	jmp    (0x0|0 <.text>)
[ 	]*[a-f0-9]+:	e9 f(e|b) ff             	jmp    (0x3|0 <.text>)	3: (R_386_PC)?(DISP)?16	xxx
[ 	]*[a-f0-9]+:	ff 26 00 00          	jmp    \*0x0	7: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 ff e7             	jmp    \*%edi
[ 	]*[a-f0-9]+:	67 ff 27             	jmp    \*\(%edi\)
[ 	]*[a-f0-9]+:	67 ff af 00 00 00 00 	ljmp   \*0x0\(%edi\)	12: (R_386_)?(dir)?32	xxx
[ 	]*[a-f0-9]+:	ff 2e 00 00          	ljmp   \*0x0	18: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 00 00 34 12       	ljmp   \$0x1234,\$0x0	1b: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 e8 db ff ff ff    	calll  (0x0|0 <.text>)
[ 	]*[a-f0-9]+:	66 e8 (fc|d5) ff ff ff    	calll  (0x27|0 <.text>)	27: (R_386_PC)?(DISP)?32	xxx
[ 	]*[a-f0-9]+:	66 ff 16 00 00       	calll  \*0x0	2e: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 ff d7             	call   \*%edi
[ 	]*[a-f0-9]+:	67 66 ff 17          	calll  \*\(%edi\)
[ 	]*[a-f0-9]+:	67 66 ff 9f 00 00 00 00 	lcalll \*0x0\(%edi\)	3b: (R_386_)?(dir)?32	xxx
[ 	]*[a-f0-9]+:	66 ff 1e 00 00       	lcalll \*0x0	42: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 9a 00 00 00 00 34 12 	lcalll \$0x1234,\$0x0	46: (R_386_)?(dir)?32	xxx
[ 	]*[a-f0-9]+:	eb b2                	jmp    (0x0|0 <.text>)
[ 	]*[a-f0-9]+:	ff 26 00 00          	jmp    \*0x0	50: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ff e7                	jmp    \*%di
[ 	]*[a-f0-9]+:	ff 25                	jmp    \*\(%di\)
[ 	]*[a-f0-9]+:	ff ad 00 00          	ljmp   \*0x0\(%di\)	58: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 ff ad 00 00       	ljmpl  \*0x0\(%di\)	5d: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ff 2e 00 00          	ljmp   \*0x0	61: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 ff 2e 00 00       	ljmpl  \*0x0	66: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 00 00 34 12       	ljmp   \$0x1234,\$0x0	69: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	e8 90 ff             	call   (0x0|0 <.text>)
[ 	]*[a-f0-9]+:	e8 (fe|8d) ff             	call   (0x71|0 <.text>)	71: (R_386_PC)?(DISP)?16	xxx
[ 	]*[a-f0-9]+:	ff 16 00 00          	call   \*0x0	75: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ff d7                	call   \*%di
[ 	]*[a-f0-9]+:	ff 15                	call   \*\(%di\)
[ 	]*[a-f0-9]+:	ff 9d 00 00          	lcall  \*0x0\(%di\)	7d: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 ff 9d 00 00       	lcalll \*0x0\(%di\)	82: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ff 1e 00 00          	lcall  \*0x0	86: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	66 ff 1e 00 00       	lcalll \*0x0	8b: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	9a 00 00 34 12       	lcall  \$0x1234,\$0x0	8e: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ff 17                	call   \*\(%bx\)
[ 	]*[a-f0-9]+:	ff 1f                	lcall  \*\(%bx\)
[ 	]*[a-f0-9]+:	66 ff 1f             	lcalll \*\(%bx\)
[ 	]*[a-f0-9]+:	ff 27                	jmp    \*\(%bx\)
[ 	]*[a-f0-9]+:	ff 2f                	ljmp   \*\(%bx\)
[ 	]*[a-f0-9]+:	66 ff 2f             	ljmpl  \*\(%bx\)
[ 	]*[a-f0-9]+:	eb 00                	jmp    (0xa2|a2 <.text\+0xa2>)
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 00                	jmp    (0xa5|a5 <.text\+0xa5>)
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	9a 10 10 90 90       	lcall  \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	9a 10 10 90 90       	lcall  \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	9a 00 00 90 90       	lcall  \$0x9090,\$0x0	b1: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	9a 00 00 90 90       	lcall  \$0x9090,\$0x0	b6: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	9a 10 10 90 90       	lcall  \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	9a 10 10 90 90       	lcall  \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	9a 00 00 90 90       	lcall  \$0x9090,\$0x0	c5: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	9a 00 00 90 90       	lcall  \$0x9090,\$0x0	ca: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 10 10 90 90       	ljmp   \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	ea 10 10 90 90       	ljmp   \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	ea 00 00 90 90       	ljmp   \$0x9090,\$0x0	d9: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 00 00 90 90       	ljmp   \$0x9090,\$0x0	de: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 10 10 90 90       	ljmp   \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	ea 10 10 90 90       	ljmp   \$0x9090,\$0x1010
[ 	]*[a-f0-9]+:	ea 00 00 90 90       	ljmp   \$0x9090,\$0x0	ed: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 00 00 90 90       	ljmp   \$0x9090,\$0x0	f2: (R_386_)?16	xxx
[ 	]*[a-f0-9]+:	ea 10 10 00 00       	ljmp   \$0x0,\$0x1010	f9: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 10 10 00 00       	ljmp   \$0x0,\$0x1010	fe: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 00 00 00 00       	ljmp   \$0x0,\$0x0	101: (R_386_)?16	xxx
[	]+103: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 00 00 00 00       	ljmp   \$0x0,\$0x0	106: (R_386_)?16	xxx
[	]+108: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 10 10 00 00       	ljmp   \$0x0,\$0x1010	10d: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 10 10 00 00       	ljmp   \$0x0,\$0x1010	112: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 00 00 00 00       	ljmp   \$0x0,\$0x0	115: (R_386_)?16	xxx
[	]+117: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	ea 00 00 00 00       	ljmp   \$0x0,\$0x0	11a: (R_386_)?16	xxx
[	]+11c: (R_386_)?16	yyy
[ 	]*[a-f0-9]+:	cf                   	iret
[ 	]*[a-f0-9]+:	cf                   	iret
[ 	]*[a-f0-9]+:	66 cf                	iretl
[ 	]*[a-f0-9]+:	cf                   	iret
[ 	]*[a-f0-9]+:	cf                   	iret
[ 	]*[a-f0-9]+:	66 cf                	iretl
#pass
