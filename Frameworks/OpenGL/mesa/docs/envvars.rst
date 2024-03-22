Environment Variables
=====================

Normally, no environment variables need to be set. Most of the
environment variables used by Mesa/Gallium are for debugging purposes,
but they can sometimes be useful for debugging end-user issues.

LibGL environment variables
---------------------------

.. envvar:: LIBGL_DEBUG

   If defined debug information will be printed to stderr. If set to
   ``verbose`` additional information will be printed.

.. envvar:: LIBGL_DRIVERS_PATH

   colon-separated list of paths to search for DRI drivers

.. envvar:: LIBGL_ALWAYS_INDIRECT

   if set to ``true``, forces an indirect rendering context/connection.

.. envvar:: LIBGL_ALWAYS_SOFTWARE

   if set to ``true``, always use software rendering

.. envvar:: LIBGL_NO_DRAWARRAYS

   if set to ``true``, do not use DrawArrays GLX protocol (for
   debugging)

.. envvar:: LIBGL_DRI2_DISABLE

   disable DRI2 if set to ``true``.

.. envvar:: LIBGL_DRI3_DISABLE

   disable DRI3 if set to ``true``.

Core Mesa environment variables
-------------------------------

.. envvar:: MESA_NO_ERROR

   if set to 1, error checking is disabled as per :ext:`GL_KHR_no_error`.
   This will result in undefined behavior for invalid use of the API, but
   can reduce CPU use for apps that are known to be error free.

.. envvar:: MESA_DEBUG

   if set, error messages are printed to stderr. For example, if the
   application generates a ``GL_INVALID_ENUM`` error, a corresponding
   error message indicating where the error occurred, and possibly why,
   will be printed to stderr. For release builds, :envvar:`MESA_DEBUG`
   defaults to off (no debug output). :envvar:`MESA_DEBUG` accepts the
   following comma-separated list of named flags, which adds extra
   behavior to just set :envvar:`MESA_DEBUG` to ``1``:

   ``silent``
      turn off debug messages. Only useful for debug builds.
   ``flush``
      flush after each drawing command
   ``incomplete_tex``
      extra debug messages when a texture is incomplete
   ``incomplete_fbo``
      extra debug messages when a FBO is incomplete
   ``context``
      create a debug context (see ``GLX_CONTEXT_DEBUG_BIT_ARB``) and
      print error and performance messages to stderr (or
      ``MESA_LOG_FILE``).

.. envvar:: MESA_PROCESS_NAME

   if set, overrides the process name string used internally for various
   purposes (e.g. for driconf option matching, logging, artifact storage,
   etc.).

.. envvar:: MESA_LOG_FILE

   specifies a file name for logging all errors, warnings, etc., rather
   than stderr

.. envvar:: MESA_EXTENSION_OVERRIDE

   can be used to enable/disable extensions. A value such as
   ``GL_EXT_foo -GL_EXT_bar`` will enable the ``GL_EXT_foo`` extension
   and disable the ``GL_EXT_bar`` extension. Note that this will override
   extensions override configured using driconf.

.. envvar:: MESA_EXTENSION_MAX_YEAR

   The ``GL_EXTENSIONS`` string returned by Mesa is sorted by extension
   year. If this variable is set to year X, only extensions defined on
   or before year X will be reported. This is to work-around a bug in
   some games where the extension string is copied into a fixed-size
   buffer without truncating. If the extension string is too long, the
   buffer overrun can cause the game to crash. This is a work-around for
   that.

.. envvar:: MESA_GL_VERSION_OVERRIDE

   changes the value returned by ``glGetString(GL_VERSION)`` and
   possibly the GL API type.

   -  The format should be ``MAJOR.MINOR[FC|COMPAT]``
   -  ``FC`` is an optional suffix that indicates a forward compatible
      context. This is only valid for versions >= 3.0.
   -  ``COMPAT`` is an optional suffix that indicates a compatibility
      context or :ext:`GL_ARB_compatibility` support. This is only valid
      for versions >= 3.1.
   -  GL versions <= 3.0 are set to a compatibility (non-Core) profile
   -  GL versions = 3.1, depending on the driver, it may or may not have
      the :ext:`GL_ARB_compatibility` extension enabled.
   -  GL versions >= 3.2 are set to a Core profile
   -  Examples:

      ``2.1``
         select a compatibility (non-Core) profile with GL version 2.1.
      ``3.0``
         select a compatibility (non-Core) profile with GL version 3.0.
      ``3.0FC``
         select a Core+Forward Compatible profile with GL version 3.0.
      ``3.1``
         select GL version 3.1 with :ext:`GL_ARB_compatibility` enabled
         per the driver default.
      ``3.1FC``
         select GL version 3.1 with forward compatibility and
         :ext:`GL_ARB_compatibility` disabled.
      ``3.1COMPAT``
         select GL version 3.1 with :ext:`GL_ARB_compatibility` enabled.
      ``X.Y``
         override GL version to X.Y without changing the profile.
      ``X.YFC``
         select a Core+Forward Compatible profile with GL version X.Y.
      ``X.YCOMPAT``
         select a Compatibility profile with GL version X.Y.

   -  Mesa may not really implement all the features of the given
      version. (for developers only)

.. envvar:: MESA_GLES_VERSION_OVERRIDE

   changes the value returned by ``glGetString(GL_VERSION)`` for OpenGL
   ES.

   -  The format should be ``MAJOR.MINOR``
   -  Examples: ``2.0``, ``3.0``, ``3.1``
   -  Mesa may not really implement all the features of the given
      version. (for developers only)

.. envvar:: MESA_GLSL_VERSION_OVERRIDE

   changes the value returned by
   ``glGetString(GL_SHADING_LANGUAGE_VERSION)``. Valid values are
   integers, such as ``130``. Mesa will not really implement all the
   features of the given language version if it's higher than what's
   normally reported. (for developers only)

.. envvar:: MESA_DRICONF_EXECUTABLE_OVERRIDE

   if set, overrides the "executable" string used specifically for driconf
   option matching. This takes higher precedence over more general process
   name override (e.g. MESA_PROCESS_NAME).

.. envvar:: MESA_SHADER_CACHE_DISABLE

   if set to ``true``, disables the on-disk shader cache. If set to
   ``false``, enables the on-disk shader cache when it is disabled by
   default.  Note that EGL_ANDROID_blob_cache is still enabled even
   if on-disk shader cache is disabled.

.. envvar:: MESA_SHADER_CACHE_MAX_SIZE

   if set, determines the maximum size of the on-disk cache of compiled
   shader programs. Should be set to a number optionally followed by
   ``K``, ``M``, or ``G`` to specify a size in kilobytes, megabytes, or
   gigabytes. By default, gigabytes will be assumed. And if unset, a
   maximum size of 1GB will be used.

   .. note::

      A separate cache might be created for each architecture that Mesa is
      installed for on your system. For example under the default settings
      you may end up with a 1GB cache for x86_64 and another 1GB cache for
      i386.

.. envvar:: MESA_SHADER_CACHE_DIR

   if set, determines the directory to be used for the on-disk cache of
   compiled shader programs. If this variable is not set, then the cache
   will be stored in ``$XDG_CACHE_HOME/mesa_shader_cache`` (if that
   variable is set), or else within ``.cache/mesa_shader_cache`` within
   the user's home directory.

.. envvar:: MESA_SHADER_CACHE_SHOW_STATS

   if set to ``true``, keeps hit/miss statistics for the shader cache.
   These statistics are printed when the app terminates.

