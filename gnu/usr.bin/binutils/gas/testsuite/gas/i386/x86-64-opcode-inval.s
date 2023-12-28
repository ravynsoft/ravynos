	.text
# All the followings are illegal opcodes for x86-64.
aaa:
	aaa
aad0:
	aad
aad1:
	aad $2
aam0:
	aam
aam1:
	aam $2
aas:
	aas
bound:
	bound  %edx,(%eax)
daa:
	daa
das:
	das
into:
	into
pusha:
	pusha
popa:
	popa
