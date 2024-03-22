; RUN: ./amdgcn_glslc %s | FileCheck -check-prefix=GCN -check-prefix=FUNC %s

; We don't want any "v_and" or "v_or" here. v_cvt_f16 only writes the lower 16 bits.

; FUNC-LABEL: {{^}}@packhalf:
; GCN: main
; GCN: v_interp_mov
; GCN: v_interp_mov
; GCN-NEXT: v_cvt_f16_f32
; GCN-NEXT: v_lshlrev_b32
; GCN-NEXT: v_cvt_f16_f32
; GCN-NEXT: epilog

#shader fs packhalf
#version 420
flat in vec2 v;
out uvec4 o;
void main() {
    o.x = packHalf2x16(v);
}


; FUNC-LABEL: {{^}}@unpackhalf:
; GCN: main
; GCN: v_interp_mov
; GCN-NEXT: v_cvt_f32_f16
; GCN-NEXT: v_lshrrev_b32
; GCN-NEXT: v_cvt_f32_f16
; GCN-NEXT: epilog

#shader fs unpackhalf
#version 420
flat in uint u;
out vec4 o;
void main() {
    o.xy = unpackHalf2x16(u);
}