.. envvar:: MESA_DISK_CACHE_SINGLE_FILE

   if set to 1, enables the single file Fossilize DB on-disk shader
   cache implementation instead of the default multi-file cache
   implementation. This implementation reduces the overall disk usage by
   the shader cache and also allows for loading of precompiled cache
   DBs via :envvar:`MESA_DISK_CACHE_READ_ONLY_FOZ_DBS` or
   :envvar:`MESA_DISK_CACHE_READ_ONLY_FOZ_DBS_DYNAMIC_LIST`. This
   implementation does not support cache size limits via
   :envvar:`MESA_SHADER_CACHE_MAX_SIZE`. If
   :envvar:`MESA_SHADER_CACHE_DIR` is not set, the cache will be stored
   in ``$XDG_CACHE_HOME/mesa_shader_cache_sf`` (if that variable is set)
   or else within ``.cache/mesa_shader_cache_sf`` within the user's home
   directory.

.. envvar:: MESA_DISK_CACHE_READ_ONLY_FOZ_DBS

   if set with :envvar:`MESA_DISK_CACHE_SINGLE_FILE` enabled, references
   a string of comma separated file paths to read only Fossilize DB
   shader caches for loading at initialization. The file paths are
   relative to the cache directory and do not include suffixes,
   referencing both the cache DB and its index file. E.g.
   ``MESA_DISK_CACHE_SINGLE_FILE=filename1`` refers to ``filename1.foz``
   and ``filename1_idx.foz``. A limit of 8 DBs can be loaded and this limit
   is shared with :envvar:`MESA_DISK_CACHE_READ_ONLY_FOZ_DBS_DYNAMIC_LIST.`

.. envvar:: MESA_DISK_CACHE_DATABASE

   if set to 1, enables the Mesa-DB single file on-disk shader cache
   implementation instead of the default multi-file cache implementation.
   Like :envvar:`MESA_DISK_CACHE_SINGLE_FILE`, Mesa-DB reduces overall
   disk usage but Mesa-DB supports cache size limits via
   :envvar:`MESA_SHADER_CACHE_MAX_SIZE`. If
   :envvar:`MESA_SHADER_CACHE_DIR` is not set, the cache will be stored
   in ``$XDG_CACHE_HOME/mesa_shader_cache_db`` (if that variable is set)
   or else within ``.cache/mesa_shader_cache_db`` within the user's home
   directory.

.. envvar:: MESA_DISK_CACHE_DATABASE_NUM_PARTS

   specifies number of mesa-db cache parts, default is 50.

.. envvar:: MESA_DISK_CACHE_DATABASE_EVICTION_SCORE_2X_PERIOD

   Mesa-DB cache eviction algorithm calculates weighted score for the
   cache items. The weight is doubled based on the last access time of
   cache entry. By default period of weight doubling is set to one month.
   Period value is given in seconds.

.. envvar:: MESA_DISK_CACHE_READ_ONLY_FOZ_DBS_DYNAMIC_LIST

   if set with :envvar:`MESA_DISK_CACHE_SINGLE_FILE` enabled, references
   a text file that contains a new-line separated list of read only
   Fossilize DB shader caches to load. The list file is modifiable at
   runtime to allow for loading read only caches after initialization
   unlike :envvar:`MESA_DISK_CACHE_READ_ONLY_FOZ_DBS`. This variable
   takes an absolute path to the list file. The list file must exist at
   initialization for updating to occur. Cache files in the list take
   relative paths to the current cache directory like
   :envvar:`MESA_DISK_CACHE_READ_ONLY_FOZ_DBS`. A limit of 8 DBs can be
   loaded and this limit is shared with
   :envvar:`MESA_DISK_CACHE_READ_ONLY_FOZ_DBS`.

.. envvar:: MESA_DISK_CACHE_COMBINE_RW_WITH_RO_FOZ

   if set to 1, enables simultaneous use of :abbr:`RW (read-write)` and
   :abbr:`RW (read-write)` Fossilize DB caches. At first, data will be
   retrieved from the RO Fossilize cache. If data isn't found in the RO
   cache, then it will be retrieved from the RW cache.

.. envvar:: MESA_GLSL

   :ref:`shading language compiler options <envvars>`

.. envvar:: MESA_NO_MINMAX_CACHE

   when set, the minmax index cache is globally disabled.

.. envvar:: MESA_SHADER_CAPTURE_PATH

   see :ref:`Capturing Shaders <capture>`

.. envvar:: MESA_SHADER_DUMP_PATH

   see :ref:`Experimenting with Shader Replacements <replacement>`

.. envvar:: MESA_SHADER_READ_PATH

   see :ref:`Experimenting with Shader Replacements <replacement>`

.. envvar:: MESA_VK_VERSION_OVERRIDE

   changes the Vulkan physical device version as returned in
   ``VkPhysicalDeviceProperties::apiVersion``.

   -  The format should be ``MAJOR.MINOR[.PATCH]``
   -  This will not let you force a version higher than the driver's
      instance version as advertised by ``vkEnumerateInstanceVersion``
   -  This can be very useful for debugging but some features may not be
      implemented correctly. (For developers only)

.. envvar:: MESA_VK_WSI_PRESENT_MODE

   overrides the WSI present mode clients specify in
   ``VkSwapchainCreateInfoKHR::presentMode``. Values can be ``fifo``,
   ``relaxed``, ``mailbox`` or ``immediate``.

