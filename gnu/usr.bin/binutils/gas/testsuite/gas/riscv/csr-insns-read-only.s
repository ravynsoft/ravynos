# CSRRW and CSRRWI always write CSR
# CSRRS, CSRRC, CSRRSI and CSRRCI write CSR when rs isn't zero.

# csrrw rd, csr, rs
	csrrw	a0, ustatus, a1
	csrrw	a0, cycle, a1
	csrrw	a0, cycle, zero
	csrrw	zero, cycle, a1
	csrrw	zero, cycle, zero
	fscsr	a0, a1
	fsrm	a0, a1
	fsflags a0, a1
# csrrw zero, csr, rs
	csrw	ustatus, a1
	csrw	cycle, a1
	csrw	cycle, zero
	fscsr	a1
	fsrm	a1
	fsflags a1
# csrrwi rd, csr, imm
	csrrwi	a0, ustatus, 0xb
	csrrwi	a0, cycle, 0xb
	csrrwi	a0, cycle, 0x0
	csrrwi	zero, cycle, 0xb
	csrrwi	zero, cycle, 0x0
# csrrwi zero, csr, imm
	csrwi   ustatus, 0xb
	csrwi   cycle, 0xb
	csrwi   cycle, 0x0

# csrrs rd, csr, rs
	csrrs	a0, ustatus, a1
	csrrs	a0, cycle, a1
	csrrs	a0, cycle, zero
	csrrs	zero, cycle, a1
	csrrs	zero, cycle, zero
# csrrs rd, csr, zero
	csrr	a0, ustatus
	csrr	a0, cycle
	csrr	zero, cycle
	rdinstret  a0
	rdinstret  zero
	rdinstreth a0
	rdinstreth zero
	rdcycle	   a0
	rdcycle    zero
	rdcycleh   a0
	rdcycleh   zero
	rdtime	a0
	rdtime  zero
	rdtimeh	a0
	rdtimeh zero
	frcsr	a0
	frrm	a0
	frflags a0
# csrrs zero, csr, rs
	csrs	ustatus, a0
	csrs	cycle, a0
	csrs	cycle, zero
# csrrsi rd, csr, imm
	csrrsi	a0, ustatus, 0xb
	csrrsi	a0, cycle, 0xb
	csrrsi	a0, cycle, 0x0
	csrrsi	zero, cycle, 0xb
	csrrsi	zero, cycle, 0x0
# csrrsi zero, csr, imm
	csrsi	ustatus, 0xb
	csrsi	cycle, 0xb
	csrsi	cycle, 0x0

# csrrc a0, csr, a1
	csrrc	a0, ustatus, a1
	csrrc	a0, cycle, a1
	csrrc	a0, cycle, zero
	csrrc	zero, cycle, a1
	csrrc	zero, cycle, zero
# csrrc zero, csr, rs
	csrc	ustatus, a0
	csrc	cycle, a0
	csrc	cycle, zero
# csrrci rd, csr, imm
	csrrci	a0, ustatus, 0xb
	csrrci	a0, cycle, 0xb
	csrrci	a0, cycle, 0x0
	csrrci	zero, cycle, 0xb
	csrrci	zero, cycle, 0x0
# csrrci zero, csr, imm
	csrci	ustatus, 0xb
	csrci	cycle, 0xb
	csrci	cycle, 0x0
