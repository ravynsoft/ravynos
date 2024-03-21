# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

if test "${OUTPUT_FORMAT}" = "elf32-z80"; then
  NO_REL_RELOCS=1
  NO_RELA_RELOCS=1
  NO_SMALL_DATA=1
  EMBEDDED=1
  ALIGNMENT=1
  source_sh $srcdir/scripttempl/elf.sc
  return 0
fi

cat << EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH("${ARCH}")
SECTIONS
{
.isr :	{
	${RELOCATING+ __Labs = .;}
	*(.isr)
	*(isr)
	${RELOCATING+ __Habs = .;}
	}
.text :	{
	${RELOCATING+ __Ltext = .;}
	*(.text)
	*(text)
	${RELOCATING+ __Htext = .;}
	}
.data :	{
	${RELOCATING+ __Ldata = .;}
	*(.data)
	*(data)
	${RELOCATING+ __Hdata = .;}
	}
.bss :	{
	${RELOCATING+ __Lbss = .;}
	*(.bss)
	*(bss)
	${RELOCATING+ __Hbss = .;}
	}
EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
}
EOF
