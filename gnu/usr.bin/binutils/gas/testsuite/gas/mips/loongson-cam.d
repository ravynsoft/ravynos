#as: -mloongson-cam -mabi=64
#objdump: -M reg-names=numeric -M loongson-cam -dp
#name: Loongson CAM tests

.*:     file format .*

private flags = .*

MIPS ABI Flags Version: 0
ISA: .*
GPR size: .*
CPR1 size: .*
CPR2 size: .*
FP ABI: .*
ISA Extension: None
ASEs:
	Loongson CAM ASE
FLAGS 1: .*
FLAGS 2: .*

Disassembly of section .text:

[0-9a-f]+ <.text>:
.*:	70601075 	campi	\$2,\$3
.*:	70a02035 	campv	\$4,\$5
.*:	70e830b5 	camwi	\$6,\$7,\$8
.*:	714048f5 	ramri	\$9,\$10
