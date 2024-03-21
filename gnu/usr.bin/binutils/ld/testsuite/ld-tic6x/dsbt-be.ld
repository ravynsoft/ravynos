OUTPUT_FORMAT("elf32-tic6x-be", "elf32-tic6x-be",
	      "elf32-tic6x-be")
EXTERN (__c6xabi_DSBT_BASE);
SECTIONS
{
  . = 0x8000;
  .hash           : { *(.hash) }
  .gnu.hash       : { *(.gnu.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rel.init       : { *(.rel.init) }
  .rela.init      : { *(.rela.init) }
  .rel.text       : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
  .rela.text      : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
  .rel.fini       : { *(.rel.fini) }
  .rela.fini      : { *(.rela.fini) }
  .rel.const     : { *(.rel.const .rel.const.* .rel.gnu.linkonce.r.*) }
  .rela.const    : { *(.rela.const .rela.const.* .rela.gnu.linkonce.r.*) }
  .rel.rodata     : { *(.rel.rodata .rel.rodata.*) }
  .rela.rodata    : { *(.rela.rodata .rela.rodata.*) }
  .rel.data.rel.ro   : { *(.rel.data.rel.ro* .rel.gnu.linkonce.d.rel.ro.*) }
  .rela.data.rel.ro   : { *(.rela.data.rel.ro* .rela.gnu.linkonce.d.rel.ro.*) }
  .rel.data       : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
  .rela.data      : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
  .rel.fardata       : { *(.rel.fardata .rel.fardata.*) }
  .rela.fardata      : { *(.rela.fardata .rela.fardata.*) }
  .rel.tdata	  : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
  .rela.tdata	  : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
  .rel.tbss	  : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
  .rela.tbss	  : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
  .rel.ctors      : { *(.rel.ctors) }
  .rela.ctors     : { *(.rela.ctors) }
  .rel.dtors      : { *(.rel.dtors) }
  .rela.dtors     : { *(.rela.dtors) }
  .rel.got        : { *(.rel.got) }
  .rela.got       : { *(.rela.got) }
  .rel.neardata      : { *(.rel.neardata .rel.neardata.* .rel.gnu.linkonce.s.*) }
  .rela.neardata     : { *(.rela.neardata .rela.neardata.* .rela.gnu.linkonce.s.*) }
  .rel.bss       : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.sb.*) }
  .rela.bss      : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.sb.*) }
  .rel.neardata2     : { *(.rel.neardata2 .rel.neardata2.* .rel.gnu.linkonce.s2.*) }
  .rela.neardata2    : { *(.rela.neardata2 .rela.neardata2.* .rela.gnu.linkonce.s2.*) }
  .rel.bss2      : { *(.rel.bss2 .rel.bss2.* .rel.gnu.linkonce.sb2.*) }
  .rela.bss2     : { *(.rela.bss2 .rela.bss2.* .rela.gnu.linkonce.sb2.*) }
  .rel.far        : { *(.rel.far .rel.far.* .rel.gnu.linkonce.b.*) }
  .rela.far       : { *(.rela.far .rela.far.* .rela.gnu.linkonce.b.*) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }
  .dynamic        : { *(.dynamic) }
  .data : { *(.data*) }
  . = 0x10000000;
  .rel.plt        : { *(.rel.plt) }
  .rela.plt       : { *(.rela.plt) }
  .plt            : { *(.plt) }
  .text : { *(.text*) }
  . = .; PROVIDE_HIDDEN (__c6xabi_DSBT_BASE = .);
  .got   : { *(.dsbt) *(.got.plt) *(.igot.plt) *(.got) *(.igot) }
  .neardata : { *(.neardata .neardata.* .gnu.linkonce.s.*) }
  .rodata   : { *(.rodata .rodata.*) }
  .bss : { *(.dynbss) *(.bss .bss.* .gnu.linkonce.sb.*) *(.scommon) }
  .far : { *(.dynfar) *(.far .far.* .gnu.linkonce.b.*) *(COMMON) }
  .c6xabi.attributes 0 : { KEEP (*(.c6xabi.attributes)) KEEP (*(.gnu.attributes)) }
  /DISCARD/ : { *(*) }
}
