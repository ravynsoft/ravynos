
Relocation section '\.rel\.text' at offset .* contains 3 entries:
 Offset     Info    Type            Sym\.Value  Sym\. Name
00000000  00000028 R_ARM_V4BX       
00000004  00000028 R_ARM_V4BX       
00000008  00000028 R_ARM_V4BX       

Relocation section '\.rel\.ARM\.exidx' at offset .* contains 5 entries:
 Offset     Info    Type            Sym\.Value  Sym\. Name
00000000  0000012a R_ARM_PREL31      00000000   \.text
00000000  00000e00 R_ARM_NONE        00000000   __aeabi_unwind_cpp_pr0
00000008  0000012a R_ARM_PREL31      00000000   \.text
00000010  0000012a R_ARM_PREL31      00000000   \.text
00000010  00000e00 R_ARM_NONE        00000000   __aeabi_unwind_cpp_pr0

Unwind section '\.ARM\.exidx' at offset .* contains 3 entries:

0x0: 0x80a8b0b0
  Compact model index: 0
  0xa8      pop {r4, r14}
  0xb0      finish
  0xb0      finish

0x4 <test>: 0x1 \[cantunwind\]

0x8 <end>: 0x80a8b0b0
  Compact model index: 0
  0xa8      pop {r4, r14}
  0xb0      finish
  0xb0      finish