.. envvar:: MESA_VK_WSI_HEADLESS_SWAPCHAIN

   Forces all swapchains to be headless (no rendering will be display
   in the swapchain's window).

.. envvar:: MESA_VK_ABORT_ON_DEVICE_LOSS

   causes the Vulkan driver to call abort() immediately after detecting a
   lost device.  This is extremely useful when testing as it prevents the
   test suite from continuing on with a lost device.

.. envvar:: MESA_VK_ENABLE_SUBMIT_THREAD

   for Vulkan drivers which support real timeline semaphores, this forces
   them to use a submit thread from the beginning, regardless of whether or
   not they ever see a wait-before-signal condition.

.. envvar:: MESA_VK_DEVICE_SELECT_DEBUG

   print debug info about device selection decision-making

.. envvar:: MESA_VK_TRACE

   A comma-separated list of trace types used for offline analysis. The
   option names are equal to the file extension. Traces are dumped into ``/tmp``.
   Captures can be triggered by pressing ``F1`` with the application window
   focused (Currently X11 only) or via :envvar:`MESA_VK_TRACE_FRAME` and
   :envvar:`MESA_VK_TRACE_TRIGGER`.

   .. list-table::
      :header-rows: 1

      * - File extension
        - Offline analysis tool
        - Supported drivers
      * - ``rmv``
        - Radeon Memory Visualizer
        - ``RADV``
      * - ``rgp``
        - Radeon GPU Profiler
        - ``RADV``
      * - ``rra``
        - Radeon Raytracing Analyzer
        - ``RADV``

   - Creating RMV captures requires the ``scripts/setup.sh`` script in the
     Radeon Developer Tools folder to be run beforehand

.. envvar:: MESA_VK_TRACE_FRAME

   Specifies a frame index at which a trace capture is automatically triggered.

.. envvar:: MESA_VK_TRACE_TRIGGER

   Specifies a trigger file. Creating the file triggers the capture. (e.g.
   ``export MESA_VK_TRACE_TRIGGER=/tmp/trigger`` and then ``touch /tmp/trigger``)

.. envvar:: MESA_LOADER_DRIVER_OVERRIDE

   chooses a different driver binary such as ``etnaviv`` or ``zink``.

.. envvar:: DRI_PRIME_DEBUG

   print debug info about device selection decision-making

.. envvar:: DRI_PRIME

   the default GPU is the one used by Wayland/Xorg or the one connected to a
   display. This variable allows to select a different GPU. It applies to OpenGL
   and Vulkan (in this case "select" means the GPU will be first in the reported
   physical devices list). The supported syntaxes are:

   - ``DRI_PRIME=N``: selects the Nth non-default GPU (N > 0).
   - ``DRI_PRIME=pci-0000_02_00_0``: selects the GPU connected to this PCIe bus
   - ``DRI_PRIME=vendor_id:device_id``: selects the first GPU matching these ids.

   For Vulkan it's possible to append ``!``, in which case only the selected GPU
   will be exposed to the application (eg: DRI_PRIME=1!).

   .. note::

      ``lspci -nn | grep VGA`` can be used to know the PCIe bus or ids to use.

.. envvar:: DRIRC_CONFIGDIR

   If set, overrides the global search-directories used when searching for
   drirc config files. The user-local one will still be used. Mostly useful for
   internal debugging.

NIR passes environment variables
--------------------------------

The following are only applicable for drivers that uses NIR, as they
modify the behavior for the common ``NIR_PASS`` and ``NIR_PASS_V`` macros,
that wrap calls to NIR lowering/optimizations.

.. envvar:: NIR_DEBUG

   a comma-separated list of debug options to apply to NIR
   shaders. Use ``NIR_DEBUG=help`` to print a list of available options.

.. envvar:: NIR_SKIP

   a comma-separated list of optimization/lowering passes to skip.

Mesa Xlib driver environment variables
--------------------------------------

The following are only applicable to the Mesa Xlib software driver. See
the :doc:`Xlib software driver page <xlibdriver>` for details.

.. envvar:: MESA_RGB_VISUAL

   specifies the X visual and depth for RGB mode

.. envvar:: MESA_BACK_BUFFER

   specifies how to implement the back color buffer, either ``pixmap``
   or ``ximage``

.. envvar:: MESA_XSYNC

   enable synchronous X behavior (for debugging only)

.. envvar:: MESA_GLX_FORCE_ALPHA

   if set, forces RGB windows to have an alpha channel.

.. envvar:: MESA_GLX_DEPTH_BITS

   specifies default number of bits for depth buffer.

.. envvar:: MESA_GLX_ALPHA_BITS

   specifies default number of bits for alpha channel.

Mesa WGL driver environment variables
-------------------------------------

The following are only applicable to the Mesa WGL driver, which is in use
on Windows.

.. envvar:: WGL_FORCE_MSAA

   if set to a positive value, specifies the number of MSAA samples to
   force when choosing the display configuration.

.. envvar:: WGL_DISABLE_ERROR_DIALOGS

   if set to 1, true or yes, disables Win32 error dialogs. Useful for
   automated test-runs.

Intel driver environment variables
----------------------------------------------------

.. envvar:: ANV_NO_GPL

   If set to 1, true, or yes, then VK_EXT_graphics_pipeline_library
   will be disabled.

.. envvar:: INTEL_BLACKHOLE_DEFAULT

   if set to 1, true or yes, then the OpenGL implementation will
   default ``GL_BLACKHOLE_RENDER_INTEL`` to true, thus disabling any
   rendering.

.. envvar:: INTEL_COMPUTE_CLASS

   If set to 1, true or yes, then I915_ENGINE_CLASS_COMPUTE will be
   supported. For OpenGL, iris will attempt to use a compute engine
   for compute dispatches if one is detected. For Vulkan, anvil will
   advertise support for a compute queue if a compute engine is
   detected.

.. envvar:: INTEL_COPY_CLASS

   If set to 1, true or yes, then I915_ENGINE_CLASS_COPY will be
   supported. For Vulkan, anvil will advertise support for a transfer
   queue if a copy engine is detected.

.. envvar:: INTEL_DEBUG

   a comma-separated list of named flags, which do various things:

   ``ann``
      annotate IR in assembly dumps
   ``bat``
      emit batch information. Can control in which frames batches
      get dumped using ``INTEL_DEBUG_BATCH_FRAME_*``, where
      ``INTEL_DEBUG_BATCH_FRAME_START`` <= frame < ``INTEL_DEBUG_BATCH_FRAME_STOP``
   ``blit``
      emit messages about blit operations
   ``blorp``
      emit messages about the blorp operations (blits & clears)
   ``buf``
      emit messages about buffer objects
   ``bt``
      emit messages binding tables
   ``capture-all``
      flag all buffers to be captured by the kernel driver when
      generating an error stage after a GPU hang
   ``clip``
      emit messages about the clip unit (for old gens, includes the CLIP
      program)
   ``color``
      use color in output
   ``cs``
      dump shader assembly for compute shaders
   ``do32``
      generate compute shader SIMD32 programs even if workgroup size
      doesn't exceed the SIMD16 limit
   ``draw_bkp``
      Add semaphore wait before/after draw call count.
      ``INTEL_DEBUG_BKP_BEFORE_DRAW_COUNT`` or
      ``INTEL_DEBUG_BKP_AFTER_DRAW_COUNT`` can control draw call number.
      To make test wait forever, we need to set preempt_timeout_ms and
      i915.enable_hangcheck to zero.
   ``fall``
      emit messages about performance issues (same as ``perf``)
   ``fs``
      dump shader assembly for fragment shaders
   ``gs``
      dump shader assembly for geometry shaders
   ``heaps``
      print information about the driver's heaps (Anv only)
   ``hex``
      print instruction hex dump with the disassembly
   ``l3``
      emit messages about the new L3 state during transitions
   ``mesh``
      dump shader assembly for mesh shaders
   ``no8``
      don't generate SIMD8 fragment shader
   ``no16``
      suppress generation of 16-wide fragment shaders. useful for
      debugging broken shaders
   ``no32``
      suppress generation of 32-wide fragment shaders. useful for
      debugging broken shaders
   ``no-oaconfig``
      disable HW performance metric configuration, and anything
      related to i915-perf (useful when running on simulation)
   ``nocompact``
      disable instruction compaction
   ``nodualobj``
      suppress generation of dual-object geometry shader code
   ``nofc``
      disable fast clears
   ``noccs``
      disable lossless color compression
   ``optimizer``
      dump shader assembly to files at each optimization pass and
      iteration that make progress
   ``pc``
      emit messages about PIPE_CONTROL instruction usage
   ``perf``
      emit messages about performance issues
   ``perfmon``
      emit messages about :ext:`GL_AMD_performance_monitor`
   ``perf-symbol-names``
      use performance counter symbols instead of the counter name
      (counter symbols are like variable names, it's sometimes easier
      to work with when you have lots of metrics to collect)
   ``reemit``
      mark all state dirty on each draw call
   ``rt``
      dump shader assembly for ray tracing shaders
   ``sf``
      emit messages about the strips & fans unit (for old gens, includes
      the SF program)
   ``soft64``
      enable implementation of software 64bit floating point support
   ``sparse``
      dump usage of sparse resources
   ``spill_fs``
      force spilling of all registers in the scalar backend (useful to
      debug spilling code)
   ``spill_vec4``
      force spilling of all registers in the vec4 backend (useful to
      debug spilling code)
   ``stall``
      inserts a stall on the GPU after each draw/dispatch command to
      wait for it to finish before starting any new work.
   ``submit``
      emit batchbuffer usage statistics
   ``sync``
      after sending each batch, wait on the CPU for that batch to
      finish rendering
   ``swsb-stall``
      Insert sync NOP after each instruction. This is only valid for Gfx12+.
   ``task``
      dump shader assembly for task shaders
   ``tcs``
      dump shader assembly for tessellation control shaders
   ``tcs8``
      force usage of 8-patches tessellation control shaders (only
      for gfx 9-11)
   ``tes``
      dump shader assembly for tessellation evaluation shaders
   ``tex``
      emit messages about textures.
   ``urb``
      emit messages about URB setup
   ``vs``
      dump shader assembly for vertex shaders
   ``wm``
      dump shader assembly for fragment shaders (same as ``fs``)

.. envvar:: INTEL_DECODE

   a comma-separated list of enable/disable flags configuring the
   output produced by ``INTEL_DEBUG=bat`` (use with
   ``INTEL_DECODE=+color,-floats``) :

   ``color``
      print colored output

   ``floats``
      try to decode floating point data in buffers

   ``full``
      print additional custom information for instructions (usually
      pulling more information by inspecting memory)

   ``offsets``
      print offsets of instructions

.. envvar:: INTEL_EXTENDED_METRICS

   By default, only a standard set of gpu metrics are advertised. This
   reduces time to collect metrics and hides infrequently used metrics.
   To enable all metrics, set value to 1.

.. envvar:: INTEL_MEASURE

   Collects GPU timestamps over common intervals, and generates a CSV report
   to show how long rendering took.  The overhead of collection is limited to
   the flushing that is required at the interval boundaries for accurate
   timestamps. By default, timing data is sent to ``stderr``.  To direct output
   to a file:

   ``INTEL_MEASURE=file=/tmp/measure.csv {workload}``

   To begin capturing timestamps at a particular frame:

   ``INTEL_MEASURE=file=/tmp/measure.csv,start=15 {workload}``

   To capture only 23 frames:

   ``INTEL_MEASURE=count=23 {workload}``

   To capture frames 15-37, stopping before frame 38:

   ``INTEL_MEASURE=start=15,count=23 {workload}``

   Designate an asynchronous control file with:

   ``INTEL_MEASURE=control=path/to/control.fifo {workload}``

   As the workload runs, enable capture for 5 frames with:

   ``$ echo 5 > path/to/control.fifo``

   Enable unbounded capture:

   ``$ echo -1 > path/to/control.fifo``

   and disable with:

   ``$ echo 0 > path/to/control.fifo``

   Select the boundaries of each snapshot with:

   ``INTEL_MEASURE=draw``
      Collects timings for every render (DEFAULT)

   ``INTEL_MEASURE=rt``
      Collects timings when the render target changes

   ``INTEL_MEASURE=batch``
      Collects timings when batches are submitted

   ``INTEL_MEASURE=frame``
      Collects timings at frame boundaries

   With ``INTEL_MEASURE=interval=5``, the duration of 5 events will be
   combined into a single record in the output.  When possible, a single
   start and end event will be submitted to the GPU to minimize
   stalling.  Combined events will not span batches, except in
   the case of ``INTEL_MEASURE=frame``.

   Collect CPU timestamps instead of GPU timestamps.  Prints results
   immediately instead of waiting for GPU execution.  Useful when used
   with interactive debug to know which frame, or where in frame, you
   are currently in.

   ``INTEL_MEASURE=cpu {workload}``

.. envvar:: INTEL_MODIFIER_OVERRIDE

   if set, determines the single DRM modifier reported back to (Vulkan)
   applications, in order to make selecting modifier deterministic
   between Vulkan driver and applications. The value can be one of the
   supported modifiers on a platform, but other values are also acceptable
   for debug purposes.

.. envvar:: INTEL_NO_HW

   if set to 1, true or yes, prevents batches from being submitted to the
   hardware. This is useful for debugging hangs, etc.

.. envvar:: INTEL_PRECISE_TRIG

   if set to 1, true or yes, then the driver prefers accuracy over
   performance in trig functions.

.. envvar:: INTEL_SHADER_OPTIMIZER_PATH

   if set, determines the directory to be used for overriding shader
   assembly. The binaries with custom assembly should be placed in
   this folder and have a name formatted as ``sha1_of_assembly.bin``.
   The SHA-1 of a shader assembly is printed when assembly is dumped via
   corresponding :envvar:`INTEL_DEBUG` flag (e.g. ``vs`` for vertex shader).
   A binary could be generated from a dumped assembly by ``i965_asm``.
   For :envvar:`INTEL_SHADER_ASM_READ_PATH` to work it is necessary to enable
   dumping of corresponding shader stages via :envvar:`INTEL_DEBUG`.
   It is advised to use ``nocompact`` flag of :envvar:`INTEL_DEBUG` when
   dumping and overriding shader assemblies.
   The success of assembly override would be signified by "Successfully
   overrode shader with sha1 <SHA-1>" in stderr replacing the original
   assembly.

.. envvar:: INTEL_SHADER_ASM_READ_PATH

   if set, determines the directory to be used for overriding shader
   assembly. The binaries with custom assembly should be placed in
   this folder and have a name formatted as ``sha1_of_assembly.bin``.
   The SHA-1 of a shader assembly is printed when assembly is dumped via
   corresponding :envvar:`INTEL_DEBUG` flag (e.g. ``vs`` for vertex shader).
   A binary could be generated from a dumped assembly by ``i965_asm``.
   For :envvar:`INTEL_SHADER_ASM_READ_PATH` to work it is necessary to enable
   dumping of corresponding shader stages via :envvar:`INTEL_DEBUG`.
   It is advised to use ``nocompact`` flag of :envvar:`INTEL_DEBUG` when
   dumping and overriding shader assemblies.
   The success of assembly override would be signified by "Successfully
   overrode shader with sha1 <SHA-1>" in stderr replacing the original
   assembly.

.. envvar:: INTEL_SHADER_BIN_DUMP_PATH

   if set, determines the directory to which the compiled shaders will be
   dumped. They will be dumped as ``sha1_of_assembly.bin``, where the sha1
   values will be the same as can be found in the :envvar:`INTEL_DEBUG`
   output, and can be used for :envvar:`INTEL_SHADER_ASM_READ_PATH` input.

   .. note::
      Unlike the text form of shader dumping, :envvar:`INTEL_DEBUG`
      does not affect on the list of shaders to dump. All generated shaders
      are always dumped if :envvar:`INTEL_SHADER_BIN_DUMP_PATH` variable is
      set.

.. envvar:: INTEL_SIMD_DEBUG

   a comma-separated list of named flags, which control simd dispatch widths:

   ``fs8``
      allow generation of SIMD8 fragment shader
   ``fs16``
      allow generation of SIMD16 fragment shader
   ``fs32``
      allow generation of SIMD32 fragment shader
   ``cs8``
      allow generation of SIMD8 compute shader
   ``cs16``
      allow generation of SIMD16 compute shader
   ``cs32``
      allow generation of SIMD32 compute shader
   ``ts8``
      allow generation of SIMD8 task shader
   ``ts16``
      allow generation of SIMD16 task shader
   ``ts32``
      allow generation of SIMD32 task shader
   ``ms8``
      allow generation of SIMD8 mesh shader
   ``ms16``
      allow generation of SIMD16 mesh shader
   ``ms32``
      allow generation of SIMD32 mesh shader
   ``rt8``
      allow generation of SIMD8 ray-tracing shader
   ``rt16``
      allow generation of SIMD16 ray-tracing shader
   ``rt32``
      allow generation of SIMD32 ray-tracing shader

   If none of widths for particular shader stage was specified, then all
   widths are allowed.

DRI environment variables
-------------------------

.. envvar:: DRI_NO_MSAA

   disable MSAA for GLX/EGL MSAA visuals


Vulkan mesa device select layer environment variables
-----------------------------------------------------

.. envvar:: MESA_VK_DEVICE_SELECT

   when set to "list" prints the list of devices.
   when set to "vid:did" number from PCI device. That PCI device is
   selected as default. The default device is returned as the first
   device in vkEnumeratePhysicalDevices API.
   Using "vid:did!" will have the same effect as using the
   ``MESA_VK_DEVICE_SELECT_FORCE_DEFAULT_DEVICE`` variable.

.. envvar:: MESA_VK_DEVICE_SELECT_FORCE_DEFAULT_DEVICE

   when set to 1, the device identified as default will be the only
   one returned in vkEnumeratePhysicalDevices API.


EGL environment variables
-------------------------

Mesa EGL supports different sets of environment variables. See the
:doc:`Mesa EGL <egl>` page for the details.

Gallium environment variables
-----------------------------

.. envvar:: GALLIUM_HUD

   draws various information on the screen, like framerate, CPU load,
   driver statistics, performance counters, etc. Set
   :envvar:`GALLIUM_HUD` to ``help`` and run e.g. ``glxgears`` for more info.

.. envvar:: GALLIUM_HUD_PERIOD

   sets the HUD update rate in seconds (float). Use zero to update every
   frame. The default period is 1/2 second.

.. envvar:: GALLIUM_HUD_VISIBLE

   control default visibility, defaults to true.

.. envvar:: GALLIUM_HUD_OPACITY

   control background opacity as an integer percentage (1-100), defaults to 66%.

.. envvar:: GALLIUM_HUD_TOGGLE_SIGNAL

   toggle visibility via user specified signal. Especially useful to
   toggle HUD at specific points of application and disable for
   unencumbered viewing the rest of the time. For example, set
   :envvar:`GALLIUM_HUD_VISIBLE` to ``false`` and
   :envvar:`GALLIUM_HUD_TOGGLE_SIGNAL` to ``10`` (``SIGUSR1``). Use
   ``kill -10 <pid>`` to toggle the HUD as desired.

.. envvar:: GALLIUM_HUD_SCALE

   Scale HUD by an integer factor, for high DPI displays. Default is 1.

.. envvar:: GALLIUM_HUD_ROTATION

   Rotate the HUD by an integer number of degrees, the specified value must be
   a multiple of 90. Default is 0.

.. envvar:: GALLIUM_HUD_DUMP_DIR

   specifies a directory for writing the displayed HUD values into
   files.

.. envvar:: GALLIUM_DRIVER

   useful in combination with :envvar:`LIBGL_ALWAYS_SOFTWARE` = ``true`` for
   choosing one of the software renderers ``softpipe`` or ``llvmpipe``.

.. envvar:: GALLIUM_LOG_FILE

   specifies a file for logging all errors, warnings, etc. rather than
   stderr.

.. envvar:: GALLIUM_PIPE_SEARCH_DIR

   specifies an alternate search directory for pipe-loader which overrides
   the compile-time path based on the install location.

.. envvar:: GALLIUM_PRINT_OPTIONS

   if non-zero, print all the Gallium environment variables which are
   used, and their current values.

.. envvar:: GALLIUM_TRACE

   If set, this variable will cause the :ref:`trace` output to be written to the
   specified file. Paths may be relative or absolute; relative paths are relative
   to the working directory.  For example, setting it to "trace.xml" will cause
   the trace to be written to a file of the same name in the working directory.

.. envvar:: GALLIUM_TRACE_TC

   If enabled while :ref:`trace` is active, this variable specifies that the threaded context
   should be traced for drivers which implement it. By default, the driver thread is traced,
   which will include any reordering of the command stream from threaded context.

.. envvar:: GALLIUM_TRACE_TRIGGER

   If set while :ref:`trace` is active, this variable specifies a filename to monitor.
   Once the file exists (e.g., from the user running 'touch /path/to/file'), a single
   frame will be recorded into the trace output.
   Paths may be relative or absolute; relative paths are relative to the working directory.

.. envvar:: GALLIUM_DUMP_CPU

   if non-zero, print information about the CPU on start-up

.. envvar:: TGSI_PRINT_SANITY

   if set, do extra sanity checking on TGSI shaders and print any errors
   to stderr.

.. envvar:: DRAW_FSE

   Enable fetch-shade-emit middle-end even though its not correct (e.g.
   for Softpipe)

.. envvar:: DRAW_NO_FSE

   Disable fetch-shade-emit middle-end even when it is correct

.. envvar:: DRAW_USE_LLVM

   if set to zero, the draw module will not use LLVM to execute shaders,
   vertex fetch, etc.

.. envvar:: ST_DEBUG

   controls debug output from the Mesa/Gallium state tracker. Setting to
   ``tgsi``, for example, will print all the TGSI shaders. See
   :file:`src/mesa/state_tracker/st_debug.c` for other options.

.. envvar:: GALLIUM_OVERRIDE_CPU_CAPS

   Override CPU capabilities for LLVMpipe and Softpipe, possible values for x86:
   ``nosse``
   ``sse``
   ``sse2``
   ``sse3``
   ``ssse3``
   ``sse4.1``
   ``avx``

Clover environment variables
----------------------------

.. envvar:: CLOVER_DEVICE_TYPE

   allows to overwrite the device type of devices. Possible values are
   ``accelerator``, ``cpu``, ``custom`` and ``gpu``

.. envvar:: CLOVER_DEVICE_VERSION_OVERRIDE

   overwrites the auto detected OpenCL version of a device. Possible values:
   ``1.0``
   ``1.1``
   ``1.2``
   ``2.0``
   ``2.1``
   ``2.2``
   ``3.0``

.. envvar:: CLOVER_DEVICE_CLC_VERSION_OVERRIDE

   overwrites the auto detected CLC version. Possible values:
   ``1.0``
   ``1.1``
   ``1.2``
   ``2.0``
   ``2.1``
   ``2.2``
   ``3.0``

.. envvar:: CLOVER_EXTRA_BUILD_OPTIONS

   allows specifying additional compiler and linker options. Specified
   options are appended after the options set by the OpenCL program in
   ``clBuildProgram``.

.. envvar:: CLOVER_EXTRA_COMPILE_OPTIONS

   allows specifying additional compiler options. Specified options are
   appended after the options set by the OpenCL program in
   ``clCompileProgram``.

.. envvar:: CLOVER_EXTRA_LINK_OPTIONS

   allows specifying additional linker options. Specified options are
   appended after the options set by the OpenCL program in
   ``clLinkProgram``.

.. _rusticl-env-var:

.. envvar:: IRIS_ENABLE_CLOVER

   allows to enable experimental Clover NIR support with the iris driver if
   set to 1 or true.

Rusticl environment variables
-----------------------------

.. envvar:: RUSTICL_DEVICE_TYPE

   allows to overwrite the device type of devices. Possible values are
   ``accelerator``, ``cpu``, ``custom`` and ``gpu``

.. envvar:: RUSTICL_CL_VERSION

   overwrites the auto detected OpenCL version of all devices. Specified as
   ``major.minor``.

.. envvar:: RUSTICL_ENABLE

   a comma-separated list of drivers to enable CL on. An optional list of
   comma-separated integers can be passed per driver to specify which devices
   to enable. Examples:

   -  ``RUSTICL_ENABLE=iris`` (enables all iris devices)
   -  ``RUSTICL_ENABLE=iris:1,radeonsi:0,2`` (enables second iris and first
      and third radeonsi device)

   Supported drivers (decent support with maybe a few conformance issues or bugs):
   ``iris``,
   ``llvmpipe``,
   ``nouveau``,
   ``panfrost``,
   ``radeonsi``,
   Experimental drivers (unknown level of support, expect conformance issues or major bugs):
   ``r600``

.. envvar:: RUSTICL_FEATURES

   a comma-separated list of features to enable. Those are disabled by default
   as they might not be stable enough or break OpenCL conformance.

   - ``fp16`` enables OpenCL half support
   - ``fp64`` enables OpenCL double support

.. envvar:: RUSTICL_DEBUG

   a comma-separated list of debug channels to enable.

   - ``allow_invalid_spirv`` disables validation of any input SPIR-V
   - ``clc`` dumps all OpenCL C source being compiled
   - ``program`` dumps compilation logs to stderr
   - ``sync`` waits on the GPU to complete after every event

.. _clc-env-var:

clc environment variables
-----------------------------

.. envvar:: CLC_DEBUG

   a comma-separated list of debug channels to enable.

   - ``dump_llvm`` Dumps all generated LLVM IRs
   - ``dump_spirv`` Dumps all compiled, linked and specialized SPIR-Vs
   - ``verbose`` Enable debug logging of clc code

Nine frontend environment variables
-----------------------------------

.. envvar:: D3D_ALWAYS_SOFTWARE

   an integer, which forces Nine to use the CPU instead of GPU acceleration.

.. envvar:: NINE_DEBUG

   a comma-separated list of named flags that do debugging things.
   Use ``NINE_DEBUG=help`` to print a list of available options.

.. envvar:: NINE_FF_DUMP

   a boolean, which dumps shaders generated by a fixed function (FF).

.. envvar:: NINE_SHADER

   a comma-separated list of named flags, which do alternate shader handling.
   Use ``NINE_SHADER=help`` to print a list of available options.

.. envvar:: NINE_QUIRKS

   a comma-separated list of named flags that do various things.
   Use ``NINE_DEBUG=help`` to print a list of available options.

Softpipe driver environment variables
-------------------------------------

.. envvar:: SOFTPIPE_DEBUG

   a comma-separated list of named flags, which do various things:

   ``vs``
      Dump vertex shader assembly to stderr
   ``fs``
      Dump fragment shader assembly to stderr
   ``gs``
      Dump geometry shader assembly to stderr
   ``cs``
      Dump compute shader assembly to stderr
   ``no_rast``
      rasterization is disabled. For profiling purposes.
   ``use_llvm``
      the Softpipe driver will try to use LLVM JIT for vertex
      shading processing.

LLVMpipe driver environment variables
-------------------------------------

.. envvar:: LP_NO_RAST

   if set LLVMpipe will no-op rasterization

.. envvar:: LP_DEBUG

   a comma-separated list of debug options is accepted. See the source
   code for details.

.. envvar:: LP_PERF

   a comma-separated list of options to selectively no-op various parts
   of the driver. See the source code for details.

.. envvar:: LP_NUM_THREADS

   an integer indicating how many threads to use for rendering. Zero
   turns off threading completely. The default value is the number of
   CPU cores present.

VMware SVGA driver environment variables
----------------------------------------

.. envvar:: SVGA_FORCE_SWTNL

   force use of software vertex transformation

.. envvar:: SVGA_NO_SWTNL

   don't allow software vertex transformation fallbacks (will often
   result in incorrect rendering).

