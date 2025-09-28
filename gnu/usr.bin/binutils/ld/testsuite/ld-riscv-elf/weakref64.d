
.*:     file format elf64-(little|big)riscv


Disassembly of section \.text:

0000000090000000 <_start>:
    90000000:	00000793          	li	a5,0
    90000004:	02078663          	beqz	a5,90000030 <_start\+0x30>
    90000008:	00000793          	li	a5,0
    9000000c:	02078263          	beqz	a5,90000030 <_start\+0x30>
    90000010:	ff010113          	addi?	sp,sp,-16
    90000014:	00113423          	sd	ra,8\(sp\)
    90000018:	00000097          	auipc	ra,0x0
    9000001c:	000000e7          	jalr	zero # 0 <_start\-0x90000000>
    90000020:	00813083          	ld	ra,8\(sp\)
    90000024:	01010113          	addi?	sp,sp,16
    90000028:	00000317          	auipc	t1,0x0
    9000002c:	00000067          	jr	zero # 0 <_start\-0x90000000>
    90000030:	00008067          	ret
