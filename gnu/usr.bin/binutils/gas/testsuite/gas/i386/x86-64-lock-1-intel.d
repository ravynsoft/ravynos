#objdump: -dwMintel
#name: x86-64 lockable insns (Intel disassembly)
#source: x86-64-lock-1.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f0 01 03             	lock add DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 03 64          	lock add DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 11 03             	lock adc DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 13 64          	lock adc DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 21 03             	lock and DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 23 64          	lock and DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f bb 03          	lock btc DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 3b 64       	lock btc DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b3 03          	lock btr DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 33 64       	lock btr DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f ab 03          	lock bts DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 2b 64       	lock bts DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b1 03          	lock cmpxchg DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f c7 0b          	lock cmpxchg8b QWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 48 0f c7 0b       	lock cmpxchg16b OWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 ff 0b             	lock dec DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 ff 03             	lock inc DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 f7 1b             	lock neg DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 f7 13             	lock not DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 09 03             	lock or DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 0b 64          	lock or DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 19 03             	lock sbb DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 1b 64          	lock sbb DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 29 03             	lock sub DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 2b 64          	lock sub DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f c1 03          	lock xadd DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 31 03             	lock xor DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 33 64          	lock xor DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 01 03             	lock add DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 03 64          	lock add DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 11 03             	lock adc DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 13 64          	lock adc DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 21 03             	lock and DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 23 64          	lock and DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f bb 03          	lock btc DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 3b 64       	lock btc DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b3 03          	lock btr DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 33 64       	lock btr DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f ab 03          	lock bts DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 2b 64       	lock bts DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b1 03          	lock cmpxchg DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 0f c7 0b          	lock cmpxchg8b QWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 48 0f c7 0b       	lock cmpxchg16b OWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 ff 0b             	lock dec DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 ff 03             	lock inc DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 f7 1b             	lock neg DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 f7 13             	lock not DWORD PTR \[rbx\]
[ 	]*[a-f0-9]+:	f0 09 03             	lock or DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 0b 64          	lock or DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 19 03             	lock sbb DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 1b 64          	lock sbb DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 29 03             	lock sub DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 2b 64          	lock sub DWORD PTR \[rbx\],0x64
[ 	]*[a-f0-9]+:	f0 0f c1 03          	lock xadd DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 31 03             	lock xor DWORD PTR \[rbx\],eax
[ 	]*[a-f0-9]+:	f0 83 33 64          	lock xor DWORD PTR \[rbx\],0x64
#pass
