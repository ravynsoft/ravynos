 .data
 .macro MACRO1
 .endm
 .macro macro2
 .endm
	MACRO1
	MACRO2
	macro1
	macro2
 .purgem MACRO1
 .purgem macro2
	MACRO1
	MACRO2
	macro1
	macro2
 .purgem macro1
 .purgem MACRO2
 .macro macro1
 .endm
 .macro MACRO2
 .endm
	MACRO1
	MACRO2
	macro1
	macro2
 .purgem MACRO1
 .purgem macro2
