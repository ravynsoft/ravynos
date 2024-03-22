; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@frexp:
; GCN: main
; GCN: v_interp_mov
; GCN-DAG: v_frexp_mant_f32
; GCN-DAG: v_frexp_exp_i32_f32
; GCN-NEXT: epilog

#shader fs frexp
#version 400
flat in float f;
void main() {
    gl_FragColor.x = frexp(f, gl_FragColor.y);
}
