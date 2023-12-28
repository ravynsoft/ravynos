# Source file used to test the LDWM instruction
	
foo:
	ldwm	{r2},--(r23)
	ldwm	{r3},--(r23)
	ldwm	{r4},--(r23)
	ldwm	{r5},--(r23)
	ldwm	{r6},--(r23)
	ldwm	{r7},--(r23)
	ldwm	{r8},--(r23)
	ldwm	{r9},--(r23)
	ldwm	{r10},--(r23)
	ldwm	{r11},--(r23)
	ldwm	{r12},--(r23)
	ldwm	{r13},--(r23)
	ldwm	{r14},(r2)++
	ldwm	{r14,r15},(r2)++
	ldwm	{r14,r15,r16},(r2)++
	ldwm	{r14,r15,r16,r17},(r2)++
	ldwm	{r14,r15,r16,r17,r18},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19,r20},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19,r20,r21},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22,r23},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,fp},(r2)++
	ldwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,fp,ra},(r2)++
	ldwm	{r2,r7,r11},(r13)++
	ldwm	{r2,r7,r11},(r13)++,ret
	ldwm	{r2,r7,r11},(r13)++,writeback
	ldwm	{r2,r7,r11},(r13)++,ret,writeback
