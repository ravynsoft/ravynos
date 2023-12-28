
  .allow_index_reg
  .text
_start:
  ldtilecfg  (%rcx,%rdx,2)
  sttilecfg  (%rcx,%rdx,2)
  tdpbf16ps %tmm5, %tmm4, %tmm3
  tdpbssd %tmm3, %tmm2, %tmm1
  tdpbsud %tmm3, %tmm2, %tmm1
  tdpbusd %tmm3, %tmm2, %tmm1
  tdpbuud %tmm3, %tmm2, %tmm1
  tileloadd foo, %tmm5
  tileloadd (%rcx), %tmm5
  tileloadd (%ecx), %tmm5
  tileloadd (%rcx,%rdx,1), %tmm5
  tileloadd (%ecx,%edx,2), %tmm1
  tileloaddt1 foo, %tmm5
  tileloaddt1 (%rcx), %tmm5
  tileloaddt1 (%ecx), %tmm5
  tileloaddt1 (%rcx,%rdx,1), %tmm5
  tileloaddt1 (%ecx,%edx,2), %tmm1
  tileloaddt1 (%rcx,%riz,2), %tmm1
  tilerelease
  tilestored %tmm5, (%rcx)
  tilestored %tmm5, (%ecx)
  tilestored %tmm5, (%rcx,%rdx,1)
  tilestored %tmm1, (%ecx,%edx,2)
  tilezero %tmm0
  tilezero %tmm5
  tilezero %tmm7


  .intel_syntax noprefix
  ldtilecfg  [rcx]
  ldtilecfg  [rbx]
  sttilecfg  [rcx]
  sttilecfg  [rbx]
  tdpbf16ps tmm3, tmm4, tmm5
  tdpbssd tmm1, tmm2, tmm3
  tdpbsud tmm1, tmm2, tmm3
  tdpbusd tmm1, tmm2, tmm3
  tdpbuud tmm1, tmm2, tmm3
  tileloadd tmm5, foo
  tileloadd tmm5, [rcx]
  tileloadd tmm5, [ecx]
  tileloadd tmm5, [rcx+rdx]
  tileloadd tmm1, [ecx+edx*2]
  tileloaddt1 tmm5, foo
  tileloaddt1 tmm5, [rcx]
  tileloaddt1 tmm5, [ecx]
  tileloaddt1 tmm5, [rcx+rdx]
  tileloaddt1 tmm1, [ecx+edx*2]
  tileloaddt1 tmm1, [rcx+riz*2]
  tilerelease
  tilestored [rcx], tmm5
  tilestored [ecx], tmm5
  tilestored [rcx+rdx], tmm5
  tilestored [ecx+edx*2], tmm1
  tilezero tmm0
  tilezero tmm5
  tilezero tmm7
