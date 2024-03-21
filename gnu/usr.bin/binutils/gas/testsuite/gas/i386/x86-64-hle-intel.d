#objdump: -dwMintel
#name: x86-64 HLE insns (Intel disassembly)
#source: x86-64-hle.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 11 64       	lock xacquire adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 11 64       	lock xrelease adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 01 64       	lock xacquire add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 01 64       	lock xrelease add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 21 64       	lock xacquire and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 21 64       	lock xrelease and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 c6 01 64          	xrelease mov BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 09 64       	lock xacquire or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 09 64       	lock xrelease or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 19 64       	lock xacquire sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 19 64       	lock xrelease sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 29 64       	lock xacquire sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 29 64       	lock xrelease sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 31 64       	lock xacquire xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 31 64       	lock xrelease xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 81 11 e8 03 	xacquire lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 11 e8 03 	xacquire lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 11 e8 03 	xrelease lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 11 e8 03 	xrelease lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 11 e8 03 	lock xacquire adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 11 e8 03 	lock xrelease adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 01 e8 03 	xacquire lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 01 e8 03 	xacquire lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 01 e8 03 	xrelease lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 01 e8 03 	xrelease lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 01 e8 03 	lock xacquire add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 01 e8 03 	lock xrelease add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 21 e8 03 	xacquire lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 21 e8 03 	xacquire lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 21 e8 03 	xrelease lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 21 e8 03 	xrelease lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 21 e8 03 	lock xacquire and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 21 e8 03 	lock xrelease and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 c7 01 e8 03    	xrelease mov WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 09 e8 03 	xacquire lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 09 e8 03 	xacquire lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 09 e8 03 	xrelease lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 09 e8 03 	xrelease lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 09 e8 03 	lock xacquire or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 09 e8 03 	lock xrelease or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 19 e8 03 	xacquire lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 19 e8 03 	xacquire lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 19 e8 03 	xrelease lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 19 e8 03 	xrelease lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 19 e8 03 	lock xacquire sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 19 e8 03 	lock xrelease sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 29 e8 03 	xacquire lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 29 e8 03 	xacquire lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 29 e8 03 	xrelease lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 29 e8 03 	xrelease lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 29 e8 03 	lock xacquire sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 29 e8 03 	lock xrelease sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 31 e8 03 	xacquire lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 31 e8 03 	xacquire lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 31 e8 03 	xrelease lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 31 e8 03 	xrelease lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 31 e8 03 	lock xacquire xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 31 e8 03 	lock xrelease xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f2 f0 81 11 80 96 98 00 	xacquire lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 11 80 96 98 00 	xacquire lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 11 80 96 98 00 	xrelease lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 11 80 96 98 00 	xrelease lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 11 80 96 98 00 	lock xacquire adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 11 80 96 98 00 	lock xrelease adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 01 80 96 98 00 	xacquire lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 01 80 96 98 00 	xacquire lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 01 80 96 98 00 	xrelease lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 01 80 96 98 00 	xrelease lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 01 80 96 98 00 	lock xacquire add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 01 80 96 98 00 	lock xrelease add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 21 80 96 98 00 	xacquire lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 21 80 96 98 00 	xacquire lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 21 80 96 98 00 	xrelease lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 21 80 96 98 00 	xrelease lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 21 80 96 98 00 	lock xacquire and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 21 80 96 98 00 	lock xrelease and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 c7 01 80 96 98 00 	xrelease mov DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 09 80 96 98 00 	xacquire lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 09 80 96 98 00 	xacquire lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 09 80 96 98 00 	xrelease lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 09 80 96 98 00 	xrelease lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 09 80 96 98 00 	lock xacquire or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 09 80 96 98 00 	lock xrelease or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 19 80 96 98 00 	xacquire lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 19 80 96 98 00 	xacquire lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 19 80 96 98 00 	xrelease lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 19 80 96 98 00 	xrelease lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 19 80 96 98 00 	lock xacquire sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 19 80 96 98 00 	lock xrelease sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 29 80 96 98 00 	xacquire lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 29 80 96 98 00 	xacquire lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 29 80 96 98 00 	xrelease lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 29 80 96 98 00 	xrelease lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 29 80 96 98 00 	lock xacquire sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 29 80 96 98 00 	lock xrelease sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 31 80 96 98 00 	xacquire lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 31 80 96 98 00 	xacquire lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 31 80 96 98 00 	xrelease lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 31 80 96 98 00 	xrelease lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 31 80 96 98 00 	lock xacquire xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 31 80 96 98 00 	lock xrelease xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 11 80 96 98 00 	xacquire lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 11 80 96 98 00 	xacquire lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 11 80 96 98 00 	xrelease lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 11 80 96 98 00 	xrelease lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 11 80 96 98 00 	lock xacquire adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 11 80 96 98 00 	lock xrelease adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 01 80 96 98 00 	xacquire lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 01 80 96 98 00 	xacquire lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 01 80 96 98 00 	xrelease lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 01 80 96 98 00 	xrelease lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 01 80 96 98 00 	lock xacquire add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 01 80 96 98 00 	lock xrelease add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 21 80 96 98 00 	xacquire lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 21 80 96 98 00 	xacquire lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 21 80 96 98 00 	xrelease lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 21 80 96 98 00 	xrelease lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 21 80 96 98 00 	lock xacquire and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 21 80 96 98 00 	lock xrelease and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 48 c7 01 80 96 98 00 	xrelease mov QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 09 80 96 98 00 	xacquire lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 09 80 96 98 00 	xacquire lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 09 80 96 98 00 	xrelease lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 09 80 96 98 00 	xrelease lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 09 80 96 98 00 	lock xacquire or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 09 80 96 98 00 	lock xrelease or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 19 80 96 98 00 	xacquire lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 19 80 96 98 00 	xacquire lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 19 80 96 98 00 	xrelease lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 19 80 96 98 00 	xrelease lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 19 80 96 98 00 	lock xacquire sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 19 80 96 98 00 	lock xrelease sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 29 80 96 98 00 	xacquire lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 29 80 96 98 00 	xacquire lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 29 80 96 98 00 	xrelease lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 29 80 96 98 00 	xrelease lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 29 80 96 98 00 	lock xacquire sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 29 80 96 98 00 	lock xrelease sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 31 80 96 98 00 	xacquire lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 31 80 96 98 00 	xacquire lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 31 80 96 98 00 	xrelease lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 31 80 96 98 00 	xrelease lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 31 80 96 98 00 	lock xacquire xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 31 80 96 98 00 	lock xrelease xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	66 f2 f0 83 11 64    	xacquire lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 11 64    	xacquire lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 11 64    	xrelease lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 11 64    	xrelease lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 11 64    	lock xacquire adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 11 64    	lock xrelease adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 01 64    	xacquire lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 01 64    	xacquire lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 01 64    	xrelease lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 01 64    	xrelease lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 01 64    	lock xacquire add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 01 64    	lock xrelease add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 21 64    	xacquire lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 21 64    	xacquire lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 21 64    	xrelease lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 21 64    	xrelease lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 21 64    	lock xacquire and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 21 64    	lock xrelease and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 39 64 	xacquire lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 39 64 	xacquire lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 39 64 	xrelease lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 39 64 	xrelease lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 0f ba 39 64 	lock xacquire btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 0f ba 39 64 	lock xrelease btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 31 64 	xacquire lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 31 64 	xacquire lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 31 64 	xrelease lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 31 64 	xrelease lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 0f ba 31 64 	lock xacquire btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 0f ba 31 64 	lock xrelease btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 29 64 	xacquire lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 29 64 	xacquire lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 29 64 	xrelease lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 29 64 	xrelease lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 0f ba 29 64 	lock xacquire bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 0f ba 29 64 	lock xrelease bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 c7 01 64 00    	xrelease mov WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 09 64    	xacquire lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 09 64    	xacquire lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 09 64    	xrelease lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 09 64    	xrelease lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 09 64    	lock xacquire or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 09 64    	lock xrelease or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 19 64    	xacquire lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 19 64    	xacquire lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 19 64    	xrelease lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 19 64    	xrelease lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 19 64    	lock xacquire sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 19 64    	lock xrelease sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 29 64    	xacquire lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 29 64    	xacquire lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 29 64    	xrelease lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 29 64    	xrelease lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 29 64    	lock xacquire sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 29 64    	lock xrelease sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 31 64    	xacquire lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 31 64    	xacquire lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 31 64    	xrelease lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 31 64    	xrelease lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 31 64    	lock xacquire xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 31 64    	lock xrelease xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 11 64       	xacquire lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 11 64       	xacquire lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 11 64       	xrelease lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 11 64       	xrelease lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 11 64       	lock xacquire adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 11 64       	lock xrelease adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 01 64       	xacquire lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 01 64       	xacquire lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 01 64       	xrelease lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 01 64       	xrelease lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 01 64       	lock xacquire add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 01 64       	lock xrelease add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 21 64       	xacquire lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 21 64       	xacquire lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 21 64       	xrelease lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 21 64       	xrelease lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 21 64       	lock xacquire and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 21 64       	lock xrelease and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 39 64    	xacquire lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 39 64    	xacquire lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 39 64    	xrelease lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 39 64    	xrelease lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 0f ba 39 64    	lock xacquire btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 0f ba 39 64    	lock xrelease btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 31 64    	xacquire lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 31 64    	xacquire lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 31 64    	xrelease lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 31 64    	xrelease lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 0f ba 31 64    	lock xacquire btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 0f ba 31 64    	lock xrelease btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 29 64    	xacquire lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 29 64    	xacquire lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 29 64    	xrelease lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 29 64    	xrelease lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 0f ba 29 64    	lock xacquire bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 0f ba 29 64    	lock xrelease bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 c7 01 64 00 00 00 	xrelease mov DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 09 64       	xacquire lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 09 64       	xacquire lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 09 64       	xrelease lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 09 64       	xrelease lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 09 64       	lock xacquire or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 09 64       	lock xrelease or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 19 64       	xacquire lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 19 64       	xacquire lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 19 64       	xrelease lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 19 64       	xrelease lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 19 64       	lock xacquire sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 19 64       	lock xrelease sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 29 64       	xacquire lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 29 64       	xacquire lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 29 64       	xrelease lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 29 64       	xrelease lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 29 64       	lock xacquire sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 29 64       	lock xrelease sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 31 64       	xacquire lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 31 64       	xacquire lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 31 64       	xrelease lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 31 64       	xrelease lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 31 64       	lock xacquire xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 31 64       	lock xrelease xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 11 64    	xacquire lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 11 64    	xacquire lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 11 64    	xrelease lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 11 64    	xrelease lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 11 64    	lock xacquire adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 11 64    	lock xrelease adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 01 64    	xacquire lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 01 64    	xacquire lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 01 64    	xrelease lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 01 64    	xrelease lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 01 64    	lock xacquire add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 01 64    	lock xrelease add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 21 64    	xacquire lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 21 64    	xacquire lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 21 64    	xrelease lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 21 64    	xrelease lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 21 64    	lock xacquire and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 21 64    	lock xrelease and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 39 64 	xacquire lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 39 64 	xacquire lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 39 64 	xrelease lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 39 64 	xrelease lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 0f ba 39 64 	lock xacquire btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 0f ba 39 64 	lock xrelease btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 31 64 	xacquire lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 31 64 	xacquire lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 31 64 	xrelease lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 31 64 	xrelease lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 0f ba 31 64 	lock xacquire btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 0f ba 31 64 	lock xrelease btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 29 64 	xacquire lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 29 64 	xacquire lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 29 64 	xrelease lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 29 64 	xrelease lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 0f ba 29 64 	lock xacquire bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 0f ba 29 64 	lock xrelease bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 48 c7 01 64 00 00 00 	xrelease mov QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 09 64    	xacquire lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 09 64    	xacquire lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 09 64    	xrelease lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 09 64    	xrelease lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 09 64    	lock xacquire or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 09 64    	lock xrelease or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 19 64    	xacquire lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 19 64    	xacquire lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 19 64    	xrelease lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 19 64    	xrelease lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 19 64    	lock xacquire sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 19 64    	lock xrelease sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 29 64    	xacquire lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 29 64    	xacquire lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 29 64    	xrelease lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 29 64    	xrelease lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 29 64    	lock xacquire sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 29 64    	lock xrelease sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 31 64    	xacquire lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 31 64    	xacquire lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 31 64    	xrelease lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 31 64    	xrelease lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 31 64    	lock xacquire xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 31 64    	lock xrelease xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 11 64       	lock xacquire adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 11 64       	lock xrelease adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 01 64       	lock xacquire add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 01 64       	lock xrelease add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 21 64       	lock xacquire and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 21 64       	lock xrelease and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 c6 01 64          	xrelease mov BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 09 64       	lock xacquire or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 09 64       	lock xrelease or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 19 64       	lock xacquire sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 19 64       	lock xrelease sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 29 64       	lock xacquire sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 29 64       	lock xrelease sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 31 64       	lock xacquire xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 31 64       	lock xrelease xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 10 01          	xacquire lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 10 01          	xacquire lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 10 01          	xrelease lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 10 01          	xrelease lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 10 01          	lock xacquire adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 10 01          	lock xrelease adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 00 01          	xacquire lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 00 01          	xacquire lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 00 01          	xrelease lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 00 01          	xrelease lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 00 01          	lock xacquire add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 00 01          	lock xrelease add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 20 01          	xacquire lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 20 01          	xacquire lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 20 01          	xrelease lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 20 01          	xrelease lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 20 01          	lock xacquire and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 20 01          	lock xrelease and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 88 01             	xrelease mov BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 88 04 25 78 56 34 12 	xrelease mov BYTE PTR (ds:)?0x12345678,al
[ 	]*[a-f0-9]+:	67 f3 88 04 25 21 43 65 87 	xrelease mov BYTE PTR \[eiz\*1\+0x87654321\],al
[ 	]*[a-f0-9]+:	f2 f0 08 01          	xacquire lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 08 01          	xacquire lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 08 01          	xrelease lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 08 01          	xrelease lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 08 01          	lock xacquire or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 08 01          	lock xrelease or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 18 01          	xacquire lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 18 01          	xacquire lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 18 01          	xrelease lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 18 01          	xrelease lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 18 01          	lock xacquire sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 18 01          	lock xrelease sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 28 01          	xacquire lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 28 01          	xacquire lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 28 01          	xrelease lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 28 01          	xrelease lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 28 01          	lock xacquire sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 28 01          	lock xrelease sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 86 01          	xacquire lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 86 01          	xacquire lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 86 01             	xacquire xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 86 01          	xrelease lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 86 01          	xrelease lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 86 01             	xrelease xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 86 01          	lock xacquire xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 86 01          	lock xrelease xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 30 01          	xacquire lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 30 01          	xacquire lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 30 01          	xrelease lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 30 01          	xrelease lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 30 01          	lock xacquire xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 30 01          	lock xrelease xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	66 f2 f0 11 01       	xacquire lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 11 01       	xacquire lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 11 01       	xrelease lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 11 01       	xrelease lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 11 01       	lock xacquire adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 11 01       	lock xrelease adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 01 01       	xacquire lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 01 01       	xacquire lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 01 01       	xrelease lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 01 01       	xrelease lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 01 01       	lock xacquire add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 01 01       	lock xrelease add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 21 01       	xacquire lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 21 01       	xacquire lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 21 01       	xrelease lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 21 01       	xrelease lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 21 01       	lock xacquire and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 21 01       	lock xrelease and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 89 01          	xrelease mov WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 89 04 25 78 56 34 12 	xrelease mov WORD PTR (ds:)?0x12345678,ax
[ 	]*[a-f0-9]+:	67 66 f3 89 04 25 21 43 65 87 	xrelease mov WORD PTR \[eiz\*1\+0x87654321\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 09 01       	xacquire lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 09 01       	xacquire lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 09 01       	xrelease lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 09 01       	xrelease lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 09 01       	lock xacquire or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 09 01       	lock xrelease or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 19 01       	xacquire lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 19 01       	xacquire lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 19 01       	xrelease lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 19 01       	xrelease lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 19 01       	lock xacquire sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 19 01       	lock xrelease sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 29 01       	xacquire lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 29 01       	xacquire lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 29 01       	xrelease lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 29 01       	xrelease lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 29 01       	lock xacquire sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 29 01       	lock xrelease sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 87 01       	xacquire lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 87 01       	xacquire lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 87 01          	xacquire xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 87 01       	xrelease lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 87 01       	xrelease lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 87 01          	xrelease xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 87 01       	lock xacquire xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 87 01       	lock xrelease xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 31 01       	xacquire lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 31 01       	xacquire lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 31 01       	xrelease lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 31 01       	xrelease lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 31 01       	lock xacquire xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 31 01       	lock xrelease xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f2 f0 11 01          	xacquire lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 11 01          	xacquire lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 11 01          	xrelease lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 11 01          	xrelease lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 11 01          	lock xacquire adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 11 01          	lock xrelease adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 01 01          	xacquire lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 01 01          	xacquire lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 01 01          	xrelease lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 01 01          	xrelease lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 01 01          	lock xacquire add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 01 01          	lock xrelease add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 21 01          	xacquire lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 21 01          	xacquire lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 21 01          	xrelease lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 21 01          	xrelease lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 21 01          	lock xacquire and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 21 01          	lock xrelease and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 89 01             	xrelease mov DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 89 04 25 78 56 34 12 	xrelease mov DWORD PTR (ds:)?0x12345678,eax
[ 	]*[a-f0-9]+:	67 f3 89 04 25 21 43 65 87 	xrelease mov DWORD PTR \[eiz\*1\+0x87654321\],eax
[ 	]*[a-f0-9]+:	f2 f0 09 01          	xacquire lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 09 01          	xacquire lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 09 01          	xrelease lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 09 01          	xrelease lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 09 01          	lock xacquire or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 09 01          	lock xrelease or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 19 01          	xacquire lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 19 01          	xacquire lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 19 01          	xrelease lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 19 01          	xrelease lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 19 01          	lock xacquire sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 19 01          	lock xrelease sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 29 01          	xacquire lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 29 01          	xacquire lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 29 01          	xrelease lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 29 01          	xrelease lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 29 01          	lock xacquire sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 29 01          	lock xrelease sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 87 01          	xacquire lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 87 01          	xacquire lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 87 01             	xacquire xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 87 01          	xrelease lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 87 01          	xrelease lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 87 01             	xrelease xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 87 01          	lock xacquire xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 87 01          	lock xrelease xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 31 01          	xacquire lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 31 01          	xacquire lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 31 01          	xrelease lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 31 01          	xrelease lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 31 01          	lock xacquire xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 31 01          	lock xrelease xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 48 11 01       	xacquire lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 11 01       	xacquire lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 11 01       	xrelease lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 11 01       	xrelease lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 11 01       	lock xacquire adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 11 01       	lock xrelease adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 01 01       	xacquire lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 01 01       	xacquire lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 01 01       	xrelease lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 01 01       	xrelease lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 01 01       	lock xacquire add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 01 01       	lock xrelease add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 21 01       	xacquire lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 21 01       	xacquire lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 21 01       	xrelease lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 21 01       	xrelease lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 21 01       	lock xacquire and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 21 01       	lock xrelease and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 48 89 01          	xrelease mov QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 48 89 04 25 78 56 34 12 	xrelease mov QWORD PTR (ds:)?0x12345678,rax
[ 	]*[a-f0-9]+:	67 f3 48 89 04 25 21 43 65 87 	xrelease mov QWORD PTR \[eiz\*1\+0x87654321\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 09 01       	xacquire lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 09 01       	xacquire lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 09 01       	xrelease lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 09 01       	xrelease lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 09 01       	lock xacquire or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 09 01       	lock xrelease or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 19 01       	xacquire lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 19 01       	xacquire lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 19 01       	xrelease lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 19 01       	xrelease lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 19 01       	lock xacquire sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 19 01       	lock xrelease sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 29 01       	xacquire lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 29 01       	xacquire lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 29 01       	xrelease lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 29 01       	xrelease lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 29 01       	lock xacquire sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 29 01       	lock xrelease sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 87 01       	xacquire lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 87 01       	xacquire lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 48 87 01          	xacquire xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 87 01       	xrelease lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 87 01       	xrelease lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 48 87 01          	xrelease xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 87 01       	lock xacquire xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 87 01       	lock xrelease xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 31 01       	xacquire lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 31 01       	xacquire lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 31 01       	xrelease lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 31 01       	xrelease lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 31 01       	lock xacquire xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 31 01       	lock xrelease xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	66 f2 f0 0f bb 01    	xacquire lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f bb 01    	xacquire lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f bb 01    	xrelease lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f bb 01    	xrelease lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f bb 01    	lock xacquire btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f bb 01    	lock xrelease btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b3 01    	xacquire lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b3 01    	xacquire lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b3 01    	xrelease lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b3 01    	xrelease lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f b3 01    	lock xacquire btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f b3 01    	lock xrelease btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f ab 01    	xacquire lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f ab 01    	xacquire lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f ab 01    	xrelease lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f ab 01    	xrelease lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f ab 01    	lock xacquire bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f ab 01    	lock xrelease bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b1 01    	xacquire lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b1 01    	xacquire lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b1 01    	xrelease lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b1 01    	xrelease lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f b1 01    	lock xacquire cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f b1 01    	lock xrelease cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f c1 01    	xacquire lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f c1 01    	xacquire lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f c1 01    	xrelease lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f c1 01    	xrelease lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f c1 01    	lock xacquire xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f c1 01    	lock xrelease xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f2 f0 0f bb 01       	xacquire lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f bb 01       	xacquire lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f bb 01       	xrelease lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f bb 01       	xrelease lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f bb 01       	lock xacquire btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f bb 01       	lock xrelease btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b3 01       	xacquire lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b3 01       	xacquire lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b3 01       	xrelease lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b3 01       	xrelease lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f b3 01       	lock xacquire btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f b3 01       	lock xrelease btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f ab 01       	xacquire lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f ab 01       	xacquire lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f ab 01       	xrelease lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f ab 01       	xrelease lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f ab 01       	lock xacquire bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f ab 01       	lock xrelease bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b1 01       	xacquire lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b1 01       	xacquire lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b1 01       	xrelease lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b1 01       	xrelease lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f b1 01       	lock xacquire cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f b1 01       	lock xrelease cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f c1 01       	xacquire lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f c1 01       	xacquire lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f c1 01       	xrelease lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f c1 01       	xrelease lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f c1 01       	lock xacquire xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f c1 01       	lock xrelease xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 48 0f bb 01    	xacquire lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f bb 01    	xacquire lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f bb 01    	xrelease lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f bb 01    	xrelease lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f bb 01    	lock xacquire btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f bb 01    	lock xrelease btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b3 01    	xacquire lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b3 01    	xacquire lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b3 01    	xrelease lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b3 01    	xrelease lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f b3 01    	lock xacquire btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f b3 01    	lock xrelease btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f ab 01    	xacquire lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f ab 01    	xacquire lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f ab 01    	xrelease lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f ab 01    	xrelease lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f ab 01    	lock xacquire bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f ab 01    	lock xrelease bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b1 01    	xacquire lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b1 01    	xacquire lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b1 01    	xrelease lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b1 01    	xrelease lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f b1 01    	lock xacquire cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f b1 01    	lock xrelease cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f c1 01    	xacquire lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f c1 01    	xacquire lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f c1 01    	xrelease lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f c1 01    	xrelease lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f c1 01    	lock xacquire xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f c1 01    	lock xrelease xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 fe 09          	xacquire lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 fe 09          	xacquire lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 09          	xrelease lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 09          	xrelease lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 fe 09          	lock xacquire dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 fe 09          	lock xrelease dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 fe 01          	xacquire lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 fe 01          	xacquire lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 01          	xrelease lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 01          	xrelease lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 fe 01          	lock xacquire inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 fe 01          	lock xrelease inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 19          	xacquire lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 19          	xacquire lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 19          	xrelease lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 19          	xrelease lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f6 19          	lock xacquire neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f6 19          	lock xrelease neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 11          	xacquire lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 11          	xacquire lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 11          	xrelease lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 11          	xrelease lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f6 11          	lock xacquire not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f6 11          	lock xrelease not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 09       	xacquire lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 09       	xacquire lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 09       	xrelease lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 09       	xrelease lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 ff 09       	lock xacquire dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 ff 09       	lock xrelease dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 01       	xacquire lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 01       	xacquire lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 01       	xrelease lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 01       	xrelease lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 ff 01       	lock xacquire inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 ff 01       	lock xrelease inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 19       	xacquire lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 19       	xacquire lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 19       	xrelease lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 19       	xrelease lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 f7 19       	lock xacquire neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 f7 19       	lock xrelease neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 11       	xacquire lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 11       	xacquire lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 11       	xrelease lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 11       	xrelease lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 f7 11       	lock xacquire not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 f7 11       	lock xrelease not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 09          	xacquire lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 09          	xacquire lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 09          	xrelease lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 09          	xrelease lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 ff 09          	lock xacquire dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 ff 09          	lock xrelease dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 01          	xacquire lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 01          	xacquire lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 01          	xrelease lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 01          	xrelease lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 ff 01          	lock xacquire inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 ff 01          	lock xrelease inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 19          	xacquire lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 19          	xacquire lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 19          	xrelease lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 19          	xrelease lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f7 19          	lock xacquire neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f7 19          	lock xrelease neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 11          	xacquire lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 11          	xacquire lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 11          	xrelease lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 11          	xrelease lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f7 11          	lock xacquire not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f7 11          	lock xrelease not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 09       	xacquire lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 09       	xacquire lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 09       	xrelease lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 09       	xrelease lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 ff 09       	lock xacquire dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 ff 09       	lock xrelease dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 01       	xacquire lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 01       	xacquire lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 01       	xrelease lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 01       	xrelease lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 ff 01       	lock xacquire inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 ff 01       	lock xrelease inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 19       	xacquire lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 19       	xacquire lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 19       	xrelease lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 19       	xrelease lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 f7 19       	lock xacquire neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 f7 19       	lock xrelease neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 11       	xacquire lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 11       	xacquire lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 11       	xrelease lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 11       	xrelease lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 f7 11       	lock xacquire not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 f7 11       	lock xrelease not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 0f c7 09       	xacquire lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 0f c7 09       	xacquire lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 0f c7 09       	xrelease lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 0f c7 09       	xrelease lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 0f c7 09       	lock xacquire cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 0f c7 09       	lock xrelease cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 0f b0 09       	xacquire lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 0f b0 09       	xacquire lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f b0 09       	xrelease lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f b0 09       	xrelease lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f2 0f b0 09       	lock xacquire cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f3 0f b0 09       	lock xrelease cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 0f c0 09       	xacquire lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 0f c0 09       	xacquire lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f c0 09       	xrelease lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f c0 09       	xrelease lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f2 0f c0 09       	lock xacquire xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f3 0f c0 09       	lock xrelease xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 11 64       	lock xacquire adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 11 64       	lock xrelease adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 01 64       	lock xacquire add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 01 64       	lock xrelease add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 21 64       	lock xacquire and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 21 64       	lock xrelease and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 c6 01 64          	xrelease mov BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 09 64       	lock xacquire or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 09 64       	lock xrelease or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 19 64       	lock xacquire sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 19 64       	lock xrelease sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 29 64       	lock xacquire sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 29 64       	lock xrelease sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 31 64       	lock xacquire xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 31 64       	lock xrelease xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 81 11 e8 03 	xacquire lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 11 e8 03 	xacquire lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 11 e8 03 	xrelease lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 11 e8 03 	xrelease lock adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 11 e8 03 	lock xacquire adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 11 e8 03 	lock xrelease adc WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 01 e8 03 	xacquire lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 01 e8 03 	xacquire lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 01 e8 03 	xrelease lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 01 e8 03 	xrelease lock add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 01 e8 03 	lock xacquire add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 01 e8 03 	lock xrelease add WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 21 e8 03 	xacquire lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 21 e8 03 	xacquire lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 21 e8 03 	xrelease lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 21 e8 03 	xrelease lock and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 21 e8 03 	lock xacquire and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 21 e8 03 	lock xrelease and WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 c7 01 e8 03    	xrelease mov WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 09 e8 03 	xacquire lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 09 e8 03 	xacquire lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 09 e8 03 	xrelease lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 09 e8 03 	xrelease lock or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 09 e8 03 	lock xacquire or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 09 e8 03 	lock xrelease or WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 19 e8 03 	xacquire lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 19 e8 03 	xacquire lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 19 e8 03 	xrelease lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 19 e8 03 	xrelease lock sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 19 e8 03 	lock xacquire sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 19 e8 03 	lock xrelease sbb WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 29 e8 03 	xacquire lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 29 e8 03 	xacquire lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 29 e8 03 	xrelease lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 29 e8 03 	xrelease lock sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 29 e8 03 	lock xacquire sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 29 e8 03 	lock xrelease sub WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 31 e8 03 	xacquire lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f2 f0 81 31 e8 03 	xacquire lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 31 e8 03 	xrelease lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	66 f3 f0 81 31 e8 03 	xrelease lock xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f2 66 81 31 e8 03 	lock xacquire xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f0 f3 66 81 31 e8 03 	lock xrelease xor WORD PTR \[rcx\],0x3e8
[ 	]*[a-f0-9]+:	f2 f0 81 11 80 96 98 00 	xacquire lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 11 80 96 98 00 	xacquire lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 11 80 96 98 00 	xrelease lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 11 80 96 98 00 	xrelease lock adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 11 80 96 98 00 	lock xacquire adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 11 80 96 98 00 	lock xrelease adc DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 01 80 96 98 00 	xacquire lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 01 80 96 98 00 	xacquire lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 01 80 96 98 00 	xrelease lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 01 80 96 98 00 	xrelease lock add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 01 80 96 98 00 	lock xacquire add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 01 80 96 98 00 	lock xrelease add DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 21 80 96 98 00 	xacquire lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 21 80 96 98 00 	xacquire lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 21 80 96 98 00 	xrelease lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 21 80 96 98 00 	xrelease lock and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 21 80 96 98 00 	lock xacquire and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 21 80 96 98 00 	lock xrelease and DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 c7 01 80 96 98 00 	xrelease mov DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 09 80 96 98 00 	xacquire lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 09 80 96 98 00 	xacquire lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 09 80 96 98 00 	xrelease lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 09 80 96 98 00 	xrelease lock or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 09 80 96 98 00 	lock xacquire or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 09 80 96 98 00 	lock xrelease or DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 19 80 96 98 00 	xacquire lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 19 80 96 98 00 	xacquire lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 19 80 96 98 00 	xrelease lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 19 80 96 98 00 	xrelease lock sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 19 80 96 98 00 	lock xacquire sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 19 80 96 98 00 	lock xrelease sbb DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 29 80 96 98 00 	xacquire lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 29 80 96 98 00 	xacquire lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 29 80 96 98 00 	xrelease lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 29 80 96 98 00 	xrelease lock sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 29 80 96 98 00 	lock xacquire sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 29 80 96 98 00 	lock xrelease sub DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 31 80 96 98 00 	xacquire lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 81 31 80 96 98 00 	xacquire lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 31 80 96 98 00 	xrelease lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 81 31 80 96 98 00 	xrelease lock xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 81 31 80 96 98 00 	lock xacquire xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 81 31 80 96 98 00 	lock xrelease xor DWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 11 80 96 98 00 	xacquire lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 11 80 96 98 00 	xacquire lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 11 80 96 98 00 	xrelease lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 11 80 96 98 00 	xrelease lock adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 11 80 96 98 00 	lock xacquire adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 11 80 96 98 00 	lock xrelease adc QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 01 80 96 98 00 	xacquire lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 01 80 96 98 00 	xacquire lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 01 80 96 98 00 	xrelease lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 01 80 96 98 00 	xrelease lock add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 01 80 96 98 00 	lock xacquire add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 01 80 96 98 00 	lock xrelease add QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 21 80 96 98 00 	xacquire lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 21 80 96 98 00 	xacquire lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 21 80 96 98 00 	xrelease lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 21 80 96 98 00 	xrelease lock and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 21 80 96 98 00 	lock xacquire and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 21 80 96 98 00 	lock xrelease and QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 48 c7 01 80 96 98 00 	xrelease mov QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 09 80 96 98 00 	xacquire lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 09 80 96 98 00 	xacquire lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 09 80 96 98 00 	xrelease lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 09 80 96 98 00 	xrelease lock or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 09 80 96 98 00 	lock xacquire or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 09 80 96 98 00 	lock xrelease or QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 19 80 96 98 00 	xacquire lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 19 80 96 98 00 	xacquire lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 19 80 96 98 00 	xrelease lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 19 80 96 98 00 	xrelease lock sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 19 80 96 98 00 	lock xacquire sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 19 80 96 98 00 	lock xrelease sbb QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 29 80 96 98 00 	xacquire lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 29 80 96 98 00 	xacquire lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 29 80 96 98 00 	xrelease lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 29 80 96 98 00 	xrelease lock sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 29 80 96 98 00 	lock xacquire sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 29 80 96 98 00 	lock xrelease sub QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 31 80 96 98 00 	xacquire lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f2 f0 48 81 31 80 96 98 00 	xacquire lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 31 80 96 98 00 	xrelease lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f3 f0 48 81 31 80 96 98 00 	xrelease lock xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f2 48 81 31 80 96 98 00 	lock xacquire xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	f0 f3 48 81 31 80 96 98 00 	lock xrelease xor QWORD PTR \[rcx\],0x989680
[ 	]*[a-f0-9]+:	66 f2 f0 83 11 64    	xacquire lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 11 64    	xacquire lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 11 64    	xrelease lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 11 64    	xrelease lock adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 11 64    	lock xacquire adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 11 64    	lock xrelease adc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 01 64    	xacquire lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 01 64    	xacquire lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 01 64    	xrelease lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 01 64    	xrelease lock add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 01 64    	lock xacquire add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 01 64    	lock xrelease add WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 21 64    	xacquire lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 21 64    	xacquire lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 21 64    	xrelease lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 21 64    	xrelease lock and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 21 64    	lock xacquire and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 21 64    	lock xrelease and WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 39 64 	xacquire lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 39 64 	xacquire lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 39 64 	xrelease lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 39 64 	xrelease lock btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 0f ba 39 64 	lock xacquire btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 0f ba 39 64 	lock xrelease btc WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 31 64 	xacquire lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 31 64 	xacquire lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 31 64 	xrelease lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 31 64 	xrelease lock btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 0f ba 31 64 	lock xacquire btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 0f ba 31 64 	lock xrelease btr WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 29 64 	xacquire lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 0f ba 29 64 	xacquire lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 29 64 	xrelease lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 0f ba 29 64 	xrelease lock bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 0f ba 29 64 	lock xacquire bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 0f ba 29 64 	lock xrelease bts WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 c7 01 64 00    	xrelease mov WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 09 64    	xacquire lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 09 64    	xacquire lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 09 64    	xrelease lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 09 64    	xrelease lock or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 09 64    	lock xacquire or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 09 64    	lock xrelease or WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 19 64    	xacquire lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 19 64    	xacquire lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 19 64    	xrelease lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 19 64    	xrelease lock sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 19 64    	lock xacquire sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 19 64    	lock xrelease sbb WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 29 64    	xacquire lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 29 64    	xacquire lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 29 64    	xrelease lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 29 64    	xrelease lock sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 29 64    	lock xacquire sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 29 64    	lock xrelease sub WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 31 64    	xacquire lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f2 f0 83 31 64    	xacquire lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 31 64    	xrelease lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	66 f3 f0 83 31 64    	xrelease lock xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 66 83 31 64    	lock xacquire xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 66 83 31 64    	lock xrelease xor WORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 11 64       	xacquire lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 11 64       	xacquire lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 11 64       	xrelease lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 11 64       	xrelease lock adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 11 64       	lock xacquire adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 11 64       	lock xrelease adc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 01 64       	xacquire lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 01 64       	xacquire lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 01 64       	xrelease lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 01 64       	xrelease lock add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 01 64       	lock xacquire add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 01 64       	lock xrelease add DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 21 64       	xacquire lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 21 64       	xacquire lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 21 64       	xrelease lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 21 64       	xrelease lock and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 21 64       	lock xacquire and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 21 64       	lock xrelease and DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 39 64    	xacquire lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 39 64    	xacquire lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 39 64    	xrelease lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 39 64    	xrelease lock btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 0f ba 39 64    	lock xacquire btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 0f ba 39 64    	lock xrelease btc DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 31 64    	xacquire lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 31 64    	xacquire lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 31 64    	xrelease lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 31 64    	xrelease lock btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 0f ba 31 64    	lock xacquire btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 0f ba 31 64    	lock xrelease btr DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 29 64    	xacquire lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 0f ba 29 64    	xacquire lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 29 64    	xrelease lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 0f ba 29 64    	xrelease lock bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 0f ba 29 64    	lock xacquire bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 0f ba 29 64    	lock xrelease bts DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 c7 01 64 00 00 00 	xrelease mov DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 09 64       	xacquire lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 09 64       	xacquire lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 09 64       	xrelease lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 09 64       	xrelease lock or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 09 64       	lock xacquire or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 09 64       	lock xrelease or DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 19 64       	xacquire lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 19 64       	xacquire lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 19 64       	xrelease lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 19 64       	xrelease lock sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 19 64       	lock xacquire sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 19 64       	lock xrelease sbb DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 29 64       	xacquire lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 29 64       	xacquire lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 29 64       	xrelease lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 29 64       	xrelease lock sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 29 64       	lock xacquire sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 29 64       	lock xrelease sub DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 31 64       	xacquire lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 83 31 64       	xacquire lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 31 64       	xrelease lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 83 31 64       	xrelease lock xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 83 31 64       	lock xacquire xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 83 31 64       	lock xrelease xor DWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 11 64    	xacquire lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 11 64    	xacquire lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 11 64    	xrelease lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 11 64    	xrelease lock adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 11 64    	lock xacquire adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 11 64    	lock xrelease adc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 01 64    	xacquire lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 01 64    	xacquire lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 01 64    	xrelease lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 01 64    	xrelease lock add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 01 64    	lock xacquire add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 01 64    	lock xrelease add QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 21 64    	xacquire lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 21 64    	xacquire lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 21 64    	xrelease lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 21 64    	xrelease lock and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 21 64    	lock xacquire and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 21 64    	lock xrelease and QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 39 64 	xacquire lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 39 64 	xacquire lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 39 64 	xrelease lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 39 64 	xrelease lock btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 0f ba 39 64 	lock xacquire btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 0f ba 39 64 	lock xrelease btc QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 31 64 	xacquire lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 31 64 	xacquire lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 31 64 	xrelease lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 31 64 	xrelease lock btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 0f ba 31 64 	lock xacquire btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 0f ba 31 64 	lock xrelease btr QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 29 64 	xacquire lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 0f ba 29 64 	xacquire lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 29 64 	xrelease lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 0f ba 29 64 	xrelease lock bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 0f ba 29 64 	lock xacquire bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 0f ba 29 64 	lock xrelease bts QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 48 c7 01 64 00 00 00 	xrelease mov QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 09 64    	xacquire lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 09 64    	xacquire lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 09 64    	xrelease lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 09 64    	xrelease lock or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 09 64    	lock xacquire or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 09 64    	lock xrelease or QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 19 64    	xacquire lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 19 64    	xacquire lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 19 64    	xrelease lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 19 64    	xrelease lock sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 19 64    	lock xacquire sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 19 64    	lock xrelease sbb QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 29 64    	xacquire lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 29 64    	xacquire lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 29 64    	xrelease lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 29 64    	xrelease lock sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 29 64    	lock xacquire sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 29 64    	lock xrelease sub QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 31 64    	xacquire lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 48 83 31 64    	xacquire lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 31 64    	xrelease lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 48 83 31 64    	xrelease lock xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 48 83 31 64    	lock xacquire xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 48 83 31 64    	lock xrelease xor QWORD PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 11 64       	xacquire lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 11 64       	xrelease lock adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 11 64       	lock xacquire adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 11 64       	lock xrelease adc BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 01 64       	xacquire lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 01 64       	xrelease lock add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 01 64       	lock xacquire add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 01 64       	lock xrelease add BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 21 64       	xacquire lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 21 64       	xrelease lock and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 21 64       	lock xacquire and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 21 64       	lock xrelease and BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 c6 01 64          	xrelease mov BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 09 64       	xacquire lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 09 64       	xrelease lock or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 09 64       	lock xacquire or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 09 64       	lock xrelease or BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 19 64       	xacquire lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 19 64       	xrelease lock sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 19 64       	lock xacquire sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 19 64       	lock xrelease sbb BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 29 64       	xacquire lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 29 64       	xrelease lock sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 29 64       	lock xacquire sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 29 64       	lock xrelease sub BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 80 31 64       	xacquire lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f3 f0 80 31 64       	xrelease lock xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f2 80 31 64       	lock xacquire xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f0 f3 80 31 64       	lock xrelease xor BYTE PTR \[rcx\],0x64
[ 	]*[a-f0-9]+:	f2 f0 10 01          	xacquire lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 10 01          	xacquire lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 10 01          	xrelease lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 10 01          	xrelease lock adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 10 01          	lock xacquire adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 10 01          	lock xrelease adc BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 00 01          	xacquire lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 00 01          	xacquire lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 00 01          	xrelease lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 00 01          	xrelease lock add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 00 01          	lock xacquire add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 00 01          	lock xrelease add BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 20 01          	xacquire lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 20 01          	xacquire lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 20 01          	xrelease lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 20 01          	xrelease lock and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 20 01          	lock xacquire and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 20 01          	lock xrelease and BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 88 01             	xrelease mov BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 08 01          	xacquire lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 08 01          	xacquire lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 08 01          	xrelease lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 08 01          	xrelease lock or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 08 01          	lock xacquire or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 08 01          	lock xrelease or BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 18 01          	xacquire lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 18 01          	xacquire lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 18 01          	xrelease lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 18 01          	xrelease lock sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 18 01          	lock xacquire sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 18 01          	lock xrelease sbb BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 28 01          	xacquire lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 28 01          	xacquire lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 28 01          	xrelease lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 28 01          	xrelease lock sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 28 01          	lock xacquire sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 28 01          	lock xrelease sub BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 86 01          	xacquire lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 86 01          	xacquire lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 86 01             	xacquire xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 86 01          	xrelease lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 86 01          	xrelease lock xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 86 01             	xrelease xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 86 01          	lock xacquire xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 86 01          	lock xrelease xchg BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 30 01          	xacquire lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f2 f0 30 01          	xacquire lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 30 01          	xrelease lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f3 f0 30 01          	xrelease lock xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f2 30 01          	lock xacquire xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	f0 f3 30 01          	lock xrelease xor BYTE PTR \[rcx\],al
[ 	]*[a-f0-9]+:	66 f2 f0 11 01       	xacquire lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 11 01       	xacquire lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 11 01       	xrelease lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 11 01       	xrelease lock adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 11 01       	lock xacquire adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 11 01       	lock xrelease adc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 01 01       	xacquire lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 01 01       	xacquire lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 01 01       	xrelease lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 01 01       	xrelease lock add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 01 01       	lock xacquire add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 01 01       	lock xrelease add WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 21 01       	xacquire lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 21 01       	xacquire lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 21 01       	xrelease lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 21 01       	xrelease lock and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 21 01       	lock xacquire and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 21 01       	lock xrelease and WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 89 01          	xrelease mov WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 09 01       	xacquire lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 09 01       	xacquire lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 09 01       	xrelease lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 09 01       	xrelease lock or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 09 01       	lock xacquire or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 09 01       	lock xrelease or WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 19 01       	xacquire lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 19 01       	xacquire lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 19 01       	xrelease lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 19 01       	xrelease lock sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 19 01       	lock xacquire sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 19 01       	lock xrelease sbb WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 29 01       	xacquire lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 29 01       	xacquire lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 29 01       	xrelease lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 29 01       	xrelease lock sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 29 01       	lock xacquire sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 29 01       	lock xrelease sub WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 87 01       	xacquire lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 87 01       	xacquire lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 87 01          	xacquire xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 87 01       	xrelease lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 87 01       	xrelease lock xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 87 01          	xrelease xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 87 01       	lock xacquire xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 87 01       	lock xrelease xchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 31 01       	xacquire lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 31 01       	xacquire lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 31 01       	xrelease lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 31 01       	xrelease lock xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 31 01       	lock xacquire xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 31 01       	lock xrelease xor WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f2 f0 11 01          	xacquire lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 11 01          	xacquire lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 11 01          	xrelease lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 11 01          	xrelease lock adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 11 01          	lock xacquire adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 11 01          	lock xrelease adc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 01 01          	xacquire lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 01 01          	xacquire lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 01 01          	xrelease lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 01 01          	xrelease lock add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 01 01          	lock xacquire add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 01 01          	lock xrelease add DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 21 01          	xacquire lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 21 01          	xacquire lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 21 01          	xrelease lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 21 01          	xrelease lock and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 21 01          	lock xacquire and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 21 01          	lock xrelease and DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 89 01             	xrelease mov DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 09 01          	xacquire lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 09 01          	xacquire lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 09 01          	xrelease lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 09 01          	xrelease lock or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 09 01          	lock xacquire or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 09 01          	lock xrelease or DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 19 01          	xacquire lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 19 01          	xacquire lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 19 01          	xrelease lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 19 01          	xrelease lock sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 19 01          	lock xacquire sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 19 01          	lock xrelease sbb DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 29 01          	xacquire lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 29 01          	xacquire lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 29 01          	xrelease lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 29 01          	xrelease lock sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 29 01          	lock xacquire sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 29 01          	lock xrelease sub DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 87 01          	xacquire lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 87 01          	xacquire lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 87 01             	xacquire xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 87 01          	xrelease lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 87 01          	xrelease lock xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 87 01             	xrelease xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 87 01          	lock xacquire xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 87 01          	lock xrelease xchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 31 01          	xacquire lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 31 01          	xacquire lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 31 01          	xrelease lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 31 01          	xrelease lock xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 31 01          	lock xacquire xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 31 01          	lock xrelease xor DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 48 11 01       	xacquire lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 11 01       	xacquire lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 11 01       	xrelease lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 11 01       	xrelease lock adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 11 01       	lock xacquire adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 11 01       	lock xrelease adc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 01 01       	xacquire lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 01 01       	xacquire lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 01 01       	xrelease lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 01 01       	xrelease lock add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 01 01       	lock xacquire add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 01 01       	lock xrelease add QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 21 01       	xacquire lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 21 01       	xacquire lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 21 01       	xrelease lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 21 01       	xrelease lock and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 21 01       	lock xacquire and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 21 01       	lock xrelease and QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 48 89 01          	xrelease mov QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 09 01       	xacquire lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 09 01       	xacquire lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 09 01       	xrelease lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 09 01       	xrelease lock or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 09 01       	lock xacquire or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 09 01       	lock xrelease or QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 19 01       	xacquire lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 19 01       	xacquire lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 19 01       	xrelease lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 19 01       	xrelease lock sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 19 01       	lock xacquire sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 19 01       	lock xrelease sbb QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 29 01       	xacquire lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 29 01       	xacquire lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 29 01       	xrelease lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 29 01       	xrelease lock sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 29 01       	lock xacquire sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 29 01       	lock xrelease sub QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 87 01       	xacquire lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 87 01       	xacquire lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 48 87 01          	xacquire xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 87 01       	xrelease lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 87 01       	xrelease lock xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 48 87 01          	xrelease xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 87 01       	lock xacquire xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 87 01       	lock xrelease xchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 31 01       	xacquire lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 31 01       	xacquire lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 31 01       	xrelease lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 31 01       	xrelease lock xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 31 01       	lock xacquire xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 31 01       	lock xrelease xor QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	66 f2 f0 0f bb 01    	xacquire lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f bb 01    	xacquire lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f bb 01    	xrelease lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f bb 01    	xrelease lock btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f bb 01    	lock xacquire btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f bb 01    	lock xrelease btc WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b3 01    	xacquire lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b3 01    	xacquire lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b3 01    	xrelease lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b3 01    	xrelease lock btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f b3 01    	lock xacquire btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f b3 01    	lock xrelease btr WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f ab 01    	xacquire lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f ab 01    	xacquire lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f ab 01    	xrelease lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f ab 01    	xrelease lock bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f ab 01    	lock xacquire bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f ab 01    	lock xrelease bts WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b1 01    	xacquire lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f b1 01    	xacquire lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b1 01    	xrelease lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f b1 01    	xrelease lock cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f b1 01    	lock xacquire cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f b1 01    	lock xrelease cmpxchg WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f c1 01    	xacquire lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f2 f0 0f c1 01    	xacquire lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f c1 01    	xrelease lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	66 f3 f0 0f c1 01    	xrelease lock xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f2 66 0f c1 01    	lock xacquire xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f0 f3 66 0f c1 01    	lock xrelease xadd WORD PTR \[rcx\],ax
[ 	]*[a-f0-9]+:	f2 f0 0f bb 01       	xacquire lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f bb 01       	xacquire lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f bb 01       	xrelease lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f bb 01       	xrelease lock btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f bb 01       	lock xacquire btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f bb 01       	lock xrelease btc DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b3 01       	xacquire lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b3 01       	xacquire lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b3 01       	xrelease lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b3 01       	xrelease lock btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f b3 01       	lock xacquire btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f b3 01       	lock xrelease btr DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f ab 01       	xacquire lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f ab 01       	xacquire lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f ab 01       	xrelease lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f ab 01       	xrelease lock bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f ab 01       	lock xacquire bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f ab 01       	lock xrelease bts DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b1 01       	xacquire lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f b1 01       	xacquire lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b1 01       	xrelease lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f b1 01       	xrelease lock cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f b1 01       	lock xacquire cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f b1 01       	lock xrelease cmpxchg DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f c1 01       	xacquire lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 0f c1 01       	xacquire lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f c1 01       	xrelease lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f3 f0 0f c1 01       	xrelease lock xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f2 0f c1 01       	lock xacquire xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f0 f3 0f c1 01       	lock xrelease xadd DWORD PTR \[rcx\],eax
[ 	]*[a-f0-9]+:	f2 f0 48 0f bb 01    	xacquire lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f bb 01    	xacquire lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f bb 01    	xrelease lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f bb 01    	xrelease lock btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f bb 01    	lock xacquire btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f bb 01    	lock xrelease btc QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b3 01    	xacquire lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b3 01    	xacquire lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b3 01    	xrelease lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b3 01    	xrelease lock btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f b3 01    	lock xacquire btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f b3 01    	lock xrelease btr QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f ab 01    	xacquire lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f ab 01    	xacquire lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f ab 01    	xrelease lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f ab 01    	xrelease lock bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f ab 01    	lock xacquire bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f ab 01    	lock xrelease bts QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b1 01    	xacquire lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f b1 01    	xacquire lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b1 01    	xrelease lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f b1 01    	xrelease lock cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f b1 01    	lock xacquire cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f b1 01    	lock xrelease cmpxchg QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f c1 01    	xacquire lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 48 0f c1 01    	xacquire lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f c1 01    	xrelease lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f3 f0 48 0f c1 01    	xrelease lock xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f2 48 0f c1 01    	lock xacquire xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f0 f3 48 0f c1 01    	lock xrelease xadd QWORD PTR \[rcx\],rax
[ 	]*[a-f0-9]+:	f2 f0 fe 09          	xacquire lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 fe 09          	xacquire lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 09          	xrelease lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 09          	xrelease lock dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 fe 09          	lock xacquire dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 fe 09          	lock xrelease dec BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 fe 01          	xacquire lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 fe 01          	xacquire lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 01          	xrelease lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 fe 01          	xrelease lock inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 fe 01          	lock xacquire inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 fe 01          	lock xrelease inc BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 19          	xacquire lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 19          	xacquire lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 19          	xrelease lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 19          	xrelease lock neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f6 19          	lock xacquire neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f6 19          	lock xrelease neg BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 11          	xacquire lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f6 11          	xacquire lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 11          	xrelease lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f6 11          	xrelease lock not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f6 11          	lock xacquire not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f6 11          	lock xrelease not BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 09       	xacquire lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 09       	xacquire lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 09       	xrelease lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 09       	xrelease lock dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 ff 09       	lock xacquire dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 ff 09       	lock xrelease dec WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 01       	xacquire lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 ff 01       	xacquire lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 01       	xrelease lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 ff 01       	xrelease lock inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 ff 01       	lock xacquire inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 ff 01       	lock xrelease inc WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 19       	xacquire lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 19       	xacquire lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 19       	xrelease lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 19       	xrelease lock neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 f7 19       	lock xacquire neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 f7 19       	lock xrelease neg WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 11       	xacquire lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f2 f0 f7 11       	xacquire lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 11       	xrelease lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	66 f3 f0 f7 11       	xrelease lock not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 66 f7 11       	lock xacquire not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 66 f7 11       	lock xrelease not WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 09          	xacquire lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 09          	xacquire lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 09          	xrelease lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 09          	xrelease lock dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 ff 09          	lock xacquire dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 ff 09          	lock xrelease dec DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 01          	xacquire lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 ff 01          	xacquire lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 01          	xrelease lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 ff 01          	xrelease lock inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 ff 01          	lock xacquire inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 ff 01          	lock xrelease inc DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 19          	xacquire lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 19          	xacquire lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 19          	xrelease lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 19          	xrelease lock neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f7 19          	lock xacquire neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f7 19          	lock xrelease neg DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 11          	xacquire lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 f7 11          	xacquire lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 11          	xrelease lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 f7 11          	xrelease lock not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 f7 11          	lock xacquire not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 f7 11          	lock xrelease not DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 09       	xacquire lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 09       	xacquire lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 09       	xrelease lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 09       	xrelease lock dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 ff 09       	lock xacquire dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 ff 09       	lock xrelease dec QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 01       	xacquire lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 ff 01       	xacquire lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 01       	xrelease lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 ff 01       	xrelease lock inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 ff 01       	lock xacquire inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 ff 01       	lock xrelease inc QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 19       	xacquire lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 19       	xacquire lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 19       	xrelease lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 19       	xrelease lock neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 f7 19       	lock xacquire neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 f7 19       	lock xrelease neg QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 11       	xacquire lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 48 f7 11       	xacquire lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 11       	xrelease lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 48 f7 11       	xrelease lock not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 48 f7 11       	lock xacquire not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 48 f7 11       	lock xrelease not QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 0f c7 09       	xacquire lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 0f c7 09       	xacquire lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 0f c7 09       	xrelease lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f3 f0 0f c7 09       	xrelease lock cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f2 0f c7 09       	lock xacquire cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f0 f3 0f c7 09       	lock xrelease cmpxchg8b QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:	f2 f0 0f b0 09       	xacquire lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 0f b0 09       	xacquire lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f b0 09       	xrelease lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f b0 09       	xrelease lock cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f2 0f b0 09       	lock xacquire cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f3 0f b0 09       	lock xrelease cmpxchg BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 0f c0 09       	xacquire lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f2 f0 0f c0 09       	xacquire lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f c0 09       	xrelease lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f3 f0 0f c0 09       	xrelease lock xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f2 0f c0 09       	lock xacquire xadd BYTE PTR \[rcx\],cl
[ 	]*[a-f0-9]+:	f0 f3 0f c0 09       	lock xrelease xadd BYTE PTR \[rcx\],cl
#pass
