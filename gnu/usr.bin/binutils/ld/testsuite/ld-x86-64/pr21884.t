OUTPUT_FORMAT("elf64-x86-64");
OUTPUT_ARCH(i386:x86-64);

ENTRY(_start);
SECTIONS {
        . = 0x10000;
        _start = . ;
        .data : {
                *(.data)
        }
}
