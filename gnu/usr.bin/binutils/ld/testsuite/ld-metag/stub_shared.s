	.text
	.global _lib_func
	.type   _lib_func,function
_lib_func:
        MOV     D0FrT,A0FrP
        ADD     A0FrP,A0StP,#0
        SETL    [A0StP+#8++],D0.4,D1RtP
	SETD    [A0StP+#8++],A1LbP
        ADD     A0StP,A0StP,#8
        ADDT    A1LbP,CPC1,#HI(__GLOBAL_OFFSET_TABLE__)
        ADD     A1LbP,A1LbP,#LO(__GLOBAL_OFFSET_TABLE__+4)
	CALLR   D1RtP,_far2@PLT
        GETD    A1LbP,[A0StP+#(-(8+8))]
	GETL    D0.4,D1RtP,[A0FrP+#8++]
        SUB     A0StP,A0FrP,#(8)
	MOV     A0FrP,D0.4
	MOV     PC,D1RtP
	.size   _lib_func,.-_lib_func

	.data
	.balign 4
	.type _lib_data,@object
	.size _lib_data,4
	.global _lib_data
_lib_data:
	.long 0
