#objdump: -dwMintel
#name: i386 ADX (Intel disassembly)
#source: adx.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[       ]*[a-f0-9]+:	66 0f 38 f6 81 90 01 00 00 	adcx   eax,DWORD PTR \[ecx\+0x190\]
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   ecx,edx
[       ]*[a-f0-9]+:	66 0f 38 f6 94 f4 0f 04 f6 ff 	adcx   edx,DWORD PTR \[esp\+esi\*8-0x9fbf1\]
[       ]*[a-f0-9]+:	66 0f 38 f6 00       	adcx   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   ecx,edx
[       ]*[a-f0-9]+:	66 0f 38 f6 00       	adcx   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 81 90 01 00 00 	adox   eax,DWORD PTR \[ecx\+0x190\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   ecx,edx
[       ]*[a-f0-9]+:	f3 0f 38 f6 94 f4 0f 04 f6 ff 	adox   edx,DWORD PTR \[esp\+esi\*8-0x9fbf1\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 00       	adox   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   ecx,edx
[       ]*[a-f0-9]+:	f3 0f 38 f6 00       	adox   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	66 0f 38 f6 42 24    	adcx   eax,DWORD PTR \[edx\+0x24\]
[       ]*[a-f0-9]+:	66 0f 38 f6 d1       	adcx   edx,ecx
[       ]*[a-f0-9]+:	66 0f 38 f6 54 f4 f4 	adcx   edx,DWORD PTR \[esp\+esi\*8-0xc\]
[       ]*[a-f0-9]+:	66 0f 38 f6 00       	adcx   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 42 24    	adox   eax,DWORD PTR \[edx\+0x24\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 d1       	adox   edx,ecx
[       ]*[a-f0-9]+:	f3 0f 38 f6 54 f4 f4 	adox   edx,DWORD PTR \[esp\+esi\*8-0xc\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 00       	adox   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	67 66 0f 38 f6 42 24 	adcx   eax,DWORD PTR \[bp\+si\+0x24\]
[       ]*[a-f0-9]+:	66 0f 38 f6 d1       	adcx   edx,ecx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 54 f4 	adcx   edx,DWORD PTR \[si-0xc\]
[       ]*[a-f0-9]+:	f4                   	hlt
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   eax,DWORD PTR \[bx\+si\]
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 42 24 	adox   eax,DWORD PTR \[bp\+si\+0x24\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 d1       	adox   edx,ecx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 54 f4 	adox   edx,DWORD PTR \[si-0xc\]
[       ]*[a-f0-9]+:	f4                   	hlt
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   eax,DWORD PTR \[bx\+si\]
#pass
