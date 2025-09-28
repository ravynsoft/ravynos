TXT_ORIGIN = 0x100;
TXT_LENGTH = 16K;
MEMORY
{
  R_TEXTMEM (ARX) : ORIGIN = TXT_ORIGIN, LENGTH = TXT_LENGTH
  R_DATAMEM (AW)  : org = DATA_ORIGIN, l = DATA_LENGTH
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
