#as:
#objdump: -dw -Mintel
#name: x86_64 CMPCCXADD insns (Intel disassembly)
#source: x86-64-cmpccxadd.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*c4 a2 79 e6 8c f5 00 00 00 10\s+cmpbexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e6 09\s+cmpbexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e6 89 fc 01 00 00\s+cmpbexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e6 8a 00 fe ff ff\s+cmpbexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e6 8c f5 00 00 00 10\s+cmpbexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e6 09\s+cmpbexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e6 89 f8 03 00 00\s+cmpbexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e6 8a 00 fc ff ff\s+cmpbexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e2 8c f5 00 00 00 10\s+cmpbxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e2 09\s+cmpbxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e2 89 fc 01 00 00\s+cmpbxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e2 8a 00 fe ff ff\s+cmpbxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e2 8c f5 00 00 00 10\s+cmpbxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e2 09\s+cmpbxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e2 89 f8 03 00 00\s+cmpbxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e2 8a 00 fc ff ff\s+cmpbxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ee 8c f5 00 00 00 10\s+cmplexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ee 09\s+cmplexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ee 89 fc 01 00 00\s+cmplexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ee 8a 00 fe ff ff\s+cmplexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ee 8c f5 00 00 00 10\s+cmplexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ee 09\s+cmplexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ee 89 f8 03 00 00\s+cmplexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ee 8a 00 fc ff ff\s+cmplexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ec 8c f5 00 00 00 10\s+cmplxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ec 09\s+cmplxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ec 89 fc 01 00 00\s+cmplxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ec 8a 00 fe ff ff\s+cmplxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ec 8c f5 00 00 00 10\s+cmplxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ec 09\s+cmplxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ec 89 f8 03 00 00\s+cmplxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ec 8a 00 fc ff ff\s+cmplxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e7 8c f5 00 00 00 10\s+cmpnbexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e7 09\s+cmpnbexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e7 89 fc 01 00 00\s+cmpnbexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e7 8a 00 fe ff ff\s+cmpnbexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e7 8c f5 00 00 00 10\s+cmpnbexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e7 09\s+cmpnbexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e7 89 f8 03 00 00\s+cmpnbexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e7 8a 00 fc ff ff\s+cmpnbexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e3 8c f5 00 00 00 10\s+cmpnbxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e3 09\s+cmpnbxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e3 89 fc 01 00 00\s+cmpnbxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e3 8a 00 fe ff ff\s+cmpnbxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e3 8c f5 00 00 00 10\s+cmpnbxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e3 09\s+cmpnbxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e3 89 f8 03 00 00\s+cmpnbxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e3 8a 00 fc ff ff\s+cmpnbxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ef 8c f5 00 00 00 10\s+cmpnlexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ef 09\s+cmpnlexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ef 89 fc 01 00 00\s+cmpnlexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ef 8a 00 fe ff ff\s+cmpnlexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ef 8c f5 00 00 00 10\s+cmpnlexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ef 09\s+cmpnlexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ef 89 f8 03 00 00\s+cmpnlexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ef 8a 00 fc ff ff\s+cmpnlexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ed 8c f5 00 00 00 10\s+cmpnlxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ed 09\s+cmpnlxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ed 89 fc 01 00 00\s+cmpnlxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ed 8a 00 fe ff ff\s+cmpnlxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ed 8c f5 00 00 00 10\s+cmpnlxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ed 09\s+cmpnlxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ed 89 f8 03 00 00\s+cmpnlxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ed 8a 00 fc ff ff\s+cmpnlxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e1 8c f5 00 00 00 10\s+cmpnoxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e1 09\s+cmpnoxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e1 89 fc 01 00 00\s+cmpnoxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e1 8a 00 fe ff ff\s+cmpnoxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e1 8c f5 00 00 00 10\s+cmpnoxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e1 09\s+cmpnoxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e1 89 f8 03 00 00\s+cmpnoxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e1 8a 00 fc ff ff\s+cmpnoxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 eb 8c f5 00 00 00 10\s+cmpnpxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 eb 09\s+cmpnpxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 eb 89 fc 01 00 00\s+cmpnpxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 eb 8a 00 fe ff ff\s+cmpnpxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 eb 8c f5 00 00 00 10\s+cmpnpxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 eb 09\s+cmpnpxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 eb 89 f8 03 00 00\s+cmpnpxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 eb 8a 00 fc ff ff\s+cmpnpxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e9 8c f5 00 00 00 10\s+cmpnsxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e9 09\s+cmpnsxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e9 89 fc 01 00 00\s+cmpnsxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e9 8a 00 fe ff ff\s+cmpnsxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e9 8c f5 00 00 00 10\s+cmpnsxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e9 09\s+cmpnsxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e9 89 f8 03 00 00\s+cmpnsxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e9 8a 00 fc ff ff\s+cmpnsxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e5 8c f5 00 00 00 10\s+cmpnzxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e5 09\s+cmpnzxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e5 89 fc 01 00 00\s+cmpnzxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e5 8a 00 fe ff ff\s+cmpnzxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e5 8c f5 00 00 00 10\s+cmpnzxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e5 09\s+cmpnzxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e5 89 f8 03 00 00\s+cmpnzxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e5 8a 00 fc ff ff\s+cmpnzxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e0 8c f5 00 00 00 10\s+cmpoxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e0 09\s+cmpoxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e0 89 fc 01 00 00\s+cmpoxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e0 8a 00 fe ff ff\s+cmpoxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e0 8c f5 00 00 00 10\s+cmpoxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e0 09\s+cmpoxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e0 89 f8 03 00 00\s+cmpoxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e0 8a 00 fc ff ff\s+cmpoxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ea 8c f5 00 00 00 10\s+cmppxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ea 09\s+cmppxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ea 89 fc 01 00 00\s+cmppxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ea 8a 00 fe ff ff\s+cmppxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ea 8c f5 00 00 00 10\s+cmppxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ea 09\s+cmppxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ea 89 f8 03 00 00\s+cmppxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ea 8a 00 fc ff ff\s+cmppxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e8 8c f5 00 00 00 10\s+cmpsxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e8 09\s+cmpsxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e8 89 fc 01 00 00\s+cmpsxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e8 8a 00 fe ff ff\s+cmpsxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e8 8c f5 00 00 00 10\s+cmpsxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e8 09\s+cmpsxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e8 89 f8 03 00 00\s+cmpsxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e8 8a 00 fc ff ff\s+cmpsxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e4 8c f5 00 00 00 10\s+cmpzxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e4 09\s+cmpzxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e4 89 fc 01 00 00\s+cmpzxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e4 8a 00 fe ff ff\s+cmpzxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e4 8c f5 00 00 00 10\s+cmpzxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e4 09\s+cmpzxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e4 89 f8 03 00 00\s+cmpzxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e4 8a 00 fc ff ff\s+cmpzxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e6 8c f5 00 00 00 10\s+cmpbexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e6 09\s+cmpbexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e6 89 fc 01 00 00\s+cmpbexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e6 8a 00 fe ff ff\s+cmpbexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e6 8c f5 00 00 00 10\s+cmpbexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e6 09\s+cmpbexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e6 89 f8 03 00 00\s+cmpbexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e6 8a 00 fc ff ff\s+cmpbexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e2 8c f5 00 00 00 10\s+cmpbxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e2 09\s+cmpbxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e2 89 fc 01 00 00\s+cmpbxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e2 8a 00 fe ff ff\s+cmpbxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e2 8c f5 00 00 00 10\s+cmpbxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e2 09\s+cmpbxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e2 89 f8 03 00 00\s+cmpbxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e2 8a 00 fc ff ff\s+cmpbxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ee 8c f5 00 00 00 10\s+cmplexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ee 09\s+cmplexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ee 89 fc 01 00 00\s+cmplexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ee 8a 00 fe ff ff\s+cmplexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ee 8c f5 00 00 00 10\s+cmplexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ee 09\s+cmplexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ee 89 f8 03 00 00\s+cmplexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ee 8a 00 fc ff ff\s+cmplexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ec 8c f5 00 00 00 10\s+cmplxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ec 09\s+cmplxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ec 89 fc 01 00 00\s+cmplxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ec 8a 00 fe ff ff\s+cmplxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ec 8c f5 00 00 00 10\s+cmplxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ec 09\s+cmplxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ec 89 f8 03 00 00\s+cmplxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ec 8a 00 fc ff ff\s+cmplxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e7 8c f5 00 00 00 10\s+cmpnbexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e7 09\s+cmpnbexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e7 89 fc 01 00 00\s+cmpnbexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e7 8a 00 fe ff ff\s+cmpnbexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e7 8c f5 00 00 00 10\s+cmpnbexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e7 09\s+cmpnbexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e7 89 f8 03 00 00\s+cmpnbexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e7 8a 00 fc ff ff\s+cmpnbexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e3 8c f5 00 00 00 10\s+cmpnbxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e3 09\s+cmpnbxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e3 89 fc 01 00 00\s+cmpnbxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e3 8a 00 fe ff ff\s+cmpnbxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e3 8c f5 00 00 00 10\s+cmpnbxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e3 09\s+cmpnbxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e3 89 f8 03 00 00\s+cmpnbxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e3 8a 00 fc ff ff\s+cmpnbxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ef 8c f5 00 00 00 10\s+cmpnlexadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ef 09\s+cmpnlexadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ef 89 fc 01 00 00\s+cmpnlexadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ef 8a 00 fe ff ff\s+cmpnlexadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ef 8c f5 00 00 00 10\s+cmpnlexadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ef 09\s+cmpnlexadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ef 89 f8 03 00 00\s+cmpnlexadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ef 8a 00 fc ff ff\s+cmpnlexadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ed 8c f5 00 00 00 10\s+cmpnlxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ed 09\s+cmpnlxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ed 89 fc 01 00 00\s+cmpnlxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ed 8a 00 fe ff ff\s+cmpnlxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ed 8c f5 00 00 00 10\s+cmpnlxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ed 09\s+cmpnlxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ed 89 f8 03 00 00\s+cmpnlxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ed 8a 00 fc ff ff\s+cmpnlxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e1 8c f5 00 00 00 10\s+cmpnoxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e1 09\s+cmpnoxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e1 89 fc 01 00 00\s+cmpnoxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e1 8a 00 fe ff ff\s+cmpnoxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e1 8c f5 00 00 00 10\s+cmpnoxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e1 09\s+cmpnoxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e1 89 f8 03 00 00\s+cmpnoxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e1 8a 00 fc ff ff\s+cmpnoxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 eb 8c f5 00 00 00 10\s+cmpnpxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 eb 09\s+cmpnpxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 eb 89 fc 01 00 00\s+cmpnpxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 eb 8a 00 fe ff ff\s+cmpnpxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 eb 8c f5 00 00 00 10\s+cmpnpxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 eb 09\s+cmpnpxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 eb 89 f8 03 00 00\s+cmpnpxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 eb 8a 00 fc ff ff\s+cmpnpxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e9 8c f5 00 00 00 10\s+cmpnsxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e9 09\s+cmpnsxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e9 89 fc 01 00 00\s+cmpnsxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e9 8a 00 fe ff ff\s+cmpnsxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e9 8c f5 00 00 00 10\s+cmpnsxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e9 09\s+cmpnsxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e9 89 f8 03 00 00\s+cmpnsxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e9 8a 00 fc ff ff\s+cmpnsxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e5 8c f5 00 00 00 10\s+cmpnzxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e5 09\s+cmpnzxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e5 89 fc 01 00 00\s+cmpnzxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e5 8a 00 fe ff ff\s+cmpnzxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e5 8c f5 00 00 00 10\s+cmpnzxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e5 09\s+cmpnzxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e5 89 f8 03 00 00\s+cmpnzxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e5 8a 00 fc ff ff\s+cmpnzxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e0 8c f5 00 00 00 10\s+cmpoxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e0 09\s+cmpoxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e0 89 fc 01 00 00\s+cmpoxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e0 8a 00 fe ff ff\s+cmpoxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e0 8c f5 00 00 00 10\s+cmpoxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e0 09\s+cmpoxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e0 89 f8 03 00 00\s+cmpoxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e0 8a 00 fc ff ff\s+cmpoxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 ea 8c f5 00 00 00 10\s+cmppxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 ea 09\s+cmppxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 ea 89 fc 01 00 00\s+cmppxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 ea 8a 00 fe ff ff\s+cmppxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 ea 8c f5 00 00 00 10\s+cmppxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 ea 09\s+cmppxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 ea 89 f8 03 00 00\s+cmppxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 ea 8a 00 fc ff ff\s+cmppxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e8 8c f5 00 00 00 10\s+cmpsxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e8 09\s+cmpsxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e8 89 fc 01 00 00\s+cmpsxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e8 8a 00 fe ff ff\s+cmpsxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e8 8c f5 00 00 00 10\s+cmpsxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e8 09\s+cmpsxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e8 89 f8 03 00 00\s+cmpsxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e8 8a 00 fc ff ff\s+cmpsxadd QWORD PTR \[rdx-0x400\],rcx,rbx
\s*[a-f0-9]+:\s*c4 a2 79 e4 8c f5 00 00 00 10\s+cmpzxadd DWORD PTR \[rbp\+r14\*8\+0x10000000\],ecx,eax
\s*[a-f0-9]+:\s*c4 c2 61 e4 09\s+cmpzxadd DWORD PTR \[r9\],ecx,ebx
\s*[a-f0-9]+:\s*c4 e2 79 e4 89 fc 01 00 00\s+cmpzxadd DWORD PTR \[rcx\+0x1fc\],ecx,eax
\s*[a-f0-9]+:\s*c4 e2 61 e4 8a 00 fe ff ff\s+cmpzxadd DWORD PTR \[rdx-0x200\],ecx,ebx
\s*[a-f0-9]+:\s*c4 a2 f9 e4 8c f5 00 00 00 10\s+cmpzxadd QWORD PTR \[rbp\+r14\*8\+0x10000000\],rcx,rax
\s*[a-f0-9]+:\s*c4 c2 e1 e4 09\s+cmpzxadd QWORD PTR \[r9\],rcx,rbx
\s*[a-f0-9]+:\s*c4 e2 f9 e4 89 f8 03 00 00\s+cmpzxadd QWORD PTR \[rcx\+0x3f8\],rcx,rax
\s*[a-f0-9]+:\s*c4 e2 e1 e4 8a 00 fc ff ff\s+cmpzxadd QWORD PTR \[rdx-0x400\],rcx,rbx
#pass
