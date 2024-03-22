Dispatch
=============

This chapter attempts to document the Vulkan dispatch infrastructure in the
Mesa Vulkan runtime.  There are a lot of moving pieces here but the end
result has proven quite effective for implementing all the various Vulkan
API requirements.


Extension tables
----------------

The Vulkan runtime defines two extension table structures, one for instance
extensions and one for device extensions which contain a Boolean per
extension.  The device table looks like this:

.. code-block:: c

    #define VK_DEVICE_EXTENSION_COUNT 238

    struct vk_device_extension_table {
       union {
          bool extensions[VK_DEVICE_EXTENSION_COUNT];
          struct {
             bool KHR_8bit_storage;
             bool KHR_16bit_storage;
             bool KHR_acceleration_structure;
             bool KHR_bind_memory2;
             ...
          };
       };
    };

The instance extension table is similar except that it includes the
instance level extensions.  Both tables are actually unions so that you can
access the table either by name or as an array.  Accessing by name is
typically better for human-written code which needs to query for specific
enabled extensions or declare a table of which extensions a driver
supports.  The array form is convenient for more automatic code which wants
to iterate over the table.

These tables are are generated automatically using a bit of python code that
parses the vk.xml from the `Vulkan-Docs repo
<https://github.com/KhronosGroup/Vulkan-docs/>`__, enumerates the
extensions, sorts them by instance vs. device and generates the table.
Generating it from XML means that we never have to manually maintain any of
these data structures; they get automatically updated when someone imports
a new version of vk.xml.  We also generates a matching pair of tables of
``VkExtensionProperties``.  This makes it easy to implement
``vkEnumerate*ExtensionProperties()`` with a simple loop that walks a table
of supported extensions and copies the VkExtensionProperties for each
enabled entry.  Similarly, we can have a loop in ``vkCreateInstance()`` or
``vkCreateDevice()`` which takes the ``ppEnabledExtensionNames`` and fills
out the table with all enabled extensions.


Entrypoint and dispatch tables
------------------------------

Entrypoint tables contain a function pointer for every Vulkan entrypoint
within a particular scope.  There are separate tables for instance,
physical device, and device-level functionality.  The device entrypoint
table looks like this:

.. code-block:: c

    struct vk_device_entrypoint_table {
       PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
       PFN_vkDestroyDevice DestroyDevice;
       PFN_vkGetDeviceQueue GetDeviceQueue;
       PFN_vkQueueSubmit QueueSubmit;
       ...
    #ifdef VK_USE_PLATFORM_WIN32_KHR
       PFN_vkGetSemaphoreWin32HandleKHR GetSemaphoreWin32HandleKHR;
    #else
       PFN_vkVoidFunction GetSemaphoreWin32HandleKHR;
    # endif
       ...
    };

Every entry that requires some sort of platform define is wrapped in an
``#ifdef`` and declared as the actual function pointer type if the platform
define is set and declared as a void function otherwise.  This ensures that
the layout of the structure doesn't change based on preprocessor symbols
but anyone who has the platform defines set gets the real prototype and
anyone who doesn't can use the table without needing to pull in all the
platform headers.

Dispatch tables are similar to entrypoint tables except that they're
deduplicated so that aliased entrypoints have only one entry in the table.
The device dispatch table looks like this:

.. code-block:: c

    struct vk_device_dispatch_table {
        PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
        PFN_vkDestroyDevice DestroyDevice;
        PFN_vkGetDeviceQueue GetDeviceQueue;
        PFN_vkQueueSubmit QueueSubmit;
        ...
        union {
            PFN_vkResetQueryPool ResetQueryPool;
            PFN_vkResetQueryPoolEXT ResetQueryPoolEXT;
        };
        ...
    };

In order to allow code to use any of the aliases for a given entrypoint,
such entrypoints are wrapped in a union.  This is important because we need
to be able to add new aliases potentially at any Vulkan release and we want
to do so without having to update all the driver code which uses one of the
newly aliased entrypoints.  We could require that everyone use the first
name an entrypoint ever has but that gets weird if, for instance, it's
introduced in an EXT extension and some driver only ever implements the KHR
or core version of the feature.  It's easier for everyone if we make all
the entrypoint names work.

