ENTRY (foo_global)
SECTIONS
{
  . = SIZEOF_HEADERS;
  .interp		: { *(.interp) }
  .hash			: { *(.hash) }
  .dynsym		: { *(.dynsym) }
  .dynstr		: { *(.dynstr) }
  .gnu.version		: { *(.gnu.version) }
  .gnu.version_d	: { *(.gnu.version_d) }
  .rela.plt		: { *(.rela.plt) }

  . = 0x1000;
  .plt			: { *(.plt) }

  . = 0x2000;
  .text			: { *(.text) }

  . = 0x3000;
  .dynamic		: { *(.dynamic) }
  .got			: { *(.got.plt) }
};
VERSION
{
  {
    global:
			foo_extern;
			foo_global;
			foo_hidden;
			foo_rehidden;
    local:
			foo_local;
  };
}
