VC4
===

Mesa's VC4 graphics driver supports multiple implementations of
Broadcom's VideoCore IV GPU. It is notably used in the Raspberry Pi 0
through Raspberry Pi 3 hardware, and the driver is included as an
option as of the 2016-02-09 Raspbian release using ``raspi-config``.
On most other distributions such as Debian or Fedora, you need no
configuration to enable the driver.

This Mesa driver talks directly to the `VC4
<https://www.kernel.org/doc/html/latest/gpu/vc4.html>`__ kernel DRM
driver for scheduling graphics commands, and that module also provides
KMS display support.  The driver makes no use of the closed source VPU
firmware on the VideoCore IV block, instead talking directly to the
GPU block from Linux.

GLES2 support
-------------

The VC4 driver is a nearly conformant GLES2 driver, and the hardware
has achieved GLES2 conformance with other driver stacks.

OpenGL support
--------------

Along with GLES 2.0, the Mesa driver also exposes OpenGL 2.1, which is
mostly correct but with a few caveats.

* 4-byte index buffers.

GLES2.0, and VC4, don't have ``GL_UNSIGNED_INT`` index buffers. To support
them in VC4, we create a shadow copy of your index buffer with the
indices truncated to 2 bytes. This is incorrect (and will assertion
fail in debug builds of Mesa) if any of the indices were >65535. To
fix that, we would need to detect this case and rewrite the index
buffer and vertex buffers to do a series of draws each with small
indices and new vertex attrib bindings.

To avoid this problem, ensure that all index buffers are written using
``GL_UNSIGNED_SHORT``, even at the cost of doing multiple draw calls
with updated vertex attrib bindings.

* Occlusion queries

The VC4 hardware has no support for occlusion queries.  GL 2.0
requires that you support the occlusion queries extension, but you can
report 0 from ``glGetQueryiv(GL_SAMPLES_PASSED,
GL_QUERY_COUNTER_BITS)``. This is absurd, but it's how OpenGL handles
"we want the functions to be present everywhere, but we want it to be
optional for hardware to support it. Sadly, gallium doesn't yet allow
the driver to report 0 query bits.

* Primitive mode

VC4 doesn't support reducing triangles/quads/polygons to lines and
points like desktop GL. If front/back mode matched, we could rewrite
the index buffer to the new primitive type, but we don't. If
front/back mode don't match, we would need to run the vertex shader in
software, classify the prims, write new index buffers, and emit
(possibly many) new draw calls to rasterize the new prims in the same
order.

Bug Reporting
-------------

VC4 rendering bugs should go to Mesa's GitLab `issues
<https://gitlab.freedesktop.org/mesa/mesa/-/issues>`__ page.

By far the easiest way to communicate bug reports for rendering
problems is to take an apitrace. This passes exactly the drawing you
saw to the developer, without the developer needing to download and
build the application and replicate whatever steps you took to produce
the problem.  Traces attached to bug reports should ideally be small.

For GPU hangs, if you can get a short apitrace that produces the
problem, that's still the best.  If the problem takes a long time to
reproduce or you can't capture it in a trace, describing how to
reproduce and including a GPU hang dump would be the most
useful. Install `vc4-gpu-tools
<https://github.com/anholt/vc4-gpu-tools/>`__ and use
``vc4_dump_hang_state my-app.hang``. Sometimes the hang file will
provide useful information.

Tiled Rendering
---------------

VC4 is a tiled renderer, chopping the screen into 64x64 (non-MSAA) or
32x32 (MSAA) tiles and rendering the scene per tile. Rasterization
looks like::

    (CPU) Allocate space to store a list of draw commands per tile
    (CPU) Set up a command list per tile that does:
        Either load the current tile's color buffer from memory, or clear it.
        Either load the current tile's depth buffer from memory, or clear it.
        Branch into the draw list for the tile
        Store the depth buffer if anybody might read it.
        Store the color buffer if anybody might read it.
    (GPU) Initialize the per-tile draw call lists to empty.
    (GPU) Run all draw calls collecting vertex data
    (GPU) For each tile covered by a draw call's primitive.
        Emit state packets to the list to update it to the current draw call's state.
        Emit a primitive description into the tile's draw call list.

