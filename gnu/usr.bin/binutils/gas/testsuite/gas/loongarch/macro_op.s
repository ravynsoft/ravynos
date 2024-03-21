li.w  $r4, 0
li.w  $r4, 0xffffffff
li.d  $r4, 0
li.d  $r4, 0xffffffffffffffff
la  $r4, .L1
la.global  $r4, .L1
la.local  $r4, .L1
la.abs  $r4, .L1
la.pcrel  $r4, .L1
la.got  $r4, .L1

la.tls.le  $r4, TLS1
la.tls.ie  $r4, TLS1
la.tls.ld  $r4, TLS1
la.tls.gd  $r4, TLS1
