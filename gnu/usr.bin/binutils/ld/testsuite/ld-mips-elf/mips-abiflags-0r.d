#readelf: -x .MIPS.abiflags
#name: MIPS .MIPS.abiflags section size 0 (relocatable)
#source: empty.s RUN_OBJCOPY
#objcopy_objects: -R .MIPS.abiflags
#ld: -r -T mips-abiflags-0.ld
#dump: mips-abiflags-0.d