Tiled rendering avoids the need for large render target caches, at the
expense of increasing the cost of vertex processing. Unlike some tiled
renderers, VC4 has no non-tiled rendering mode.

Performance Tricks
------------------

* Reducing memory bandwidth by clearing.

Even if your drawing is going to cover the entire render target, it's
more efficient for VC4 if you emit a ``glClear()`` of the color and
depth buffers. This means we can skip the load of the previous state
from memory, in favor of a cheap GPU-side ``memset()`` of the tile
buffer before we start running the draw calls.

* Reducing memory bandwidth with scissoring.

If all draw calls for the frame are with a ``glScissor()`` to only
part of the screen, then we can skip setting up the tiles for that
area, which means a little less memory used setting up the empty bins,
and a lot less memory used loading/storing the unchanged tiles.

* Reducing memory bandwidth with ``glInvalidateFramebuffer()``.

If we don't know who might use the contents of the framebuffer's depth
or color in the future, then we have to store it for later. If you use
glInvalidateFramebuffer() before accessing the results of your
rendering, then we can skip the store of the depth or color
buffer. Note that this is unimplemented.

* Avoid non-constant GLSL array indexing

In VC4 the only non-constant-index array access supported in hardware
is uniforms. For everything else (inputs, outputs, temporaries), we
have to lower them to an IF ladder like::

  if (index == 0)
     return array[0]
  else if (index == 1)
    return array[1]
  ...

This is very expensive as we probably have to execute every branch of
every IF statement due to it being a SIMD machine. So, it is
recommended (if you can) to avoid non-uniform non-constant array
indexing.

Note that if you do variable indexing within a bounded loop that Mesa
can unroll, that can actually count as constant indexing.

* Increasing GPU memory Increase CMA pool size

The memory for the VC4 driver is allocated from the standard Linux CMA
pool. The size of this pool defaults to 64 MB.  To increase this, pass
an additional parameter on the kernel command line.  Edit the boot
partition's ``cmdline.txt`` to add::

  cma=256M@256M

``cmdline.txt`` is a single line with whitespace separated parameters.

The first value is the size of the pool and the second parameter is
the start address of the pool. The pool size can be increased further,
but it must fit into the memory, so size + start address must be below
1024M (Pi 2, 3, 3+) or 512M (Pi B, B+, Zero, Zero W). Also this
reduces the memory available to Linux.

* Decrease firmware memory

The firmware allocates a fixed chunk of memory before booting
Linux. If firmware functions are not required, this amount can be
reduced.

In ``config.txt`` edit ``gpu_mem`` to 16, if you do not need video decoding,
edit gpu_mem to 64 if you need video decoding.

Performance debugging
---------------------

* Step 1: Known issues

The first tool to look at is running your application with the
environment variable ``VC4_DEBUG=perf`` set. This will report debug
information for many known causes of performance problems on the
console. Not all of them will cause visible performance improvements
when fixed, but it's a good first step to see what might going wrong.

* Step 2: CPU vs GPU

The primary question is figuring out whether the CPU is busy in your
application, the CPU is busy in the GL driver, the GPU is waiting for
the CPU, or the CPU is waiting for the GPU. Ideally, you get to the
point where the CPU is waiting for the GPU infrequently but for a
significant amount of time (however long it takes the GPU to draw a
frame).

Start with top while your application is running. Is the CPU usage
around 90%+? If so, then our performance analysis will be with
sysprof. If it's not very high, is the GPU staying busy? We don't have
a clean tool for this yet, but ``cat /debug/dri/0/v3d_regs`` could be
useful. If ``CT0CA`` != ``CT0EA`` or ``CT1CA`` != ``CT1EA``, that
means that the GPU is currently busy processing some rendering job.

* sysprof for CPU usage

