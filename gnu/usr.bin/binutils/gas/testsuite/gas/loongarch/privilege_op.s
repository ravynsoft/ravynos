csrrd  $r4,0
csrrd  $r4,0x3fff
csrwr  $r4,0
csrwr  $r4,0x3fff
csrxchg  $r4,$r5,0
csrxchg  $r4,$r5,0x3fff
cacop  0,$r5,0
cacop  0x1f,$r5,0
cacop  0,$r5,0x7ff
cacop  0x1f,$r5,0x7ff
cacop  0,$r5,-0x7ff
cacop  0x1f,$r5,-0x7ff
lddir  $r4,$r5,0
lddir  $r4,$r5,0xff
ldpte  $r5,0
ldpte  $r5,0xff
iocsrrd.b  $r4,$r5
iocsrrd.h  $r4,$r5
iocsrrd.w  $r4,$r5
iocsrrd.d  $r4,$r5
iocsrwr.b  $r4,$r5
iocsrwr.h  $r4,$r5
iocsrwr.w  $r4,$r5
iocsrwr.d  $r4,$r5
tlbclr
tlbflush
tlbsrch
tlbrd
tlbwr
tlbfill
ertn
idle  0
idle  0x7fff
invtlb  0,$r5,$r6
invtlb  0x1f,$r5,$r6
