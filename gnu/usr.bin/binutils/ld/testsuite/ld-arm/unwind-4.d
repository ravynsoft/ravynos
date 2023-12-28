#ld: -q -T arm.ld
#objdump: -sr

.*:     file format.*

#...
RELOCATION RECORDS FOR \[\.ARM\.exidx\]:
OFFSET +TYPE +VALUE
00000000 R_ARM_PREL31      \.text
00000000 R_ARM_NONE        __aeabi_unwind_cpp_pr0
00000008 R_ARM_PREL31      \.text
00000008 R_ARM_NONE        __aeabi_unwind_cpp_pr1
0000000c R_ARM_PREL31      \.text
00000010 R_ARM_PREL31      \.text
00000010 R_ARM_NONE        __aeabi_unwind_cpp_pr0
00000010 R_ARM_PREL31      \.text
00000010 R_ARM_NONE        __aeabi_unwind_cpp_pr0
00000018 R_ARM_PREL31      \.text


Contents of section .text:
#...
Contents of section .ARM.exidx:
 8020 (e0ffff7f b0b0a880 dcffff7f e8ffff7f|7fffffe0 80a8b0b0 7fffffdc 7fffffe8)  .*
 8030 (d8ffff7f b0b0a880 d8ffff7f 01000000|7fffffd8 80a8b0b0 7fffffd8 00000001)  .*
Contents of section .far:
#...
