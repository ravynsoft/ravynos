; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@bitcount:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_bcnt_u32
; GCN-NEXT: epilog

#shader fs bitcount
#version 400
flat in int i;
out ivec4 o;
void main() {
    o.x = bitCount(i);
}
