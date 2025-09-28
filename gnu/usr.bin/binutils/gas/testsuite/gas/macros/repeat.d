#objdump: -r
#name: nested irp/irpc/rept
# darwin (mach-o) reverse relocs.
#xfail: *-*-darwin*

.*: +file format .*

RELOCATION RECORDS FOR .*
OFFSET[ 	]+TYPE[ 	]+VALUE.*
0+00[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irp_19
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+04[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irp_18
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+08[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irp_29
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+0c[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irp_28
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+10[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irpc_19
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+14[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irpc_18
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+18[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irpc_29
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+1c[ 	]+[a-zA-Z0-9_]+[ 	]+irp_irpc_28
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+20[ 	]+[a-zA-Z0-9_]+[ 	]+irp_rept_1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+24[ 	]+[a-zA-Z0-9_]+[ 	]+irp_rept_1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+28[ 	]+[a-zA-Z0-9_]+[ 	]+irp_rept_2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+2c[ 	]+[a-zA-Z0-9_]+[ 	]+irp_rept_2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+30[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irp_19
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+34[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irp_18
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+38[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irp_29
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+3c[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irp_28
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+40[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irpc_19
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+44[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irpc_18
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+48[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irpc_29
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+4c[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_irpc_28
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+50[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_rept_1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+54[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_rept_1
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+58[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_rept_2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+5c[ 	]+[a-zA-Z0-9_]+[ 	]+irpc_rept_2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+60[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irp_9
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+64[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irp_8
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+68[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irp_9
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+6c[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irp_8
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+70[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irpc_9
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+74[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irpc_8
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+78[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irpc_9
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+7c[ 	]+[a-zA-Z0-9_]+[ 	]+rept_irpc_8
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+80[ 	]+[a-zA-Z0-9_]+[ 	]+rept_rept
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+84[ 	]+[a-zA-Z0-9_]+[ 	]+rept_rept
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+88[ 	]+[a-zA-Z0-9_]+[ 	]+rept_rept
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+8c[ 	]+[a-zA-Z0-9_]+[ 	]+rept_rept
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
#pass
