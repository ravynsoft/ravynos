
#ifndef _WINSYS_HANDLE_H_
#define _WINSYS_HANDLE_H_

#ifdef _WIN32
typedef void *HANDLE;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define WINSYS_HANDLE_TYPE_SHARED 0
#define WINSYS_HANDLE_TYPE_KMS    1
#define WINSYS_HANDLE_TYPE_FD     2
/* Win32 handles serve the same purpose as FD, just on Windows, so alias the value */
#define WINSYS_HANDLE_TYPE_WIN32_HANDLE WINSYS_HANDLE_TYPE_FD
#define WINSYS_HANDLE_TYPE_SHMID   3
#define WINSYS_HANDLE_TYPE_D3D12_RES 4
#define WINSYS_HANDLE_TYPE_WIN32_NAME 5

/**
 * For use with pipe_screen::{resource_from_handle|resource_get_handle}.
 */
struct winsys_handle
{
   /**
    * Input for resource_from_handle, valid values are
    * WINSYS_HANDLE_TYPE_SHARED or WINSYS_HANDLE_TYPE_FD.
    * Input to resource_get_handle,
    * to select handle for kms, flink, or prime.
    */
   unsigned type;
   /**
    * Input for resource_get_handle, allows to export the offset
    * of a specific layer of an array texture.
    */
   unsigned layer;
   /**
    * Input for resource_get_handle, allows to export of a specific plane of a
    * texture.
    */
   unsigned plane;
   /**
    * Input to resource_from_handle.
    * Output for resource_get_handle.
    */
#ifdef _WIN32
   HANDLE handle;
#else
   unsigned handle;
#endif
   /**
    * Input to resource_from_handle.
    * Output for resource_get_handle.
    */
   unsigned stride;
   /**
    * Input to resource_from_handle.
    * Output for resource_get_handle.
    */
   unsigned offset;

   /**
    * Input to resource_from_handle.
    * Output from resource_get_handle.
    */
   uint64_t format;

   /**
    * Input to resource_from_handle.
    * Output from resource_get_handle.
    */
   uint64_t modifier;

   union
   {
      /**
       * Input to resource_from_handle.
       * Output for resource_get_handle.
       */
      void *com_obj;

      /**
       * String name for an object.
       * Input to resource_from_handle.
       */
      const void *name;
   };

   /**
    * Total size of the object.
    * Output for resource_get_handle.
    */
   uint64_t size;
};

#ifdef __cplusplus
}
#endif

#endif /* _WINSYS_HANDLE_H_ */
