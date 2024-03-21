#as:
#objdump: -dw -Mintel
#name: i386 AVX512VBMI2 insns (Intel disassembly)
#source: avx512vbmi2.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 31[ 	]*vpcompressb ZMMWORD PTR \[ecx\]\{k7\},zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb ZMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 72 7e[ 	]*vpcompressb ZMMWORD PTR \[edx\+0x7e\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 ee[ 	]*vpcompressb zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 ee[ 	]*vpcompressb zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 63 ee[ 	]*vpcompressb zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 31[ 	]*vpcompressw ZMMWORD PTR \[ecx\]\{k7\},zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw ZMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 72 40[ 	]*vpcompressw ZMMWORD PTR \[edx\+0x80\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 ee[ 	]*vpcompressw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 ee[ 	]*vpcompressw zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 63 ee[ 	]*vpcompressw zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 31[ 	]*vpexpandb zmm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 31[ 	]*vpexpandb zmm6\{k7\}\{z\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 72 7e[ 	]*vpexpandb zmm6,ZMMWORD PTR \[edx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 f5[ 	]*vpexpandb zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 f5[ 	]*vpexpandb zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 f5[ 	]*vpexpandb zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 31[ 	]*vpexpandw zmm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 31[ 	]*vpexpandw zmm6\{k7\}\{z\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 72 40[ 	]*vpexpandw zmm6,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 f5[ 	]*vpexpandw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 f5[ 	]*vpexpandw zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 f5[ 	]*vpexpandw zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 f4[ 	]*vpshldvw zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 70 f4[ 	]*vpshldvw zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 70 f4[ 	]*vpshldvw zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 72 02[ 	]*vpshldvw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 f4[ 	]*vpshldvd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 71 f4[ 	]*vpshldvd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 71 f4[ 	]*vpshldvd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 72 02[ 	]*vpshldvd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 71 72 7f[ 	]*vpshldvd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 f4[ 	]*vpshldvq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 71 f4[ 	]*vpshldvq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 71 f4[ 	]*vpshldvq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 72 02[ 	]*vpshldvq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 71 72 7f[ 	]*vpshldvq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 f4[ 	]*vpshrdvw zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 72 f4[ 	]*vpshrdvw zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 72 f4[ 	]*vpshrdvw zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 72 02[ 	]*vpshrdvw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 f4[ 	]*vpshrdvd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 73 f4[ 	]*vpshrdvd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 73 f4[ 	]*vpshrdvd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 72 02[ 	]*vpshrdvd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 73 72 7f[ 	]*vpshrdvd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 f4[ 	]*vpshrdvq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 73 f4[ 	]*vpshrdvq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 73 f4[ 	]*vpshrdvq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 72 02[ 	]*vpshrdvq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 73 72 7f[ 	]*vpshrdvq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 70 f4 ab[ 	]*vpshldw zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 70 f4 ab[ 	]*vpshldw zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 f4 7b[ 	]*vpshldw zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 72 02 7b[ 	]*vpshldw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 71 f4 ab[ 	]*vpshldd zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 71 f4 ab[ 	]*vpshldd zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 f4 7b[ 	]*vpshldd zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 72 02 7b[ 	]*vpshldd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 71 72 7f 7b[ 	]*vpshldd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 71 f4 ab[ 	]*vpshldq zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 71 f4 ab[ 	]*vpshldq zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 72 02 7b[ 	]*vpshldq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 71 72 7f 7b[ 	]*vpshldq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 f4 ab[ 	]*vpshrdw zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 72 f4 ab[ 	]*vpshrdw zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 72 f4 ab[ 	]*vpshrdw zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 72 02 7b[ 	]*vpshrdw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 f4 ab[ 	]*vpshrdd zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 73 f4 ab[ 	]*vpshrdd zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 73 f4 ab[ 	]*vpshrdd zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 72 02 7b[ 	]*vpshrdd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 73 72 7f 7b[ 	]*vpshrdd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 73 f4 ab[ 	]*vpshrdq zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 73 f4 ab[ 	]*vpshrdq zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 f4 7b[ 	]*vpshrdq zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 72 02 7b[ 	]*vpshrdq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 73 72 7f 7b[ 	]*vpshrdq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 31[ 	]*vpcompressb ZMMWORD PTR \[ecx\]\{k7\},zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressb ZMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 72 7e[ 	]*vpcompressb ZMMWORD PTR \[edx\+0x7e\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 63 ee[ 	]*vpcompressb zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 63 ee[ 	]*vpcompressb zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 63 ee[ 	]*vpcompressb zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 31[ 	]*vpcompressw ZMMWORD PTR \[ecx\]\{k7\},zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 b4 f4 c0 1d fe ff[ 	]*vpcompressw ZMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 72 40[ 	]*vpcompressw ZMMWORD PTR \[edx\+0x80\],zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 63 ee[ 	]*vpcompressw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 63 ee[ 	]*vpcompressw zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 63 ee[ 	]*vpcompressw zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 31[ 	]*vpexpandb zmm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 31[ 	]*vpexpandb zmm6\{k7\}\{z\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandb zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 72 7e[ 	]*vpexpandb zmm6,ZMMWORD PTR \[edx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 62 f5[ 	]*vpexpandb zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 62 f5[ 	]*vpexpandb zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 62 f5[ 	]*vpexpandb zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 31[ 	]*vpexpandw zmm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 31[ 	]*vpexpandw zmm6\{k7\}\{z\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 b4 f4 c0 1d fe ff[ 	]*vpexpandw zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 72 40[ 	]*vpexpandw zmm6,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 62 f5[ 	]*vpexpandw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 62 f5[ 	]*vpexpandw zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 62 f5[ 	]*vpexpandw zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 f4[ 	]*vpshldvw zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 70 f4[ 	]*vpshldvw zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 70 f4[ 	]*vpshldvw zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 b4 f4 c0 1d fe ff[ 	]*vpshldvw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 70 72 02[ 	]*vpshldvw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 f4[ 	]*vpshldvd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 71 f4[ 	]*vpshldvd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 71 f4[ 	]*vpshldvd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 71 72 02[ 	]*vpshldvd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 71 72 7f[ 	]*vpshldvd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 f4[ 	]*vpshldvq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 71 f4[ 	]*vpshldvq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 71 f4[ 	]*vpshldvq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 b4 f4 c0 1d fe ff[ 	]*vpshldvq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 71 72 02[ 	]*vpshldvq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 71 72 7f[ 	]*vpshldvq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 f4[ 	]*vpshrdvw zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 72 f4[ 	]*vpshrdvw zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 72 f4[ 	]*vpshrdvw zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 b4 f4 c0 1d fe ff[ 	]*vpshrdvw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 72 72 02[ 	]*vpshrdvw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 f4[ 	]*vpshrdvd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f 73 f4[ 	]*vpshrdvd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf 73 f4[ 	]*vpshrdvd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 73 72 02[ 	]*vpshrdvd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 58 73 72 7f[ 	]*vpshrdvd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 f4[ 	]*vpshrdvq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 73 f4[ 	]*vpshrdvq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 73 f4[ 	]*vpshrdvq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 b4 f4 c0 1d fe ff[ 	]*vpshrdvq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 73 72 02[ 	]*vpshrdvq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 73 72 7f[ 	]*vpshrdvq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 f4 ab[ 	]*vpshldw zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 70 f4 ab[ 	]*vpshldw zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 70 f4 ab[ 	]*vpshldw zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshldw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 70 72 02 7b[ 	]*vpshldw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 f4 ab[ 	]*vpshldd zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 71 f4 ab[ 	]*vpshldd zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 71 f4 ab[ 	]*vpshldd zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 71 72 02 7b[ 	]*vpshldd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 71 72 7f 7b[ 	]*vpshldd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 f4 ab[ 	]*vpshldq zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 71 f4 ab[ 	]*vpshldq zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 71 f4 ab[ 	]*vpshldq zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 b4 f4 c0 1d fe ff 7b[ 	]*vpshldq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 71 72 02 7b[ 	]*vpshldq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 71 72 7f 7b[ 	]*vpshldq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 f4 ab[ 	]*vpshrdw zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 72 f4 ab[ 	]*vpshrdw zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 72 f4 ab[ 	]*vpshrdw zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdw zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 72 72 02 7b[ 	]*vpshrdw zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 f4 ab[ 	]*vpshrdd zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 73 f4 ab[ 	]*vpshrdd zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 73 f4 ab[ 	]*vpshrdd zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 73 72 02 7b[ 	]*vpshrdd zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 73 72 7f 7b[ 	]*vpshrdd zmm6,zmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 f4 ab[ 	]*vpshrdq zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 73 f4 ab[ 	]*vpshrdq zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 73 f4 ab[ 	]*vpshrdq zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 b4 f4 c0 1d fe ff 7b[ 	]*vpshrdq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 73 72 02 7b[ 	]*vpshrdq zmm6,zmm5,ZMMWORD PTR \[edx\+0x80\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 73 72 7f 7b[ 	]*vpshrdq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
#pass