.. envvar:: SVGA_DEBUG

   for dumping shaders, constant buffers, etc. See the code for details.

.. envvar:: SVGA_EXTRA_LOGGING

   if set, enables extra logging to the ``vmware.log`` file, such as the
   OpenGL program's name and command line arguments.

.. envvar:: SVGA_NO_LOGGING

   if set, disables logging to the ``vmware.log`` file. This is useful
   when using Valgrind because it otherwise crashes when initializing
   the host log feature.

See the driver code for other, lesser-used variables.

WGL environment variables
-------------------------

.. envvar:: WGL_SWAP_INTERVAL

   to set a swap interval, equivalent to calling
   ``wglSwapIntervalEXT()`` in an application. If this environment
   variable is set, application calls to ``wglSwapIntervalEXT()`` will
   have no effect.

VA-API environment variables
----------------------------

.. envvar:: VAAPI_MPEG4_ENABLED

   enable MPEG4 for VA-API, disabled by default.

VC4 driver environment variables
--------------------------------

.. envvar:: VC4_DEBUG

   a comma-separated list of named flags, which do various things. Use
   ``VC4_DEBUG=help`` to print a list of available options.

V3D/V3DV driver environment variables
-------------------------------------

.. envvar:: V3D_DEBUG

   a comma-separated list of debug options. Use ``V3D_DEBUG=help`` to
   print a list of available options.


