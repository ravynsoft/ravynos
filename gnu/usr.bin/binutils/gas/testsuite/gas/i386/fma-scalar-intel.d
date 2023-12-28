#as: -mavxscalar=256
#objdump: -dwMintel
#name: i386 FMA scalar insns (Intel disassembly)
#source: fma-scalar.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e2 cd 99 d4       	vfmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 99 11       	vfmadd132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a9 d4       	vfmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd a9 11       	vfmadd213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b9 d4       	vfmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd b9 11       	vfmadd231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9b d4       	vfmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9b 11       	vfmsub132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ab d4       	vfmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd ab 11       	vfmsub213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bb d4       	vfmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd bb 11       	vfmsub231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9d d4       	vfnmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9d 11       	vfnmadd132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ad d4       	vfnmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd ad 11       	vfnmadd213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bd d4       	vfnmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd bd 11       	vfnmadd231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9f d4       	vfnmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9f 11       	vfnmsub132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd af d4       	vfnmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd af 11       	vfnmsub213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bf d4       	vfnmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd bf 11       	vfnmsub231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 99 d4       	vfmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 99 11       	vfmadd132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a9 d4       	vfmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d a9 11       	vfmadd213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b9 d4       	vfmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d b9 11       	vfmadd231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9b d4       	vfmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9b 11       	vfmsub132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ab d4       	vfmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d ab 11       	vfmsub213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bb d4       	vfmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d bb 11       	vfmsub231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9d d4       	vfnmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9d 11       	vfnmadd132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ad d4       	vfnmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d ad 11       	vfnmadd213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bd d4       	vfnmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d bd 11       	vfnmadd231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9f d4       	vfnmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9f 11       	vfnmsub132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d af d4       	vfnmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d af 11       	vfnmsub213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bf d4       	vfnmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d bf 11       	vfnmsub231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 99 d4       	vfmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 99 11       	vfmadd132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 99 11       	vfmadd132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a9 d4       	vfmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd a9 11       	vfmadd213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd a9 11       	vfmadd213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b9 d4       	vfmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd b9 11       	vfmadd231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd b9 11       	vfmadd231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9b d4       	vfmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9b 11       	vfmsub132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9b 11       	vfmsub132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ab d4       	vfmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd ab 11       	vfmsub213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ab 11       	vfmsub213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bb d4       	vfmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd bb 11       	vfmsub231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bb 11       	vfmsub231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9d d4       	vfnmadd132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9d 11       	vfnmadd132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9d 11       	vfnmadd132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ad d4       	vfnmadd213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd ad 11       	vfnmadd213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd ad 11       	vfnmadd213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bd d4       	vfnmadd231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd bd 11       	vfnmadd231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bd 11       	vfnmadd231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9f d4       	vfnmsub132sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd 9f 11       	vfnmsub132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd 9f 11       	vfnmsub132sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd af d4       	vfnmsub213sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd af 11       	vfnmsub213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd af 11       	vfnmsub213sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bf d4       	vfnmsub231sd xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 cd bf 11       	vfnmsub231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 cd bf 11       	vfnmsub231sd xmm2,xmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 99 d4       	vfmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 99 11       	vfmadd132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 99 11       	vfmadd132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a9 d4       	vfmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d a9 11       	vfmadd213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d a9 11       	vfmadd213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b9 d4       	vfmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d b9 11       	vfmadd231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d b9 11       	vfmadd231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9b d4       	vfmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9b 11       	vfmsub132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9b 11       	vfmsub132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ab d4       	vfmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d ab 11       	vfmsub213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ab 11       	vfmsub213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bb d4       	vfmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d bb 11       	vfmsub231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bb 11       	vfmsub231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9d d4       	vfnmadd132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9d 11       	vfnmadd132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9d 11       	vfnmadd132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ad d4       	vfnmadd213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d ad 11       	vfnmadd213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d ad 11       	vfnmadd213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bd d4       	vfnmadd231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d bd 11       	vfnmadd231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bd 11       	vfnmadd231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9f d4       	vfnmsub132ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d 9f 11       	vfnmsub132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 9f 11       	vfnmsub132ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d af d4       	vfnmsub213ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d af 11       	vfnmsub213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d af 11       	vfnmsub213ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bf d4       	vfnmsub231ss xmm2,xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 4d bf 11       	vfnmsub231ss xmm2,xmm6,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d bf 11       	vfnmsub231ss xmm2,xmm6,DWORD PTR \[ecx\]
#pass
