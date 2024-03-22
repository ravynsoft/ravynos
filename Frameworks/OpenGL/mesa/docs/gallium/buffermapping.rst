Buffer mapping patterns
-----------------------

There are two main strategies the driver has for CPU access to GL buffer
objects. One is that the GL calls allocate temporary storage and blit to the GPU
at
``glBufferSubData()``/``glBufferData()``/``glFlushMappedBufferRange()``/``glUnmapBuffer()``
time. This makes the behavior easily match. However, this may be more costly
than direct mapping of the GL BO on some platforms, and is essentially not
available to tiling GPUs (since tiling involves running through the command
stream multiple times). Thus, GL has additional interfaces to help make it so
apps can directly access memory while avoiding implicit blocking on the GPU
rendering from those BOs.

Rendering engines have a variety of knobs to set on those GL interfaces for data
upload, and as a whole they seem to take just about every path available. Let's
look at some examples to see how they might constrain GL driver buffer upload
behavior.

Portal 2
========

.. code-block:: console

  1030842 glXSwapBuffers(dpy = 0x82a8000, drawable = 20971540)
  1030876 glBufferDataARB(target = GL_ELEMENT_ARRAY_BUFFER, size = 65536, data = NULL, usage = GL_DYNAMIC_DRAW)
  1030877 glBufferSubData(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, size = 576, data = blob(576))
  1030896 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 526, count = 252, type = GL_UNSIGNED_SHORT, indices = NULL, basevertex = 0)
  1030915 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 19657, count = 36, type = GL_UNSIGNED_SHORT, indices = 0x1f8, basevertex = 0)
  1030917 glBufferDataARB(target = GL_ARRAY_BUFFER, size = 1572864, data = NULL, usage = GL_DYNAMIC_DRAW)
  1030918 glBufferSubData(target = GL_ARRAY_BUFFER, offset = 0, size = 128, data = blob(128))
  1030919 glBufferSubData(target = GL_ELEMENT_ARRAY_BUFFER, offset = 576, size = 12, data = blob(12))
  1030936 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 3, count = 6, type = GL_UNSIGNED_SHORT, indices = 0x240, basevertex = 0)
  1030937 glBufferSubData(target = GL_ARRAY_BUFFER, offset = 128, size = 128, data = blob(128))
  1030938 glBufferSubData(target = GL_ELEMENT_ARRAY_BUFFER, offset = 588, size = 12, data = blob(12))
  1030940 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 4, end = 7, count = 6, type = GL_UNSIGNED_SHORT, indices = 0x24c, basevertex = 0)
  [... repeated draws at increasing offsets]
  1033097 glXSwapBuffers(dpy = 0x82a8000, drawable = 20971540)

From this sequence, we can see that it is important that the driver either
implement ``glBufferSubData()`` as a blit from a streaming uploader in sequence with
the ``glDraw*()`` calls (a common behavior for non-tiled GPUs, particularly those with
dedicated memory), or that you:

1) Track the valid range of the buffer so that you don't have to flush the draws
   and synchronize on each following ``glBufferSubData()``.

2) Reallocate the buffer storage on ``glBufferData`` so that your first
   ``glBufferSubData()`` of the frame doesn't stall on the last frame's
   rendering completing.

You can't just empty your valid range on ``glBufferData()`` unless you know that
the GPU access from the previous frame has completed. This pattern of
incrementing ``glBufferSubData()`` offsets interleaved with draws from that data
is common among newer Valve games.

