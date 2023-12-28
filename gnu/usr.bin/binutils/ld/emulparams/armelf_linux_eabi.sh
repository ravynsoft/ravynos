source_sh ${srcdir}/emulparams/armelf_linux.sh

# Use the ARM ABI-compliant exception-handling sections.
OTHER_READONLY_SECTIONS="
  .ARM.extab ${RELOCATING-0} : { *(.ARM.extab${RELOCATING+* .gnu.linkonce.armextab.*}) }
  .ARM.exidx ${RELOCATING-0} :
    {
      ${RELOCATING+PROVIDE_HIDDEN (__exidx_start = .);}
      *(.ARM.exidx${RELOCATING+* .gnu.linkonce.armexidx.*})
      ${RELOCATING+PROVIDE_HIDDEN (__exidx_end = .);}
    }"
