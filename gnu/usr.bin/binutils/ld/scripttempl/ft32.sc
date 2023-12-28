TORS=".tors :
  {
    ___ctors = . ;
    *(.ctors)
    ___ctors_end = . ;
    ___dtors = . ;
    *(.dtors)
    ___dtors_end = . ;
    . = ALIGN(4);
  } ${RELOCATING+ > ram}"

cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})
${LIB_SEARCH_DIRS}
EOF

test -n "${RELOCATING}" && cat <<EOF
/* Allow the command line to override the memory region sizes.  */
__PMSIZE = DEFINED(__PMSIZE)  ? __PMSIZE : 256K;
__RAMSIZE = DEFINED(__RAMSIZE) ? __RAMSIZE : 64K;

MEMORY
{
  flash     (rx)   : ORIGIN = 0,        LENGTH = __PMSIZE
  ram       (rw!x) : ORIGIN = 0x800000, LENGTH = __RAMSIZE
}
EOF

cat <<EOF
SECTIONS
{
  .text :
  {
    *(.text${RELOCATING+*})
    ${RELOCATING+*(.strings)
    *(._pm*)
    KEEP (*(SORT_NONE(.init)))
    KEEP (*(SORT_NONE(.fini)))
    _etext = .;
    . = ALIGN(4);}
  } ${RELOCATING+ > flash}
  ${CONSTRUCTING+${TORS}}
  .data	: ${RELOCATING+ AT (ADDR (.text) + SIZEOF (.text))}
  {
    *(.data)
    ${RELOCATING+*(.rodata)
    *(.rodata*)
    _edata = .;
    . = ALIGN(4);}
  } ${RELOCATING+ > ram}
  .bss  ${RELOCATING+ SIZEOF(.data) + ADDR(.data)} :
  {
    ${RELOCATING+ _bss_start = . ; }
    *(.bss)
    ${RELOCATING+*(COMMON)
    _end = .;
    . = ALIGN(4);}
  } ${RELOCATING+ > ram}

  ${RELOCATING+ __data_load_start = LOADADDR(.data); }
  ${RELOCATING+ __data_load_end = __data_load_start + SIZEOF(.data); }
EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
}
EOF
