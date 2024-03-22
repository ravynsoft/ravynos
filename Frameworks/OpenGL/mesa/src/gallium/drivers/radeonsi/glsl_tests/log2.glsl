; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; FUNC-LABEL: {{^}}@log2:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_log_f32
; GCN-NEXT: epilog

#shader fs log2
#version 400
flat in float f;
void main() {
    gl_FragColor.x = log2(f);
}
