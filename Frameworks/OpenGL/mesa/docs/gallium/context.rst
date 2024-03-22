.. _context:

Context
=======

A Gallium rendering context encapsulates the state which effects 3D
rendering such as blend state, depth/stencil state, texture samplers,
etc.

Note that resource/texture allocation is not per-context but per-screen.


Methods
-------

CSO State
^^^^^^^^^

All Constant State Object (CSO) state is created, bound, and destroyed,
with triplets of methods that all follow a specific naming scheme.
For example, ``create_blend_state``, ``bind_blend_state``, and
``destroy_blend_state``.

CSO objects handled by the context object:

* :ref:`Blend`: ``*_blend_state``
* :ref:`Sampler`: Texture sampler states are bound separately for fragment,
  vertex, geometry and compute shaders with the ``bind_sampler_states``
  function.  The ``start`` and ``num_samplers`` parameters indicate a range
  of samplers to change.  NOTE: at this time, start is always zero and
  the CSO module will always replace all samplers at once (no sub-ranges).
  This may change in the future.
* :ref:`Rasterizer`: ``*_rasterizer_state``
* :ref:`depth-stencil-alpha`: ``*_depth_stencil_alpha_state``
* :ref:`Shader`: These are create, bind and destroy methods for vertex,
  fragment and geometry shaders.
* :ref:`vertexelements`: ``*_vertex_elements_state``


Resource Binding State
^^^^^^^^^^^^^^^^^^^^^^

This state describes how resources in various flavors (textures,
buffers, surfaces) are bound to the driver.


* ``set_constant_buffer`` sets a constant buffer to be used for a given shader
  type. index is used to indicate which buffer to set (some APIs may allow
  multiple ones to be set, and binding a specific one later, though drivers
  are mostly restricted to the first one right now).
  If take_ownership is true, the buffer reference is passed to the driver, so
  that the driver doesn't have to increment the reference count.

* ``set_inlinable_constants`` sets inlinable constants for constant buffer 0.

These are constants that the driver would like to inline in the IR
of the current shader and recompile it. Drivers can determine which
constants they prefer to inline in finalize_nir and store that
information in shader_info::*inlinable_uniform*. When the state tracker
or frontend uploads constants to a constant buffer, it can pass
inlinable constants separately via this call.

Any ``set_constant_buffer`` call invalidates inlinable constants, so
``set_inlinable_constants`` must be called after it. Binding a shader also
invalidates this state.

There is no ``PIPE_CAP`` for this. Drivers shouldn't set the shader_info
fields if they don't implement ``set_inlinable_constants``.

* ``set_framebuffer_state``

* ``set_vertex_buffers``


Non-CSO State
^^^^^^^^^^^^^

These pieces of state are too small, variable, and/or trivial to have CSO
objects. They all follow simple, one-method binding calls, e.g.
``set_blend_color``.

* ``set_stencil_ref`` sets the stencil front and back reference values
  which are used as comparison values in stencil test.
* ``set_blend_color``
* ``set_sample_mask``  sets the per-context multisample sample mask.  Note
  that this takes effect even if multisampling is not explicitly enabled if
  the framebuffer surface(s) are multisampled.  Also, this mask is AND-ed
  with the optional fragment shader sample mask output (when emitted).
* ``set_sample_locations`` sets the sample locations used for rasterization.
  ```get_sample_position``` still returns the default locations. When NULL,
  the default locations are used.
* ``set_min_samples`` sets the minimum number of samples that must be run.
* ``set_clip_state``
* ``set_polygon_stipple``
* ``set_scissor_states`` sets the bounds for the scissor test, which culls
  pixels before blending to render targets. If the :ref:`Rasterizer` does
  not have the scissor test enabled, then the scissor bounds never need to
  be set since they will not be used.  Note that scissor xmin and ymin are
  inclusive, but  xmax and ymax are exclusive.  The inclusive ranges in x
  and y would be [xmin..xmax-1] and [ymin..ymax-1]. The number of scissors
  should be the same as the number of set viewports and can be up to
  PIPE_MAX_VIEWPORTS.
* ``set_viewport_states``
* ``set_window_rectangles`` sets the window rectangles to be used for
  rendering, as defined by :ext:`GL_EXT_window_rectangles`. There are two
  modes - include and exclude, which define whether the supplied
  rectangles are to be used for including fragments or excluding
  them. All of the rectangles are ORed together, so in exclude mode,
  any fragment inside any rectangle would be culled, while in include
  mode, any fragment outside all rectangles would be culled. xmin/ymin
  are inclusive, while xmax/ymax are exclusive (same as scissor states
  above). Note that this only applies to draws, not clears or
  blits. (Blits have their own way to pass the requisite rectangles
  in.)
