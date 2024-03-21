#as:
#objdump: -dw
#name: i386 RAO_INT insns
#source: raoint.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
\s*[a-f0-9]+:\s*0f 38 fc 10\s+aadd   %edx,\(%eax\)
\s*[a-f0-9]+:\s*66 0f 38 fc 10\s+aand   %edx,\(%eax\)
\s*[a-f0-9]+:\s*f2 0f 38 fc 10\s+aor    %edx,\(%eax\)
\s*[a-f0-9]+:\s*f3 0f 38 fc 10\s+axor   %edx,\(%eax\)
\s*[a-f0-9]+:\s*0f 38 fc 10\s+aadd   %edx,\(%eax\)
\s*[a-f0-9]+:\s*66 0f 38 fc 10\s+aand   %edx,\(%eax\)
\s*[a-f0-9]+:\s*f2 0f 38 fc 10\s+aor    %edx,\(%eax\)
\s*[a-f0-9]+:\s*f3 0f 38 fc 10\s+axor   %edx,\(%eax\)
#pass
