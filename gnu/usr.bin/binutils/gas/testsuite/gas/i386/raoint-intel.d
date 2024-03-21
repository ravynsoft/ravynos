#as:
#objdump: -dw -Mintel
#name: i386 RAO_INT insns (Intel disassembly)
#source: raoint.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*0f 38 fc 10\s+aadd   DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*66 0f 38 fc 10\s+aand   DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*f2 0f 38 fc 10\s+aor    DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*f3 0f 38 fc 10\s+axor   DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*0f 38 fc 10\s+aadd   DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*66 0f 38 fc 10\s+aand   DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*f2 0f 38 fc 10\s+aor    DWORD PTR \[eax\],edx
\s*[a-f0-9]+:\s*f3 0f 38 fc 10\s+axor   DWORD PTR \[eax\],edx
#pass
