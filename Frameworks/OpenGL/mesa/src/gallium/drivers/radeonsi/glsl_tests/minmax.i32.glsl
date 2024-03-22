; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@min_i32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_min_i32
; GCN-NEXT: epilog

#shader fs min_i32
#version 400
flat in ivec2 v;
out ivec4 o;
void main() {
    o.x = min(v.x, v.y);
}


; FUNC-LABEL: {{^}}@max_i32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_max_i32
; GCN-NEXT: epilog

#shader fs max_i32
#version 400
flat in ivec2 v;
out ivec4 o;
void main() {
    o.x = max(v.x, v.y);
}
