; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@div:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_rcp_f32
; GCN-NEXT: v_mul_f32
; GCN-NEXT: epilog

#shader fs div
#version 400
flat in vec2 v;
void main() {
    gl_FragColor.x = v.x / v.y;
}


; FUNC-LABEL: {{^}}@rcp:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_rcp_f32
; GCN-NEXT: epilog

#shader fs rcp
#version 400
flat in float x;
void main() {
    gl_FragColor.x = 1 / x;
}
