[^:]*: Assembler messages:
[^:]*:6: Error: r15 not allowed here -- `ldmia r15,{r0-r3}'
[^:]*:7: Error: r15 not allowed here -- `ldmia r15!,{r0-r3}'
[^:]*:8: Error: LR and PC should not both be in register list -- `ldmia r1,{r14,r15}'
[^:]*:9: Error: having the base register in the register list when using write back is UNPREDICTABLE -- `ldmia r0!,{r0-r3}'
[^:]*:12: Error: branch must be last instruction in IT block -- `ldmiaeq r0,{r12,r15}'
[^:]*:13: Error: having the base register in the register list when using write back is UNPREDICTABLE -- `ldmiaeq r0!,{r0,r1}'
[^:]*:17: Error: having the base register in the register list when using write back is UNPREDICTABLE -- `stmia.w r0!,{r0-r3}'
[^:]*:18: Warning: value stored for r1 is UNKNOWN
[^:]*:19: Error: r15 not allowed here -- `stmia r15!,{r0-r3}'
[^:]*:20: Error: r15 not allowed here -- `stmia r15,{r0-r3}'
[^:]*:21: Error: having the base register in the register list when using write back is UNPREDICTABLE -- `stmia r8!,{r0-r11}'