.. code-block:: console

  [ during setup ]

  679259 glGenBuffersARB(n = 1, buffers = &1314)
  679260 glBindBufferARB(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 1314)
  679261 glBufferDataARB(target = GL_ELEMENT_ARRAY_BUFFER, size = 3072, data = NULL, usage = GL_STATIC_DRAW)
  679264 glMapBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 3072, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT) = 0xd7384000
  679269 glFlushMappedBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 3072)
  679270 glUnmapBuffer(target = GL_ELEMENT_ARRAY_BUFFER) = GL_TRUE
  
  [... setup of other buffers on this binding point]

  679343 glBindBufferARB(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 1314)
  679344 glMapBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 768, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT) = 0xd7384000
  679346 glFlushMappedBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 768)
  679347 glUnmapBuffer(target = GL_ELEMENT_ARRAY_BUFFER) = GL_TRUE
  679348 glMapBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 768, length = 768, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT) = 0xd7384300
  679350 glFlushMappedBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 768)
  679351 glUnmapBuffer(target = GL_ELEMENT_ARRAY_BUFFER) = GL_TRUE
  679352 glMapBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 1536, length = 768, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT) = 0xd7384600
  679354 glFlushMappedBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 768)
  679355 glUnmapBuffer(target = GL_ELEMENT_ARRAY_BUFFER) = GL_TRUE
  679356 glMapBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 2304, length = 768, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT) = 0xd7384900
  679358 glFlushMappedBufferRange(target = GL_ELEMENT_ARRAY_BUFFER, offset = 0, length = 768)
  679359 glUnmapBuffer(target = GL_ELEMENT_ARRAY_BUFFER) = GL_TRUE
  
  [... setup completes and we start drawing later]

  761845 glBindBufferARB(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 1314)
  761846 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 323, count = 384, type = GL_UNSIGNED_SHORT, indices = NULL, basevertex = 0)

This suggests that, for non-blitting drivers, resetting your "might be used on
the GPU" range after a stall could save you a bunch of additional GPU stalls
during setup.

Terraria
========

.. code-block:: console

  167581 glXSwapBuffers(dpy = 0x3004630, drawable = 25165844)

  167585 glBufferData(target = GL_ARRAY_BUFFER, size = 196608, data = NULL, usage = GL_STREAM_DRAW)
  167586 glBufferSubData(target = GL_ARRAY_BUFFER, offset = 0, size = 1728, data = blob(1728))
  167588 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 71, count = 108, type = GL_UNSIGNED_SHORT, indices = NULL, basevertex = 0)
  167589 glBufferData(target = GL_ARRAY_BUFFER, size = 196608, data = NULL, usage = GL_STREAM_DRAW)
  167590 glBufferSubData(target = GL_ARRAY_BUFFER, offset = 0, size = 27456, data = blob(27456))
  167592 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 7, count = 12, type = GL_UNSIGNED_SHORT, indices = NULL, basevertex = 0)
  167594 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 3, count = 6, type = GL_UNSIGNED_SHORT, indices = NULL, basevertex = 8)
  167596 glDrawRangeElementsBaseVertex(mode = GL_TRIANGLES, start = 0, end = 3, count = 6, type = GL_UNSIGNED_SHORT, indices = NULL, basevertex = 12)
  [...]

In this game, we can see ``glBufferData()`` being used on the same array buffer
throughout, to get new storage so that the ``glBufferSubData()`` doesn't cause
synchronization.

Don't Starve
============

.. code-block:: console

  7251917 glGenBuffers(n = 1, buffers = &115052)
  7251918 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 115052)
  7251919 glBufferData(target = GL_ARRAY_BUFFER, size = 144, data = blob(144), usage = GL_STREAM_DRAW)
  7251921 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 115052)
  7251928 glDrawArrays(mode = GL_TRIANGLES, first = 0, count = 6)
  7251930 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 114872)
  7251936 glDrawArrays(mode = GL_TRIANGLES, first = 0, count = 18)
  7251938 glGenBuffers(n = 1, buffers = &115053)
  7251939 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 115053)
  7251940 glBufferData(target = GL_ARRAY_BUFFER, size = 144, data = blob(144), usage = GL_STREAM_DRAW)
  7251942 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 115053)
  7251949 glDrawArrays(mode = GL_TRIANGLES, first = 0, count = 6)
  7251973 glXSwapBuffers(dpy = 0x86dd860, drawable = 20971540)
  [... drawing next frame]
  7252388 glDeleteBuffers(n = 1, buffers = &115052)
  7252389 glDeleteBuffers(n = 1, buffers = &115053)
  7252390 glXSwapBuffers(dpy = 0x86dd860, drawable = 20971540)

