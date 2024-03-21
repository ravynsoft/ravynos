#PROG: objcopy
#source: testranges-ia64.s
#readelf: -wR --wide
#name: unordered .debug_info references to .debug_ranges
#target: ia64-*-*

Contents of the .debug_ranges section:

    Offset   Begin    End
    00000000 00000001 00000002 ?
    00000000 <End of list>
    00000010 00000000 00000002 ?
    00000010 <End of list>

#pass