* ``set_tess_state`` configures the default tessellation parameters:

  * ``default_outer_level`` is the default value for the outer tessellation
    levels. This corresponds to GL's ``PATCH_DEFAULT_OUTER_LEVEL``.
  * ``default_inner_level`` is the default value for the inner tessellation
    levels. This corresponds to GL's ``PATCH_DEFAULT_INNER_LEVEL``.
* ``set_patch_vertices`` sets the number of vertices per input patch
  for tessellation.

* ``set_debug_callback`` sets the callback to be used for reporting
  various debug messages, eventually reported via :ext:`GL_KHR_debug` and
  similar mechanisms.

Samplers
^^^^^^^^

pipe_sampler_state objects control how textures are sampled (coordinate wrap
modes, interpolation modes, etc). Samplers are only required for texture
instructions for which nir_tex_instr_need_sampler returns true. Drivers must
ignore samplers for other texture instructions. Frontends may or may not bind
samplers when no texture instruction use them. Notably, frontends may not bind
samplers for texture buffer objects, which are never accessed with samplers.

Sampler Views
^^^^^^^^^^^^^

These are the means to bind textures to shader stages. To create one, specify
its format, swizzle and LOD range in sampler view template.

If texture format is different than template format, it is said the texture
is being cast to another format. Casting can be done only between compatible
formats, that is formats that have matching component order and sizes.

Swizzle fields specify the way in which fetched texel components are placed
in the result register. For example, ``swizzle_r`` specifies what is going to be
placed in first component of result register.

The ``first_level`` and ``last_level`` fields of sampler view template specify
the LOD range the texture is going to be constrained to. Note that these
values are in addition to the respective min_lod, max_lod values in the
pipe_sampler_state (that is if min_lod is 2.0, and first_level 3, the first mip
level used for sampling from the resource is effectively the fifth).

The ``first_layer`` and ``last_layer`` fields specify the layer range the
texture is going to be constrained to. Similar to the LOD range, this is added
to the array index which is used for sampling.

* ``set_sampler_views`` binds an array of sampler views to a shader stage.
  Every binding point acquires a reference
  to a respective sampler view and releases a reference to the previous
  sampler view.

  Sampler views outside of ``[start_slot, start_slot + num_views)`` are
  unmodified.  If ``views`` is NULL, the behavior is the same as if
  ``views[n]`` was NULL for the entire range, i.e. releasing the reference
  for all the sampler views in the specified range.

* ``create_sampler_view`` creates a new sampler view. ``texture`` is associated
  with the sampler view which results in sampler view holding a reference
  to the texture. Format specified in template must be compatible
  with texture format.

* ``sampler_view_destroy`` destroys a sampler view and releases its reference
  to associated texture.

Hardware Atomic buffers
^^^^^^^^^^^^^^^^^^^^^^^

Buffers containing HW atomics are required to support the feature
on some drivers.

Drivers that require this need to fill the ``set_hw_atomic_buffers`` method.

Shader Resources
^^^^^^^^^^^^^^^^

Shader resources are textures or buffers that may be read or written
from a shader without an associated sampler.  This means that they
have no support for floating point coordinates, address wrap modes or
filtering.

There are 2 types of shader resources: buffers and images.

Buffers are specified using the ``set_shader_buffers`` method.

Images are specified using the ``set_shader_images`` method. When binding
images, the ``level``, ``first_layer`` and ``last_layer`` pipe_image_view
fields specify the mipmap level and the range of layers the image will be
constrained to.

Surfaces
^^^^^^^^

These are the means to use resources as color render targets or depthstencil
attachments. To create one, specify the mip level, the range of layers, and
the bind flags (either PIPE_BIND_DEPTH_STENCIL or PIPE_BIND_RENDER_TARGET).
Note that layer values are in addition to what is indicated by the geometry
shader output variable XXX_FIXME (that is if first_layer is 3 and geometry
shader indicates index 2, the 5th layer of the resource will be used). These
first_layer and last_layer parameters will only be used for 1d array, 2d array,
cube, and 3d textures otherwise they are 0.

* ``create_surface`` creates a new surface.

* ``surface_destroy`` destroys a surface and releases its reference to the
  associated resource.

Stream output targets
^^^^^^^^^^^^^^^^^^^^^

Stream output, also known as transform feedback, allows writing the primitives
produced by the vertex pipeline to buffers. This is done after the geometry
shader or vertex shader if no geometry shader is present.

The stream output targets are views into buffer resources which can be bound
as stream outputs and specify a memory range where it's valid to write
primitives. The pipe driver must implement memory protection such that any
primitives written outside of the specified memory range are discarded.

Two stream output targets can use the same resource at the same time, but
with a disjoint memory range.

Additionally, the stream output target internally maintains the offset
into the buffer which is incremented every time something is written to it.
The internal offset is equal to how much data has already been written.
It can be stored in device memory and the CPU actually doesn't have to query
it.

