	.text

	.global lib_func1
	.type   lib_func1,function
lib_func1:
        MOV     D0FrT,A0FrP
        ADD     A0FrP,A0StP,#0
        SETL    [A0StP+#8++],D0.4,D1RtP
	SETD    [A0StP+#8++],A1LbP
        ADD     A0StP,A0StP,#8
        ADDT    A1LbP,CPC1,#HI(__GLOBAL_OFFSET_TABLE__)
        ADD     A1LbP,A1LbP,#LO(__GLOBAL_OFFSET_TABLE__+4)
	CALLR   D1RtP,app_func2@PLT
	GETD    D0Ar6,[A1LbP+#(_var1@GOT)]
	ADD     D0Re0,D0Re0,D0Ar6
	MOV     D1Re0,A1LbP
	ADDT    D1Re0,D1Re0,#HI(_local_var1@GOTOFF)
	ADD     D1Re0,D1Re0,#LO(_local_var1@GOTOFF)
        GETD    A1LbP,[A0StP+#(-(8+8))]
	GETL    D0.4,D1RtP,[A0FrP+#8++]
        SUB     A0StP,A0FrP,#(8)
	MOV     A0FrP,D0.4
	MOV     PC,D1RtP
	.size   lib_func1,.-lib_func1

	.data
_local_var1:
	.long 0
