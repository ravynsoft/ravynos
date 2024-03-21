SECTIONS
{
  .text_vle	0x00001000 :
  {
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.text_vle)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.text)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.init)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.init_vle) 
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.fini)
	INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.fini_vle) 		
  }
  .text_iv	ALIGN(16)  : { *(.text_iv) }
  .data		0x00001400 :
  { *(.data) *(.ctors) *(.dtors) *(.eh_frame) *(.jcr) }
  .iv_handlers	0x0001F000 : { *(.iv_handlers) }
}
