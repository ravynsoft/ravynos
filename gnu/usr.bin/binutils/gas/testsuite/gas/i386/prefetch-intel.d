#as: --divide
#objdump: -dw -Mintel
#name: i386 prefetch (Intel disassembly)
#source: prefetch.s

.*: +file format .*

Disassembly of section .text:

0+ <amd_prefetch>:
\s*[a-f0-9]+:	0f 0d 00             	prefetch BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 08             	prefetchw BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 10             	prefetchwt1 BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 18             	prefetch BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 20             	prefetch BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 28             	prefetch BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 30             	prefetch BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 0d 38             	prefetch BYTE PTR \[eax\]

0+[0-9a-f]+ <intel_prefetch>:
\s*[a-f0-9]+:	0f 18 00             	prefetchnta BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 18 08             	prefetcht0 BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 18 10             	prefetcht1 BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 18 18             	prefetcht2 BYTE PTR \[eax\]
\s*[a-f0-9]+:	0f 18 20             	nop    DWORD PTR \[eax\]
\s*[a-f0-9]+:	0f 18 28             	nop    DWORD PTR \[eax\]
\s*[a-f0-9]+:	0f 18 30             	nop    DWORD PTR \[eax\]
\s*[a-f0-9]+:	0f 18 38             	nop    DWORD PTR \[eax\]
