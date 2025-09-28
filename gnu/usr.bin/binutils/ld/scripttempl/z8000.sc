# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH("${OUTPUT_ARCH}")
${RELOCATING+ENTRY (_start)}

SECTIONS
{
.text ${BIG+ ${RELOCATING+ 0x0000000}} :
	{
	  *(.text)
	  *(.strings)
	  *(.rdata)
	}

.ctors ${BIG+ ${RELOCATING+ 0x2000000}}  :
	{
	  ${CONSTRUCTING+ ___ctors = . ;  }
	  *(.ctors);
	  ${CONSTRUCTING+ ___ctors_end = . ; }
	  ___dtors = . ;
	  *(.dtors);
	  ${CONSTRUCTING+ ___dtors_end = . ; }
	}

.data ${BIG+ ${RELOCATING+ 0x3000000}} :
	{
	   *(.data)
	}

.bss ${BIG+ ${RELOCATING+ 0x4000000}} :
	{
	  ${RELOCATING+ __start_bss = . ; }
	  *(.bss);
	  *(COMMON);
	  ${RELOCATING+ __end_bss = . ; }
	}

.heap ${BIG+ ${RELOCATING+ 0x5000000}} :
	{
	  ${RELOCATING+ __start_heap = . ; }
	  ${RELOCATING+ . = . + 20k  ; }
	  ${RELOCATING+ __end_heap = . ; }
	}

.stack ${RELOCATING+ 0xf000 }  :
	{
	  ${RELOCATING+ _stack = . ; }
	  *(.stack)
	  ${RELOCATING+ __stack_top = . ; }
	}

}
EOF




