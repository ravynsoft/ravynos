; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@fma:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_mac_f32
; GCN-NEXT: epilog

#shader fs fma
#version 400
flat in vec3 v;
void main() {
    gl_FragColor.x = fma(v.x, v.y, v.z);
}
