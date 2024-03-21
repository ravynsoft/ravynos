MEMORY
{
  R_TEXTMEM (ARX) : ORIGIN = 0x100, LENGTH = 16K
  R_DATAMEM (AW)  : org = 0x1000, l = (4 * 1024)
}

REGION_ALIAS ("A_TEXTMEM", R_TEXTMEM);
REGION_ALIAS ("A_DATAMEM", R_DATAMEM);

REGION_ALIAS ("TEXTMEM", A_TEXTMEM);
REGION_ALIAS ("DATAMEM", A_DATAMEM);

SECTIONS
{
  . = 0;
  .text :
  {
    text_start = ORIGIN (TEXTMEM);
    *(.text)
    *(.pr)
    text_end = .;
  } > TEXTMEM
  
  data_start = ORIGIN (DATAMEM);
  .data :
  {
    *(.data)
    *(.rw)
    data_end = .;
  } >DATAMEM

  fred = ORIGIN(DATAMEM) + LENGTH(DATAMEM);
  tred = ORIGIN(TEXTMEM) + LENGTH(TEXTMEM);
}
