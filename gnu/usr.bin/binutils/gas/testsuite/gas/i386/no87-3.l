.*: Assembler messages:
.*:4: Error: .*generic.*
.*:7: Error: .*8087.*
.*:10: Error: .*287.*
.*:13: Error: .*387.*
.*:17: Error: .*no687.*
.*:20: Error: .*no387.*
.*:23: Error: .*no287.*
.*:26: Error: .*no87.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \[\.x87|\.nox87\]
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch generic32
[ 	]*4[ 	]+fneni
[ 	]*5[ 	]+\.arch \.8087
[ 	]*6[ 	]+\?\?\?\? DBE0     		fneni
[ 	]*7[ 	]+fsetpm
[ 	]*8[ 	]+\.arch \.287
[ 	]*9[ 	]+\?\?\?\? 9BDBE4   		fsetpm
[ 	]*10[ 	]+fprem1
[ 	]*11[ 	]+\.arch \.387
[ 	]*12[ 	]+\?\?\?\? D9F5     		fprem1
[ 	]*13[ 	]+fcomi
[ 	]*14[ 	]+\.arch \.687
[ 	]*15[ 	]+\?\?\?\? DBF1     		fcomi
[ 	]*16[ 	]+\.arch \.no687
[ 	]*17[ 	]+fcomi
[ 	]*18[ 	]+\?\?\?\? D9F5     		fprem1
[ 	]*19[ 	]+\.arch \.no387
[ 	]*20[ 	]+fprem1
[ 	]*21[ 	]+\?\?\?\? 9BDBE4   		fsetpm
[ 	]*22[ 	]+\.arch \.no287
[ 	]*23[ 	]+fsetpm
[ 	]*24[ 	]+\?\?\?\? DBE0     		fneni
[ 	]*25[ 	]+\.arch \.no87
[ 	]*26[ 	]+fneni
[ 	]*27[ 	]+\.p2align 4
#pass
