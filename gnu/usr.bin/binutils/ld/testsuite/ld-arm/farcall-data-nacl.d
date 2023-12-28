.*:     file format .*

Disassembly of section .text:

0+8000 <_start>:
\s*8000:\s+ea000002\s+b\s+8010 <__far_veneer>
#...

0+8010 <__far_veneer>:
\s*8010:\s+e59fc00c\s+ldr\s+ip, \[pc, #12\]\s+@ 8024 <__far_veneer\+0x14>
\s*8014:\s+e3ccc13f\s+bic\s+ip, ip, #-1073741809\s+@ 0xc000000f
\s*8018:\s+e12fff1c\s+bx\s+ip
\s*801c:\s+e320f000\s+nop\s+\{0\}
\s*8020:\s+e125be70\s+bkpt\s+0x5be0
\s*8024:\s+12340000\s+.word\s+0x12340000
#...

0+8030 <after>:
\s*8030:\s+11111111\s+\.word\s+0x11111111

Disassembly of section \.far:

12340000 <far>:
12340000:\s+e12fff1e\s+bx\s+lr
