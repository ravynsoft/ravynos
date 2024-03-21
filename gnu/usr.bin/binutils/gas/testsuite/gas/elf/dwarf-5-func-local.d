#as: --gdwarf-5 --defsym LOCAL=1
#name: Dwarf5 local function debug info
#readelf: -W -wai
#source: dwarf-3-func.s
#target: i?86-*-* x86_64-*-*

Contents of the .debug_info section:

 +Compilation Unit @ offset (0x)?0:
 +Length: .*
 +Version: +5
 +Unit Type: +DW_UT_compile \(1\)
 +Abbrev Offset: +(0x)?0
 +Pointer Size: .*
#...
 <0><[0-9a-f]+>: Abbrev Number: 1 \(DW_TAG_compile_unit\)
#...
 <1><[0-9a-f]+>: Abbrev Number: 2 \(DW_TAG_subprogram\)
 +<[0-9a-f]+> +DW_AT_name +: \(strp\) \(offset: (0x)?[0-9a-f]+\): lfunc1
 +<[0-9a-f]+> +DW_AT_type +: \(ref_udata\) <0x[0-9a-f]+>
 +<[0-9a-f]+> +DW_AT_low_pc +: \(addr\) (0x)?0
 +<[0-9a-f]+> +DW_AT_high_pc +: \(udata\) 17
 <1><[0-9a-f]+>: Abbrev Number: 2 \(DW_TAG_subprogram\)
 +<[0-9a-f]+> +DW_AT_name +: \(strp\) \(offset: (0x)?[0-9a-f]+\): lfunc2
 +<[0-9a-f]+> +DW_AT_type +: \(ref_udata\) <0x[0-9a-f]+>
 +<[0-9a-f]+> +DW_AT_low_pc +: \(addr\) (0x)?11
 +<[0-9a-f]+> +DW_AT_high_pc +: \(udata\) 3
 <1><[0-9a-f]+>: Abbrev Number: 3 \(DW_TAG_unspecified_type\)
 <1><[0-9a-f]+>: Abbrev Number: 0

Contents of the .debug_abbrev section:

 +Number TAG \((0x)?0\)
 +1 +DW_TAG_compile_unit +\[has children\]
#...
 +2 +DW_TAG_subprogram +\[no children\]
 +DW_AT_name +DW_FORM_strp
 +DW_AT_type +DW_FORM_ref_udata
 +DW_AT_low_pc +DW_FORM_addr
 +DW_AT_high_pc +DW_FORM_udata
 +DW_AT value: 0 +DW_FORM value: 0
#pass
