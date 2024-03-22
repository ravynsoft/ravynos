Command Pools
=============

The Vulkan runtime code provides a common ``VkCommandPool`` implementation
which makes managing the lifetimes of command buffers and recycling their
internal state easier.  To use the common command pool a driver needs to
fill out a :c:struct:`vk_command_buffer_ops` struct and set the
``command_buffer_ops`` field of :c:struct:`vk_device`.

.. c:autostruct:: vk_command_buffer_ops
   :file: src/vulkan/runtime/vk_command_buffer.h
   :members:

By reducing the entirety of command buffer lifetime management to these
three functions, much of the complexity of command pools can be implemented
in common code, providing better, more consistent behavior across Mesa.


Command Buffer Recycling
------------------------

The common command pool provides automatic command buffer recycling as long
as the driver uses the common ``vkAllocateCommandBuffers()`` and
``vkFreeCommandBuffers()`` implementations.  The driver must also provide the
``reset`` function pointer in :c:struct:`vk_command_buffer_ops`.

With the common command buffer pool, when the client calls
``vkFreeCommandBuffers()``, the command buffers are not immediately freed.
Instead, they are reset with
``VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT``, their base object is
recycled, and they are added to a free list inside the pool.  When the
client then calls ``vkAllocateCommandBuffers()``, we check the free list
and return a recycled command buffer, if any are available.  This provides
some basic command buffer pooling without the driver doing any additional
work.


Custom command pools
--------------------

If a driver wishes to recycle at a finer granularity than whole command
buffers, they can do so by providing their own command pool implementation
which wraps :c:struct:`vk_command_pool`.  The common use-case here is if
the driver wants to pool command-buffer-internal objects at a finer
granularity than whole command buffers.  The command pool provides a place
where things like GPU command buffers or upload buffers can be cached
without having to take a lock.

When implementing a custom command pool, drivers need only implement three
entrypoints:

 - ``vkCreateCommandPool()``
 - ``vkDestroyCommandPool()``
 - ``vkTrimCommandPool()``

All of the other entrypoints will be handled by common code so long as the
driver's command pool derives from :c:struct:`vk_command_pool`.

The driver implementation of the command buffer ``recycle()`` function
should respect ``VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT`` and, when
set, return any recyclable resources to the command pool.  This may be set
by the client when it calls ``vkResetCommandBuffer()``, come from a
whole-pool reset via ``VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT``, or
come from the common command buffer code when a command buffer is recycled.

The driver's implementation of ``vkTrimCommandPool()`` should free any
resources that have been cached within the command pool back to the device
or back to the OS.  It **must** also call :c:func:`vk_command_pool_trim`
to allow the common code to free any recycled command buffers.

Reference
---------

.. c:autostruct:: vk_command_pool
   :file: src/vulkan/runtime/vk_command_pool.h
   :members:

.. c:autofunction:: vk_command_pool_init
   :file: src/vulkan/runtime/vk_command_pool.h

.. c:autofunction:: vk_command_pool_finish
   :file: src/vulkan/runtime/vk_command_pool.h

.. c:autofunction:: vk_command_pool_trim
   :file: src/vulkan/runtime/vk_command_pool.h
