        .data

msg:    .asciz  "hello world.\n"
msglen = .-msg-1
msglen=msglen & 0xff
