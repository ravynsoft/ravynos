cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}","${OUTPUT_FORMAT}","${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

EOF

test -n "${RELOCATING}" && cat <<EOF
/* Allow memory sizes to be overridden from command line.  */
__IMEM_SIZE = DEFINED(__IMEM_SIZE) ? __IMEM_SIZE : $TEXT_LENGTH;
__DMEM_SIZE = DEFINED(__DMEM_SIZE) ? __DMEM_SIZE : $DATA_LENGTH;

MEMORY
{
  imem   (x)   : ORIGIN = $TEXT_ORIGIN, LENGTH = __IMEM_SIZE
  dmem   (rw!x) : ORIGIN = $DATA_ORIGIN, LENGTH = __DMEM_SIZE
}

__HEAP_SIZE = DEFINED(__HEAP_SIZE) ? __HEAP_SIZE : 32;
__STACK_SIZE = DEFINED(__STACK_SIZE) ? __STACK_SIZE : 512;

PROVIDE (_stack_top = ORIGIN(dmem) + LENGTH(dmem));

ENTRY (_start)

EOF

OUTPUT_SECTION_ALIGN="
    ${RELOCATING+/* In case this is the last input section,
      align to keep the loadable segment size a multiple of the common page size.
      Some SoCs have stricter memory size requirements than others.  */
    . = ALIGN (CONSTANT (COMMONPAGESIZE));}"

cat <<EOF
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash        ${RELOCATING-0} : { *(.hash)		}
  .dynsym      ${RELOCATING-0} : { *(.dynsym)		}
  .dynstr      ${RELOCATING-0} : { *(.dynstr)		}
  .gnu.version ${RELOCATING-0} : { *(.gnu.version)	}
  .gnu.version_d ${RELOCATING-0} : { *(.gnu.version_d)	}
  .gnu.version_r ${RELOCATING-0} : { *(.gnu.version_r)	}

  .rel.init    ${RELOCATING-0} : { *(.rel.init)		}
  .rela.init   ${RELOCATING-0} : { *(.rela.init)	}
  .rel.text    ${RELOCATING-0} :
    {
      *(.rel.text)
      ${RELOCATING+*(.rel.text.*)}
      ${RELOCATING+*(.rel.text:*)}
      ${RELOCATING+*(.rel.gnu.linkonce.t*)}
    }
  .rela.text   ${RELOCATING-0} :
    {
      *(.rela.text)
      ${RELOCATING+*(.rela.text.*)}
      ${RELOCATING+*(.rela.text:*)}
      ${RELOCATING+*(.rela.gnu.linkonce.t*)}
    }
  .rel.fini    ${RELOCATING-0} : { *(.rel.fini)		}
  .rela.fini   ${RELOCATING-0} : { *(.rela.fini)	}
  .rel.rodata  ${RELOCATING-0} :
    {
      *(.rel.rodata)
      ${RELOCATING+*(.rel.rodata.*)}
      ${RELOCATING+*(.rel.rodata:*)}
      ${RELOCATING+*(.rel.gnu.linkonce.r*)}
    }
  .rela.rodata ${RELOCATING-0} :
    {
      *(.rela.rodata)
      ${RELOCATING+*(.rela.rodata.*)}
      ${RELOCATING+*(.rela.rodata:*)}
      ${RELOCATING+*(.rela.gnu.linkonce.r*)}
    }
  .rel.data    ${RELOCATING-0} :
    {
      *(.rel.data)
      ${RELOCATING+*(.rel.data.*)}
      ${RELOCATING+*(.rel.data:*)}
      ${RELOCATING+*(.rel.gnu.linkonce.d*)}
    }
  .rela.data   ${RELOCATING-0} :
    {
      *(.rela.data)
      ${RELOCATING+*(.rela.data.*)}
      ${RELOCATING+*(.rela.data:*)}
      ${RELOCATING+*(.rela.gnu.linkonce.d*)}
    }
  .rel.init_array   	${RELOCATING-0} : { *(.rel.init_array)	}
  .rela.init_array  	${RELOCATING-0} : { *(.rela.init_array)	}
  .rel.fini_array   	${RELOCATING-0} : { *(.rel.fini_array)	}
  .rela.fini_array  	${RELOCATING-0} : { *(.rela.fini_array)	}
  .rel.got     		${RELOCATING-0} : { *(.rel.got)		}
  .rela.got    		${RELOCATING-0} : { *(.rela.got)	}
  .rel.bss     		${RELOCATING-0} : { *(.rel.bss)		}
  .rela.bss    		${RELOCATING-0} : { *(.rela.bss)	}
  .rel.plt     		${RELOCATING-0} : { *(.rel.plt)		}
  .rela.plt    		${RELOCATING-0} : { *(.rela.plt)	}

  /* Internal text space.  */
  .text ${RELOCATING-0} :
  {
    ${RELOCATING+ _text_start = . ; }

    ${RELOCATING+. = ALIGN(4);}

    ${RELOCATING+*(.init0)  /* Start here after reset.  */}
    ${RELOCATING+KEEP (*(.init0))}

    ${RELOCATING+. = ALIGN(4);}
    *(.text)
    ${RELOCATING+. = ALIGN(4);}
    ${RELOCATING+*(.text.*)}
    ${RELOCATING+. = ALIGN(4);}
    ${RELOCATING+*(.text:*)}
    ${RELOCATING+. = ALIGN(4);}
    ${RELOCATING+*(.gnu.linkonce.t*)}
    ${RELOCATING+. = ALIGN(4);}

    ${RELOCATING+ _text_end = . ; }
  } ${RELOCATING+ > imem}

  .data        ${RELOCATING-0} :
  {
    /* Optional variable that user is prepared to have NULL address.  */
    ${RELOCATING+ *(.data.atzero*)}

    /* CRT is prepared for constructor/destructor table to have
       a "valid" NULL address.  */
    ${CONSTRUCTING+ __init_array_start = . ; }
    ${CONSTRUCTING+ KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)))}
    ${CONSTRUCTING+ KEEP (*(.init_array))}
    ${CONSTRUCTING+ __init_array_end = . ; }
    ${CONSTRUCTING+ __fini_array_start = . ; }
    ${CONSTRUCTING+ KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*)))}
    ${CONSTRUCTING+ KEEP (*(.fini_array))}
    ${CONSTRUCTING+ __fini_array_end = . ; }

    ${RELOCATING+/* DATA memory starts at address 0.  So to avoid placing a valid static
       variable at the invalid NULL address, we introduce the .data.atzero
       section.  If CRT can make some use of it - great.  Otherwise skip a
       word.  In all cases .data/.bss sections must start at non-zero.  */
    . += (. == 0 ? 4 : 0);}

    ${RELOCATING+ PROVIDE (_data_start = .) ; }
    *(.data)
    ${RELOCATING+ *(.data*)}
    ${RELOCATING+ *(.data:*)}
    ${RELOCATING+ *(.rodata)  /* We need to include .rodata here if gcc is used.  */}
    ${RELOCATING+ *(.rodata.*) /* with -fdata-sections.  */}
    ${RELOCATING+ *(.rodata:*)}
    ${RELOCATING+*(.gnu.linkonce.d*)}
    ${RELOCATING+*(.gnu.linkonce.r*)}
    ${RELOCATING+ PROVIDE (_data_end = .) ; }

    ${RELOCATING+/* Merge the bss input sections into the output
      data section.  The Linux kernel's remoteproc PRU ELF loader
      will not memzero the bss section.  The CRT0 will not either, in order
      to reduce the final firmware's instruction memory size.  Hence
      present bss sections as regular data sections, at the negligible
      expense of increasing the ELF file size.  */}
    ${RELOCATING+ PROVIDE (_bss_start = .) ; }
    *(.bss)
    ${RELOCATING+ *(.bss.*)}
    ${RELOCATING+ *(.bss:*)}
    ${RELOCATING+*(.gnu.linkonce.b*)}
    ${RELOCATING+*(COMMON)}
    ${RELOCATING+ PROVIDE (_bss_end = .) ; }

    ${OUTPUT_SECTION_ALIGN}
  } ${RELOCATING+ > dmem}

  .eh_frame ${RELOCATING-0} :
  {
    KEEP (*(.eh_frame))${RELOCATING+ *(.eh_frame.*)}
    ${OUTPUT_SECTION_ALIGN}
  } ${RELOCATING+ > dmem}

  .gnu_extab ${RELOCATING-0} :
  {
    *(.gnu_extab)
    ${OUTPUT_SECTION_ALIGN}
  } ${RELOCATING+ > dmem}

  .gcc_except_table ${RELOCATING-0} :
  {
    *(.gcc_except_table${RELOCATING+ .gcc_except_table.*})
    ${OUTPUT_SECTION_ALIGN}
  } ${RELOCATING+ > dmem}

  /* Linux remoteproc loader requires the resource_table section
     start address to be aligned to 8 bytes for SoCs with AARCH64
     host processors.  */
  .resource_table ${RELOCATING-0} ${RELOCATING+ ALIGN (CONSTANT (MAXPAGESIZE))} :
  {
    KEEP (*(.resource_table))
    ${OUTPUT_SECTION_ALIGN}
  } ${RELOCATING+ > dmem}

  /* Global data not cleared after reset.  */
  .noinit ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (_noinit_start = .) ; }
    *(.noinit${RELOCATING+ .noinit.* .gnu.linkonce.n.*})
    ${RELOCATING+ PROVIDE (_noinit_end = .) ; }
    ${RELOCATING+ PROVIDE (_heap_start = .) ; }
    ${RELOCATING+ . += __HEAP_SIZE ; }
    ${RELOCATING+/* Stack is not here really.  It will be put at the end of DMEM.
       But we take into account its size here, in order to allow
       for MEMORY overflow checking during link time.  */}
    ${RELOCATING+ . += __STACK_SIZE ; }
  } ${RELOCATING+ > dmem}

  /* Remoteproc loader in Linux kernel 5.10 and later reads this section
     to setup the PRUSS interrupt controller.  The interrupt map section
     is never referenced from PRU firmware, so there is no need to
     place it in the target dmem memory.  */
  .pru_irq_map 0 : { *(.pru_irq_map) }

  .note.gnu.build-id ${RELOCATING-0} : { *(.note.gnu.build-id) }
EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
}
EOF
