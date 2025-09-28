#as: -O0
#objdump: -drw
#name: i386 AT&T register names

.*: +file format .*i386.*

Disassembly of section \.text:
0+0 <.*>:
.*[ 	]+R_386_16[ 	]+eax
.*[ 	]+R_386_16[ 	]+rax
.*[ 	]+R_386_16[ 	]+axl
.*[ 	]+R_386_16[ 	]+r8b
.*[ 	]+R_386_16[ 	]+r8w
.*[ 	]+R_386_16[ 	]+r8d
.*[ 	]+R_386_16[ 	]+r8
.*[ 	]+R_386_16[ 	]+fs
.*[ 	]+R_386_16[ 	]+st
.*[ 	]+R_386_16[ 	]+cr0
.*[ 	]+R_386_16[ 	]+dr0
.*[ 	]+R_386_16[ 	]+tr0
.*[ 	]+R_386_16[ 	]+mm0
.*[ 	]+R_386_16[ 	]+xmm0
.*[ 	]+R_386_16[ 	]+ymm0
.*[ 	]+R_386_32[ 	]+rax
.*[ 	]+R_386_32[ 	]+axl
.*[ 	]+R_386_32[ 	]+r8b
.*[ 	]+R_386_32[ 	]+r8w
.*[ 	]+R_386_32[ 	]+r8d
.*[ 	]+R_386_32[ 	]+r8
.*[ 	]+R_386_32[ 	]+st
.*:[ 	]+0f 20 c0[ 	]+mov[ 	]+%cr0,%eax
.*:[ 	]+0f 21 c0[ 	]+mov[ 	]+%db0,%eax
.*:[ 	]+0f 24 c0[ 	]+mov[ 	]+%tr0,%eax
.*[ 	]+R_386_32[ 	]+mm0
.*[ 	]+R_386_32[ 	]+xmm0
.*[ 	]+R_386_32[ 	]+ymm0
.*:[ 	]+dd c0[ 	]+ffree[ 	]+%st(\(0\))?
.*:[ 	]+0f ef c0[ 	]+pxor[ 	]+%mm0,%mm0
.*:[ 	]+0f 57 c0[ 	]+xorps[ 	]+%xmm0,%xmm0
.*:[ 	]+c5 fc 57 c0[ 	]+vxorps[ 	]+%ymm0,%ymm0,%ymm0
.*:[ 	]+44[ 	]+inc    %esp
.*:[ 	]+88 c0[ 	]+mov[ 	]+%al,%al
.*:[ 	]+66 44[ 	]+inc[ 	]+%sp
.*:[ 	]+89 c0[ 	]+mov[ 	]+%eax,%eax
.*:[ 	]+44[ 	]+inc    %esp
.*:[ 	]+89 c0[ 	]+mov[ 	]+%eax,%eax
.*:[ 	]+4c[ 	]+dec    %esp
.*:[ 	]+89 c0[ 	]+mov[ 	]+%eax,%eax

.* <ymm8>:
.*[ 	]+<ymm8>
#pass
