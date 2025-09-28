[^:]*: Assembler messages:
[^:]*:5: Warning: destination register same as write-back base
[^:]*:9: Error: ldr to register 15 must be 4-byte aligned -- `ldr r15,\[r15,#5\]'
[^:]*:12: Error: ldr to register 15 must be 4-byte aligned -- `ldr r15,.-0xab7'
[^:]*:15: Warning: destination register same as write-back base
[^:]*:16: Error: cannot use register index with PC-relative addressing -- `ldr r2,\[r15,r2\]!'
[^:]*:19: Error: cannot use register index with PC-relative addressing -- `ldr r1,\[r1,r15\]'
[^:]*:22: Warning: source register same as write-back base
[^:]*:23: Error: cannot use register index with PC-relative addressing -- `str r1,\[r15,#10\]!'
[^:]*:26: Warning: source register same as write-back base
[^:]*:27: Error: cannot use register index with PC-relative addressing -- `str r1,\[r15,r2\]!'

