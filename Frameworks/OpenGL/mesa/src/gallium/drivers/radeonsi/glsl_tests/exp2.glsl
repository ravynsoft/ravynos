; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@exp2:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_exp_f32
; GCN-NEXT: epilog

#shader fs exp2
#version 400
flat in float f;
void main() {
    gl_FragColor.x = exp2(f);
}
