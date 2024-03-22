; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@bfi_i32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_bfm_b32
; GCN-NEXT: v_lshlrev_b32
; GCN-NEXT: v_bfi_b32
; GCN-NEXT: epilog

#shader fs bfi_i32
#version 400
flat in ivec4 v;
out ivec4 o;
void main() {
    o.x = bitfieldInsert(v.x, v.y, v.z, v.w);
}


; FUNC-LABEL: {{^}}@bfi_u32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_bfm_b32
; GCN-NEXT: v_lshlrev_b32
; GCN-NEXT: v_bfi_b32
; GCN-NEXT: epilog

#shader fs bfi_u32
#version 400
flat in uvec4 v;
out uvec4 o;
void main() {
    o.x = bitfieldInsert(v.x, v.y, int(v.z), int(v.w));
}
