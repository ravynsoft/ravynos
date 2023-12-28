#objdump: -dw
#name: i386 ADX

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[       ]*[a-f0-9]+:	66 0f 38 f6 81 90 01 00 00 	adcx   0x190\(%ecx\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   %edx,%ecx
[       ]*[a-f0-9]+:	66 0f 38 f6 94 f4 0f 04 f6 ff 	adcx   -0x9fbf1\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	66 0f 38 f6 00       	adcx   \(%eax\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   %edx,%ecx
[       ]*[a-f0-9]+:	66 0f 38 f6 00       	adcx   \(%eax\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 81 90 01 00 00 	adox   0x190\(%ecx\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   %edx,%ecx
[       ]*[a-f0-9]+:	f3 0f 38 f6 94 f4 0f 04 f6 ff 	adox   -0x9fbf1\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	f3 0f 38 f6 00       	adox   \(%eax\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   %edx,%ecx
[       ]*[a-f0-9]+:	f3 0f 38 f6 00       	adox   \(%eax\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 42 24    	adcx   0x24\(%edx\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 d1       	adcx   %ecx,%edx
[       ]*[a-f0-9]+:	66 0f 38 f6 54 f4 f4 	adcx   -0xc\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	66 0f 38 f6 00       	adcx   \(%eax\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 42 24    	adox   0x24\(%edx\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 d1       	adox   %ecx,%edx
[       ]*[a-f0-9]+:	f3 0f 38 f6 54 f4 f4 	adox   -0xc\(%esp,%esi,8\),%edx
[       ]*[a-f0-9]+:	f3 0f 38 f6 00       	adox   \(%eax\),%eax
[       ]*[a-f0-9]+:	67 66 0f 38 f6 42 24 	adcx   0x24\(%bp,%si\),%eax
[       ]*[a-f0-9]+:	66 0f 38 f6 d1       	adcx   %ecx,%edx
[       ]*[a-f0-9]+:	67 66 0f 38 f6 54 f4 	adcx   -0xc\(%si\),%edx
[       ]*[a-f0-9]+:	f4                   	hlt
[       ]*[a-f0-9]+:	67 66 0f 38 f6 00    	adcx   \(%bx,%si\),%eax
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 42 24 	adox   0x24\(%bp,%si\),%eax
[       ]*[a-f0-9]+:	f3 0f 38 f6 d1       	adox   %ecx,%edx
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 54 f4 	adox   -0xc\(%si\),%edx
[       ]*[a-f0-9]+:	f4                   	hlt
[       ]*[a-f0-9]+:	67 f3 0f 38 f6 00    	adox   \(%bx,%si\),%eax
#pass
