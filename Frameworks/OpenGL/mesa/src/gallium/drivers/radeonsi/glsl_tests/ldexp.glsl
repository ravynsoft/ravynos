; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@ldexp:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_ldexp_f32
; GCN-NEXT: epilog

#shader fs ldexp
#version 400
flat in float f;
flat in int i;
void main() {
    gl_FragColor.x = ldexp(f, i);
}
