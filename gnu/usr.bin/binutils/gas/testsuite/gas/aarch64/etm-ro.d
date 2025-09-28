#name: ETM read-only system registers
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:	d5317ec0 	mrs	x0, trcauthstatus
[^:]+:	d5317ce0 	mrs	x0, trccidr0
[^:]+:	d5317de0 	mrs	x0, trccidr1
[^:]+:	d5317ee0 	mrs	x0, trccidr2
[^:]+:	d5317fe0 	mrs	x0, trccidr3
[^:]+:	d5317ac0 	mrs	x0, trcdevaff0
[^:]+:	d5317bc0 	mrs	x0, trcdevaff1
[^:]+:	d5317fc0 	mrs	x0, trcdevarch
[^:]+:	d53172e0 	mrs	x0, trcdevid
[^:]+:	d53173e0 	mrs	x0, trcdevtype
[^:]+:	d53108e0 	mrs	x0, trcidr0
[^:]+:	d53109e0 	mrs	x0, trcidr1
[^:]+:	d5310ae0 	mrs	x0, trcidr2
[^:]+:	d5310be0 	mrs	x0, trcidr3
[^:]+:	d5310ce0 	mrs	x0, trcidr4
[^:]+:	d5310de0 	mrs	x0, trcidr5
[^:]+:	d5310ee0 	mrs	x0, trcidr6
[^:]+:	d5310fe0 	mrs	x0, trcidr7
[^:]+:	d53100c0 	mrs	x0, trcidr8
[^:]+:	d53101c0 	mrs	x0, trcidr9
[^:]+:	d53102c0 	mrs	x0, trcidr10
[^:]+:	d53103c0 	mrs	x0, trcidr11
[^:]+:	d53104c0 	mrs	x0, trcidr12
[^:]+:	d53105c0 	mrs	x0, trcidr13
[^:]+:	d5317dc0 	mrs	x0, trclsr
[^:]+:	d5311180 	mrs	x0, trcoslsr
[^:]+:	d5311580 	mrs	x0, trcpdsr
[^:]+:	d53178e0 	mrs	x0, trcpidr0
[^:]+:	d53179e0 	mrs	x0, trcpidr1
[^:]+:	d5317ae0 	mrs	x0, trcpidr2
[^:]+:	d5317be0 	mrs	x0, trcpidr3
[^:]+:	d53174e0 	mrs	x0, trcpidr4
[^:]+:	d53175e0 	mrs	x0, trcpidr5
[^:]+:	d53176e0 	mrs	x0, trcpidr6
[^:]+:	d53177e0 	mrs	x0, trcpidr7
[^:]+:	d5310300 	mrs	x0, trcstatr
