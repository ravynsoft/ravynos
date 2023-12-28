#as: -march=armv8-a+sme2
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	c120d001 	uzp	{z0\.b-z1\.b}, z0\.b, z0\.b
[^:]+:	c120d01f 	uzp	{z30\.b-z31\.b}, z0\.b, z0\.b
[^:]+:	c120d3e1 	uzp	{z0\.b-z1\.b}, z31\.b, z0\.b
[^:]+:	c13fd001 	uzp	{z0\.b-z1\.b}, z0\.b, z31\.b
[^:]+:	c139d173 	uzp	{z18\.b-z19\.b}, z11\.b, z25\.b
[^:]+:	c160d001 	uzp	{z0\.h-z1\.h}, z0\.h, z0\.h
[^:]+:	c160d01f 	uzp	{z30\.h-z31\.h}, z0\.h, z0\.h
[^:]+:	c160d3e1 	uzp	{z0\.h-z1\.h}, z31\.h, z0\.h
[^:]+:	c17fd001 	uzp	{z0\.h-z1\.h}, z0\.h, z31\.h
[^:]+:	c176d107 	uzp	{z6\.h-z7\.h}, z8\.h, z22\.h
[^:]+:	c1a0d001 	uzp	{z0\.s-z1\.s}, z0\.s, z0\.s
[^:]+:	c1a0d01f 	uzp	{z30\.s-z31\.s}, z0\.s, z0\.s
[^:]+:	c1a0d3e1 	uzp	{z0\.s-z1\.s}, z31\.s, z0\.s
[^:]+:	c1bfd001 	uzp	{z0\.s-z1\.s}, z0\.s, z31\.s
[^:]+:	c1a2d279 	uzp	{z24\.s-z25\.s}, z19\.s, z2\.s
[^:]+:	c1e0d001 	uzp	{z0\.d-z1\.d}, z0\.d, z0\.d
[^:]+:	c1e0d01f 	uzp	{z30\.d-z31\.d}, z0\.d, z0\.d
[^:]+:	c1e0d3e1 	uzp	{z0\.d-z1\.d}, z31\.d, z0\.d
[^:]+:	c1ffd001 	uzp	{z0\.d-z1\.d}, z0\.d, z31\.d
[^:]+:	c1e5d3a3 	uzp	{z2\.d-z3\.d}, z29\.d, z5\.d
[^:]+:	c120d401 	uzp	{z0\.q-z1\.q}, z0\.q, z0\.q
[^:]+:	c120d41f 	uzp	{z30\.q-z31\.q}, z0\.q, z0\.q
[^:]+:	c120d7e1 	uzp	{z0\.q-z1\.q}, z31\.q, z0\.q
[^:]+:	c13fd401 	uzp	{z0\.q-z1\.q}, z0\.q, z31\.q
[^:]+:	c129d70f 	uzp	{z14\.q-z15\.q}, z24\.q, z9\.q
[^:]+:	c136e002 	uzp	{z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c136e01e 	uzp	{z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c136e382 	uzp	{z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c136e306 	uzp	{z4\.b-z7\.b}, {z24\.b-z27\.b}
[^:]+:	c176e002 	uzp	{z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c176e01e 	uzp	{z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c176e382 	uzp	{z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c176e112 	uzp	{z16\.h-z19\.h}, {z8\.h-z11\.h}
[^:]+:	c1b6e002 	uzp	{z0\.s-z3\.s}, {z0\.s-z3\.s}
[^:]+:	c1b6e01e 	uzp	{z28\.s-z31\.s}, {z0\.s-z3\.s}
[^:]+:	c1b6e382 	uzp	{z0\.s-z3\.s}, {z28\.s-z31\.s}
[^:]+:	c1b6e196 	uzp	{z20\.s-z23\.s}, {z12\.s-z15\.s}
[^:]+:	c1f6e002 	uzp	{z0\.d-z3\.d}, {z0\.d-z3\.d}
[^:]+:	c1f6e01e 	uzp	{z28\.d-z31\.d}, {z0\.d-z3\.d}
[^:]+:	c1f6e382 	uzp	{z0\.d-z3\.d}, {z28\.d-z31\.d}
[^:]+:	c1f6e20a 	uzp	{z8\.d-z11\.d}, {z16\.d-z19\.d}
[^:]+:	c137e002 	uzp	{z0\.q-z3\.q}, {z0\.q-z3\.q}
[^:]+:	c137e01e 	uzp	{z28\.q-z31\.q}, {z0\.q-z3\.q}
[^:]+:	c137e382 	uzp	{z0\.q-z3\.q}, {z28\.q-z31\.q}
[^:]+:	c137e08e 	uzp	{z12\.q-z15\.q}, {z4\.q-z7\.q}
[^:]+:	c120d000 	zip	{z0\.b-z1\.b}, z0\.b, z0\.b
[^:]+:	c120d01e 	zip	{z30\.b-z31\.b}, z0\.b, z0\.b
[^:]+:	c120d3e0 	zip	{z0\.b-z1\.b}, z31\.b, z0\.b
[^:]+:	c13fd000 	zip	{z0\.b-z1\.b}, z0\.b, z31\.b
[^:]+:	c139d172 	zip	{z18\.b-z19\.b}, z11\.b, z25\.b
[^:]+:	c160d000 	zip	{z0\.h-z1\.h}, z0\.h, z0\.h
[^:]+:	c160d01e 	zip	{z30\.h-z31\.h}, z0\.h, z0\.h
[^:]+:	c160d3e0 	zip	{z0\.h-z1\.h}, z31\.h, z0\.h
[^:]+:	c17fd000 	zip	{z0\.h-z1\.h}, z0\.h, z31\.h
[^:]+:	c176d106 	zip	{z6\.h-z7\.h}, z8\.h, z22\.h
[^:]+:	c1a0d000 	zip	{z0\.s-z1\.s}, z0\.s, z0\.s
[^:]+:	c1a0d01e 	zip	{z30\.s-z31\.s}, z0\.s, z0\.s
[^:]+:	c1a0d3e0 	zip	{z0\.s-z1\.s}, z31\.s, z0\.s
[^:]+:	c1bfd000 	zip	{z0\.s-z1\.s}, z0\.s, z31\.s
[^:]+:	c1a2d278 	zip	{z24\.s-z25\.s}, z19\.s, z2\.s
[^:]+:	c1e0d000 	zip	{z0\.d-z1\.d}, z0\.d, z0\.d
[^:]+:	c1e0d01e 	zip	{z30\.d-z31\.d}, z0\.d, z0\.d
[^:]+:	c1e0d3e0 	zip	{z0\.d-z1\.d}, z31\.d, z0\.d
[^:]+:	c1ffd000 	zip	{z0\.d-z1\.d}, z0\.d, z31\.d
[^:]+:	c1e5d3a2 	zip	{z2\.d-z3\.d}, z29\.d, z5\.d
[^:]+:	c120d400 	zip	{z0\.q-z1\.q}, z0\.q, z0\.q
[^:]+:	c120d41e 	zip	{z30\.q-z31\.q}, z0\.q, z0\.q
[^:]+:	c120d7e0 	zip	{z0\.q-z1\.q}, z31\.q, z0\.q
[^:]+:	c13fd400 	zip	{z0\.q-z1\.q}, z0\.q, z31\.q
[^:]+:	c129d70e 	zip	{z14\.q-z15\.q}, z24\.q, z9\.q
[^:]+:	c136e000 	zip	{z0\.b-z3\.b}, {z0\.b-z3\.b}
[^:]+:	c136e01c 	zip	{z28\.b-z31\.b}, {z0\.b-z3\.b}
[^:]+:	c136e380 	zip	{z0\.b-z3\.b}, {z28\.b-z31\.b}
[^:]+:	c136e304 	zip	{z4\.b-z7\.b}, {z24\.b-z27\.b}
[^:]+:	c176e000 	zip	{z0\.h-z3\.h}, {z0\.h-z3\.h}
[^:]+:	c176e01c 	zip	{z28\.h-z31\.h}, {z0\.h-z3\.h}
[^:]+:	c176e380 	zip	{z0\.h-z3\.h}, {z28\.h-z31\.h}
[^:]+:	c176e110 	zip	{z16\.h-z19\.h}, {z8\.h-z11\.h}
[^:]+:	c1b6e000 	zip	{z0\.s-z3\.s}, {z0\.s-z3\.s}
[^:]+:	c1b6e01c 	zip	{z28\.s-z31\.s}, {z0\.s-z3\.s}
[^:]+:	c1b6e380 	zip	{z0\.s-z3\.s}, {z28\.s-z31\.s}
[^:]+:	c1b6e194 	zip	{z20\.s-z23\.s}, {z12\.s-z15\.s}
[^:]+:	c1f6e000 	zip	{z0\.d-z3\.d}, {z0\.d-z3\.d}
[^:]+:	c1f6e01c 	zip	{z28\.d-z31\.d}, {z0\.d-z3\.d}
[^:]+:	c1f6e380 	zip	{z0\.d-z3\.d}, {z28\.d-z31\.d}
[^:]+:	c1f6e208 	zip	{z8\.d-z11\.d}, {z16\.d-z19\.d}
[^:]+:	c137e000 	zip	{z0\.q-z3\.q}, {z0\.q-z3\.q}
[^:]+:	c137e01c 	zip	{z28\.q-z31\.q}, {z0\.q-z3\.q}
[^:]+:	c137e380 	zip	{z0\.q-z3\.q}, {z28\.q-z31\.q}
[^:]+:	c137e08c 	zip	{z12\.q-z15\.q}, {z4\.q-z7\.q}
