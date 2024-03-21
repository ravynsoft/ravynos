.*: Assembler messages:
.*:3: Error: .*
.*:4: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:14: Error: .*
.*:15: Error: .*
.*:16: Error: .*
.*:18: Error: .*
.*:19: Error: .*
.*:20: Error: .*
.*:21: Error: .*
.*:22: Error: .*
.*:23: Error: .*
.*:25: Error: .*
.*:26: Error: .*
.*:27: Error: .*
.*:28: Error: .*
.*:29: Error: .*
.*:30: Error: .*
.*:31: Error: .*
.*:32: Error: .*
.*:33: Error: .*
.*:34: Error: .*
.*:35: Error: .*
.*:36: Error: .*
.*:37: Error: .*
.*:38: Error: .*
.*:39: Error: .*
.*:40: Error: .*
.*:41: Error: .*
.*:42: Error: .*
.*:43: Error: .*
.*:44: Error: .*
.*:45: Error: .*
.*:46: Error: .*
.*:47: Error: .*
.*:48: Error: .*
.*:49: Error: .*
.*:50: Error: .*
.*:51: Error: .*
.*:52: Error: .*
.*:53: Error: .*
.*:54: Error: .*
.*:56: Error: .*
.*:57: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\# All the following should be illegal
[ 	]*3[ 	]+xchg %bx,%eax
[ 	]*4[ 	]+xchg %eax,%bx
[ 	]*5[ 	]+
[ 	]*6[ 	]+imul	%bx,%ecx
[ 	]*7[ 	]+imul	\$10,%bx,%ecx
[ 	]*8[ 	]+imul	\$0x200,%bx,%ecx
[ 	]*9[ 	]+
[ 	]*10[ 	]+shld \$0x90, %bx,%ecx
[ 	]*11[ 	]+shld %cl, %bx,%ecx
[ 	]*12[ 	]+shld %bx,%ecx
[ 	]*13[ 	]+
[ 	]*14[ 	]+shrd \$0x90, %bx,%ecx
[ 	]*15[ 	]+shrd %cl, %bx,%ecx
[ 	]*16[ 	]+shrd %bx,%ecx
[ 	]*17[ 	]+
[ 	]*18[ 	]+bsf %bx,%ecx
[ 	]*19[ 	]+bsr %bx,%ecx
[ 	]*20[ 	]+bt %bx,%ecx
[ 	]*21[ 	]+btc %bx,%ecx
[ 	]*22[ 	]+btr %bx,%ecx
[ 	]*23[ 	]+bts %bx,%ecx
[ 	]*24[ 	]+
[ 	]*25[ 	]+cmovo %bx,%ecx
[ 	]*26[ 	]+cmovno %bx,%ecx
[ 	]*27[ 	]+cmovb %bx,%ecx
[ 	]*28[ 	]+cmovc %bx,%ecx
[ 	]*29[ 	]+cmovnae %bx,%ecx
[ 	]*30[ 	]+cmovae %bx,%ecx
[ 	]*31[ 	]+cmovnc %bx,%ecx
[ 	]*32[ 	]+cmovnb %bx,%ecx
[ 	]*33[ 	]+cmove %bx,%ecx
[ 	]*34[ 	]+cmovz %bx,%ecx
[ 	]*35[ 	]+cmovne %bx,%ecx
[ 	]*36[ 	]+cmovnz %bx,%ecx
[ 	]*37[ 	]+cmovbe %bx,%ecx
[ 	]*38[ 	]+cmovna %bx,%ecx
[ 	]*39[ 	]+cmova %bx,%ecx
[ 	]*40[ 	]+cmovnbe %bx,%ecx
[ 	]*41[ 	]+cmovs %bx,%ecx
[ 	]*42[ 	]+cmovns %bx,%ecx
[ 	]*43[ 	]+cmovp %bx,%ecx
[ 	]*44[ 	]+cmovnp %bx,%ecx
[ 	]*45[ 	]+cmovl %bx,%ecx
[ 	]*46[ 	]+cmovnge %bx,%ecx
[ 	]*47[ 	]+cmovge %bx,%ecx
[ 	]*48[ 	]+cmovnl %bx,%ecx
[ 	]*49[ 	]+cmovle %bx,%ecx
[ 	]*50[ 	]+cmovng %bx,%ecx
[ 	]*51[ 	]+cmovg %bx,%ecx
[ 	]*52[ 	]+cmovnle %bx,%ecx
[ 	]*53[ 	]+cmovpe %bx,%ecx
[ 	]*54[ 	]+cmovpo %bx,%ecx
[ 	]*55[ 	]+
[ 	]*56[ 	]+popcnt %bx,%ecx
[ 	]*57[ 	]+lzcnt %bx,%ecx
GAS LISTING .*


