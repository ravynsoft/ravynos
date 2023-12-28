#as: -march=rv64ic -defsym rv64=1
#source: dis-addr-overflow.s
#objdump: -d

.*:     file format elf64-(little|big)riscv


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+fffff2b7[ 	]+lui[  	]+t0,0xfffff
[ 	]+[0-9a-f]+:[ 	]+ffc2a903[ 	]+lw[   	]+s2,-4\(t0\) # ffffffffffffeffc <addr_load>
[ 	]+[0-9a-f]+:[ 	]+ffffe337[ 	]+lui[  	]+t1,0xffffe
[ 	]+[0-9a-f]+:[ 	]+ff332c23[ 	]+sw[   	]+s3,-8\(t1\) # ffffffffffffdff8 <addr_store>
[ 	]+[0-9a-f]+:[ 	]+ffffd3b7[ 	]+lui[  	]+t2,0xffffd
[ 	]+[0-9a-f]+:[ 	]+000380e7[ 	]+jalr[ 	]+t2 # ffffffffffffd000 <addr_jalr_1>
[ 	]+[0-9a-f]+:[ 	]+ffffce37[ 	]+lui[  	]+t3,0xffffc
[ 	]+[0-9a-f]+:[ 	]+ff4e00e7[ 	]+jalr[ 	]+-12\(t3\) # ffffffffffffbff4 <addr_jalr_2>
[ 	]+[0-9a-f]+:[ 	]+ffffbeb7[ 	]+lui[  	]+t4,0xffffb
[ 	]+[0-9a-f]+:[ 	]+000e8a67[ 	]+jalr[ 	]+s4,t4 # ffffffffffffb000 <addr_jalr_3>
[ 	]+[0-9a-f]+:[ 	]+ffffaf37[ 	]+lui[  	]+t5,0xffffa
[ 	]+[0-9a-f]+:[ 	]+ff0f0a93[ 	]+add[ 	]+s5,t5,-16 # ffffffffffff9ff0 <addr_loadaddr>
[ 	]+[0-9a-f]+:[ 	]+ffff9fb7[ 	]+lui[  	]+t6,0xffff9
[ 	]+[0-9a-f]+:[ 	]+1fb1[ 	]+add[ 	]+t6,t6,-20 # ffffffffffff8fec <addr_loadaddr_c>
[ 	]+[0-9a-f]+:[ 	]+ffff8b37[ 	]+lui[  	]+s6,0xffff8
[ 	]+[0-9a-f]+:[ 	]+fe8b0b9b[ 	]+addw[ 	]+s7,s6,-24 # ffffffffffff7fe8 <addr_loadaddr_w>
[ 	]+[0-9a-f]+:[ 	]+ffff7c37[ 	]+lui[  	]+s8,0xffff7
[ 	]+[0-9a-f]+:[ 	]+3c11[ 	]+addw[ 	]+s8,s8,-28 # ffffffffffff6fe4 <addr_loadaddr_w_c>
[ 	]+[0-9a-f]+:[ 	]+4001a283[ 	]+lw[   	]+t0,1024\(gp\) # 600 <addr_rel_gp_pos>
[ 	]+[0-9a-f]+:[ 	]+c001a303[ 	]+lw[   	]+t1,-1024\(gp\) # fffffffffffffe00 <addr_rel_gp_neg>
[ 	]+[0-9a-f]+:[ 	]+10002383[ 	]+lw[   	]+t2,256\(zero\) # 100 <addr_rel_zero_pos>
[ 	]+[0-9a-f]+:[ 	]+80002e03[ 	]+lw[   	]+t3,-2048\(zero\) # fffffffffffff800 <addr_rel_zero_neg>
[ 	]+[0-9a-f]+:[ 	]+10400ee7[ 	]+jalr[ 	]+t4,260\(zero\) # 104 <addr_jalr_rel_zero_pos>
[ 	]+[0-9a-f]+:[ 	]+80400f67[ 	]+jalr[ 	]+t5,-2044\(zero\) # fffffffffffff804 <addr_jalr_rel_zero_neg>