The stream output target can be used in a draw command to provide
the vertex count. The vertex count is derived from the internal offset
discussed above.

* ``create_stream_output_target`` create a new target.

* ``stream_output_target_destroy`` destroys a target. Users of this should
  use pipe_so_target_reference instead.

* ``set_stream_output_targets`` binds stream output targets. The parameter
  offset is an array which specifies the internal offset of the buffer. The
  internal offset is, besides writing, used for reading the data during the
  draw_auto stage, i.e. it specifies how much data there is in the buffer
  for the purposes of the draw_auto stage. -1 means the buffer should
  be appended to, and everything else sets the internal offset.

* ``stream_output_target_offset`` Retrieve the internal stream offset from
  an streamout target. This is used to implement Vulkan pause/resume support
  which needs to pass the internal offset to the API.

NOTE: The currently-bound vertex or geometry shader must be compiled with
the properly-filled-in structure pipe_stream_output_info describing which
outputs should be written to buffers and how. The structure is part of
pipe_shader_state.

Clearing
^^^^^^^^

Clear is one of the most difficult concepts to nail down to a single
interface (due to both different requirements from APIs and also driver/HW
specific differences).

``clear`` initializes some or all of the surfaces currently bound to
the framebuffer to particular RGBA, depth, or stencil values.
Currently, this does not take into account color or stencil write masks (as
used by GL), and always clears the whole surfaces (no scissoring as used by
GL clear or explicit rectangles like d3d9 uses). It can, however, also clear
only depth or stencil in a combined depth/stencil surface.
If a surface includes several layers then all layers will be cleared.

``clear_render_target`` clears a single color rendertarget with the specified
color value. While it is only possible to clear one surface at a time (which can
include several layers), this surface need not be bound to the framebuffer.
If render_condition_enabled is false, any current rendering condition is ignored
and the clear will be unconditional.

``clear_depth_stencil`` clears a single depth, stencil or depth/stencil surface
with the specified depth and stencil values (for combined depth/stencil buffers,
it is also possible to only clear one or the other part). While it is only
possible to clear one surface at a time (which can include several layers),
this surface need not be bound to the framebuffer.
If render_condition_enabled is false, any current rendering condition is ignored
and the clear will be unconditional.

``clear_texture`` clears a non-PIPE_BUFFER resource's specified level
and bounding box with a clear value provided in that resource's native
format.

``clear_buffer`` clears a PIPE_BUFFER resource with the specified clear value
(which may be multiple bytes in length). Logically this is a memset with a
multi-byte element value starting at offset bytes from resource start, going
for size bytes. It is guaranteed that size % clear_value_size == 0.

Evaluating Depth Buffers
^^^^^^^^^^^^^^^^^^^^^^^^

``evaluate_depth_buffer`` is a hint to decompress the current depth buffer
assuming the current sample locations to avoid problems that could arise when
using programmable sample locations.

If a depth buffer is rendered with different sample location state than
what is current at the time of reading the depth buffer, the values may differ
because depth buffer compression can depend the sample locations.


Uploading
^^^^^^^^^

For simple single-use uploads, use ``pipe_context::stream_uploader`` or
``pipe_context::const_uploader``. The latter should be used for uploading
constants, while the former should be used for uploading everything else.
PIPE_USAGE_STREAM is implied in both cases, so don't use the uploaders
for static allocations.

Usage:

Call u_upload_alloc or u_upload_data as many times as you want. After you are
done, call u_upload_unmap. If the driver doesn't support persistent mappings,
u_upload_unmap makes sure the previously mapped memory is unmapped.

Gotchas:
- Always fill the memory immediately after u_upload_alloc. Any following call
to u_upload_alloc and u_upload_data can unmap memory returned by previous
u_upload_alloc.
- Don't interleave calls using stream_uploader and const_uploader. If you use
one of them, do the upload, unmap, and only then can you use the other one.


Drawing
^^^^^^^

``draw_vbo`` draws a specified primitive.  The primitive mode and other
properties are described by ``pipe_draw_info``.

The ``mode``, ``start``, and ``count`` fields of ``pipe_draw_info`` specify the
the mode of the primitive and the vertices to be fetched, in the range between
``start`` to ``start``+``count``-1, inclusive.

Every instance with instanceID in the range between ``start_instance`` and
``start_instance``+``instance_count``-1, inclusive, will be drawn.

If  ``index_size`` != 0, all vertex indices will be looked up from the index
buffer.

In indexed draw, ``min_index`` and ``max_index`` respectively provide a lower
and upper bound of the indices contained in the index buffer inside the range
between ``start`` to ``start``+``count``-1.  This allows the driver to
determine which subset of vertices will be referenced during the draw call
without having to scan the index buffer.  Providing a over-estimation of the
the true bounds, for example, a ``min_index`` and ``max_index`` of 0 and
0xffffffff respectively, must give exactly the same rendering, albeit with less
performance due to unreferenced vertex buffers being unnecessarily DMA'ed or
processed.  Providing a underestimation of the true bounds will result in
undefined behavior, but should not result in program or system failure.

