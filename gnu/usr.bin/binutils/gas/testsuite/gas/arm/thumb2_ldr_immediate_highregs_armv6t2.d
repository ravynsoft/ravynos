# name: Ldr small immediate high registers on armv6t2
# as: -march=armv6t2
# objdump: -dr --prefix-addresses --show-raw-insn
# notarget: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section \.text:
0[0-9a-f]+ <[^>]+> f04f 0000[[:space:]]+mov\.w[[:space:]]+r0, #0.*
0[0-9a-f]+ <[^>]+> f04f 0108[[:space:]]+mov\.w[[:space:]]+r1, #8.*
0[0-9a-f]+ <[^>]+> f04f 0251[[:space:]]+mov\.w[[:space:]]+r2, #81.*
0[0-9a-f]+ <[^>]+> f04f 031f[[:space:]]+mov\.w[[:space:]]+r3, #31.*
0[0-9a-f]+ <[^>]+> f04f 042f[[:space:]]+mov\.w[[:space:]]+r4, #47.*
0[0-9a-f]+ <[^>]+> f04f 053f[[:space:]]+mov\.w[[:space:]]+r5, #63.*
0[0-9a-f]+ <[^>]+> f04f 0680[[:space:]]+mov\.w[[:space:]]+r6, #128.*
0[0-9a-f]+ <[^>]+> f04f 07ff[[:space:]]+mov\.w[[:space:]]+r7, #255.*
0[0-9a-f]+ <[^>]+> f04f 0800[[:space:]]+mov\.w[[:space:]]+r8, #0.*
0[0-9a-f]+ <[^>]+> f04f 0908[[:space:]]+mov\.w[[:space:]]+r9, #8.*
0[0-9a-f]+ <[^>]+> f04f 0a51[[:space:]]+mov\.w[[:space:]]+sl, #81.*
0[0-9a-f]+ <[^>]+> f04f 0b1f[[:space:]]+mov\.w[[:space:]]+fp, #31.*
0[0-9a-f]+ <[^>]+> f04f 0c2f[[:space:]]+mov\.w[[:space:]]+ip, #47.*
0[0-9a-f]+ <[^>]+> f04f 0e80[[:space:]]+mov\.w[[:space:]]+lr, #128.*
0[0-9a-f]+ <[^>]+> f64f 78ff[[:space:]]+movw[[:space:]]+r8, #65535.*
0[0-9a-f]+ <[^>]+> f24f 09f0[[:space:]]+movw[[:space:]]+r9, #61680.*
0[0-9a-f]+ <[^>]+> f8df d004[[:space:]]+ldr\.w[[:space:]]+sp, \[pc, #4\].*
0[0-9a-f]+ <[^>]+> f8df f004[[:space:]]+ldr\.w[[:space:]]+pc, \[pc, #4\].*
0[0-9a-f]+ <[^>]+> 0000003f[[:space:]]+.word[[:space:]]+0x0000003f.*
0[0-9a-f]+ <[^>]+> 000000ff[[:space:]]+.word[[:space:]]+0x000000ff.*
