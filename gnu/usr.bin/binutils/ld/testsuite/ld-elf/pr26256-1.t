SECTIONS
{
  .text : { *(.text*) }
  .init.data :
  {
    *(.init.data);
    *(__patchable_function_entries);
  }
  /DISCARD/ :
  {
    *(.reginfo) *(.MIPS.abiflags) *(.MIPS.options) *(.trampolines)
  }
}