In case of non-indexed draw, ``min_index`` should be set to
``start`` and ``max_index`` should be set to ``start``+``count``-1.

``index_bias`` is a value added to every vertex index after lookup and before
fetching vertex attributes.

When drawing indexed primitives, the primitive restart index can be
used to draw disjoint primitive strips.  For example, several separate
line strips can be drawn by designating a special index value as the
restart index.  The ``primitive_restart`` flag enables/disables this
feature.  The ``restart_index`` field specifies the restart index value.

When primitive restart is in use, array indexes are compared to the
restart index before adding the index_bias offset.

If a given vertex element has ``instance_divisor`` set to 0, it is said
it contains per-vertex data and effective vertex attribute address needs
to be recalculated for every index.

  attribAddr = ``stride`` * index + ``src_offset``

If a given vertex element has ``instance_divisor`` set to non-zero,
it is said it contains per-instance data and effective vertex attribute
address needs to recalculated for every ``instance_divisor``-th instance.

  attribAddr = ``stride`` * instanceID / ``instance_divisor`` + ``src_offset``

In the above formulas, ``src_offset`` is taken from the given vertex element
and ``stride`` is taken from a vertex buffer associated with the given
vertex element.

The calculated attribAddr is used as an offset into the vertex buffer to
fetch the attribute data.

The value of ``instanceID`` can be read in a vertex shader through a system
value register declared with INSTANCEID semantic name.


Queries
^^^^^^^

Queries gather some statistic from the 3D pipeline over one or more
draws.  Queries may be nested, though not all gallium frontends exercise this.

Queries can be created with ``create_query`` and deleted with
``destroy_query``. To start a query, use ``begin_query``, and when finished,
use ``end_query`` to end the query.

``create_query`` takes a query type (``PIPE_QUERY_*``), as well as an index,
which is the vertex stream for ``PIPE_QUERY_PRIMITIVES_GENERATED`` and
``PIPE_QUERY_PRIMITIVES_EMITTED``, and allocates a query structure.

``begin_query`` will clear/reset previous query results.

``get_query_result`` is used to retrieve the results of a query.  If
the ``wait`` parameter is TRUE, then the ``get_query_result`` call
will block until the results of the query are ready (and TRUE will be
returned).  Otherwise, if the ``wait`` parameter is FALSE, the call
will not block and the return value will be TRUE if the query has
completed or FALSE otherwise.

``get_query_result_resource`` is used to store the result of a query into
a resource without synchronizing with the CPU. This write will optionally
wait for the query to complete, and will optionally write whether the value
is available instead of the value itself.

``set_active_query_state`` Set whether all current non-driver queries except
TIME_ELAPSED are active or paused.

The interface currently includes the following types of queries:

``PIPE_QUERY_OCCLUSION_COUNTER`` counts the number of fragments which
are written to the framebuffer without being culled by
:ref:`depth-stencil-alpha` testing or shader KILL instructions.
The result is an unsigned 64-bit integer.
This query can be used with ``render_condition``.

In cases where a boolean result of an occlusion query is enough,
``PIPE_QUERY_OCCLUSION_PREDICATE`` should be used. It is just like
``PIPE_QUERY_OCCLUSION_COUNTER`` except that the result is a boolean
value of FALSE for cases where COUNTER would result in 0 and TRUE
for all other cases.
This query can be used with ``render_condition``.

In cases where a conservative approximation of an occlusion query is enough,
``PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE`` should be used. It behaves
like ``PIPE_QUERY_OCCLUSION_PREDICATE``, except that it may return TRUE in
additional, implementation-dependent cases.
This query can be used with ``render_condition``.

``PIPE_QUERY_TIME_ELAPSED`` returns the amount of time, in nanoseconds,
the context takes to perform operations.
The result is an unsigned 64-bit integer.

``PIPE_QUERY_TIMESTAMP`` returns a device/driver internal timestamp,
scaled to nanoseconds, recorded after all commands issued prior to
``end_query`` have been processed.
This query does not require a call to ``begin_query``.
The result is an unsigned 64-bit integer.

``PIPE_QUERY_TIMESTAMP_DISJOINT`` can be used to check the
internal timer resolution and whether the timestamp counter has become
unreliable due to things like throttling etc. - only if this is FALSE
a timestamp query (within the timestamp_disjoint query) should be trusted.
The result is a 64-bit integer specifying the timer resolution in Hz,
followed by a boolean value indicating whether the timestamp counter
is discontinuous or disjoint.

``PIPE_QUERY_PRIMITIVES_GENERATED`` returns a 64-bit integer indicating
the number of primitives processed by the pipeline (regardless of whether
stream output is active or not).

