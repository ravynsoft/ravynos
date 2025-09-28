SECTIONS
{
    .foo : {
      __foo_start = .;
      KEEP(*(.foo))
      __foo_end = .;
      }
}
