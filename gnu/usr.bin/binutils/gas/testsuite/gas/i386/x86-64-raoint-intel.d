#as:
#objdump: -dw -Mintel
#name: x86_64 RAO_INT insns (Intel disassembly)
#source: x86-64-raoint.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*48 0f 38 fc 10\s+aadd   QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*66 48 0f 38 fc 10\s+aand   QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*f2 48 0f 38 fc 10\s+aor    QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*f3 48 0f 38 fc 10\s+axor   QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*48 0f 38 fc 10\s+aadd   QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*66 48 0f 38 fc 10\s+aand   QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*f2 48 0f 38 fc 10\s+aor    QWORD PTR \[rax\],rdx
\s*[a-f0-9]+:\s*f3 48 0f 38 fc 10\s+axor   QWORD PTR \[rax\],rdx
#pass