``PIPE_QUERY_PRIMITIVES_EMITTED`` returns a 64-bit integer indicating
the number of primitives written to stream output buffers.

``PIPE_QUERY_SO_STATISTICS`` returns 2 64-bit integers corresponding to
the result of
``PIPE_QUERY_PRIMITIVES_EMITTED`` and
the number of primitives that would have been written to stream output buffers
if they had infinite space available (primitives_storage_needed), in this order.
XXX the 2nd value is equivalent to ``PIPE_QUERY_PRIMITIVES_GENERATED`` but it is
unclear if it should be increased if stream output is not active.

``PIPE_QUERY_SO_OVERFLOW_PREDICATE`` returns a boolean value indicating
whether a selected stream output target has overflowed as a result of the
commands issued between ``begin_query`` and ``end_query``.
This query can be used with ``render_condition``. The output stream is
selected by the stream number passed to ``create_query``.

``PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE`` returns a boolean value indicating
whether any stream output target has overflowed as a result of the commands
issued between ``begin_query`` and ``end_query``. This query can be used
with ``render_condition``, and its result is the logical OR of multiple
``PIPE_QUERY_SO_OVERFLOW_PREDICATE`` queries, one for each stream output
target.

``PIPE_QUERY_GPU_FINISHED`` returns a boolean value indicating whether
all commands issued before ``end_query`` have completed. However, this
does not imply serialization.
This query does not require a call to ``begin_query``.

``PIPE_QUERY_PIPELINE_STATISTICS`` returns an array of the following
64-bit integers:
Number of vertices read from vertex buffers.
Number of primitives read from vertex buffers.
Number of vertex shader threads launched.
Number of geometry shader threads launched.
Number of primitives generated by geometry shaders.
Number of primitives forwarded to the rasterizer.
Number of primitives rasterized.
Number of fragment shader threads launched.
Number of tessellation control shader threads launched.
Number of tessellation evaluation shader threads launched.
If a shader type is not supported by the device/driver,
the corresponding values should be set to 0.

``PIPE_QUERY_PIPELINE_STATISTICS_SINGLE`` returns a single counter from
the ``PIPE_QUERY_PIPELINE_STATISTICS`` group.  The specific counter must
be selected when calling ``create_query`` by passing one of the
``PIPE_STAT_QUERY`` enums as the query's ``index``.

Gallium does not guarantee the availability of any query types; one must
always check the capabilities of the :ref:`Screen` first.


Conditional Rendering
^^^^^^^^^^^^^^^^^^^^^

A drawing command can be skipped depending on the outcome of a query
(typically an occlusion query, or streamout overflow predicate).
The ``render_condition`` function specifies the query which should be checked
prior to rendering anything. Functions always honoring render_condition include
(and are limited to) draw_vbo and clear.
The blit, clear_render_target and clear_depth_stencil functions (but
not resource_copy_region, which seems inconsistent) can also optionally honor
the current render condition.

If ``render_condition`` is called with ``query`` = NULL, conditional
rendering is disabled and drawing takes place normally.

If ``render_condition`` is called with a non-null ``query`` subsequent
drawing commands will be predicated on the outcome of the query.
Commands will be skipped if ``condition`` is equal to the predicate result
(for non-boolean queries such as OCCLUSION_QUERY, zero counts as FALSE,
non-zero as TRUE).

If ``mode`` is PIPE_RENDER_COND_WAIT the driver will wait for the
query to complete before deciding whether to render.

If ``mode`` is PIPE_RENDER_COND_NO_WAIT and the query has not yet
completed, the drawing command will be executed normally.  If the query
has completed, drawing will be predicated on the outcome of the query.

If ``mode`` is PIPE_RENDER_COND_BY_REGION_WAIT or
PIPE_RENDER_COND_BY_REGION_NO_WAIT rendering will be predicated as above
for the non-REGION modes but in the case that an occlusion query returns
a non-zero result, regions which were occluded may be omitted by subsequent
drawing commands.  This can result in better performance with some GPUs.
Normally, if the occlusion query returned a non-zero result subsequent
drawing happens normally so fragments may be generated, shaded and
processed even where they're known to be obscured.

The ''render_condition_mem'' function specifies the drawing is dependent
on a value in memory. A buffer resource and offset denote which 32-bit
value to use for the query. This is used for Vulkan API.

Flushing
^^^^^^^^

``flush``

PIPE_FLUSH_END_OF_FRAME: Whether the flush marks the end of frame.

PIPE_FLUSH_DEFERRED: It is not required to flush right away, but it is required
to return a valid fence. If fence_finish is called with the returned fence
and the context is still unflushed, and the ctx parameter of fence_finish is
equal to the context where the fence was created, fence_finish will flush
the context.

