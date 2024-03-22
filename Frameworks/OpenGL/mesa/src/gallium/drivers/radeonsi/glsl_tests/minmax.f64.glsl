; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@min_f64:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_min_f64
; GCN-NEXT: epilog

#shader fs min_f64
#version 400
flat in dvec2 v;
out uvec4 o;
void main() {
    o.xy = unpackDouble2x32(min(v.x, v.y));
}


; FUNC-LABEL: {{^}}@max_f64:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_max_f64
; GCN-NEXT: epilog

#shader fs max_f64
#version 400
flat in dvec2 v;
out uvec4 o;
void main() {
    o.xy = unpackDouble2x32(max(v.x, v.y));
}
