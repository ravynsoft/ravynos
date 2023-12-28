.text
LRW:
   lrw      r2, 0x100
   lrw16    r2, 0x1000
   lrw32    r2, 0x10000
   lrw      r2, 0x12341234
   lrw      r2, L1
   lrw      r2, [L1]

L1:
   mov      r2, r3