.. _radv env-vars:

RADV driver environment variables
---------------------------------

.. envvar:: RADV_DEBUG

   a comma-separated list of named flags, which do various things:

   ``llvm``
      enable LLVM compiler backend
   ``allbos``
      force all allocated buffers to be referenced in submissions
   ``checkir``
      validate the LLVM IR before LLVM compiles the shader
   ``epilogs``
      dump fragment shader epilogs
   ``extra_md``
      add extra information in bo metadatas to help tools (umr)
   ``forcecompress``
      Enables DCC,FMASK,CMASK,HTILE in situations where the driver supports it
      but normally does not deem it beneficial.
   ``hang``
      enable GPU hangs detection and dump a report to
      $HOME/radv_dumps_<pid>_<time> if a GPU hang is detected
   ``img``
      Print image info
   ``info``
      show GPU-related information
   ``invariantgeom``
      Mark geometry-affecting outputs as invariant. This works around a common
      class of application bugs appearing as flickering.
   ``metashaders``
      dump internal meta shaders
   ``noatocdithering``
      disable dithering for alpha to coverage
   ``nobinning``
      disable primitive binning
   ``nocache``
      disable shaders cache
   ``nocompute``
      disable compute queue
   ``nodcc``
      disable Delta Color Compression (DCC) on images
   ``nodisplaydcc``
      disable Delta Color Compression (DCC) on displayable images
   ``nodynamicbounds``
      do not check OOB access for dynamic descriptors
   ``nofastclears``
      disable fast color/depthstencil clears
   ``nofmask``
      disable FMASK compression on MSAA images (GFX6-GFX10.3)
   ``nogpl``
      disable VK_EXT_graphics_pipeline_library
   ``nohiz``
      disable HIZ for depthstencil images
   ``noibs``
      disable directly recording command buffers in GPU-visible memory
   ``nomemorycache``
      disable memory shaders cache
   ``nomeshshader``
      disable mesh shader support on GFX10.3+
   ``nongg``
      disable NGG for GFX10 and GFX10.3
   ``nonggc``
      disable NGG culling on GPUs where it's enabled by default (GFX10.3+ only).
   ``nort``
      skip executing vkCmdTraceRays and ray queries (RT extensions will still be
      advertised)
   ``notccompatcmask``
      disable TC-compat CMASK for MSAA surfaces
   ``noumr``
      disable UMR dumps during GPU hang detection (only with
      :envvar:`RADV_DEBUG` = ``hang``)
   ``novrsflatshading``
      disable VRS for flat shading (only on GFX10.3+)
   ``preoptir``
      dump LLVM IR before any optimizations
   ``prologs``
      dump vertex shader prologs
   ``shaders``
      dump shaders
   ``shaderstats``
      dump shader statistics
   ``shadowregs``
      enable register shadowing
   ``spirv``
      dump SPIR-V
   ``splitfma``
      split application-provided fused multiply-add in geometry stages
   ``startup``
      display info at startup
   ``syncshaders``
      synchronize shaders after all draws/dispatches
   ``zerovram``
      initialize all memory allocated in VRAM as zero