PIPE_FLUSH_ASYNC: The flush is allowed to be asynchronous. Unlike
``PIPE_FLUSH_DEFERRED``, the driver must still ensure that the returned fence
will finish in finite time. However, subsequent operations in other contexts of
the same screen are no longer guaranteed to happen after the flush. Drivers
which use this flag must implement pipe_context::fence_server_sync.

PIPE_FLUSH_HINT_FINISH: Hints to the driver that the caller will immediately
wait for the returned fence.

Additional flags may be set together with ``PIPE_FLUSH_DEFERRED`` for even
finer-grained fences. Note that as a general rule, GPU caches may not have been
flushed yet when these fences are signaled. Drivers are free to ignore these
flags and create normal fences instead. At most one of the following flags can
be specified:

PIPE_FLUSH_TOP_OF_PIPE: The fence should be signaled as soon as the next
command is ready to start executing at the top of the pipeline, before any of
its data is actually read (including indirect draw parameters).

PIPE_FLUSH_BOTTOM_OF_PIPE: The fence should be signaled as soon as the previous
command has finished executing on the GPU entirely (but data written by the
command may still be in caches and inaccessible to the CPU).


``flush_resource``

Flush the resource cache, so that the resource can be used
by an external client. Possible usage:
- flushing a resource before presenting it on the screen
- flushing a resource if some other process or device wants to use it
This shouldn't be used to flush caches if the resource is only managed
by a single pipe_screen and is not shared with another process.
(i.e. you shouldn't use it to flush caches explicitly if you want to e.g.
use the resource for texturing)

Fences
^^^^^^

``pipe_fence_handle``, and related methods, are used to synchronize
execution between multiple parties. Examples include CPU <-> GPU synchronization,
renderer <-> windowing system, multiple external APIs, etc.

A ``pipe_fence_handle`` can either be 'one time use' or 're-usable'. A 'one time use'
fence behaves like a traditional GPU fence. Once it reaches the signaled state it
is forever considered to be signaled.

Once a re-usable ``pipe_fence_handle`` becomes signaled, it can be reset
back into an unsignaled state. The ``pipe_fence_handle`` will be reset to
the unsignaled state by performing a wait operation on said object, i.e.
``fence_server_sync``. As a corollary to this behavior, a re-usable
``pipe_fence_handle`` can only have one waiter.

This behavior is useful in producer <-> consumer chains. It helps avoid
unnecessarily sharing a new ``pipe_fence_handle`` each time a new frame is
ready. Instead, the fences are exchanged once ahead of time, and access is synchronized
through GPU signaling instead of direct producer <-> consumer communication.

``fence_server_sync`` inserts a wait command into the GPU's command stream.

``fence_server_signal`` inserts a signal command into the GPU's command stream.

There are no guarantees that the wait/signal commands will be flushed when
calling ``fence_server_sync`` or ``fence_server_signal``. An explicit
call to ``flush`` is required to make sure the commands are emitted to the GPU.

The Gallium implementation may implicitly ``flush`` the command stream during a
``fence_server_sync`` or ``fence_server_signal`` call if necessary.

Resource Busy Queries
^^^^^^^^^^^^^^^^^^^^^

``is_resource_referenced``



Blitting
^^^^^^^^

These methods emulate classic blitter controls.

These methods operate directly on ``pipe_resource`` objects, and stand
apart from any 3D state in the context. Each method is assumed to have an
implicit memory barrier around itself. They do not need any explicit
``memory_barrier``. Blitting functionality may be moved to a separate
abstraction at some point in the future.

``resource_copy_region`` blits a region of a resource to a region of another
resource, provided that both resources have the same format, or compatible
formats, i.e., formats for which copying the bytes from the source resource
unmodified to the destination resource will achieve the same effect of a
textured quad blitter.. The source and destination may be the same resource,
but overlapping blits are not permitted.
This can be considered the equivalent of a CPU memcpy.

``blit`` blits a region of a resource to a region of another resource, including
scaling, format conversion, and up-/downsampling, as well as a destination clip
rectangle (scissors) and window rectangles. It can also optionally honor the
current render condition (but either way the blit itself never contributes
anything to queries currently gathering data).
As opposed to manually drawing a textured quad, this lets the pipe driver choose
the optimal method for blitting (like using a special 2D engine), and usually
offers, for example, accelerated stencil-only copies even where
PIPE_CAP_SHADER_STENCIL_EXPORT is not available.


Transfers
^^^^^^^^^

These methods are used to get data to/from a resource.

``transfer_map`` creates a memory mapping and the transfer object
associated with it.
The returned pointer points to the start of the mapped range according to
the box region, not the beginning of the resource. If transfer_map fails,
the returned pointer to the buffer memory is NULL, and the pointer
to the transfer object remains unchanged (i.e. it can be non-NULL).

