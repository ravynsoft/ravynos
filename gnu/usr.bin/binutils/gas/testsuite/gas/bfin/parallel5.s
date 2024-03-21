# { dg-do assemble { target bfin-*-* } }
# { dg-options "--mcpu=bf537-0.2" }
	.section .text;
	R0 = W[P1++] (X) || R1.L = W[I1++];
