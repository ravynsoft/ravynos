.*: Assembler messages:
.*:5: Warning: instruction length.*
.*:6: Warning: instruction length.*
.*:7: Warning: instruction length.*
.*:9: Warning: instruction length.*
.*:10: Warning: instruction length.*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.intel_syntax noprefix
[ 	]*3[ 	]+
[ 	]*4[ 	]+long64:
[ 	]*5[ 	]+0000 64678FEA[ 	]+lwpins	rax, fs:\[eax\*8\], 0x11223344
\*\*\*\*  Warning: instruction length of 16 bytes exceeds the limit of 15
[ 	]*5[ 	]+F81204C5[ 	]*
[ 	]*5[ 	]+00000000[ 	]*
[ 	]*5[ 	]+44332211[ 	]*
[ 	]*6[ 	]+0010 64678FEA[ 	]+lwpval	eax, fs:\[eax\*4\], 0x11223344
\*\*\*\*  Warning: instruction length of 16 bytes exceeds the limit of 15
[ 	]*6[ 	]+78120C85[ 	]*
[ 	]*6[ 	]+00000000[ 	]*
[ 	]*6[ 	]+44332211[ 	]*
[ 	]*7[ 	]+0020 64678FEA[ 	]+bextr	rax, fs:\[eax\*2\], 0x11223344
\*\*\*\*  Warning: instruction length of 16 bytes exceeds the limit of 15
[ 	]*7[ 	]+F8100445[ 	]*
[ 	]*7[ 	]+00000000[ 	]*
[ 	]*7[ 	]+44332211[ 	]*
[ 	]*8[ 	]+
[ 	]*9[ 	]+0030 6567F2F0[ 	]+xacquire lock add qword ptr gs:\[eax\*8\], 0x11223344
\*\*\*\*  Warning: instruction length of 16 bytes exceeds the limit of 15
[ 	]*9[ 	]+488104C5[ 	]*
[ 	]*9[ 	]+00000000[ 	]*
[ 	]*9[ 	]+44332211[ 	]*
[ 	]*10[ 	]+0040 6567F3F0[ 	]+xrelease lock sub qword ptr gs:\[eax\*8\], 0x11223344
\*\*\*\*  Warning: instruction length of 16 bytes exceeds the limit of 15
[ 	]*10[ 	]+48812CC5[ 	]*
[ 	]*10[ 	]+00000000[ 	]*
[ 	]*10[ 	]+44332211[ 	]*
#pass
