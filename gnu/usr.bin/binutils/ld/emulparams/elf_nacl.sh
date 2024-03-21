ENABLE_INITFINI_ARRAY=yes
SEPARATE_CODE=yes
TEXT_START_ADDR=0x20000
NACL_RODATA_DISTANCE=0x10000000

nacl_rodata_addr()
{
  nacl_text_addr="SEGMENT_START(\"text-segment\", $1)"
  nacl_rodata_addr="$nacl_text_addr + ${NACL_RODATA_DISTANCE}"
  echo "$nacl_rodata_addr"
}

RODATA_ADDR=`nacl_rodata_addr "${TEXT_START_ADDR}"`
SHLIB_RODATA_ADDR=`nacl_rodata_addr 0`
