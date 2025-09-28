# source file to test objdump's disassembly using various styles of
# CP1 register names.

	.set noreorder
	.set noat

	.globl text_label .text
text_label:

	ctc1	$0, $0
	ctc1	$0, $1
	ctc1	$0, $2
	ctc1	$0, $3
	ctc1	$0, $4
	ctc1	$0, $5
	ctc1	$0, $6
	ctc1	$0, $7
	ctc1	$0, $8
	ctc1	$0, $9
	ctc1	$0, $10
	ctc1	$0, $11
	ctc1	$0, $12
	ctc1	$0, $13
	ctc1	$0, $14
	ctc1	$0, $15
	ctc1	$0, $16
	ctc1	$0, $17
	ctc1	$0, $18
	ctc1	$0, $19
	ctc1	$0, $20
	ctc1	$0, $21
	ctc1	$0, $22
	ctc1	$0, $23
	ctc1	$0, $24
	ctc1	$0, $25
	ctc1	$0, $26
	ctc1	$0, $27
	ctc1	$0, $28
	ctc1	$0, $29
	ctc1	$0, $30
	ctc1	$0, $31

	cfc1	$0, $0
	cfc1	$0, $1
	cfc1	$0, $2
	cfc1	$0, $3
	cfc1	$0, $4
	cfc1	$0, $5
	cfc1	$0, $6
	cfc1	$0, $7
	cfc1	$0, $8
	cfc1	$0, $9
	cfc1	$0, $10
	cfc1	$0, $11
	cfc1	$0, $12
	cfc1	$0, $13
	cfc1	$0, $14
	cfc1	$0, $15
	cfc1	$0, $16
	cfc1	$0, $17
	cfc1	$0, $18
	cfc1	$0, $19
	cfc1	$0, $20
	cfc1	$0, $21
	cfc1	$0, $22
	cfc1	$0, $23
	cfc1	$0, $24
	cfc1	$0, $25
	cfc1	$0, $26
	cfc1	$0, $27
	cfc1	$0, $28
	cfc1	$0, $29
	cfc1	$0, $30
	cfc1	$0, $31

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
      .space  8