In this game we have a lot of tiny ``glBufferData()`` calls, suggesting that we
could see working set wins and possibly CPU overhead reduction by packing small
GL buffers in the same BO. Interestingly, the deletes of the temporary buffers
always happen at the end of the next frame.

Euro Truck Simulator
====================

.. code-block:: console

  [usage of VBO 14,15]
  [...]
  885199 glXSwapBuffers(dpy = 0x379a3e0, drawable = 20971527)
  885203 glInvalidateBufferData(buffer = 14)
  885204 glInvalidateBufferData(buffer = 15)
  [...]
  889330 glXSwapBuffers(dpy = 0x379a3e0, drawable = 20971527)
  889334 glInvalidateBufferData(buffer = 12)
  889335 glInvalidateBufferData(buffer = 16)
  [...]
  893461 glXSwapBuffers(dpy = 0x379a3e0, drawable = 20971527)
  893462 glClientWaitSync(sync = 0x77eee10, flags = 0x0, timeout = 0) = GL_ALREADY_SIGNALED
  893463 glDeleteSync(sync = 0x780a630)
  893464 glFenceSync(condition = GL_SYNC_GPU_COMMANDS_COMPLETE, flags = 0) = 0x78ec730
  893465 glInvalidateBufferData(buffer = 13)
  893466 glInvalidateBufferData(buffer = 17)
  893505 glBindBuffer(target = GL_COPY_READ_BUFFER, buffer = 14)
  893506 glMapBufferRange(target = GL_COPY_READ_BUFFER, offset = 0, length = 788, access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b034efd1000
  893508 glUnmapBuffer(target = GL_COPY_READ_BUFFER) = GL_TRUE
  893509 glBindBuffer(target = GL_COPY_READ_BUFFER, buffer = 15)
  893510 glMapBufferRange(target = GL_COPY_READ_BUFFER, offset = 0, length = 32, access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b034e5df000
  893512 glUnmapBuffer(target = GL_COPY_READ_BUFFER) = GL_TRUE
  893532 glBindVertexBuffers(first = 0, count = 2, buffers = {10, 15}, offsets = {0, 0}, strides = {52, 16})
  893552 glDrawElementsInstancedBaseVertex(mode = GL_TRIANGLES, count = 18, type = GL_UNSIGNED_SHORT, indices = 0x13f280, instancecount = 1, basevertex = 25131)
  893609 glDrawArrays(mode = GL_TRIANGLES, first = 0, count = 6)
  893732 glBindVertexBuffers(first = 0, count = 1, buffers = &14, offsets = &0, strides = &48)
  893733 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 14)
  893744 glDrawElementsBaseVertex(mode = GL_TRIANGLES, count = 6, type = GL_UNSIGNED_SHORT, indices = 0xf0, basevertex = 0)
  893759 glDrawElementsBaseVertex(mode = GL_TRIANGLES, count = 24, type = GL_UNSIGNED_SHORT, indices = 0x2e0, basevertex = 6)
  893786 glDrawElementsBaseVertex(mode = GL_TRIANGLES, count = 600, type = GL_UNSIGNED_SHORT, indices = 0xe87b0, basevertex = 21515)
  893822 glDrawArrays(mode = GL_TRIANGLES, first = 0, count = 6)
  893845 glBindBuffer(target = GL_COPY_READ_BUFFER, buffer = 14)
  893846 glMapBufferRange(target = GL_COPY_READ_BUFFER, offset = 788, length = 788, access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b034efd1314
  893848 glUnmapBuffer(target = GL_COPY_READ_BUFFER) = GL_TRUE
  893886 glDrawElementsInstancedBaseVertex(mode = GL_TRIANGLES, count = 18, type = GL_UNSIGNED_SHORT, indices = 0x13f280, instancecount = 1, basevertex = 25131)
  893943 glDrawArrays(mode = GL_TRIANGLES, first = 0, count = 6)

At the start of this frame, buffer 14 and 15 haven't been used in the previous 2
frames, and the :ext:`GL_ARB_sync` fence has ensured that the GPU has at least started
frame n-1 as the CPU starts the current frame. The first map is ``offset = 0,
INVALIDATE_BUFFER | UNSYNCHRONIZED``, which suggests that the driver should
reallocate storage for the mapping even in the ``UNSYNCHRONIZED`` case, except
that the buffer is definitely going to be idle, making reallocation unnecessary
(you may need to empty your valid range, though, to prevent unnecessary batch
flushes).

Also note the use of a totally unrelated binding point for the mapping of the
vertex array -- you can't effectively use it as a hint for any buffer placement
in memory. The game does also use ``glCopyBufferSubData()``, but only on a
different buffer.


Plague Inc
==========

.. code-block:: console

  1640732 glXSwapBuffers(dpy = 0xb218f20, drawable = 23068674)
  1640733 glClientWaitSync(sync = 0xb4141430, flags = 0x0, timeout = 0) = GL_ALREADY_SIGNALED
  1640734 glDeleteSync(sync = 0xb4141430)
  1640735 glFenceSync(condition = GL_SYNC_GPU_COMMANDS_COMPLETE, flags = 0) = 0xb4141430
  
  1640780 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 78)
  1640787 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 79)
  1640788 glDrawElements(mode = GL_TRIANGLES, count = 9636, type = GL_UNSIGNED_SHORT, indices = NULL)
  1640795 glDrawElements(mode = GL_TRIANGLES, count = 9636, type = GL_UNSIGNED_SHORT, indices = NULL)
  1640813 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1096)
  1640814 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 67584, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0xbfef4000
  1640815 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1091)
  1640816 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 12, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0xc3998000
  1640817 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1096)
  1640819 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 352)
  1640820 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1640821 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1091)
  1640823 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 12)
  1640824 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1640825 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 1096)
  1640831 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 1091)
  1640832 glDrawElements(mode = GL_TRIANGLES, count = 6, type = GL_UNSIGNED_SHORT, indices = NULL)
  
  1640847 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1096)
  1640848 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 352, length = 67584, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0xbfef4160
  1640849 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1091)
  1640850 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 88, length = 12, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0xc3998058
  1640851 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1096)
  1640853 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 352)
  1640854 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1640855 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 1091)
  1640857 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 12)
  1640858 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1640863 glDrawElementsBaseVertex(mode = GL_TRIANGLES, count = 6, type = GL_UNSIGNED_SHORT, indices = 0x58, basevertex = 4)