When mapping an MSAA surface, the samples are implicitly resolved to
single-sampled for reads (returning the first sample for depth/stencil/integer,
averaged for others).  See u_transfer_helper's U_TRANSFER_HELPER_MSAA_MAP for a
way to get that behavior using a resolve blit.

``transfer_unmap`` remove the memory mapping for and destroy
the transfer object. The pointer into the resource should be considered
invalid and discarded.

``texture_subdata`` and ``buffer_subdata`` perform a simplified
transfer for simple writes. Basically transfer_map, data write, and
transfer_unmap all in one.


The box parameter to some of these functions defines a 1D, 2D or 3D
region of pixels.  This is self-explanatory for 1D, 2D and 3D texture
targets.

For PIPE_TEXTURE_1D_ARRAY and PIPE_TEXTURE_2D_ARRAY, the box::z and box::depth
fields refer to the array dimension of the texture.

For PIPE_TEXTURE_CUBE, the box:z and box::depth fields refer to the
faces of the cube map (z + depth <= 6).

For PIPE_TEXTURE_CUBE_ARRAY, the box:z and box::depth fields refer to both
the face and array dimension of the texture (face = z % 6, array = z / 6).


.. _transfer_flush_region:

transfer_flush_region
%%%%%%%%%%%%%%%%%%%%%

If a transfer was created with ``FLUSH_EXPLICIT``, it will not automatically
be flushed on write or unmap. Flushes must be requested with
``transfer_flush_region``. Flush ranges are relative to the mapped range, not
the beginning of the resource.



.. _texture_barrier:

texture_barrier
%%%%%%%%%%%%%%%

This function flushes all pending writes to the currently-set surfaces and
invalidates all read caches of the currently-set samplers. This can be used
for both regular textures as well as for framebuffers read via FBFETCH.



.. _memory_barrier:

memory_barrier
%%%%%%%%%%%%%%%

This function flushes caches according to which of the PIPE_BARRIER_* flags
are set.



.. _resource_commit:

resource_commit
%%%%%%%%%%%%%%%

This function changes the commit state of a part of a sparse resource. Sparse
resources are created by setting the ``PIPE_RESOURCE_FLAG_SPARSE`` flag when
calling ``resource_create``. Initially, sparse resources only reserve a virtual
memory region that is not backed by memory (i.e., it is uncommitted). The
``resource_commit`` function can be called to commit or uncommit parts (or all)
of a resource. The driver manages the underlying backing memory.

The contents of newly committed memory regions are undefined. Calling this
function to commit an already committed memory region is allowed and leaves its
content unchanged. Similarly, calling this function to uncommit an already
uncommitted memory region is allowed.

For buffers, the given box must be aligned to multiples of
``PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE``. As an exception to this rule, if the size
of the buffer is not a multiple of the page size, changing the commit state of
the last (partial) page requires a box that ends at the end of the buffer
(i.e., box->x + box->width == buffer->width0).



.. _pipe_transfer:

PIPE_MAP
^^^^^^^^^^^^^

These flags control the behavior of a transfer object.

``PIPE_MAP_READ``
  Resource contents read back (or accessed directly) at transfer create time.

``PIPE_MAP_WRITE``
  Resource contents will be written back at transfer_unmap time (or modified
  as a result of being accessed directly).

``PIPE_MAP_DIRECTLY``
  a transfer should directly map the resource. May return NULL if not supported.

``PIPE_MAP_DISCARD_RANGE``
  The memory within the mapped region is discarded.  Cannot be used with
  ``PIPE_MAP_READ``.

``PIPE_MAP_DISCARD_WHOLE_RESOURCE``
  Discards all memory backing the resource.  It should not be used with
  ``PIPE_MAP_READ``.

``PIPE_MAP_DONTBLOCK``
  Fail if the resource cannot be mapped immediately.

``PIPE_MAP_UNSYNCHRONIZED``
  Do not synchronize pending operations on the resource when mapping. The
  interaction of any writes to the map and any operations pending on the
  resource are undefined. Cannot be used with ``PIPE_MAP_READ``.

``PIPE_MAP_FLUSH_EXPLICIT``
  Written ranges will be notified later with :ref:`transfer_flush_region`.
  Cannot be used with ``PIPE_MAP_READ``.

``PIPE_MAP_PERSISTENT``
  Allows the resource to be used for rendering while mapped.
  PIPE_RESOURCE_FLAG_MAP_PERSISTENT must be set when creating
  the resource.
  If COHERENT is not set, memory_barrier(PIPE_BARRIER_MAPPED_BUFFER)
  must be called to ensure the device can see what the CPU has written.

``PIPE_MAP_COHERENT``
  If PERSISTENT is set, this ensures any writes done by the device are
  immediately visible to the CPU and vice versa.
  PIPE_RESOURCE_FLAG_MAP_COHERENT must be set when creating
  the resource.

