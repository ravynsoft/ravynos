; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@sqrt:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_sqrt_f32
; GCN-NEXT: epilog

#shader fs sqrt
#version 400
flat in float f;
void main() {
    gl_FragColor.x = sqrt(f);
}


; FUNC-LABEL: {{^}}@inv_sqrt:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_rsq_f32
; GCN-NEXT: epilog

#shader fs inv_sqrt
#version 400
flat in float f;
void main() {
    gl_FragColor.x = 1 / sqrt(f);
}


; FUNC-LABEL: {{^}}@rsq:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_rsq_f32
; GCN-NEXT: epilog

#shader fs rsq
#version 400
flat in float f;
void main() {
    gl_FragColor.x = inversesqrt(f);
}


; FUNC-LABEL: {{^}}@inv_rsq:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_sqrt_f32
; GCN-NEXT: epilog

#shader fs inv_rsq
#version 400
flat in float f;
void main() {
    gl_FragColor.x = 1 / inversesqrt(f);
}
