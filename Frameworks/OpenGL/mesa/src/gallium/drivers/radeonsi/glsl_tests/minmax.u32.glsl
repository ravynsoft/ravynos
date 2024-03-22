; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@min_u32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_min_u32
; GCN-NEXT: epilog

#shader fs min_u32
#version 400
flat in uvec2 v;
out uvec4 o;
void main() {
    o.x = min(v.x, v.y);
}


; FUNC-LABEL: {{^}}@max_u32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_max_u32
; GCN-NEXT: epilog

#shader fs max_u32
#version 400
flat in uvec2 v;
out uvec4 o;
void main() {
    o.x = max(v.x, v.y);
}
