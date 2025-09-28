MEMORY
{
  region1        : ORIGIN = 0x1000, LENGTH = 0x1000 ,
  INCLUDE memory_test_inc_1.t
  region3 (wx)   :      o = 0x4000, l      =      4
  region4 (!r)   : o = 0x6000 + 60, len    = 0x30 * 0x6
}

SECTIONS
{
  .sec0 : { *(*.sec0) }
  
  .sec1 ORIGIN (region1) : { *(*.sec1) } AT> region2

  INCLUDE memory_test_inc_2.t
  
  .sec2 : { *(*.sec2) } > region3 AT> region4

  .sec3 0x5000 : { 
    INCLUDE memory_test_inc_3.t
  }

  .sec4 : { *(*.sec4) } AT> region2

  .sec5 : { LONG(0x5555) } > region2

  /DISCARD/ : { *(*) }
}
