
MEMORY
{
        code_rom (rxw)	:	org = 0x00001000, len = 0x1EF000
        irpt_rom (rx)	:	org = 0x001F0000, len = 0x2000
        int__ram (rxw)	:	org = 0x40000000, len = 256K
}

REGION_ALIAS("INTR", irpt_rom)
REGION_ALIAS("CODE", code_rom)
REGION_ALIAS("RODATA", code_rom)
REGION_ALIAS("RAM", int__ram)

SECTIONS
{
	.iv_handlers :
	{
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.iv_handlers)
	} > INTR

	.text_vle :
	{ 
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.text_vle)
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.text)
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.init)
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.init_vle) 
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.fini)
		INPUT_SECTION_FLAGS (SHF_PPC_VLE) *(.fini_vle) 		
	} > CODE
        
	.rodata :
	{
		*(.rodata)
	} > RODATA
        
	.data   :
	{
		*(.data)
		*(.data.*)
		*(.ctors)
		*(.dtors)    
	}  > RAM AT>RODATA

}
