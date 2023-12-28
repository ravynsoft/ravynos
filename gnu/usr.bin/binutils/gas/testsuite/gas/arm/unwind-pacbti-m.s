
	.arch armv8.1-m.main
	.arch_extension pacbti
	.eabi_attribute 50, 2
	.eabi_attribute 52, 2
	.eabi_attribute 74, 1
	.eabi_attribute 76, 1
	.text
	.syntax unified
	.thumb
	.thumb_func
foo:
	.fnstart
	push	{r4, lr}
	.save {r4, lr}
	push	{r12}
	.save {ra_auth_code}
	pop	{r12}
	push	{r4-r7, ip, lr}
	.save {r4-r7, ra_auth_code, lr}
	pop	{r4-r7, ip, lr}
	push	{ip, lr}
	.save {ra_auth_code, lr}
	pop {ip, lr}
	pop	{r4, pc}
	.fnend
