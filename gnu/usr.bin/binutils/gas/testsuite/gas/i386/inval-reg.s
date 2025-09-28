	.text
# All the following should be illegal
xchg %bx,%eax
xchg %eax,%bx

imul	%bx,%ecx
imul	$10,%bx,%ecx
imul	$0x200,%bx,%ecx

shld $0x90, %bx,%ecx
shld %cl, %bx,%ecx
shld %bx,%ecx

shrd $0x90, %bx,%ecx
shrd %cl, %bx,%ecx
shrd %bx,%ecx

bsf %bx,%ecx
bsr %bx,%ecx
bt %bx,%ecx
btc %bx,%ecx
btr %bx,%ecx
bts %bx,%ecx

cmovo %bx,%ecx
cmovno %bx,%ecx
cmovb %bx,%ecx
cmovc %bx,%ecx
cmovnae %bx,%ecx
cmovae %bx,%ecx
cmovnc %bx,%ecx
cmovnb %bx,%ecx
cmove %bx,%ecx
cmovz %bx,%ecx
cmovne %bx,%ecx
cmovnz %bx,%ecx
cmovbe %bx,%ecx
cmovna %bx,%ecx
cmova %bx,%ecx
cmovnbe %bx,%ecx
cmovs %bx,%ecx
cmovns %bx,%ecx
cmovp %bx,%ecx
cmovnp %bx,%ecx
cmovl %bx,%ecx
cmovnge %bx,%ecx
cmovge %bx,%ecx
cmovnl %bx,%ecx
cmovle %bx,%ecx
cmovng %bx,%ecx
cmovg %bx,%ecx
cmovnle %bx,%ecx
cmovpe %bx,%ecx
cmovpo %bx,%ecx

popcnt %bx,%ecx
lzcnt %bx,%ecx
