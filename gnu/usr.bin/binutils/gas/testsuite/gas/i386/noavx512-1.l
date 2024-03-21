.*: Assembler messages:
.*:8: Error: .*operand size mismatch.*
.*:9: Error: .*unsupported masking.*
.*:10: Error: .*unsupported masking.*
.*:25: Error: .*not supported.*
.*:26: Error: .*not supported.*
.*:27: Error: .*not supported.*
.*:11: Error: .*not supported.*
.*:12: Error: .*not supported.*
.*:13: Error: .*not supported.*
.*:14: Error: .*not supported.*
.*:15: Error: .*not supported.*
.*:16: Error: .*not supported.*
.*:17: Error: .*not supported.*
.*:21: Error: .*operand.*mismatch.*
.*:22: Error: .*unsupported masking.*
.*:23: Error: .*unsupported masking.*
.*:24: Error: .*not supported.*
.*:25: Error: .*not supported.*
.*:26: Error: .*not supported.*
.*:27: Error: .*not supported.*
.*:8: Error: .*bad register name.*
.*:9: Error: .*unknown vector operation.*
.*:10: Error: .*unknown vector operation.*
.*:11: Error: .*not supported.*
.*:12: Error: .*not supported.*
.*:13: Error: .*not supported.*
.*:14: Error: .*not supported.*
.*:15: Error: .*not supported.*
.*:16: Error: .*not supported.*
.*:17: Error: .*not supported.*
.*:18: Error: .*bad register name.*
.*:19: Error: .*unknown vector operation.*
.*:20: Error: .*unknown vector operation.*
.*:21: Error: .*bad register name.*
.*:22: Error: .*unknown vector operation.*
.*:23: Error: .*unknown vector operation.*
.*:24: Error: .*not supported.*
.*:25: Error: .*not supported.*
.*:26: Error: .*not supported.*
.*:27: Error: .*not supported.*
#...
[ 	]*[0-9]+[ 	]+\# Test \.arch \.noavx512XX
[ 	]*[0-9]+[ 	]+\.text
[ 	]*[0-9]+[ 	]*
[ 	]*[0-9]+[ 	]+\.irp isa, default, .*
#...
[ 	]*[0-9]+[ 	]+\.endr
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2554F 	>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2550F 	>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2552F 	>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512bw
[ 	]*[0-9]+[ 	]+>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512cd
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2554F 	>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2550F 	>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2552F 	>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512dq
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2554F 	>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2550F 	>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2552F 	>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512er
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2554F 	>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2550F 	>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2552F 	>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512ifma
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2554F 	>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2550F 	>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2552F 	>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512pf
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2554F 	>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2550F 	>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2552F 	>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+8DF4
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512vbmi
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D0F 	>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D2F 	>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+1CF5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D48 	>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D08 	>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D28 	>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+C4F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD4F 	>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD0F 	>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1FD2F 	>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+7B31
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F27D4F 	>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+C8F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D54F 	>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D50F 	>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F1D52F 	>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+58F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D54F 	>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D50F 	>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2D52F 	>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+B4F4
[ 	]*[0-9]+[ 	]+\?\?\?\? 62F2FD49 	>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+C68CFD17 *
[ 	]*[0-9]+[ 	]+000000
[ 	]*[0-9]+[ 	]+>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
#...
[ 	]*[0-9]+[ 	]+>  \.arch default
[ 	]*[0-9]+[ 	]+>  \.arch \.noavx512f
[ 	]*[0-9]+[ 	]+>  vpabsb %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpabsb %xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpabsb %ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpconflictd %zmm5,%zmm6
[ 	]*[0-9]+[ 	]+>  vpconflictd %xmm5,%xmm6
[ 	]*[0-9]+[ 	]+>  vpconflictd %ymm5,%ymm6
[ 	]*[0-9]+[ 	]+>  vcvtpd2qq \(%ecx\),%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vcvtpd2qq \(%ecx\),%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vcvtpd2qq \(%ecx\),%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vexp2ps %zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vaddpd %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vaddpd %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vaddpd %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpmadd52luq %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpmadd52luq %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpmadd52luq %ymm4,%ymm5,%ymm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}
[ 	]*[0-9]+[ 	]+>  vpermb %zmm4,%zmm5,%zmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpermb %xmm4,%xmm5,%xmm6\{%k7\}
[ 	]*[0-9]+[ 	]+>  vpermb %ymm4,%ymm5,%ymm6\{%k7\}
#...
[ 	]*[0-9]+[ 	]+\?\?\?\? C4E2791C 		vpabsb %xmm5, %xmm6
[ 	]*[0-9]+[ 	]+F5
[ 	]*[0-9]+[ 	]+\?\?\?\? C4E27D1C 		vpabsb %ymm5, %ymm6
[ 	]*[0-9]+[ 	]+F5
[ 	]*[0-9]+[ 	]+\?\?\?\? C5D158F4 		vaddpd %xmm4, %xmm5, %xmm6
[ 	]*[0-9]+[ 	]+\?\?\?\? C5D558F4 		vaddpd %ymm4, %ymm5, %ymm6
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F381C 		pabsb %xmm5, %xmm6
[ 	]*[0-9]+[ 	]+F5
[ 	]*[0-9]+[ 	]+\?\?\?\? 660F58F4 		addpd %xmm4, %xmm6
#pass
