MEMORY
{
	vle_seg1 (rxw):     	org = 0x00000000, len = 0x10000
	vle_seg2 (rxw):     	org = 0x00100000, len = 0x10000
        nonvle_seg (rxw):	org = 0x001F0000, len = 0x20000
}
SECTIONS
{
  .data		0x00000100 :
  {
	*(.data)
	*(.ctors)
	*(.dtors)
	*(.eh_frame)
	*(.jcr)
  }
  .text_vle	0x00001000 :
  {
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.text*)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.init*)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.fini*)
  }  > vle_seg1

  .text_iv	0x100000 :
  {
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.text_iv)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.iv_handlers)
  } >vle_seg2

  .text 	0x101000 :
  {
	INPUT_SECTION_FLAGS (!SHF_PPC_VLE) *(.text*)
  }
}
