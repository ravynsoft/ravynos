.*:     file format .*

Disassembly of section \.text:

0+1000 <_start>:
\s*1000:\s+eb000002\s+bl\s+1010 <__bar_veneer>
#...

0+1010 <__bar_veneer>:
\s*1010:\s+e59fc00c\s+ldr\s+ip, \[pc, #12\]\s+@ 1024 <__bar_veneer\+0x14>
\s*1014:\s+e3ccc13f\s+bic\s+ip, ip, #-1073741809\s+@ 0xc000000f
\s*1018:\s+e12fff1c\s+bx\s+ip
\s*101c:\s+e320f000\s+nop\s+\{0\}
\s*1020:\s+e125be70\s+bkpt\s+0x5be0
\s*1024:\s+02001020\s+.word\s+0x02001020
#...

\s*Disassembly of section \.foo:
\s*02001020 <bar>:
\s*2001020:\s+e12fff1e\s+bx\s+lr