An entrypoint table can be converted to a dispatch table by compacting it
with one of the ``vk_*_dispatch_table_from_entrypoints()`` family of
functions:

.. code-block:: c

   void vk_instance_dispatch_table_from_entrypoints(
       struct vk_instance_dispatch_table *dispatch_table,
       const struct vk_instance_entrypoint_table *entrypoint_table,
       bool overwrite);

   void vk_physical_device_dispatch_table_from_entrypoints(
       struct vk_physical_device_dispatch_table *dispatch_table,
       const struct vk_physical_device_entrypoint_table *entrypoint_table,
       bool overwrite);

   void vk_device_dispatch_table_from_entrypoints(
       struct vk_device_dispatch_table *dispatch_table,
       const struct vk_device_entrypoint_table *entrypoint_table,
       bool overwrite);


Generating driver dispatch tables
---------------------------------

Entrypoint tables can be easily auto-generated for your driver.  Simply put
the following in the driver's ``meson.build``, modified as necessary:

.. code-block::

    drv_entrypoints = custom_target(
      'drv_entrypoints',
      input : [vk_entrypoints_gen, vk_api_xml],
      output : ['drv_entrypoints.h', 'drv_entrypoints.c'],
      command : [
        prog_python, '@INPUT0@', '--xml', '@INPUT1@', '--proto', '--weak',
        '--out-h', '@OUTPUT0@', '--out-c', '@OUTPUT1@', '--prefix', 'drv',
        '--beta', with_vulkan_beta.to_string(),
      ],
      depend_files : vk_entrypoints_gen_depend_files,
    )

The generated ``drv_entrypoints.h`` fill will contain prototypes for every
Vulkan entrypoint, prefixed with what you passed to ``--prefix`` above.
For instance, if you set ``--prefix drv`` and the entrypoint name is
``vkCreateDevice()``, the driver entrypoint will be named
``drv_CreateDevice()``.  The ``--prefix`` flag can be specified multiple
times if you want more than one table.  It also generates an entrypoint
table for each prefix and each dispatch level (instance, physical device,
and device) which is populated using the driver's functions.  Thanks to our
use of weak function pointers (or something roughly equivalent for MSVC),
any entrypoints which are not implemented will automatically show up as
``NULL`` entries in the table rather than resulting in linking errors.

The above generates entrypoint tables because, thanks to aliasing and the C
rules around const struct declarations, it's not practical to generate a
dispatch table directly.  Before they can be passed into the relevant
``vk_*_init()`` function, the entrypoint table will have to be converted to
a dispatch table.  The typical pattern for this inside a driver looks
something like this:

.. code-block:: c

    struct vk_instance_dispatch_table dispatch_table;
    vk_instance_dispatch_table_from_entrypoints(
       &dispatch_table, &anv_instance_entrypoints, true);
    vk_instance_dispatch_table_from_entrypoints(
       &dispatch_table, &wsi_instance_entrypoints, false);

    result = vk_instance_init(&instance->vk, &instance_extensions,
                              &dispatch_table, pCreateInfo, pAllocator);
    if (result != VK_SUCCESS) {
       vk_free(pAllocator, instance);
       return result;
    }

The ``vk_*_dispatch_table_from_entrypoints()`` functions are designed so
that they can be layered like this.  In this case, it starts with the
instance entrypoints from the Intel Vulkan driver and then adds in the WSI
entrypoints.  If there are any entrypoints duplicated between the two, the
first one to define the entrypoint wins.


Common Vulkan entrypoints
-------------------------

For the Vulkan runtime itself, there is a dispatch table with the
``vk_common`` prefix used to provide common implementations of various
entrypoints.  This entrypoint table is added last as part of
``vk_*_init()`` so that the driver implementation will always be used, if
there is one.

