.L1:
nop
bgtz  $r4,.L1
bgez  $r4,.L1
blez  $r4,.L1
beqz  $r4,.L1
bnez  $r4,.L1
bceqz  $fcc0,.L1
bcnez  $fcc0,.L1
jr  $r4
b  .L1
bl  .L1
beq  $r4,$r5,.L1
bne  $r4,$r5,.L1
blt  $r4,$r5,.L1
bgt  $r4,$r5,.L1
bge  $r4,$r5,.L1
ble  $r4,$r5,.L1
bltu  $r4,$r5,.L1
bgtu  $r4,$r5,.L1
bgeu  $r4,$r5,.L1
bleu  $r4,$r5,.L1
ret