At the start of this frame, the VBOs haven't been used in about 6 frames, and
the :ext:`GL_ARB_sync` fence has ensured that the GPU has started frame n-1.

Note the use of ``glFlushMappedBufferRange()`` on a small fraction of the size
of the VBO -- it is important that a blitting driver make use of the flush
ranges when in explicit mode.

Darkest Dungeon
===============

.. code-block:: console

  938384 glXSwapBuffers(dpy = 0x377fcd0, drawable = 23068692)
  
  938385 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 2)
  938386 glBufferData(target = GL_ARRAY_BUFFER, size = 1048576, data = NULL, usage = GL_STREAM_DRAW)
  938511 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 2)
  938512 glMapBufferRange(target = GL_ARRAY_BUFFER, offset = 0, length = 1048576, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7a73fcaa7000
  938514 glFlushMappedBufferRange(target = GL_ARRAY_BUFFER, offset = 0, length = 512)
  938515 glUnmapBuffer(target = GL_ARRAY_BUFFER) = GL_TRUE
  938523 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 1)
  938524 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 2)
  938525 glDrawElements(mode = GL_TRIANGLES, count = 24, type = GL_UNSIGNED_SHORT, indices = NULL)
  938527 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 2)
  938528 glMapBufferRange(target = GL_ARRAY_BUFFER, offset = 0, length = 1048576, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7a73fcaa7000
  938530 glFlushMappedBufferRange(target = GL_ARRAY_BUFFER, offset = 512, length = 512)
  938531 glUnmapBuffer(target = GL_ARRAY_BUFFER) = GL_TRUE
  938539 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 1)
  938540 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 2)
  938541 glDrawElements(mode = GL_TRIANGLES, count = 24, type = GL_UNSIGNED_SHORT, indices = 0x30)
  [... more maps and draws at increasing offsets]

