#objdump: -drwMintel
#name: x86-64 ADX(Intel mode)
#source: x86-64-adx.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[       ]*[a-f0-9]+:	67 66 0f 38 f6 81 90 01 00 00 	adcx   eax,DWORD PTR \[ecx\+0x190\]
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   ecx,edx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 94 f4 0f 04 f6 ff 	adcx   edx,DWORD PTR \[esp\+esi\*8-0x9fbf1\]
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   ecx,edx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	66 4c 0f 38 f6 99 90 01 00 00 	adcx   r11,QWORD PTR \[rcx\+0x190\]
[       ]*[a-f0-9]+:	66 4d 0f 38 f6 e6    	adcx   r12,r14
[       ]*[a-f0-9]+:	67 66 48 0f 38 f6 94 f4 0f 04 f6 ff 	adcx   rdx,QWORD PTR \[esp\+esi\*8-0x9fbf1\]
[       ]*[a-f0-9]+:	66 49 0f 38 f6 00    	adcx   rax,QWORD PTR \[r8\]
[       ]*[a-f0-9]+:	66 48 0f 38 f6 ca    	adcx   rcx,rdx
[       ]*[a-f0-9]+:	66 48 0f 38 f6 00    	adcx   rax,QWORD PTR \[rax\]
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 81 90 01 00 00 	adox   eax,DWORD PTR \[ecx\+0x190\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   ecx,edx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 94 f4 0f 04 f6 ff 	adox   edx,DWORD PTR \[esp\+esi\*8-0x9fbf1\]
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   ecx,edx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	f3 4c 0f 38 f6 99 90 01 00 00 	adox   r11,QWORD PTR \[rcx\+0x190\]
[       ]*[a-f0-9]+:	f3 4d 0f 38 f6 e6    	adox   r12,r14
[       ]*[a-f0-9]+:	67 f3 48 0f 38 f6 94 f4 0f 04 f6 ff 	adox   rdx,QWORD PTR \[esp\+esi\*8-0x9fbf1\]
[       ]*[a-f0-9]+:	f3 49 0f 38 f6 00    	adox   rax,QWORD PTR \[r8\]
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 ca    	adox   rcx,rdx
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 00    	adox   rax,QWORD PTR \[rax\]
[       ]*[a-f0-9]+:	67 66 0f 38 f6 82 8f 01 00 00 	adcx   eax,DWORD PTR \[edx\+0x18f\]
[       ]*[a-f0-9]+:	66 0f 38 f6 d1       	adcx   edx,ecx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 94 f4 c0 1d fe ff 	adcx   edx,DWORD PTR \[esp\+esi\*8-0x1e240\]
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	66 49 0f 38 f6 83 8f 01 00 00 	adcx   rax,QWORD PTR \[r11\+0x18f\]
[       ]*[a-f0-9]+:	66 49 0f 38 f6 d1    	adcx   rdx,r9
[       ]*[a-f0-9]+:	66 48 0f 38 f6 94 f4 c0 1d fe ff 	adcx   rdx,QWORD PTR \[rsp\+rsi\*8-0x1e240\]
[       ]*[a-f0-9]+:	66 48 0f 38 f6 03    	adcx   rax,QWORD PTR \[rbx\]
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 82 8f 01 00 00 	adox   eax,DWORD PTR \[edx\+0x18f\]
[       ]*[a-f0-9]+:	f3 0f 38 f6 d1       	adox   edx,ecx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 94 f4 c0 1d fe ff 	adox   edx,DWORD PTR \[esp\+esi\*8-0x1e240\]
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   eax,DWORD PTR \[eax\]
[       ]*[a-f0-9]+:	f3 49 0f 38 f6 83 8f 01 00 00 	adox   rax,QWORD PTR \[r11\+0x18f\]
[       ]*[a-f0-9]+:	f3 49 0f 38 f6 d1    	adox   rdx,r9
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 94 f4 c0 1d fe ff 	adox   rdx,QWORD PTR \[rsp\+rsi\*8-0x1e240\]
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 03    	adox   rax,QWORD PTR \[rbx\]
#pass
