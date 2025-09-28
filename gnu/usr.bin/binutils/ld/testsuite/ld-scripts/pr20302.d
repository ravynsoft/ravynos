#ld: -Tdata=0x1000 -Tdata=0x2000 -Tcross2.t
#source: align2a.s
#objdump: -h
#notarget: *-*-*aout *-*-netbsd *-*-vms ns32k-*-* rx-*-*
# AOUT and NETBSD (ns32k is aout) have fixed address for the data section.
# VMS targets need extra libraries.
# RX uses non standard section names.

#...
  . \.data[ 	]+0+[0-9a-f]+[ 	]+0+02000[ 	]+0+02000.*
#pass
