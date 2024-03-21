SECTIONS
{
  .text :
  {
    . += 4;
    *0.o(.text .pr)
    FILL (0x12)
    *1.o(.text .pr)
    . += 1;
    FILL (0x23)
    *2.o(.text .pr)
    FILL (0x003456)
    . += 4;
    FILL (0x00004567000089ab0000cdef00000123)
    . += 8;
    LONG (0xdeadbeef)
    . += 12;
    . += 16;
  } =0xcafebabe
}
