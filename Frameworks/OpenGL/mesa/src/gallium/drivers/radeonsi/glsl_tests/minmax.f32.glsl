; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@min_f32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_min_f32
; GCN-NEXT: epilog

#shader fs min_f32
#version 400
flat in vec2 v;
void main() {
    gl_FragColor.x = min(v.x, v.y);
}


; FUNC-LABEL: {{^}}@max_f32:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_max_f32
; GCN-NEXT: epilog

#shader fs max_f32
#version 400
flat in vec2 v;
void main() {
    gl_FragColor.x = max(v.x, v.y);
}
