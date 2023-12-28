ENTRY (__start)
SECTIONS
{
  .text :
  {
    *(.text .text.*)
    *(.gnu_extab .gnu_extab.*)
  }
  .eh_frame_hdr :
  {
    KEEP (*(.eh_frame_hdr))
    *(.eh_frame_entry .eh_frame_entry.*)
  }
  .data : {*(.data) }
}
