PHDRS {
 text PT_LOAD;
}

SECTIONS
{
 . = (0x8000000f + ALIGN(0x1000000, 0x1000000));
 .text : AT(ADDR(.text) - 0x8000000f) {
 } :text
}