If the CPU is totally busy and the GPU isn't terribly busy, there is
an excellent tool for debugging: sysprof. Install, run as root (so you
can get system-wide profiling), hit play and later stop. The top-left
area shows the flat profile sorted by total time of that symbol plus
its descendants. The top few are generally uninteresting (main() and
its descendants consuming a lot), but eventually you can get down to
something interesting. Click it, and to the right you get the
callchains to descendants -- where all that time actually went. On the
other hand, the lower left shows callers -- double-clicking those
selects that as the symbol to view, instead.

Note that you need debug symbols for the callgraphs in sysprof to
work, which is where most of its value is. Most distributions offer
debug symbol packages from their builds which can be installed
separately, and sysprof will find them. I've found that on arm, the
debug packages are not enough, and if someone could determine what is
necessary for callgraphs in debugging, that would be really helpful.

* perf for CPU waits on GPU

If the CPU is not very busy and the GPU is not very busy, then we're
probably ping-ponging between the two. Most cases of this would be
noticed by ``VC4_DEBUG=perf``, but not all. To see all cases where
this happens, use the perf tool from the Linux kernel (note: unrelated
to ``VC4_DEBUG=perf``)::

    sudo perf record -f -g -e vc4:vc4_wait_for_seqno_begin -c 1 openarena

If you want to see the whole system's stalls for a period of time
(very useful!), use the -a flag instead of a particular command
name. Just ``^C`` when you're done capturing data.

At exit, you'll have ``perf.data`` in the current directory. You can print
out the results with::

    perf report | less

* Debugging for GPU fully busy

As of Linux kernel 4.17 and Mesa 18.1, we now expose the hardware's
performance counters in OpenGL. Install apitrace, and trace your
application with::

    apitrace trace <application>          # for GLX applications
    apitrace trace -a egl <application>   # for EGL applications

Once you've captured a trace, you can see what counters are available
and replay it while looking while looking at some of those counters::

    apitrace replay <application>.trace --list-metrics

    apitrace replay <application>.trace --pdraw=GL_AMD_performance_monitor:QPU-total-clk-cycles-vertex-coord-shading

Multiple counters can be captured at once with commas separating them.

Once you've found what draw calls are surprisingly expensive in one of
the counters, you can work out which ones they were at the GL level by
opening the trace up in qapitrace and using ``^-G`` to jump to that call
number and ``^-L`` to look up the GL state at that call.

shader-db
---------

shader-db is often used as a proxy for real-world app performance when
working on the compiler in Mesa.  On VC4, there is a lot of
state-dependent code in the shaders (like blending or vertex attribute
format handling), so the typical `shader-db
<https://gitlab.freedesktop.org/mesa/shader-db>`__ will miss important
areas for optimization.  Instead, anholt wrote a `new one
<https://cgit.freedesktop.org/~anholt/shader-db-2/>`__ based on
apitraces.  Once you have a collection of traces, starting from
`traces-db <https://gitlab.freedesktop.org/gfx-ci/tracie/traces-db/>`__,
you can test a compiler change in this shader-db with::

  ./run.py > before
  (cd ../mesa && make install)
  ./run.py > after
  ./report.py before after

Hardware Documentation
----------------------

For driver developers, Broadcom publicly released a `specification
<https://docs.broadcom.com/doc/12358545>`__ PDF for the 21553, which
is closely related to the VC4 GPU present in the Raspberry Pi.  They
also released a `snapshot <https://docs.broadcom.com/docs/12358546>`__
of a corresponding Android graphics driver.  That graphics driver was
ported to Raspbian for a demo, but was not expected to have ongoing
development.

Developers with NDA access with Broadcom or Raspberry Pi can
potentially get access to "simpenrose", the C software simulator of
the GPU.  The Mesa driver includes a backend (``vc4_simulator.c``) to
use simpenrose from an x86 system with the i915 graphics driver with
all of the VC4 rendering commands emulated on simpenrose and memcpyed
to the real GPU.