.. envvar:: RADV_FORCE_FAMILY

   create a null device to compile shaders without a AMD GPU (e.g. VEGA10)

.. envvar:: RADV_FORCE_VRS

   allow to force per-pipeline vertex VRS rates on GFX10.3+. This is only
   forced for pipelines that don't explicitly use VRS or flat shading.
   The supported values are 2x2, 1x2, 2x1 and 1x1. Only for testing purposes.

.. envvar:: RADV_FORCE_VRS_CONFIG_FILE

   similar to ``RADV_FORCE_VRS`` but allow to configure from a file. If present,
   this supersedes ``RADV_FORCE_VRS``.

.. envvar:: RADV_PERFTEST

   a comma-separated list of named flags, which do various things:

   ``bolist``
      enable the global BO list
   ``cswave32``
      enable wave32 for compute shaders (GFX10+)
   ``dccmsaa``
      enable DCC for MSAA images
   ``dmashaders``
      upload shaders to invisible VRAM (might be useful for non-resizable BAR systems)
   ``emulate_rt``
      forces ray-tracing to be emulated in software on GFX10_3+ and enables
      rt extensions with older hardware.
   ``gewave32``
      enable wave32 for vertex/tess/geometry shaders (GFX10+)
   ``localbos``
      enable local BOs
   ``nosam``
      disable optimizations that get enabled when all VRAM is CPU visible.
   ``pswave32``
      enable wave32 for pixel shaders (GFX10+)
   ``nggc``
      enable NGG culling on GPUs where it's not enabled by default (GFX10.1 only).
   ``sam``
      enable optimizations to move more driver internal objects to VRAM.
   ``rtwave64``
      enable wave64 for ray tracing shaders (GFX10+)
   ``transfer_queue``
      enable experimental transfer queue support (GFX9+, not yet spec compliant)
   ``video_decode``
      enable experimental video decoding support
   ``gsfastlaunch2``
      use GS_FAST_LAUNCH=2 for Mesh shaders (GFX11+ dGPUs only)

