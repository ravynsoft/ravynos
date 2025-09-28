# Test that we don't move store into delay slots

	.text
func:
	addiu   $2,$3,5        
	lw      $4,0($2)
	sw      $3,0($2)
	sw      $3,8($2)
	sw      $3,16($2)
	beq     $3,0,.L1
	lw      $3,8($2)
.L1:
	lw      $5,16($2)
	.p2align        4
