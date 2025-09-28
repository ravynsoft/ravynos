SECTIONS
{
.bss :
{
*(.bss)
end = .;
}
. = 0x1000;
.got : { *(.got.plt) *(.got) }
.text :
{
*(.text)
}
. = 0x8ff5;
.rodata : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
.data :
{
*(.data)
}
edata = .;
.stac :
{
*(.stack)
}
.plt  :  { *(.plt) }
.rela.plt : { *(.rela.plt) }
.rela.dyn :
{
*(.rela.text)
}
}
