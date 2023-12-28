#objdump: -dw
#name: x86-64 ADX

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[       ]*[a-f0-9]+:	67 66 0f 38 f6 81 90 01 00 00 	adcx   0x190\(%ecx\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   %edx,%ecx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 94 f4 0f 04 f6 ff 	adcx   -0x9fbf1\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   \(%eax\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   %edx,%ecx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   \(%eax\),%eax
[       ]*[a-f0-9]+:	66 4c 0f 38 f6 99 90 01 00 00 	adcx   0x190\(%rcx\),%r11
[       ]*[a-f0-9]+:	66 4d 0f 38 f6 e6    	adcx   %r14,%r12
[       ]*[a-f0-9]+:	67 66 48 0f 38 f6 94 f4 0f 04 f6 ff 	adcx   -0x9fbf1\(%esp,%esi,8\),%rdx
[       ]*[a-f0-9]+:	66 49 0f 38 f6 00    	adcx   \(%r8\),%rax
[       ]*[a-f0-9]+:	66 48 0f 38 f6 ca    	adcx   %rdx,%rcx
[       ]*[a-f0-9]+:	66 48 0f 38 f6 00    	adcx   \(%rax\),%rax
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 81 90 01 00 00 	adox   0x190\(%ecx\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   %edx,%ecx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 94 f4 0f 04 f6 ff 	adox   -0x9fbf1\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   \(%eax\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   %edx,%ecx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   \(%eax\),%eax
[       ]*[a-f0-9]+:	f3 4c 0f 38 f6 99 90 01 00 00 	adox   0x190\(%rcx\),%r11
[       ]*[a-f0-9]+:	f3 4d 0f 38 f6 e6    	adox   %r14,%r12
[       ]*[a-f0-9]+:	67 f3 48 0f 38 f6 94 f4 0f 04 f6 ff 	adox   -0x9fbf1\(%esp,%esi,8\),%rdx
[       ]*[a-f0-9]+:	f3 49 0f 38 f6 00    	adox   \(%r8\),%rax
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 ca    	adox   %rdx,%rcx
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 00    	adox   \(%rax\),%rax
[       ]*[a-f0-9]+:	67 66 0f 38 f6 82 8f 01 00 00 	adcx   0x18f\(%edx\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 d1       	adcx   %ecx,%edx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 94 f4 c0 1d fe ff 	adcx   -0x1e240\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   \(%eax\),%eax
[       ]*[a-f0-9]+:	66 49 0f 38 f6 83 8f 01 00 00 	adcx   0x18f\(%r11\),%rax
[       ]*[a-f0-9]+:	66 49 0f 38 f6 d1    	adcx   %r9,%rdx
[       ]*[a-f0-9]+:	66 48 0f 38 f6 94 f4 c0 1d fe ff 	adcx   -0x1e240\(%rsp,%rsi,8\),%rdx
[       ]*[a-f0-9]+:	66 48 0f 38 f6 03    	adcx   \(%rbx\),%rax
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 82 8f 01 00 00 	adox   0x18f\(%edx\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 d1       	adox   %ecx,%edx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 94 f4 c0 1d fe ff 	adox   -0x1e240\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   \(%eax\),%eax
[       ]*[a-f0-9]+:	f3 49 0f 38 f6 83 8f 01 00 00 	adox   0x18f\(%r11\),%rax
[       ]*[a-f0-9]+:	f3 49 0f 38 f6 d1    	adox   %r9,%rdx
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 94 f4 c0 1d fe ff 	adox   -0x1e240\(%rsp,%rsi,8\),%rdx
[       ]*[a-f0-9]+:	f3 48 0f 38 f6 03    	adox   \(%rbx\),%rax
#pass
