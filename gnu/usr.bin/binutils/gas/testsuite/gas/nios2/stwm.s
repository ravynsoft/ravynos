# Source file used to test the SDWM instruction

foo:
	stwm	{r2},--(r23)
	stwm	{r3},--(r23)
	stwm	{r4},--(r23)
	stwm	{r5},--(r23)
	stwm	{r6},--(r23)
	stwm	{r7},--(r23)
	stwm	{r8},--(r23)
	stwm	{r9},--(r23)
	stwm	{r10},--(r23)
	stwm	{r11},--(r23)
	stwm	{r12},--(r23)
	stwm	{r13},--(r23)
	stwm	{r14},(r2)++
	stwm	{r14,r15},(r2)++
	stwm	{r14,r15,r16},(r2)++
	stwm	{r14,r15,r16,r17},(r2)++
	stwm	{r14,r15,r16,r17,r18},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19,r20},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19,r20,r21},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22,r23},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,fp},(r2)++
	stwm	{r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,fp,ra},(r2)++
	stwm	{r2,r7,r11},(r13)++
	stwm	{r2,r7,r11},(r13)++,writeback