.. envvar:: RADV_TEX_ANISO

   force anisotropy filter (up to 16)

.. envvar:: RADV_THREAD_TRACE_BUFFER_SIZE

   set the SQTT/RGP buffer size in bytes (default value is 32MiB, the buffer is
   automatically resized if too small)

.. envvar:: RADV_THREAD_TRACE_CACHE_COUNTERS

   enable/disable SQTT/RGP cache counters on GFX10+ (enabled by default)

.. envvar:: RADV_THREAD_TRACE_INSTRUCTION_TIMING

   enable/disable SQTT/RGP instruction timing (enabled by default)

.. envvar:: RADV_THREAD_TRACE_QUEUE_EVENTS

   enable/disable SQTT/RGP queue events (enabled by default)

.. envvar:: RADV_RRA_TRACE_VALIDATE

   enable validation of captured acceleration structures. Can be
   useful if RRA crashes upon opening a trace.

.. envvar:: ACO_DEBUG

   a comma-separated list of named flags, which do various things:

   ``validateir``
      validate the ACO IR at various points of compilation (enabled by
      default for debug/debugoptimized builds)
   ``novalidateir``
      disable ACO IR validation in debug/debugoptimized builds
   ``validatera``
      validate register assignment of ACO IR and catches many RA bugs
   ``perfwarn``
      abort on some suboptimal code generation
   ``force-waitcnt``
      force emitting waitcnt states if there is something to wait for
   ``force-waitdeps``
     force emitting waitcnt dependencies for debugging hazards on GFX10+
   ``novn``
      disable value numbering
   ``noopt``
      disable various optimizations
   ``nosched``
      disable instructions scheduling
   ``perfinfo``
      print information used to calculate some pipeline statistics
   ``liveinfo``
      print liveness and register demand information before scheduling

RadeonSI driver environment variables
-------------------------------------

.. envvar:: radeonsi_no_infinite_interp

   Kill PS with infinite interp coeff (might fix hangs)

.. envvar:: radeonsi_clamp_div_by_zero

   Clamp div by zero (x / 0 becomes FLT_MAX instead of NaN) (might fix rendering corruptions)

.. envvar:: radeonsi_zerovram

   Clear all allocated memory to 0 before usage (might fix rendering corruptions)

.. envvar:: AMD_DEBUG

   a comma-separated list of named flags, which do various things:

   ``nodcc``
      Disable DCC.
   ``nodccclear``
      Disable DCC fast clear
   ``nodisplaydcc``
      disable Delta Color Compression (DCC) on displayable images
   ``nodccmsaa``
      Disable DCC for MSAA
   ``nodpbb``
      Disable DPBB. Overrules the dpbb enable option.
   ``noefc``
      Disable hardware based encoder colour format conversion
   ``notiling``
      Disable tiling
   ``nofmask``
      Disable MSAA compression
   ``nohyperz``
      Disable Hyper-Z
   ``no2d``
      Disable 2D tiling
   ``info``
      Print driver information
   ``tex``
      Print texture info
   ``compute``
      Print compute info
   ``vm``
      Print virtual addresses when creating resources
   ``vs``
      Print vertex shaders
   ``ps``
      Print pixel shaders
   ``gs``
      Print geometry shaders
   ``tcs``
      Print tessellation control shaders
   ``tes``
      Print tessellation evaluation shaders
   ``cs``
      Print compute shaders
   ``noir``
      Don't print the LLVM IR
   ``nonir``
      Don't print NIR when printing shaders
   ``noasm``
      Don't print disassembled shaders
   ``preoptir``
      Print the LLVM IR before initial optimizations
   ``w32ge``
      Use Wave32 for vertex, tessellation, and geometry shaders.
   ``w32ps``
      Use Wave32 for pixel shaders.
   ``w32cs``
      Use Wave32 for computes shaders.
   ``w64ge``
      Use Wave64 for vertex, tessellation, and geometry shaders.
   ``w64ps``
      Use Wave64 for pixel shaders.
   ``w64cs``
      Use Wave64 for computes shaders.
   ``checkir``
      Enable additional sanity checks on shader IR
   ``mono``
      Use old-style monolithic shaders compiled on demand
   ``nooptvariant``
      Disable compiling optimized shader variants.
   ``useaco``
      Use ACO as shader compiler when possible
   ``nowc``
      Disable GTT write combining
   ``check_vm``
      Check VM faults and dump debug info.
   ``reserve_vmid``
      Force VMID reservation per context.
   ``nongg``
      Disable NGG and use the legacy pipeline.
   ``nggc``
      Always use NGG culling even when it can hurt.
   ``nonggc``
      Disable NGG culling.
   ``switch_on_eop``
      Program WD/IA to switch on end-of-packet.
   ``nooutoforder``
      Disable out-of-order rasterization
   ``dpbb``
      Enable DPBB. Enable DPBB for gfx9 dGPU. Default enabled for gfx9 APU and >= gfx10.
   ``extra_md``
      add extra information in bo metadatas to help tools (umr)

r600 driver environment variables
---------------------------------

