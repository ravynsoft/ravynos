#DUMPPROG: readelf
#readelf: -s
#name: MIPS .aent directive with ECOFF debug 2
#as: -32 -mdebug
#source: aent.s
#dump: aent-2.d

# Verify that the .aent directive retains function symbol type annotation,
# e.g.:
#    Num:    Value  Size Type    Bind   Vis      Ndx Name
#      8: 00000000    16 FUNC    GLOBAL DEFAULT    1 foo
#      9: 00000008     0 FUNC    GLOBAL DEFAULT    1 bar
# vs:
#    Num:    Value  Size Type    Bind   Vis      Ndx Name
#      8: 00000000    16 FUNC    GLOBAL DEFAULT    1 foo
#      9: 00000008     0 OBJECT  GLOBAL DEFAULT    1 bar
