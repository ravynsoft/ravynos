# as: -x
# objdump: -r

# Make sure gas can assemble a typical compiler-generated file that
# requires generating a stub for out-of-range or unknown target
# locations.  Check that the generated relocations are sane.

.*:     file format elf64-mmix

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
0+2a R_MMIX_BASE_PLUS_OFFSET  __stack_chk_guard
0+13a R_MMIX_BASE_PLUS_OFFSET  \.data\+0x0+8
0+1a2 R_MMIX_BASE_PLUS_OFFSET  \.data\+0x0+8
0+1ca R_MMIX_BASE_PLUS_OFFSET  \.data
0+2ce R_MMIX_BASE_PLUS_OFFSET  \.data
0+2d6 R_MMIX_BASE_PLUS_OFFSET  \.data
0+33e R_MMIX_BASE_PLUS_OFFSET  \.data
0+422 R_MMIX_BASE_PLUS_OFFSET  \.data\+0x0+10
0+5fe R_MMIX_BASE_PLUS_OFFSET  \.data\+0x0+10
0+60a R_MMIX_BASE_PLUS_OFFSET  __stack_chk_guard
0+3c R_MMIX_PUSHJ_STUBBABLE  ak
0+48 R_MMIX_PUSHJ_STUBBABLE  o
0+64 R_MMIX_PUSHJ_STUBBABLE  ag
0+c8 R_MMIX_GETA       c
0+d8 R_MMIX_PUSHJ_STUBBABLE  t
0+12c R_MMIX_PUSHJ_STUBBABLE  u
0+16c R_MMIX_PUSHJ_STUBBABLE  c
0+3a4 R_MMIX_PUSHJ_STUBBABLE  ak
0+410 R_MMIX_PUSHJ_STUBBABLE  ao
0+5ec R_MMIX_GETA       \.rodata
0+614 R_MMIX_PUSHJ_STUBBABLE  __stack_chk_fail


