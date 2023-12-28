	.section	.slot_test0,"",@progbits
	data4.ua	@slotcount(.L1-.L0)

	.text
	.align 16
foo:
[.L0:]
	mov r2 = r12
[.L1:]
	mov r8 = r14
[.L2:]
	;;
	mov r12 = r2
[.L3:]
        {
        .mii
        nop 0
[.L4:]
        nop 0
[.L5:]
        nop 0
        }
        {
[.L6:]
        nop 0
[.L7:]
        nop 0
[.L8:]
	br.ret.sptk.many b0
	;;
        }

	.section	.slot_test,"",@progbits
//     	data4.ua	@slotcount(.Lundef)

	data4.ua	@slotcount(17)

	data4.ua	@slotcount(.L1-.L0) // 1
	data4.ua	@slotcount(.L2-.L0) // 2
	data4.ua	@slotcount(.L3-.L0) // 3
	data4.ua	@slotcount(.L4-.L0) // 4
	data4.ua	@slotcount(.L5-.L0) // 5
	data4.ua	@slotcount(.L6-.L0) // 6
	data4.ua	@slotcount(.L7-.L0) // 7
	data4.ua	@slotcount(.L8-.L0) // 8

        data4.ua	@slotcount(.L3-.L1) // 2
        data4.ua	@slotcount(.L8-.L2) // 6
        data4.ua	@slotcount(.L4-.L1) // 3
        data4.ua	@slotcount(.L4-.L2) // 2
//     	data4.ua	@slotcount(.L2-.Lundef)
