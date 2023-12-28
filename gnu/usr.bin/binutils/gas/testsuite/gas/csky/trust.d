# name: trust - csky
#as: -mcpu=ck810t
#objdump: -d

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*c0003c20\s*wsc
\s*[0-9a-f]*:\s*c0006024\s*mfcr\s*r4,\s*cr<0,\s+0>
\s*[0-9a-f]*:\s*c004642b\s*mtcr\s*r4,\s*cr<11,\s+0>
\s*[0-9a-f]*:\s*c0646428\s*mtcr\s*r4,\s*cr<8,\s+3>
\s*[0-9a-f]*:\s*c0696024\s*mfcr\s*r4,\s*cr<9,\s+3>
\s*[0-9a-f]*:\s*c2007420\s*psrset\s*sie
\s*[0-9a-f]*:\s*c2007020\s*psrclr\s*sie
#...
