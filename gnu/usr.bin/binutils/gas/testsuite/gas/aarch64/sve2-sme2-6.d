#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	45bf2800 	sqrshrn	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	45bf281f 	sqrshrn	z31\.h, {z0\.s-z1\.s}, #1
[^:]+:	45bf2bc0 	sqrshrn	z0\.h, {z30\.s-z31\.s}, #1
[^:]+:	45b02800 	sqrshrn	z0\.h, {z0\.s-z1\.s}, #16
[^:]+:	45b22b41 	sqrshrn	z1\.h, {z26\.s-z27\.s}, #14
[^:]+:	45bf0800 	sqrshrun	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	45bf081f 	sqrshrun	z31\.h, {z0\.s-z1\.s}, #1
[^:]+:	45bf0bc0 	sqrshrun	z0\.h, {z30\.s-z31\.s}, #1
[^:]+:	45b00800 	sqrshrun	z0\.h, {z0\.s-z1\.s}, #16
[^:]+:	45b708cf 	sqrshrun	z15\.h, {z6\.s-z7\.s}, #9
[^:]+:	45bf3800 	uqrshrn	z0\.h, {z0\.s-z1\.s}, #1
[^:]+:	45bf381f 	uqrshrn	z31\.h, {z0\.s-z1\.s}, #1
[^:]+:	45bf3bc0 	uqrshrn	z0\.h, {z30\.s-z31\.s}, #1
[^:]+:	45b03800 	uqrshrn	z0\.h, {z0\.s-z1\.s}, #16
[^:]+:	45ba3852 	uqrshrn	z18\.h, {z2\.s-z3\.s}, #6
