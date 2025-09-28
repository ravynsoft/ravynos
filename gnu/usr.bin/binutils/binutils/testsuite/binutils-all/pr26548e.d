#source: pr26548.s
#as: --defsym ERROR=1
#readelf: -Wwi

#...
.*: Abbrev Number: 1 \(DW_TAG_variable\)
.*   DW_AT_const_value : \(sdata\).*LEB value.*
 -9223372036854775808
.*: Abbrev Number: 1 \(DW_TAG_variable\)
.*   DW_AT_const_value : \(sdata\).*LEB value.*
 9223372036854775807
