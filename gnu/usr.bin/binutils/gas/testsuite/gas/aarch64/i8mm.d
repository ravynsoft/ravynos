#as: -march=armv8.6-a+sve
#as: -march=armv8-a+sve+i8mm
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	451b9ab1 	smmla	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	45009800 	smmla	z0\.s, z0\.b, z0\.b
 *[0-9a-f]+:	45db9ab1 	ummla	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	45c09800 	ummla	z0\.s, z0\.b, z0\.b
 *[0-9a-f]+:	459b9ab1 	usmmla	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	45809800 	usmmla	z0\.s, z0\.b, z0\.b
 *[0-9a-f]+:	449b7ab1 	usdot	z17\.s, z21\.b, z27\.b
 *[0-9a-f]+:	44807800 	usdot	z0\.s, z0\.b, z0\.b
 *[0-9a-f]+:	44bf1ab1 	usdot	z17\.s, z21\.b, z7\.b\[3\]
 *[0-9a-f]+:	44b81800 	usdot	z0\.s, z0\.b, z0\.b\[3\]
 *[0-9a-f]+:	44a71ab1 	usdot	z17\.s, z21\.b, z7\.b\[0\]
 *[0-9a-f]+:	44a01800 	usdot	z0\.s, z0\.b, z0\.b\[0\]
 *[0-9a-f]+:	44bf1eb1 	sudot	z17\.s, z21\.b, z7\.b\[3\]
 *[0-9a-f]+:	44b81c00 	sudot	z0\.s, z0\.b, z0\.b\[3\]
 *[0-9a-f]+:	44a71eb1 	sudot	z17\.s, z21\.b, z7\.b\[0\]
 *[0-9a-f]+:	44a01c00 	sudot	z0\.s, z0\.b, z0\.b\[0\]
 *[0-9a-f]+:	4e9ba6b1 	smmla	v17\.4s, v21\.16b, v27\.16b
 *[0-9a-f]+:	4e9ba6b1 	smmla	v17\.4s, v21\.16b, v27\.16b
 *[0-9a-f]+:	6e9ba6b1 	ummla	v17\.4s, v21\.16b, v27\.16b
 *[0-9a-f]+:	6e80a400 	ummla	v0\.4s, v0\.16b, v0\.16b
 *[0-9a-f]+:	4e80ac00 	usmmla	v0\.4s, v0\.16b, v0\.16b
 *[0-9a-f]+:	4e9baeb1 	usmmla	v17\.4s, v21\.16b, v27\.16b
 *[0-9a-f]+:	0e9b9eb1 	usdot	v17\.2s, v21\.8b, v27\.8b
 *[0-9a-f]+:	0e809c00 	usdot	v0\.2s, v0\.8b, v0\.8b
 *[0-9a-f]+:	4e9b9eb1 	usdot	v17\.4s, v21\.16b, v27\.16b
 *[0-9a-f]+:	4e809c00 	usdot	v0\.4s, v0\.16b, v0\.16b
 *[0-9a-f]+:	0fbbfab1 	usdot	v17\.2s, v21\.8b, v27\.4b\[3\]
 *[0-9a-f]+:	0fa0f800 	usdot	v0\.2s, v0\.8b, v0\.4b\[3\]
 *[0-9a-f]+:	0f9bf2b1 	usdot	v17\.2s, v21\.8b, v27\.4b\[0\]
 *[0-9a-f]+:	0f80f000 	usdot	v0\.2s, v0\.8b, v0\.4b\[0\]
 *[0-9a-f]+:	4fbbfab1 	usdot	v17\.4s, v21\.16b, v27\.4b\[3\]
 *[0-9a-f]+:	4fa0f800 	usdot	v0\.4s, v0\.16b, v0\.4b\[3\]
 *[0-9a-f]+:	4f9bf2b1 	usdot	v17\.4s, v21\.16b, v27\.4b\[0\]
 *[0-9a-f]+:	4f80f000 	usdot	v0\.4s, v0\.16b, v0\.4b\[0\]
 *[0-9a-f]+:	0f3bfab1 	sudot	v17\.2s, v21\.8b, v27\.4b\[3\]
 *[0-9a-f]+:	0f20f800 	sudot	v0\.2s, v0\.8b, v0\.4b\[3\]
 *[0-9a-f]+:	0f1bf2b1 	sudot	v17\.2s, v21\.8b, v27\.4b\[0\]
 *[0-9a-f]+:	0f00f000 	sudot	v0\.2s, v0\.8b, v0\.4b\[0\]
 *[0-9a-f]+:	4f3bfab1 	sudot	v17\.4s, v21\.16b, v27\.4b\[3\]
 *[0-9a-f]+:	4f20f800 	sudot	v0\.4s, v0\.16b, v0\.4b\[3\]
 *[0-9a-f]+:	4f1bf2b1 	sudot	v17\.4s, v21\.16b, v27\.4b\[0\]
 *[0-9a-f]+:	4f00f000 	sudot	v0\.4s, v0\.16b, v0\.4b\[0\]
