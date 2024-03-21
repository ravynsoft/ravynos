#PROG: objcopy
#name: objcopy rename-section with flags - keep relocation
#source: needed-by-reloc.s
#objcopy: --rename-section .data=myrodata,contents,alloc,load,readonly
#objdump: -r
#notarget: alpha*-*-*vms* rx-*-elf [is_som_format] [is_aout_format]
#xfail: [is_xcoff_format]

.*: +file format .*

#...
RELOCATION RECORDS FOR .*myrodata.*:
OFFSET +TYPE +VALUE
0+ .*
#pass
