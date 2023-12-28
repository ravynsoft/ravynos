# name: csky - v2 TLS LD
#as: -mcpu=ck810 -W
#objdump: -Dr

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*R_CKCORE_TLS_LDM32\s*xxx.*
\s*[0-9a-f]*:\s*R_CKCORE_PLT32\s*__tls_get_addr
\s*[0-9a-f]*:\s*R_CKCORE_TLS_LDO32\s*xxx
#...
