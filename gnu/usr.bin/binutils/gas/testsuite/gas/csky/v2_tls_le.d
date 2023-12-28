# name: csky - v2 TLS LE
#as: -mcpu=ck810 -W
#objdump: -Dr

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*R_CKCORE_TLS_LE32\s*xxx
#...
