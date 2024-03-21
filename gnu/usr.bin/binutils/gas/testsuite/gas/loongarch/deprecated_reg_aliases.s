foo:
    li.d $v1, 0x123456789012
    fmadd.d $fv0, $fv1, $fv1, $fa0
    ldx.d $v0, $x, $a1
    ret
