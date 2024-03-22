u_trace GPU Performance Tracing
===============================

Mesa has its own GPU performance tracing framework which drivers may
choose to implement. ``gpu.renderstages.*`` producer for
:doc:`Perfetto Tracing <perfetto>` is based on u_trace.

It doesn't require external dependencies and much simpler to use. Though
it provides information only about GPU timings and is harder to analyze
for complex rendering.

u_trace is useful when one needs to quickly identify performance bottleneck,
or to build a tool to analyze the raw performance data.

Drivers which support u_trace:
   - Intel drivers: ANV, Iris
   - Adreno drivers: Freedreno, Turnip

Usage
-----

u_trace is controlled by environment variables:

.. envvar:: MESA_GPU_TRACES

   controls whether u_trace is enabled and trace output

   ``print``
      prints in a human readable text format. It should be noted that this
      is mutually exclusive with ``print_json`` and both cannot be enabled
      at the same time.
   ``print_json``
      prints in JSON format, suitable for parsing. Application should
      appropriately finish its rendering in order for trace's json to be
      valid. For the Vulkan API, it is expected to destroy the device,
      for GL it's expected to destroy the context.
   ``perfetto``
      enables Perfetto instrumentation prior to connecting, Perfetto
      traces can be collected without setting this but it may miss some
      events prior to the tracing session being started.
   ``markers``
      enables marker instrumentation, will print utrace markers into
      the CS which can then be viewed by dumping the CS from the driver.

         - For Turnip, ``cffdump`` can be used to view the markers in
           the trace.

.. envvar:: MESA_GPU_TRACEFILE

   specifies a file where to write the output instead of ``stdout``

.. envvar:: *_GPU_TRACEPOINT

   tracepoints can be enabled or disabled using driver specific environment
   variable. Most tracepoints are enabled by default. For instance
   ``TU_GPU_TRACEPOINT=-blit,+render_pass`` will disable the
   ``blit`` tracepoints and enable the ``render_pass`` tracepoints.

   .. list-table::
      :header-rows: 1

      * - Driver
        - Environment Variable
        - Tracepoint Definitions
      * - Freedreno
        - .. envvar:: FD_GPU_TRACEPOINT
        - ``src/gallium/drivers/freedreno/freedreno_tracepoints.py``
      * - Turnip
        - .. envvar:: TU_GPU_TRACEPOINT
        - ``src/freedreno/vulkan/tu_tracepoints.py``
      * - Anv
        - .. envvar:: INTEL_GPU_TRACEPOINT
        - ``src/intel/vulkan/intel_tracepoints.py``
