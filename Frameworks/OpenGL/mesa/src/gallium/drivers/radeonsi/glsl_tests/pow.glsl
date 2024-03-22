; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@pow:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_log_f32
; GCN-NEXT: v_interp_mov
; GCN-NEXT: v_mul_legacy_f32
; GCN-NEXT: v_exp_f32
; GCN-NEXT: epilog

#shader fs pow
#version 400
flat in vec2 v;
void main() {
    gl_FragColor.x = pow(v.x, v.y);
}
