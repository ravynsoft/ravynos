Overview
========

Computerator is a tool to launch compute shaders, written in assembly.
The main purpose is to have an easy way to experiment with instructions
without dealing with the entire compiler stack (which makes controlling
the order of instructions, the registers chosen, etc, difficult).  The
choice of compute shaders is simply because there is far less state
setup required.

Headers
-------

The shader assembly can be prefixed with headers to control state setup:

* ``@localsize X, Y, Z`` - configures local workgroup size
* ``@buf SZ (cN.c)`` - configures an SSBO of the specified size (in dwords).
  The order of the ``@buf`` headers determines the index, ie the first
  ``@buf`` header is ``g[0]``, the second ``g[1]``, and so on.
  The iova of the buffer is written as a vec2 to ``cN.c``
* ``@const(cN.c)`` configures a const vec4 starting at specified
  const register, ie ``@const(c1.x) 1.0, 2.0, 3.0, 4.0`` will populate
  ``c1.xyzw`` with ``vec4(1.0, 2.0, 3.0, 4.0)``
* ``@invocationid(rN.c)`` will populate a vec3 starting at the specified
  register with the local invocation-id
* ``@wgid(rN.c)`` will populate a vec3 starting at the specified register
  with the workgroup-id (must be a high-reg, ie. ``r48.x`` and above)
* ``@numwg(cN.c)`` will populate a vec3 starting at the specified const
  register

Example
-------

```
@localsize 32, 1, 1
@buf 32  ; g[0]
@const(c0.x)  0.0, 0.0, 0.0, 0.0
@const(c1.x)  1.0, 2.0, 3.0, 4.0
@wgid(r48.x)        ; r48.xyz
@invocationid(r0.x) ; r0.xyz
@numwg(c2.x)        ; c2.xyz
mov.u32u32 r0.y, r0.x
(rpt5)nop
stib.untyped.1d.u32.1 g[0] + r0.y, r0.x
end
nop
```

Usage
-----

```
cat myshader.asm | ./computerator --disasm --groups=4,4,4
```

