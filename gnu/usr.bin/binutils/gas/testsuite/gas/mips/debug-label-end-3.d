#readelf: -s
#name: MIPS ECOFF/PDR debug interaction with labels at .end 3
#as: -32 -no-mdebug -mno-pdr
#source: debug-label-end.s
#dump: debug-label-end-1.d

# Verify that .end finalizes any labels outstanding
# where both ECOFF and PDR generation is disabled, e.g.:
#    Num:    Value  Size Type    Bind   Vis      Ndx Name
#      7: 00000000     4 FUNC    GLOBAL DEFAULT    1 foo
#      8: 00000004     0 FUNC    GLOBAL DEFAULT    1 bar
#      9: 00000020     4 FUNC    GLOBAL DEFAULT    1 baz
# vs:
#    Num:    Value  Size Type    Bind   Vis      Ndx Name
#      7: 00000000     4 FUNC    GLOBAL DEFAULT    1 foo
#      8: 00000010     0 FUNC    GLOBAL DEFAULT    1 bar
#      9: 00000020     4 FUNC    GLOBAL DEFAULT    1 baz