Interesting note for this game, after the initial ``glBufferData()`` in the
frame to reallocate the storage, it unsync maps the whole buffer each time, and
just changes which region it flushes. The same GL buffer name is used in every
frame.

Tabletop Simulator
==================

.. code-block:: console

  1287594 glXSwapBuffers(dpy = 0x3e10810, drawable = 23068692)
  1287595 glClientWaitSync(sync = 0x7abf554e37b0, flags = 0x0, timeout = 0) = GL_ALREADY_SIGNALED
  1287596 glDeleteSync(sync = 0x7abf554e37b0)
  1287597 glFenceSync(condition = GL_SYNC_GPU_COMMANDS_COMPLETE, flags = 0) = 0x7abf56647490
  
  1287614 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 480)
  1287615 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 384, access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7abf2e79a000
  1287642 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 614)
  1287650 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 5)
  1287651 glBufferSubData(target = GL_COPY_WRITE_BUFFER, offset = 0, size = 1088, data = blob(1088))
  1287652 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 615)
  1287653 glDrawElements(mode = GL_TRIANGLES, count = 1788, type = GL_UNSIGNED_SHORT, indices = NULL)
  [... more draw calls]
  1289055 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 480)
  1289057 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 384)
  1289058 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1289059 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 480)
  1289066 glDrawArrays(mode = GL_TRIANGLE_STRIP, first = 12, count = 4)
  1289068 glDrawArrays(mode = GL_TRIANGLE_STRIP, first = 8, count = 4)
  1289553 glXSwapBuffers(dpy = 0x3e10810, drawable = 23068692)

In this app, buffer 480 gets used like this every other frame.  The :ext:`GL_ARB_sync`
fence ensures that frame n-1 has started on the GPU before CPU work starts on
the current frame, so the unsynchronized access to the buffers is safe.

Hollow Knight
=============

.. code-block:: console

  1873034 glXSwapBuffers(dpy = 0x28609d0, drawable = 23068692)
  1873035 glClientWaitSync(sync = 0x7b1a5ca6e130, flags = 0x0, timeout = 0) = GL_ALREADY_SIGNALED
  1873036 glDeleteSync(sync = 0x7b1a5ca6e130)
  1873037 glFenceSync(condition = GL_SYNC_GPU_COMMANDS_COMPLETE, flags = 0) = 0x7b1a5ca6e130
  1873038 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 29)
  1873039 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 8640, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b1a04c7e000
  1873040 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 30)
  1873041 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 720, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b1a07430000
  1873065 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 29)
  1873067 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 8640)
  1873068 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1873069 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 30)
  1873071 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 720)
  1873072 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1873073 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 29)
  1873074 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 8640, length = 576, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b1a04c801c0
  1873075 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 30)
  1873076 glMapBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 720, length = 72, access = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0x7b1a074302d0
  1873077 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 29)
  1873079 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 576)
  1873080 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1873081 glBindBuffer(target = GL_COPY_WRITE_BUFFER, buffer = 30)
  1873083 glFlushMappedBufferRange(target = GL_COPY_WRITE_BUFFER, offset = 0, length = 72)
  1873084 glUnmapBuffer(target = GL_COPY_WRITE_BUFFER) = GL_TRUE
  1873085 glBindBuffer(target = GL_ARRAY_BUFFER, buffer = 29)
  1873096 glBindBuffer(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 30)
  1873097 glDrawElementsBaseVertex(mode = GL_TRIANGLES, count = 36, type = GL_UNSIGNED_SHORT, indices = 0x2d0, basevertex = 240)

In this app, buffer 29/30 get used like this starting from offset 0 every other
frame.  The :ext:`GL_ARB_sync` fence is used to make sure that the GPU has reached the
start of the previous frame before we go unsynchronized writing over the n-2
frame's buffer.

Borderlands 2
=============