.. envvar:: R600_DEBUG

   a comma-separated list of named flags, which do various things:

   ``nocpdma``
      Disable CP DMA
   ``nosb``
      Disable sb backend for graphics shaders
   ``sbcl``
      Enable sb backend for compute shaders
   ``sbdry``
      Don't use optimized bytecode (just print the dumps)
   ``sbstat``
      Print optimization statistics for shaders
   ``sbdump``
      Print IR dumps after some optimization passes
   ``sbnofallback``
      Abort on errors instead of fallback
   ``sbdisasm``
      Use sb disassembler for shader dumps
   ``sbsafemath``
      Disable unsafe math optimizations
   ``nirsb``
      Enable NIR with SB optimizer
   ``tex``
      Print texture info
   ``nir``
      Enable experimental NIR shaders
   ``compute``
      Print compute info
   ``vm``
      Print virtual addresses when creating resources
   ``info``
      Print driver information
   ``fs``
      Print fetch shaders
   ``vs``
      Print vertex shaders
   ``gs``
      Print geometry shaders
   ``ps``
      Print pixel shaders
   ``cs``
      Print compute shaders
   ``tcs``
      Print tessellation control shaders
   ``tes``
      Print tessellation evaluation shaders
   ``noir``
      Don't print the LLVM IR
   ``notgsi``
      Don't print the TGSI
   ``noasm``
      Don't print disassembled shaders
   ``preoptir``
      Print the LLVM IR before initial optimizations
   ``checkir``
      Enable additional sanity checks on shader IR
   ``nooptvariant``
      Disable compiling optimized shader variants.
   ``testdma``
      Invoke SDMA tests and exit.
   ``testvmfaultcp``
      Invoke a CP VM fault test and exit.
   ``testvmfaultsdma``
      Invoke a SDMA VM fault test and exit.
   ``testvmfaultshader``
      Invoke a shader VM fault test and exit.
   ``nodma``
      Disable asynchronous DMA
   ``nohyperz``
      Disable Hyper-Z
   ``noinvalrange``
      Disable handling of INVALIDATE_RANGE map flags
   ``no2d``
      Disable 2D tiling
   ``notiling``
      Disable tiling
   ``switch_on_eop``
      Program WD/IA to switch on end-of-packet.
   ``forcedma``
      Use asynchronous DMA for all operations when possible.
   ``precompile``
      Compile one shader variant at shader creation.
   ``nowc``
      Disable GTT write combining
   ``check_vm``
      Check VM faults and dump debug info.
   ``unsafemath``
      Enable unsafe math shader optimizations

.. envvar:: R600_DEBUG_COMPUTE

   if set to ``true``, various compute-related debug information will
   be printed to stderr. Defaults to ``false``.

.. envvar:: R600_DUMP_SHADERS

   if set to ``true``, NIR shaders will be printed to stderr. Defaults
   to ``false``.

.. envvar:: R600_HYPERZ

   If set to ``false``, disables HyperZ optimizations. Defaults to ``true``.

.. envvar:: R600_NIR_DEBUG

   a comma-separated list of named flags, which do various things:

   ``instr``
      Log all consumed nir instructions
   ``ir``
      Log created R600 IR
   ``cc``
      Log R600 IR to assembly code creation
   ``noerr``
      Don't log shader conversion errors
   ``si``
      Log shader info (non-zero values)
   ``reg``
      Log register allocation and lookup
   ``io``
      Log shader in and output
   ``ass``
      Log IR to assembly conversion
   ``flow``
      Log control flow instructions
   ``merge``
      Log register merge operations
   ``nomerge``
      Skip register merge step
   ``tex``
      Log texture ops
   ``trans``
      Log generic translation messages

r300 driver environment variables
---------------------------------

.. envvar:: RADEON_DEBUG

   a comma-separated list of named flags, which do various things:

   ``info``
      Print hardware info (printed by default on debug builds
   ``fp``
      Log fragment program compilation
   ``vp``
      Log vertex program compilation
   ``draw``
      Log draw calls
   ``swtcl``
      Log SWTCL-specific info
   ``rsblock``
      Log rasterizer registers
   ``psc``
      Log vertex stream registers
   ``tex``
      Log basic info about textures
   ``texalloc``
      Log texture mipmap tree info
   ``rs``
      Log rasterizer
   ``fb``
      Log framebuffer
   ``cbzb``
      Log fast color clear info
   ``hyperz``
      Log HyperZ info
   ``scissor``
      Log scissor info
   ``msaa``
      Log MSAA resources
   ``anisohq``
      Use high quality anisotropic filtering
   ``notiling``
      Disable tiling
   ``noimmd``
      Disable immediate mode
   ``noopt``
      Disable shader optimizations
   ``nocbzb``
      Disable fast color clear
   ``nozmask``
      Disable zbuffer compression
   ``nohiz``
      Disable hierarchical zbuffer
   ``nocmask``
      Disable AA compression and fast AA clear
   ``notcl``
      Disable hardware accelerated Transform/Clip/Lighting

Asahi driver environment variables
----------------------------------

.. envvar:: ASAHI_MESA_DEBUG

   a comma-separated list of named flags, which do various things:

   ``trace``
      Trace work submitted to the GPU to files, using the agxdecode
      infrastructure. This produces a large volume of data, so should be used
      with caution. The traces are written to ``agxdecode.dump``,
      but this can be overridden using ``AGXDECODE_DUMP_FILE``.
   ``no16``
      Disable 16-bit floating point support. This may workaround application
      bugs in certain OpenGL ES applications originally written for desktops. If
      such applications are found in the wild, they should be fixed upstream (if
      possible) or added in the Mesa-wide driconf (if closed source).
   ``dirty``
      In debug builds only: disable dirty tracking optimizations.
   ``nowc``
      Disable write-combining (force all allocations to be write-through). This
      may be useful for diagnosing certain performance issues. Note imported
      buffers may still be write-combined.

.. envvar:: AGX_MESA_DEBUG

   a comma-separated list of named flags, which do various things:

   ``shaders``
      Print shaders being compiled at various stages in the pipeline.
   ``shaderdb``
      Print statistics about compiled shaders.
   ``verbose``
      Disassemble in verbose mode, including additional information that may be
      useful for debugging.
   ``internal``
      Include even internal shaders (as produced for clears, blits, and such)
      when printing shaders. Without this flag, internal shaders are ignored by
      the shaders and shaderdb flags.
   ``novalidate``
      In debug builds only: skip internal intermediate representation validation.
   ``noopt``
      Disable various backend optimizations.

.. _imagination env-vars:

PowerVR driver environment variables
------------------------------------------------

.. envvar:: PVR_DEBUG

   A comma-separated list of debug options. Use `PVR_DEBUG=help` to
   print a list of available options.

.. envvar:: ROGUE_DEBUG

   a comma-separated list of named flags for the Rogue compiler,
   which do various things:

   ``nir``
      Print the input NIR to stdout.
   ``nir_passes``
      Print the output of each NIR pass to stdout.
   ``ir``
      Print the input Rogue IR to stdout.
   ``ir_passes``
      Print the output of each Rogue IR pass to stdout.
   ``ir_details``
      Includes additional details when printing Rogue IR.
   ``vld_skip``
      Skips the compiler validation step.
   ``vld_nonfatal``
      Prints all the validation errors instead of stopping after the first.

.. envvar:: ROGUE_COLOR

   if set to ``auto`` Rogue IR will be colorized if stdout is not a pipe.
   Color is forced off if set to ``off``/``0`` or on if set to ``on``/``1``.
   Defaults to ``auto``.

i915 driver environment variables
---------------------------------

.. envvar:: I915_DEBUG

   Debug flags for the i915 driver.

.. envvar:: I915_NO_HW

   Stop the i915 driver from submitting commands to the hardware.

.. envvar:: I915_DUMP_CMD

   Dump all commands going to the hardware.

Freedreno driver environment variables
--------------------------------------

.. envvar:: FD_MESA_DEBUG

   Debug flags for the Freedreno driver.

----

Other Gallium drivers have their own environment variables. These may
change frequently so the source code should be consulted for details.
