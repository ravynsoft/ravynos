#objdump: -drw -mi8086
#name: i386 32-bit addressing in 16-bit mode.

.*: +file format .*

Disassembly of section .text:

0+000 <.text>:
[	 ]*0:[	 ]+67 a0 98 08 60 00[	 ]+addr32[	 ]+mov[ 	]+0x600898,%al
[	 ]*6:[	 ]+67 a1 98 08 60 00[	 ]+addr32[	 ]+mov[ 	]+0x600898,%ax
[	 ]*c:[	 ]+67 66 a1 98 08 60 00[	 ]+addr32[	 ]+mov[ 	]+0x600898,%eax
[	 ]*13:[	 ]+67 a2 98 08 60 00[	 ]+addr32[	 ]+mov[ 	]+%al,0x600898
[	 ]*19:[	 ]+67 a3 98 08 60 00[	 ]+addr32[	 ]+mov[ 	]+%ax,0x600898
[	 ]*1f:[	 ]+67 66 a3 98 08 60 00[	 ]+addr32[	 ]+mov[ 	]+%eax,0x600898
[	 ]*26:[	 ]+67 66 c7 04 24 01 00 00 00[	 ]+movl[	 ]+\$0x1,\(%esp\)
[	 ]*2f:[	 ]+67 66 a1 ef cd ab 89[	 ]+addr32[	 ]+mov[ 	]+0x89abcdef,%eax
[	 ]*36:[	 ]+67 66 8b 1d ef cd ab 89[	 ]+addr32[	 ]+mov[ 	]+0x89abcdef,%ebx
[	 ]*3e:[	 ]+67 66 b8 ef cd ab 89[	 ]+addr32[	 ]+mov[ 	]+\$0x89abcdef,%eax
[	 ]*45:[	 ]+67 66 bb ef cd ab 89[	 ]+addr32[	 ]+mov[ 	]+\$0x89abcdef,%ebx
[	 ]*4c:[	 ]+67 66 a3 ef cd ab 89[	 ]+addr32[	 ]+mov[ 	]+%eax,0x89abcdef
[	 ]*53:[	 ]+67 66 89 1d ef cd ab 89[	 ]+addr32[	 ]+mov[ 	]+%ebx,0x89abcdef
#pass
