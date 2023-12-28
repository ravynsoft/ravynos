# This should match the warnings when assembling line.s.

.*: Assembler messages:
line\.s:[0-9]*18: Warning: \.warning .*
line\.s:2: Warning: m1/1: 123
line\.s:[0-9]*21:  Info: .*
line\.s:4: Warning: m1/2: 123
line\.s:[0-9]*21:  Info: .*
line\.s:2: Warning: m1/1: abc
line\.s:[0-9]*22:  Info: .*
line\.s:4: Warning: m1/2: abc
line\.s:[0-9]*22:  Info: .*
line\.s:2: Warning: m1/1: XYZ
line\.s:[0-9]*23:  Info: .*
line\.s:4: Warning: m1/2: XYZ
line\.s:[0-9]*23:  Info: .*
line\.s:[0-9]*24: Warning: \.warning .*
Line2\.s:10: Warning: m2/1: 987
line\.s:[0-9]*26:  Info: .*
Line2\.s:12: Warning: m2/2: 987
line\.s:[0-9]*26:  Info: .*
Line2\.s:10: Warning: m2/1: zyx
line\.s:[0-9]*27:  Info: .*
Line2\.s:12: Warning: m2/2: zyx
line\.s:[0-9]*27:  Info: .*
Line2\.s:10: Warning: m2/1: CBA
line\.s:[0-9]*28:  Info: .*
Line2\.s:12: Warning: m2/2: CBA
line\.s:[0-9]*28:  Info: .*
line\.s:[0-9]*29: Warning: \.warning .*
line\.s:[0-9]*35: Warning: irp/1: 123
line\.s:[0-9]*37: Warning: irp/2: 123
line\.s:[0-9]*35: Warning: irp/1: 456
line\.s:[0-9]*37: Warning: irp/2: 456
line\.s:[0-9]*39: Warning: \.warning .*
line\.s:[0-9]*45: Warning: rept/1
line\.s:[0-9]*47: Warning: rept/2
line\.s:[0-9]*45: Warning: rept/1
line\.s:[0-9]*47: Warning: rept/2
line\.s:[0-9]*45: Warning: rept/1
line\.s:[0-9]*47: Warning: rept/2
line\.s:[0-9]*49: Warning: \.warning .*
