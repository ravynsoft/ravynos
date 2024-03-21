#as:
#objdump: -dw -Mintel
#name: x86_64 LKGS insns (Intel disassembly)
#source: x86-64-lkgs.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*f2 41 0f 00 f4\s+lkgs   r12w
\s*[a-f0-9]+:\s*f2 41 0f 00 f4\s+lkgs   r12w
\s*[a-f0-9]+:\s*f2 41 0f 00 f4\s+lkgs   r12w
\s*[a-f0-9]+:\s*f2 42 0f 00 b4 f5 00 00 00 10\s+lkgs   WORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*f2 41 0f 00 31\s+lkgs   WORD PTR \[r9\]
\s*[a-f0-9]+:\s*f2 0f 00 b1 fe 00 00 00\s+lkgs   WORD PTR \[rcx\+0xfe\]
\s*[a-f0-9]+:\s*f2 0f 00 b2 00 ff ff ff\s+lkgs   WORD PTR \[rdx-0x100\]
\s*[a-f0-9]+:\s*f2 41 0f 00 f4\s+lkgs   r12w
\s*[a-f0-9]+:\s*f2 41 0f 00 f4\s+lkgs   r12w
\s*[a-f0-9]+:\s*f2 41 0f 00 f4\s+lkgs   r12w
\s*[a-f0-9]+:\s*f2 42 0f 00 b4 f5 00 00 00 10\s+lkgs   WORD PTR \[rbp\+r14\*8\+0x10000000\]
\s*[a-f0-9]+:\s*f2 41 0f 00 31\s+lkgs   WORD PTR \[r9\]
\s*[a-f0-9]+:\s*f2 0f 00 b1 fe 00 00 00\s+lkgs   WORD PTR \[rcx\+0xfe\]
\s*[a-f0-9]+:\s*f2 0f 00 b2 00 ff ff ff\s+lkgs   WORD PTR \[rdx-0x100\]
#pass
