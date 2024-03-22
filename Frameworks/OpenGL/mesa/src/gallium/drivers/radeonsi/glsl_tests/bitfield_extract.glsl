; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@bfe_i32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_bfe_i32
; GCN-NEXT: epilog

#shader fs bfe_i32
#version 400
flat in ivec3 v;
out ivec4 o;
void main() {
    o.x = bitfieldExtract(v.x, v.y, v.z);
}


; FUNC-LABEL: {{^}}@bfe_u32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_bfe_u32
; GCN-NEXT: epilog

#shader fs bfe_u32
#version 400
flat in uvec3 v;
out uvec4 o;
void main() {
    o.x = bitfieldExtract(v.x, int(v.y), int(v.z));
}
