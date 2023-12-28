SECTIONS { .data ALIGN (16) : { aa = .; LONG(0xdeadbeef); . = ALIGN (16); } }
SECTIONS { .data ALIGN (16) : { bb = .; LONG(0x00c0ffee); . = ALIGN (16); } }