This is used to implement a bunch of things on behalf of the driver.  The
most common case is whenever there are ``vkFoo()`` and ``vkFoo2()``
entrypoints.  We provide wrappers for nearly all of these that implement
``vkFoo()`` in terms of ``vkFoo2()`` so a driver can switch to the new one
and throw the old one away.  For instance, ``vk_common_BindBufferMemory()``
looks like this:

.. code-block:: c

   VKAPI_ATTR VkResult VKAPI_CALL
   vk_common_BindBufferMemory(VkDevice _device,
                              VkBuffer buffer,
                              VkDeviceMemory memory,
                              VkDeviceSize memoryOffset)
   {
      VK_FROM_HANDLE(vk_device, device, _device);

      VkBindBufferMemoryInfo bind = {
         .sType         = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
         .buffer        = buffer,
         .memory        = memory,
         .memoryOffset  = memoryOffset,
      };

      return device->dispatch_table.BindBufferMemory2(_device, 1, &bind);
   }

There are, of course, far more complicated cases of implementing
``vkFoo()`` in terms of ``vkFoo2()`` such as the
``vk_common_QueueSubmit()`` implementation.  We also implement far less
trivial functionality as ``vk_common_*`` entrypoints.  For instance, we
have full implementations of ``VkFence``, ``VkSemaphore``, and
``vkQueueSubmit2()``.


Entrypoint lookup
-----------------

Implementing ``vkGet*ProcAddr()`` is quite complicated because of the
Vulkan 1.2 rules around exactly when they have to return ``NULL``.  When a
client calls ``vkGet*ProcAddr()``, we go through a three step process resolve
the function pointer:

 1. A static (generated at compile time) hash table is used to map the
    entrypoint name to an index into the corresponding entry point table.

 2. Optionally, the index is passed to an auto-generated function that
    checks against the enabled core API version and extensions.  We use an
    index into the entrypoint table, not the dispatch table, because the
    rules for when an entrypoint should be exposed are per-entrypoint.  For
    instance, ``vkBindImageMemory2`` is available on Vulkan 1.1 and later but
    ``vkBindImageMemory2KHR`` is available if :ext:`VK_KHR_bind_memory2` is
    enabled.

 3. A compaction table is used to map from the entrypoint table index to
    the dispatch table index and the function is finally fetched from the
    dispatch table.

All of this is encapsulated within the ``vk_*_dispatch_table_get()`` and
``vk_*_dispatch_table_get_if_supported()`` families of functions.  The
``_if_supported`` versions take a core version and one or more extension
tables.  The driver has to provide ``vk_icdGet*ProcAddr()`` entrypoints
which wrap these functions because those have to be exposed as actual
symbols from the ``.so`` or ``.dll`` as part of the loader interface.  It
also has to provide its own ``drv_GetInstanceProcAddr()`` because it needs
to pass the supported instance extension table to
:c:func:`vk_instance_get_proc_addr`.  The runtime will provide
``vk_common_GetDeviceProcAddr()`` implementations.


Populating layer or client dispatch tables
------------------------------------------

The entrypoint and dispatch tables actually live in ``src/vulkan/util``,
not ``src/vulkan/runtime`` so they can be used by layers and clients (such
as Zink) as well as the runtime.  Layers and clients may wish to populate
dispatch tables from an underlying Vulkan implementation.  This can be done
via the ``vk_*_dispatch_table_load()`` family of functions:

.. code-block:: c

   void
   vk_instance_dispatch_table_load(struct vk_instance_dispatch_table *table,
                                   PFN_vkGetInstanceProcAddr gpa,
                                   VkInstance instance);
   void
   vk_physical_device_dispatch_table_load(struct vk_physical_device_dispatch_table *table,
                                          PFN_vkGetInstanceProcAddr gpa,
                                          VkInstance instance);
   void
   vk_device_dispatch_table_load(struct vk_device_dispatch_table *table,
                                 PFN_vkGetDeviceProcAddr gpa,
                                 VkDevice device);

These call the given ``vkGet*ProcAddr`` function to populate the dispatch
table.  For aliased entrypoints, it will try each variant in succession to
ensure that the dispatch table entry gets populated no matter which version
of the feature you have enabled.