.. code-block:: console

  3561998 glFlush()
  3562004 glXSwapBuffers(dpy = 0xbaf0f90, drawable = 23068705)
  3562006 glClientWaitSync(sync = 0x231c2ab0, flags = GL_SYNC_FLUSH_COMMANDS_BIT, timeout = 10000000000) = GL_ALREADY_SIGNALED
  3562007 glDeleteSync(sync = 0x231c2ab0)
  3562008 glFenceSync(condition = GL_SYNC_GPU_COMMANDS_COMPLETE, flags = 0) = 0x231aadc0
  
  3562050 glBindBufferARB(target = GL_ARRAY_BUFFER, buffer = 1193)
  3562051 glMapBufferRange(target = GL_ARRAY_BUFFER, offset = 0, length = 1792, access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT) = 0xde056000
  3562053 glUnmapBufferARB(target = GL_ARRAY_BUFFER) = GL_TRUE
  3562054 glBindBufferARB(target = GL_ARRAY_BUFFER, buffer = 1194)
  3562055 glMapBufferRange(target = GL_ARRAY_BUFFER, offset = 0, length = 1280, access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT) = 0xd9426000
  3562057 glUnmapBufferARB(target = GL_ARRAY_BUFFER) = GL_TRUE
  [... unrelated draws]
  3563051 glBindBufferARB(target = GL_ARRAY_BUFFER, buffer = 1193)
  3563064 glBindBufferARB(target = GL_ELEMENT_ARRAY_BUFFER, buffer = 875)
  3563065 glDrawElementsInstancedARB(mode = GL_TRIANGLES, count = 72, type = GL_UNSIGNED_SHORT, indices = NULL, instancecount = 28)

The :ext:`GL_ARB_sync` fence ensures that the GPU has started frame n-1 before the CPU
starts on the current frame.

This sequence of buffer uploads appears in each frame with the same buffer
names, so you do need to handle the ``GL_MAP_INVALIDATE_BUFFER_BIT`` as a
reallocate if the buffer is GPU-busy (it wasn't in this trace capture) to avoid
stalls on the n-1 frame completing.

Note that this is just one small buffer. Most of the vertex data goes through a
``glBufferSubData()``/``glDraw*()`` path with the VBO used across multiple
frames, with a ``glBufferData()`` when needing to wrap.

Buffer mapping conclusions
--------------------------

* Non-blitting drivers must track the valid range of a freshly allocated buffer
  as it gets uploaded in ``pipe_transfer_map()`` and avoid stalling on the GPU
  when mapping an undefined portion of the buffer when ``glBufferSubData()`` is
  interleaved with drawing.

* Non-blitting drivers must reallocate storage on ``glBufferData(NULL)`` so that
  the following ``glBufferSubData()`` won't stall. That ``glBufferData(NULL)``
  call will appear in the driver as an ``invalidate_resource()`` call if
  ``PIPE_CAP_INVALIDATE_BUFFER`` is available. (If that flag is not set, then
  mesa/st will create a new pipe_resource for you). Storage reallocation may be
  skipped if you for some reason know that the buffer is idle, in which case you
  can just empty the valid region.

* Blitting drivers must use the ``transfer_flush_region()`` region
  instead of the mapped range when ``PIPE_MAP_FLUSH_EXPLICIT`` is set, to avoid
  blitting too much data. (When that bit is unset, you just blit the whole
  mapped range at unmap time.)

* Buffer valid range tracking in non-blitting drivers must use the
  ``transfer_flush_region()`` region instead of the mapped range when
  ``PIPE_MAP_FLUSH_EXPLICIT`` is set, to avoid excess stalls.

* Buffer valid range tracking doesn't need to be fancy, "number of bytes
  valid starting from 0" is sufficient for all examples found.

* Use the ``util_debug_callback`` to report stalls on buffer mapping to ease
  debug.

* Buffer binding points are not useful for tuning buffer placement (See all the
  ``PIPE_COPY_WRITE_BUFFER`` instances), you have to track the actual usage
  history of a GL BO name.  mesa/st does this for optimizing its state updates
  on reallocation in the ``!PIPE_CAP_INVALIDATE_BUFFER`` case, and if you set
  ``PIPE_CAP_INVALIDATE_BUFFER`` then you have to flag your own internal state
  updates (VBO addresses, XFB addresses, texture buffer addresses, etc.) on
  reallocation based on usage history.
