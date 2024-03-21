/* Linker script to undo -split-sections and merge all sections together when
 * linking relocatable object files for GHCi.
 * ld -r normally retains the individual sections, which is what you would want
 * if the intention is to eventually link into a binary with --gc-sections, but
 * it doesn't have a flag for directly doing what we want. */
SECTIONS
{
    .text : {
        *(.text*)
    }
    .rodata :
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }
    .data.rel.ro : {
        *(.data.rel.ro*)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        *(.bss*)
    }
}
