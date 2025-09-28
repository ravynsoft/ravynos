#readelf: -x .MIPS.abiflags
#name: MIPS .MIPS.abiflags section size 1
#source: reginfo-1.s RUN_OBJCOPY
#objcopy_objects: -R .MIPS.abiflags
#ld: -T mips-abiflags-1.ld
#dump: mips-abiflags-0.d
