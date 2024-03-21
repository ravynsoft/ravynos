SECTIONS
{
  . = 0;
  .hash		: { *(.hash) }
  .dynsym	: { *(.dynsym) }
  .dynstr	: { *(.dynstr) }
  .rela.plt	: { *(.rela.plt) }

  . = 0x1000;
  .plt		: { *(.plt) }

  . = 0x2000;
  .text		: { *(.text) }

  . = 0x3000;
  .dynamic	: { *(.dynamic) }
  .got		: { *(.got.plt) }
}
