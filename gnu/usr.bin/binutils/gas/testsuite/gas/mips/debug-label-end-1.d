#readelf: -s
#name: MIPS ECOFF/PDR debug interaction with labels at .end 1
#as: -32 -no-mdebug -mpdr
#source: debug-label-end.s

# Verify that .end finalizes any labels outstanding
# where PDR debug generation is enabled, e.g.:
#    Num:    Value  Size Type    Bind   Vis      Ndx Name
#      7: 00000000     4 FUNC    GLOBAL DEFAULT    1 foo
#      8: 00000004     0 FUNC    GLOBAL DEFAULT    1 bar
#      9: 00000020     4 FUNC    GLOBAL DEFAULT    1 baz
# vs:
#    Num:    Value  Size Type    Bind   Vis      Ndx Name
#      7: 00000000     4 FUNC    GLOBAL DEFAULT    1 foo
#      8: 00000010     0 FUNC    GLOBAL DEFAULT    1 bar
#      9: 00000020     4 FUNC    GLOBAL DEFAULT    1 baz
#...
 *[0-9]+: +0+000000 +4 +FUNC +GLOBAL +DEFAULT +[0-9]+ foo
 *[0-9]+: +0+000004 +0 +FUNC +GLOBAL +DEFAULT +[0-9]+ bar
 *[0-9]+: +0+000020 +4 +FUNC +GLOBAL +DEFAULT +[0-9]+ baz
#pass
