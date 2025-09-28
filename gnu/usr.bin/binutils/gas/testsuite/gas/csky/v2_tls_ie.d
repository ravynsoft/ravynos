# name: csky - v2 TLS IE
#as: -mcpu=ck810 -W
#objdump: -Dr

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*R_CKCORE_TLS_IE32\s*xxx.*
#...
