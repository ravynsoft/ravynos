.text
hello:
    jbt hello
    jbt label
.rept  33*1024
    nop
.endr

label:
    nop
