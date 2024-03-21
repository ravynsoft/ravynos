source_sh ${srcdir}/emulparams/elf32bfin.sh
unset STACK_ADDR
OUTPUT_FORMAT="elf32-bfinfdpic"
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
# This gets us program headers mapped as part of the text segment.
unset EMBEDDED
OTHER_GOT_SYMBOLS=
OTHER_READONLY_SECTIONS="
  .rofixup        : {
    ${RELOCATING+__ROFIXUP_LIST__ = .;}
    *(.rofixup)
    ${RELOCATING+__ROFIXUP_END__ = .;}
  }
"
# 0xfeb00000, 0xfec00000, 0xff700000, 0xff800000, 0xff900000
# 0xffa00000 are also used in Dynamic linker and linux kernel.
# They need to be kept synchronized.
OTHER_SECTIONS="
  .l2.text 0xfeb00000	:
  {
    *(.l2.text)
  }
  .l2.data 0xfec00000	:
  {
    *(.l2.data)
  }
  .l1.data 0xff700000	:
  {
    *(.l1.data)
  }
  .l1.data.A 0xff800000	:
  {
    *(.l1.data.A)
  }
  .l1.data.B 0xff900000	:
  {
    *(.l1.data.B)
  }
  .l1.text  0xffa00000	:
  {
    *(.l1.text)
  }
"
EXTRA_EM_FILE=bfin
