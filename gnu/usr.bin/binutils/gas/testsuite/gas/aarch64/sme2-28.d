#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c17fdc00 	sqrshrn	z0\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdc1f 	sqrshrn	z31\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdf80 	sqrshrn	z0\.b, {z28\.s-z31\.s}, #1
[^:]+:	c160dc00 	sqrshrn	z0\.b, {z0\.s-z3\.s}, #32
[^:]+:	c167dd86 	sqrshrn	z6\.b, {z12\.s-z15\.s}, #25
[^:]+:	c1ffdc00 	sqrshrn	z0\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdc1f 	sqrshrn	z31\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdf80 	sqrshrn	z0\.h, {z28\.d-z31\.d}, #1
[^:]+:	c1a0dc00 	sqrshrn	z0\.h, {z0\.d-z3\.d}, #64
[^:]+:	c1aede99 	sqrshrn	z25\.h, {z20\.d-z23\.d}, #50
[^:]+:	c17fdc40 	sqrshrun	z0\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdc5f 	sqrshrun	z31\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdfc0 	sqrshrun	z0\.b, {z28\.s-z31\.s}, #1
[^:]+:	c160dc40 	sqrshrun	z0\.b, {z0\.s-z3\.s}, #32
[^:]+:	c167ddc6 	sqrshrun	z6\.b, {z12\.s-z15\.s}, #25
[^:]+:	c17fdc20 	uqrshrn	z0\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdc3f 	uqrshrn	z31\.b, {z0\.s-z3\.s}, #1
[^:]+:	c17fdfa0 	uqrshrn	z0\.b, {z28\.s-z31\.s}, #1
[^:]+:	c160dc20 	uqrshrn	z0\.b, {z0\.s-z3\.s}, #32
[^:]+:	c167dda6 	uqrshrn	z6\.b, {z12\.s-z15\.s}, #25
[^:]+:	c1ffdc20 	uqrshrn	z0\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdc3f 	uqrshrn	z31\.h, {z0\.d-z3\.d}, #1
[^:]+:	c1ffdfa0 	uqrshrn	z0\.h, {z28\.d-z31\.d}, #1
[^:]+:	c1a0dc20 	uqrshrn	z0\.h, {z0\.d-z3\.d}, #64
[^:]+:	c1aedeb9 	uqrshrn	z25\.h, {z20\.d-z23\.d}, #50
