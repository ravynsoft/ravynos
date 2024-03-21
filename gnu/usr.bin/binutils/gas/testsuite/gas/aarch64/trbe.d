#name: TRBE System registers
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d5389b40 	mrs	x0, trbbaser_el1
.*:	d5389be0 	mrs	x0, trbidr_el1
.*:	d5389b00 	mrs	x0, trblimitr_el1
.*:	d5389b80 	mrs	x0, trbmar_el1
.*:	d5389b20 	mrs	x0, trbptr_el1
.*:	d5389b60 	mrs	x0, trbsr_el1
.*:	d5389bc0 	mrs	x0, trbtrg_el1
.*:	d5189b40 	msr	trbbaser_el1, x0
.*:	d5189b00 	msr	trblimitr_el1, x0
.*:	d5189b80 	msr	trbmar_el1, x0
.*:	d5189b20 	msr	trbptr_el1, x0
.*:	d5189b60 	msr	trbsr_el1, x0
.*:	d5189bc0 	msr	trbtrg_el1, x0
