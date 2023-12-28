#as:
#objdump: -dw
#name: x86_64 RAO_INT insns
#source: x86-64-raoint.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*48 0f 38 fc 10\s+aadd   %rdx,\(%rax\)
\s*[a-f0-9]+:\s*66 48 0f 38 fc 10\s+aand   %rdx,\(%rax\)
\s*[a-f0-9]+:\s*f2 48 0f 38 fc 10\s+aor    %rdx,\(%rax\)
\s*[a-f0-9]+:\s*f3 48 0f 38 fc 10\s+axor   %rdx,\(%rax\)
\s*[a-f0-9]+:\s*48 0f 38 fc 10\s+aadd   %rdx,\(%rax\)
\s*[a-f0-9]+:\s*66 48 0f 38 fc 10\s+aand   %rdx,\(%rax\)
\s*[a-f0-9]+:\s*f2 48 0f 38 fc 10\s+aor    %rdx,\(%rax\)
\s*[a-f0-9]+:\s*f3 48 0f 38 fc 10\s+axor   %rdx,\(%rax\)
#pass