Compute kernel execution
^^^^^^^^^^^^^^^^^^^^^^^^

A compute program can be defined, bound or destroyed using
``create_compute_state``, ``bind_compute_state`` or
``destroy_compute_state`` respectively.

Any of the subroutines contained within the compute program can be
executed on the device using the ``launch_grid`` method.  This method
will execute as many instances of the program as elements in the
specified N-dimensional grid, hopefully in parallel.

The compute program has access to four special resources:

* ``GLOBAL`` represents a memory space shared among all the threads
  running on the device.  An arbitrary buffer created with the
  ``PIPE_BIND_GLOBAL`` flag can be mapped into it using the
  ``set_global_binding`` method.

* ``LOCAL`` represents a memory space shared among all the threads
  running in the same working group.  The initial contents of this
  resource are undefined.

* ``PRIVATE`` represents a memory space local to a single thread.
  The initial contents of this resource are undefined.

* ``INPUT`` represents a read-only memory space that can be
  initialized at ``launch_grid`` time.

These resources use a byte-based addressing scheme, and they can be
accessed from the compute program by means of the LOAD/STORE TGSI
opcodes.  Additional resources to be accessed using the same opcodes
may be specified by the user with the ``set_compute_resources``
method.

In addition, normal texture sampling is allowed from the compute
program: ``bind_sampler_states`` may be used to set up texture
samplers for the compute stage and ``set_sampler_views`` may
be used to bind a number of sampler views to it.

Compute kernel queries
^^^^^^^^^^^^^^^^^^^^^^

.. _get_compute_state_info:

get_compute_state_info
%%%%%%%%%%%%%%%%%%%%%%

This function allows frontends to query kernel information defined inside
``pipe_compute_state_object_info``.

.. _get_compute_state_subgroup_size:

get_compute_state_subgroup_size
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This function returns the choosen subgroup size when `launch_grid` is
called with the given block size. This doesn't need to be implemented when
only one size is reported through ``PIPE_COMPUTE_CAP_SUBGROUP_SIZES`` or
``pipe_compute_state_object_info::simd_sizes``.

Mipmap generation
^^^^^^^^^^^^^^^^^

If PIPE_CAP_GENERATE_MIPMAP is true, ``generate_mipmap`` can be used
to generate mipmaps for the specified texture resource.
It replaces texel image levels base_level+1 through
last_level for layers range from first_layer through last_layer.
It returns TRUE if mipmap generation succeeds, otherwise it
returns FALSE. Mipmap generation may fail when it is not supported
for particular texture types or formats.

Device resets
^^^^^^^^^^^^^

Gallium frontends can query or request notifications of when the GPU
is reset for whatever reason (application error, driver error). When
a GPU reset happens, the context becomes unusable and all related state
should be considered lost and undefined. Despite that, context
notifications are single-shot, i.e. subsequent calls to
``get_device_reset_status`` will return PIPE_NO_RESET.

* ``get_device_reset_status`` queries whether a device reset has happened
  since the last call or since the last notification by callback.
* ``set_device_reset_callback`` sets a callback which will be called when
  a device reset is detected. The callback is only called synchronously.

Bindless
^^^^^^^^

If PIPE_CAP_BINDLESS_TEXTURE is TRUE, the following ``pipe_context`` functions
are used to create/delete bindless handles, and to make them resident in the
current context when they are going to be used by shaders.

* ``create_texture_handle`` creates a 64-bit unsigned integer texture handle
  that is going to be directly used in shaders.
* ``delete_texture_handle`` deletes a 64-bit unsigned integer texture handle.
* ``make_texture_handle_resident`` makes a 64-bit unsigned texture handle
  resident in the current context to be accessible by shaders for texture
  mapping.
* ``create_image_handle`` creates a 64-bit unsigned integer image handle that
  is going to be directly used in shaders.
* ``delete_image_handle`` deletes a 64-bit unsigned integer image handle.
* ``make_image_handle_resident`` makes a 64-bit unsigned integer image handle
  resident in the current context to be accessible by shaders for image loads,
  stores and atomic operations.

Using several contexts
----------------------

Several contexts from the same screen can be used at the same time. Objects
created on one context cannot be used in another context, but the objects
created by the screen methods can be used by all contexts.

Transfers
^^^^^^^^^
A transfer on one context is not expected to synchronize properly with
rendering on other contexts, thus only areas not yet used for rendering should
be locked.

A flush is required after transfer_unmap to expect other contexts to see the
uploaded data, unless:

* Using persistent mapping. Associated with coherent mapping, unmapping the
  resource is also not required to use it in other contexts. Without coherent
  mapping, memory_barrier(PIPE_BARRIER_MAPPED_BUFFER) should be called on the
  context that has mapped the resource. No flush is required.

* Mapping the resource with PIPE_MAP_DIRECTLY.
