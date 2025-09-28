	.text
	dmul	$2,$3,$4
	dmuh	$2,$3,$4
	ddiv	$2,$3,$4
	dmulu	$2,$3,$4
	dmuhu	$2,$3,$4
	dmod	$2,$3,$4
	ddivu	$2,$3,$4
	dmodu	$2,$3,$4

	dlsa	$2,$3,$4,1
	dlsa	$2,$3,$4,4

	dclz	$2,$3
	dclo	$2,$3

	lld	$2,-256($3)
	lld	$2,255($3)
	scd	$2,-256($3)
	scd	$2,255($3)

        dalign   $4, $2, $3, 0
        dalign   $4, $2, $3, 1
        dalign   $4, $2, $3, 2
        dalign   $4, $2, $3, 3
        dalign   $4, $2, $3, 4
        dalign   $4, $2, $3, 5
        dalign   $4, $2, $3, 6
        dalign   $4, $2, $3, 7

        dbitswap  $4, $2

        daui      $3, $2, 0xffff
        dahi      $3, $3, 0xffff
        dati      $3, $3, 0xffff

        lwupc      $4, 1f
        lwupc      $4, .+(-262144 << 2)
        lwupc      $4, .+(262143 << 2)
        lwu      $4, (-262144 << 2)($pc)
        lwu      $4, (262143 << 2)($pc)

        ldpc     $4, 1f
        ldpc     $4, 1f
	.align 3
3:
	ldpc     $4, 3b+(-131072 << 3)
	ldpc     $4, 3b+(-131072 << 3)
	.align 3
3:
	ldpc     $4, 3b+(131071 << 3)
	ldpc     $4, 3b+(131071 << 3)
        ld     $4, (-131072 << 3)($pc)
        ld     $4, (-131072 << 3)($pc)
        ld     $4, (131071 << 3)($pc)
        ld     $4, (131071 << 3)($pc)
        .align 3
1:
	lldp	$5, $4, $6
	scdp	$5, $4, $6
	nop
	nop

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
