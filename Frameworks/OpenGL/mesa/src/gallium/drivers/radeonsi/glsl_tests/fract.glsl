; RUN: ./amdgcn_glslc -mcpu=tahiti %s | FileCheck -check-prefix=GCN -check-prefix=FUNC -check-prefix=SI %s
; RUN: ./amdgcn_glslc -mcpu=bonaire %s | FileCheck -check-prefix=GCN -check-prefix=FUNC -check-prefix=CI %s
; RUN: ./amdgcn_glslc -mcpu=tonga %s | FileCheck -check-prefix=GCN -check-prefix=FUNC -check-prefix=CI %s

; Only SI has buggy v_fract and must use v_floor.
; The amdgcn.fract intrinsic can be used only if LLVM passes are able to move it.

; FUNC-LABEL: {{^}}@fract:
; GCN: main
; GCN: v_interp_mov
; SI-NEXT: v_floor_f32
; SI-NEXT: v_subrev_f32
; CI-NEXT: v_fract_f32
; GCN-NEXT: epilog

#shader fs fract
#version 400
flat in float f;
void main() {
    gl_FragColor.x = fract(f);
}
