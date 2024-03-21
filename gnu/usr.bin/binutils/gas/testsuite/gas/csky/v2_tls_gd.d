# name: csky - v2 TLS GD
#as: -mcpu=ck810 -W
#objdump: -Dr

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*R_CKCORE_TLS_GD32\s*xxx.*
\s*[0-9a-f]*:\s*R_CKCORE_PLT32\s*__tls_get_addr
#...
