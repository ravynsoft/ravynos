#objdump: -dwMintel
#name: i386 lockable insns (Intel disassembly)
#source: lock-1.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	f0 01 03             	lock add DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 03 64          	lock add DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 11 03             	lock adc DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 13 64          	lock adc DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 21 03             	lock and DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 23 64          	lock and DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f bb 03          	lock btc DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 3b 64       	lock btc DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b3 03          	lock btr DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 33 64       	lock btr DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f ab 03          	lock bts DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 2b 64       	lock bts DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b1 03          	lock cmpxchg DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f c7 0b          	lock cmpxchg8b QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 ff 0b             	lock dec DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 ff 03             	lock inc DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 f7 1b             	lock neg DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 f7 13             	lock not DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 09 03             	lock or DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 0b 64          	lock or DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 19 03             	lock sbb DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 1b 64          	lock sbb DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 29 03             	lock sub DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 2b 64          	lock sub DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f c1 03          	lock xadd DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 31 03             	lock xor DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 33 64          	lock xor DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 01 03             	lock add DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 03 64          	lock add DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 11 03             	lock adc DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 13 64          	lock adc DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 21 03             	lock and DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 23 64          	lock and DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f bb 03          	lock btc DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 3b 64       	lock btc DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b3 03          	lock btr DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 33 64       	lock btr DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f ab 03          	lock bts DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f ba 2b 64       	lock bts DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f b1 03          	lock cmpxchg DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 0f c7 0b          	lock cmpxchg8b QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 ff 0b             	lock dec DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 ff 03             	lock inc DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 f7 1b             	lock neg DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 f7 13             	lock not DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	f0 09 03             	lock or DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 0b 64          	lock or DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 19 03             	lock sbb DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 1b 64          	lock sbb DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 29 03             	lock sub DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 2b 64          	lock sub DWORD PTR \[ebx\],0x64
[ 	]*[a-f0-9]+:	f0 0f c1 03          	lock xadd DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 87 03             	lock xchg DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 31 03             	lock xor DWORD PTR \[ebx\],eax
[ 	]*[a-f0-9]+:	f0 83 33 64          	lock xor DWORD PTR \[ebx\],0x64
#pass
