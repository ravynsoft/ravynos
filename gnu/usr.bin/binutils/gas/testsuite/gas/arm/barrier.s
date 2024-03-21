@ Test case to validate barrier instruction operands
.section .text
.syntax unified
	@Tests to verify dsb, dmb and isb operand acceptance
	dmb sy
	dmb st
	dmb ish
	dmb sh
	dmb ishst
	dmb shst
	dmb nsh
	dmb un
	dmb nshst
	dmb unst
	dmb osh
	dmb oshst
	dsb sy
	dsb st
	dsb ish
	dsb sh
	dsb ishst
	dsb shst
	dsb nsh
	dsb un
	dsb nshst
	dsb unst
	dsb osh
	isb sy
	isb

	@Sanity checks for operands in upper case
	dmb SY
	dmb ST
	dmb ISH
	dmb SH
	dmb ISHST
	dmb SHST
	dmb NSH
	dmb UN
	dmb NSHST
	dmb UNST
	dmb OSH
	dmb OSHST
	dsb SY
	dsb ST
	dsb ISH
	dsb SH
	dsb ISHST
	dsb SHST
	dsb NSH
	dsb UN
	dsb NSHST
	dsb UNST
	dsb OSH
	isb SY

	@Tests to verify immediate operands
        dsb 0
        dsb #15

        dmb 0
        dmb #15
        
        isb 0
        isb #14
        isb #11
        isb #10
        isb #7
        isb #6
        isb #3
        isb #2

        isb #15

