#...
Disassembly of section .text:
#...
[0-9a-f]+ <a1ldr>:
[ \t0-9a-f]+:[ \t]+b8408c87[ \t]+ldr[ \t]+w7, \[x4, #8\]\!
[ \t0-9a-f]+:[ \t]+1b017c06[ \t]+mul[ \t]+w6, w0, w1
[ \t0-9a-f]+:[ \t]+f9400084[ \t]+ldr[ \t]+x4, \[x4\]
[ \t0-9a-f]+:[ \t0-9a-z]+[ \t]+b[ \t]+[0-9a-f]+ <__erratum_835769_veneer_0>
[ \t0-9a-f]+:[ \t]+aa0503e0[ \t]+mov[ \t]+x0, x5
[ \t0-9a-f]+:[ \t]+d65f03c0[ \t]+ret

[0-9a-f]+ <a5ldr>:
[ \t0-9a-f]+:[ \t]+b8408c87[ \t]+ldr[ \t]+w7, \[x4, #8\]!
[ \t0-9a-f]+:[ \t]+1b017c06[ \t]+mul[ \t]+w6, w0, w1
[ \t0-9a-f]+:[ \t]+f9400084[ \t]+ldr[ \t]+x4, \[x4\]
[ \t0-9a-f]+:[ \t0-9a-z]+[ \t]+b[ \t]+[0-9a-f]+ <__erratum_835769_veneer_1>
[ \t0-9a-f]+:[ \t]+aa0503e0[ \t]+mov[ \t]+x0, x5
[ \t0-9a-f]+:[ \t]+d65f03c0[ \t]+ret

[0-9a-f]+ <a6ldr>:
[ \t0-9a-f]+:[ \t]+b8408c87[ \t]+ldr[ \t]+w7, \[x4, #8\]!
[ \t0-9a-f]+:[ \t]+1b017c06[ \t]+mul[ \t]+w6, w0, w1
[ \t0-9a-f]+:[ \t]+f9400084[ \t]+ldr[ \t]+x4, \[x4\]
[ \t0-9a-f]+:[ \t]+9b031885[ \t]+madd[ \t]+x5, x4, x3, x6
[ \t0-9a-f]+:[ \t]+aa0503e0[ \t]+mov[ \t]+x0, x5
[ \t0-9a-f]+:[ \t]+d65f03c0[ \t]+ret

[0-9a-f]+ <a7str>:
[ \t0-9a-f]+:[ \t]+b8408c87[ \t]+ldr[ \t]+w7, \[x4, #8\]!
[ \t0-9a-f]+:[ \t]+1b017c06[ \t]+mul[ \t]+w6, w0, w1
[ \t0-9a-f]+:[ \t]+f9000084[ \t]+str[ \t]+x4, \[x4\]
[ \t0-9a-f]+:[ \t0-9a-z]+[ \t]+b[ \t]+[0-9a-f]+ <__erratum_835769_veneer_2>
[ \t0-9a-f]+:[ \t]+aa0503e0[ \t]+mov[ \t]+x0, x5
[ \t0-9a-f]+:[ \t]+d65f03c0[ \t]+ret

[ \t0-9a-f]+:[ \t]+d503201f[ \t]+nop
[ \t0-9a-f]+:[ \t]+14000008[ \t]+b[ \t]+[0-9a-f]+ <__erratum_835769_veneer_0\+0x8>
[ \t0-9a-f]+:[ \t]+d503201f[ \t]+nop
[0-9a-f]+ <__erratum_835769_veneer_2>:
[ \t0-9a-f]+:[ \t]+9b031885[ \t]+madd[ \t]+x5, x4, x3, x6
[ \t0-9a-f]+:[ \t0-9a-z]+[ \t]+b[ \t]+[0-9a-f]+ <a7str\+0x[0-9a-f]+>

[0-9a-f]+ <__erratum_835769_veneer_1>:
[ \t0-9a-f]+:[ \t]+9ba31845[ \t]+umaddl[ \t]+x5, w2, w3, x6
[ \t0-9a-f]+:[ \t0-9a-z]+[ \t]+b[ \t]+[0-9a-f]+ <a5ldr\+0x[0-9a-f]+>

[0-9a-f]+ <__erratum_835769_veneer_0>:
[ \t0-9a-f]+:[ \t]+9b031845[ \t]+madd[ \t]+x5, x2, x3, x6
[ \t0-9a-f]+:[ \t0-9a-z]+[ \t]+b[ \t]+[0-9a-f]+ <a1ldr\+0x[0-9a-f]+>
#pass
