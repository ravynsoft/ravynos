Base object structs
===================

The Vulkan runtime code provides a set of base object structs which must be
used if you want your driver to take advantage of any of the runtime code.
There are other base structs for various things which are not covered here
but those are optional.  The ones covered here are the bare minimum set
which form the core of the Vulkan runtime code:

.. contents::
   :local:

As one might expect, :c:struct:`vk_instance` is the required base struct
for implementing ``VkInstance``, :c:struct:`vk_physical_device` is
required for ``VkPhysicalDevice``, and :c:struct:`vk_device` for
``VkDevice``.  Everything else must derive from
:c:struct:`vk_object_base` or from some struct that derives from
:c:struct:`vk_object_base`.


vk_object_base
--------------

The root base struct for all Vulkan objects is
:c:struct:`vk_object_base`.  Every object exposed to the client through
the Vulkan API *must* inherit from :c:struct:`vk_object_base` by having a
:c:struct:`vk_object_base` or some struct that inherits from
:c:struct:`vk_object_base` as the driver struct's first member.  Even
though we have ``container_of()`` and use it liberally, the
:c:struct:`vk_object_base` should be the first member as there are a few
places, particularly in the logging framework, where we use void pointers
to avoid casting and this only works if the address of the driver struct is
the same as the address of the :c:struct:`vk_object_base`.

The standard pattern for defining a Vulkan object inside a driver looks
something like this:

.. code-block:: c

   struct drv_sampler {
      struct vk_object_base base;

      /* Driver fields */
   };

   VK_DEFINE_NONDISP_HANDLE_CASTS(drv_sampler, base, VkSampler,
                                  VK_OBJECT_TYPE_SAMPLER);

Then, to the object in a Vulkan entrypoint,

.. code-block:: c

   VKAPI_ATTR void VKAPI_CALL drv_DestroySampler(
       VkDevice                                    _device,
       VkSampler                                   _sampler,
       const VkAllocationCallbacks*                pAllocator)
   {
      VK_FROM_HANDLE(drv_device, device, _device);
      VK_FROM_HANDLE(drv_sampler, sampler, _sampler);

      if (!sampler)
         return;

      /* Tear down the sampler */

      vk_object_free(&device->vk, pAllocator, sampler);
   }

The :c:macro:`VK_DEFINE_NONDISP_HANDLE_CASTS()` macro defines a set of
type-safe cast functions called ``drv_sampler_from_handle()`` and
``drv_sampler_to_handle()`` which cast a :c:type:`VkSampler` to and from a
``struct drv_sampler *``.  Because compile-time type checking with Vulkan
handle types doesn't always work in C, the ``_from_handle()`` helper uses the
provided :c:type:`VkObjectType` to assert at runtime that the provided
handle is the correct type of object.  Both cast helpers properly handle
``NULL`` and ``VK_NULL_HANDLE`` as inputs.  The :c:macro:`VK_FROM_HANDLE()`
macro provides a convenient way to declare a ``drv_foo`` pointer and
initialize it from a ``VkFoo`` handle in one smooth motion.

.. c:autostruct:: vk_object_base
   :file: src/vulkan/runtime/vk_object.h
   :members:

.. c:autofunction:: vk_object_base_init

.. c:autofunction:: vk_object_base_finish

.. c:automacro:: VK_DEFINE_HANDLE_CASTS

.. c:automacro:: VK_DEFINE_NONDISP_HANDLE_CASTS

.. c:automacro:: VK_FROM_HANDLE


vk_instance
-----------

.. c:autostruct:: vk_instance
   :file: src/vulkan/runtime/vk_instance.h
   :members:

.. c:autofunction:: vk_instance_init

.. c:autofunction:: vk_instance_finish

Once a driver has a :c:struct:`vk_instance`, implementing all the various
instance-level ``vkGet*ProcAddr()`` entrypoints is trivial:

.. code-block:: c

   VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
   drv_GetInstanceProcAddr(VkInstance _instance,
                           const char *pName)
   {
      VK_FROM_HANDLE(vk_instance, instance, _instance);
      return vk_instance_get_proc_addr(instance,
                                       &drv_instance_entrypoints,
                                       pName);
   }

   PUBLIC VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
   vk_icdGetInstanceProcAddr(VkInstance instance,
                             const char *pName)
   {
      return drv_GetInstanceProcAddr(instance, pName);
   }

.. c:autofunction:: vk_instance_get_proc_addr

.. c:autofunction:: vk_instance_get_proc_addr_unchecked

.. c:autofunction:: vk_instance_get_physical_device_proc_addr

We also provide an implementation of
``vkEnumerateInstanceExtensionProperties()`` which can be used similarly:

.. code-block:: c

   VKAPI_ATTR VkResult VKAPI_CALL
   drv_EnumerateInstanceExtensionProperties(const char *pLayerName,
                                            uint32_t *pPropertyCount,
                                            VkExtensionProperties *pProperties)
   {
      if (pLayerName)
         return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

      return vk_enumerate_instance_extension_properties(
         &instance_extensions, pPropertyCount, pProperties);
   }

.. c:autofunction:: vk_enumerate_instance_extension_properties

vk_physical_device
------------------

.. c:autostruct:: vk_physical_device
   :file: src/vulkan/runtime/vk_physical_device.h
   :members:

.. c:autofunction:: vk_physical_device_init

.. c:autofunction:: vk_physical_device_finish

vk_device
------------------

.. c:autostruct:: vk_device
   :file: src/vulkan/runtime/vk_device.h
   :members:

.. c:autofunction:: vk_device_init

.. c:autofunction:: vk_device_finish
