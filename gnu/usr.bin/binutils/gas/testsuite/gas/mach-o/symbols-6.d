#as: -L -I $srcdir/$subdir
#objdump: -t
#target: i?86-*-darwin* powerpc-*-darwin*
#source: symbols-6.s
.*: +file format mach-o.*
#...
SYMBOL TABLE:
00000000 l( )+0e SECT( )+01 0000 \[.text\] Lzt0
00000002 l( )+0e SECT( )+01 0000 \[.text\] Lmt0
00000004 l( )+0e SECT( )+01 0000 \[.text\] Lat0
0000001e l( )+0e SECT( )+02 0000 \[.data\] Lzd0
00000020 l( )+0e SECT( )+02 0000 \[.data\] Lmd0
00000023 l( )+0e SECT( )+02 0000 \[.data\] Lad0
00000118 l( )+0e SECT( )+03 0000 \[.bss\] zlcomm0
0000011e l( )+0e SECT( )+03 0000 \[.bss\] mlcomm0
00000124 l( )+0e SECT( )+03 0000 \[.bss\] alcomm0
00000072 l( )+0e SECT( )+04 0000 \[__HERE.__there\] Lzs0
00000074 l( )+0e SECT( )+04 0000 \[__HERE.__there\] Lms0
00000076 l( )+0e SECT( )+04 0000 \[__HERE.__there\] Las0
00000012 l( )+0e SECT( )+01 0000 \[.text\] Lzt1
00000015 l( )+0e SECT( )+01 0000 \[.text\] Lmt1
00000017 l( )+0e SECT( )+01 0000 \[.text\] Lat1
00000030 l( )+0e SECT( )+02 0000 \[.data\] Lzd1
00000032 l( )+0e SECT( )+02 0000 \[.data\] Lmd1
00000035 l( )+0e SECT( )+02 0000 \[.data\] Lad1
0000012a l( )+0e SECT( )+03 0000 \[.bss\] zlcomm1
00000130 l( )+0e SECT( )+03 0000 \[.bss\] mlcomm1
00000136 l( )+0e SECT( )+03 0000 \[.bss\] alcomm1
00000088 l( )+0e SECT( )+04 0000 \[__HERE.__there\] Lzs1
00000090 l( )+0e SECT( )+04 0000 \[__HERE.__there\] Lms1
00000091 l( )+0e SECT( )+04 0000 \[__HERE.__there\] Las1
0000001b l( )+0e SECT( )+01 0000 \[.text\] e
0000001c l( )+0e SECT( )+01 0000 \[.text\] e1
0000001d l( )+0e SECT( )+01 0000 \[.text\] e2
00000042 l( )+0e SECT( )+02 0000 \[.data\] d
0000004a l( )+0e SECT( )+02 0000 \[.data\] d1
00000052 l( )+0e SECT( )+02 0000 \[.data\] d2
00000096 l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] La
0000009e l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] Lb
000000a6 l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] Lc
000000ae l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] Ld
000000b6 l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] Le
000000be l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] Lf
000000c6 l( )+0e SECT( )+05 0000 \[__dummy.__dummy\] Lg
000000d0 l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] La1
000000d4 l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] Lb1
000000d8 l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] Lc1
000000dc l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] Ld1
000000e0 l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] Le1
000000e4 l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] Lf1
000000e8 l( )+0e SECT( )+06 0000 \[.lazy_symbol_pointer\] Lg1
000000ec l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] La2
000000f0 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lb2
000000f4 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lc2
000000f8 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Ld2
000000fc l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Le2
00000100 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lf2
00000104 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lg2
00000108 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lf11
0000010c l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lg11
00000110 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] La12
00000114 l( )+0e SECT( )+07 0000 \[.non_lazy_symbol_pointer\] Lb12
00000022 g( )+0f SECT( )+02 0000 \[.data\] adg0
00000034 g( )+0f SECT( )+02 0000 \[.data\] adg1
00000077 g( )+0f SECT( )+04 0000 \[__HERE.__there\] asg0
0000008f g( )+0f SECT( )+04 0000 \[__HERE.__there\] asg1
00000005 g( )+0f SECT( )+01 0000 \[.text\] atg0
00000016 g( )+0f SECT( )+01 0000 \[.text\] atg1
00000018 g( )+0f SECT( )+01 0000 \[.text\] c
00000019 g( )+0f SECT( )+01 0000 \[.text\] c1
0000001a g( )+0f SECT( )+01 0000 \[.text\] c2
0000005a g( )+1f SECT( )+02 0000 \[.data\] f
00000062 g( )+1f SECT( )+02 0000 \[.data\] f1
0000006a g( )+1f SECT( )+02 0000 \[.data\] f2
00000021 g( )+0f SECT( )+02 0000 \[.data\] mdg0
00000033 g( )+0f SECT( )+02 0000 \[.data\] mdg1
00000075 g( )+0f SECT( )+04 0000 \[__HERE.__there\] msg0
0000008e g( )+0f SECT( )+04 0000 \[__HERE.__there\] msg1
00000003 g( )+0f SECT( )+01 0000 \[.text\] mtg0
00000014 g( )+0f SECT( )+01 0000 \[.text\] mtg1
0000001f g( )+0f SECT( )+02 0000 \[.data\] zdg0
00000031 g( )+0f SECT( )+02 0000 \[.data\] zdg1
00000073 g( )+0f SECT( )+04 0000 \[__HERE.__there\] zsg0
00000089 g( )+0f SECT( )+04 0000 \[__HERE.__there\] zsg1
00000001 g( )+0f SECT( )+01 0000 \[.text\] ztg0
00000013 g( )+0f SECT( )+01 0000 \[.text\] ztg1
00000000 g( )+01 UND( )+00 0000 _aud0
00000000 g( )+01 UND( )+00 0000 _aud1
00000000 g( )+01 UND( )+00 0000 _aus0
00000000 g( )+01 UND( )+00 0000 _aus1
00000000 g( )+01 UND( )+00 0000 _aut0
00000000 g( )+01 UND( )+00 0000 _mud0
00000000 g( )+01 UND( )+00 0000 _mud1
00000000 g( )+01 UND( )+00 0000 _mus0
00000000 g( )+01 UND( )+00 0000 _mus1
00000000 g( )+01 UND( )+00 0000 _mut0
00000000 g( )+01 UND( )+00 0000 _zud0
00000000 g( )+01 UND( )+00 0000 _zud1
00000000 g( )+01 UND( )+00 0000 _zus0
00000000 g( )+01 UND( )+00 0000 _zus1
00000000 g( )+01 UND( )+00 0000 _zut0
00000000 g( )+01 UND( )+00 0001 a
00000000 g( )+01 UND( )+00 0001 a1
00000000 g( )+01 UND( )+00 0000 a2
0000000a( )+01 COM( )+00 0300 acommon0
0000000a( )+01 COM( )+00 0300 acommon1
00000000 g( )+01 UND( )+00 0001 b
00000000 g( )+01 UND( )+00 0001 b1
00000000 g( )+01 UND( )+00 0000 b2
00000000 g( )+11 UND( )+00 0000 g
00000000 g( )+11 UND( )+00 0000 g1
00000000 g( )+11 UND( )+00 0000 g2
0000000a( )+01 COM( )+00 0300 mcommon0
0000000a( )+01 COM( )+00 0300 mcommon1
0000000a( )+01 COM( )+00 0300 zcommon0
0000000a( )+01 COM( )+00 0300 zcommon1
