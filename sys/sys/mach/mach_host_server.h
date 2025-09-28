#ifndef	_mach_host_server_
#define	_mach_host_server_

/* Module mach_host */

#pragma GCC diagnostic ignored "-Wredundant-decls"

#include <sys/cdefs.h>
#include <sys/types.h>
#ifdef _KERNEL
#include <sys/mach/ndr.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/notify.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/message.h>
#include <sys/mach/mig_errors.h>
#else /* !_KERNEL */
#include <string.h>
#include <mach/ndr.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#endif /*_KERNEL */

#ifdef AUTOTEST
#ifndef FUNCTION_PTR_T
#define FUNCTION_PTR_T
typedef void (*function_ptr_t)(mach_port_t, char *, mach_msg_type_number_t);
typedef struct {
        char            *name;
        function_ptr_t  function;
} function_table_entry;
typedef function_table_entry   *function_table_t;
#endif /* FUNCTION_PTR_T */
#endif /* AUTOTEST */

#ifndef	mach_host_MSG_COUNT
#define	mach_host_MSG_COUNT	25
#endif	/* mach_host_MSG_COUNT */

#include <sys/mach/std_types.h>
#include <sys/mach/mig.h>
#include <sys/mach/ipc_sync.h>
#include <sys/mach/ipc/ipc_voucher.h>
#include <sys/mach/ipc_host.h>
#include <sys/mach/ipc_tt.h>
#include <sys/mach/ipc_mig.h>
#include <sys/mach/mig.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/mach_types.h>
#include <sys/mach_debug/mach_debug_types.h>

#ifdef __BeforeMigServerHeader
__BeforeMigServerHeader
#endif /* __BeforeMigServerHeader */


/* Routine host_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_info
#if	defined(LINTLIBRARY)
    (host, flavor, host_info_out, host_info_outCnt)
	host_t host;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_info(host, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_kernel_version */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_kernel_version
#if	defined(LINTLIBRARY)
    (host, kernel_version)
	host_t host;
	kernel_version_t kernel_version;
{ return host_kernel_version(host, kernel_version); }
#else
(
	host_t host,
	kernel_version_t kernel_version
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_page_size */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_page_size
#if	defined(LINTLIBRARY)
    (host, out_page_size)
	host_t host;
	vm_size_t *out_page_size;
{ return host_page_size(host, out_page_size); }
#else
(
	host_t host,
	vm_size_t *out_page_size
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_memory_object_memory_entry */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_memory_object_memory_entry
#if	defined(LINTLIBRARY)
    (host, internal, size, permission, pager, entry_handle)
	host_t host;
	boolean_t internal;
	vm_size_t size;
	vm_prot_t permission;
	memory_object_t pager;
	mach_port_t *entry_handle;
{ return mach_memory_object_memory_entry(host, internal, size, permission, pager, entry_handle); }
#else
(
	host_t host,
	boolean_t internal,
	vm_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_processor_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_processor_info
#if	defined(LINTLIBRARY)
    (host, flavor, out_processor_count, out_processor_info, out_processor_infoCnt)
	host_t host;
	processor_flavor_t flavor;
	natural_t *out_processor_count;
	processor_info_array_t *out_processor_info;
	mach_msg_type_number_t *out_processor_infoCnt;
{ return host_processor_info(host, flavor, out_processor_count, out_processor_info, out_processor_infoCnt); }
#else
(
	host_t host,
	processor_flavor_t flavor,
	natural_t *out_processor_count,
	processor_info_array_t *out_processor_info,
	mach_msg_type_number_t *out_processor_infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_clock_service */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_clock_service
#if	defined(LINTLIBRARY)
    (host, clock_id, clock_serv)
	host_t host;
	clock_id_t clock_id;
	clock_serv_t *clock_serv;
{ return host_get_clock_service(host, clock_id, clock_serv); }
#else
(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_virtual_physical_table_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_virtual_physical_table_info
#if	defined(LINTLIBRARY)
    (host, info, infoCnt)
	host_t host;
	hash_info_bucket_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return host_virtual_physical_table_info(host, info, infoCnt); }
#else
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine processor_set_default */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_set_default
#if	defined(LINTLIBRARY)
    (host, default_set)
	host_t host;
	processor_set_name_t *default_set;
{ return processor_set_default(host, default_set); }
#else
(
	host_t host,
	processor_set_name_t *default_set
);
#endif	/* defined(LINTLIBRARY) */

/* Routine processor_set_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_set_create
#if	defined(LINTLIBRARY)
    (host, new_set, new_name)
	host_t host;
	processor_set_t *new_set;
	processor_set_name_t *new_name;
{ return processor_set_create(host, new_set, new_name); }
#else
(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_memory_object_memory_entry_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_memory_object_memory_entry_64
#if	defined(LINTLIBRARY)
    (host, internal, size, permission, pager, entry_handle)
	host_t host;
	boolean_t internal;
	memory_object_size_t size;
	vm_prot_t permission;
	memory_object_t pager;
	mach_port_t *entry_handle;
{ return mach_memory_object_memory_entry_64(host, internal, size, permission, pager, entry_handle); }
#else
(
	host_t host,
	boolean_t internal,
	memory_object_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_statistics */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_statistics
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info_out, host_info_outCnt)
	host_t host_priv;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_statistics(host_priv, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_request_notification */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_request_notification
#if	defined(LINTLIBRARY)
    (host, notify_type, notify_port)
	host_t host;
	host_flavor_t notify_type;
	mach_port_t notify_port;
{ return host_request_notification(host, notify_type, notify_port); }
#else
(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_statistics64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_statistics64
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info64_out, host_info64_outCnt)
	host_t host_priv;
	host_flavor_t flavor;
	host_info64_t host_info64_out;
	mach_msg_type_number_t *host_info64_outCnt;
{ return host_statistics64(host_priv, flavor, host_info64_out, host_info64_outCnt); }
#else
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info64_t host_info64_out,
	mach_msg_type_number_t *host_info64_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_zone_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_zone_info
#if	defined(LINTLIBRARY)
    (host, names, namesCnt, info, infoCnt)
	host_priv_t host;
	mach_zone_name_array_t *names;
	mach_msg_type_number_t *namesCnt;
	mach_zone_info_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return mach_zone_info(host, names, namesCnt, info, infoCnt); }
#else
(
	host_priv_t host,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_create_mach_voucher */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_create_mach_voucher
#if	defined(LINTLIBRARY)
    (host, recipes, recipesCnt, voucher)
	host_t host;
	mach_voucher_attr_raw_recipe_array_t recipes;
	mach_msg_type_number_t recipesCnt;
	ipc_voucher_t *voucher;
{ return host_create_mach_voucher(host, recipes, recipesCnt, voucher); }
#else
(
	host_t host,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t recipesCnt,
	ipc_voucher_t *voucher
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_register_mach_voucher_attr_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_register_mach_voucher_attr_manager
#if	defined(LINTLIBRARY)
    (host, attr_manager, default_value, new_key, new_attr_control)
	host_t host;
	mach_voucher_attr_manager_t attr_manager;
	mach_voucher_attr_value_handle_t default_value;
	mach_voucher_attr_key_t *new_key;
	ipc_voucher_attr_control_t *new_attr_control;
{ return host_register_mach_voucher_attr_manager(host, attr_manager, default_value, new_key, new_attr_control); }
#else
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t *new_key,
	ipc_voucher_attr_control_t *new_attr_control
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_register_well_known_mach_voucher_attr_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_register_well_known_mach_voucher_attr_manager
#if	defined(LINTLIBRARY)
    (host, attr_manager, default_value, key, new_attr_control)
	host_t host;
	mach_voucher_attr_manager_t attr_manager;
	mach_voucher_attr_value_handle_t default_value;
	mach_voucher_attr_key_t key;
	ipc_voucher_attr_control_t *new_attr_control;
{ return host_register_well_known_mach_voucher_attr_manager(host, attr_manager, default_value, key, new_attr_control); }
#else
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t key,
	ipc_voucher_attr_control_t *new_attr_control
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_info
#if	defined(LINTLIBRARY)
    (host, flavor, host_info_out, host_info_outCnt)
	host_t host;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_info(host, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_kernel_version */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_kernel_version
#if	defined(LINTLIBRARY)
    (host, kernel_version)
	host_t host;
	kernel_version_t kernel_version;
{ return host_kernel_version(host, kernel_version); }
#else
(
	host_t host,
	kernel_version_t kernel_version
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_page_size */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_page_size
#if	defined(LINTLIBRARY)
    (host, out_page_size)
	host_t host;
	vm_size_t *out_page_size;
{ return host_page_size(host, out_page_size); }
#else
(
	host_t host,
	vm_size_t *out_page_size
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_memory_object_memory_entry */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_memory_object_memory_entry
#if	defined(LINTLIBRARY)
    (host, internal, size, permission, pager, entry_handle)
	host_t host;
	boolean_t internal;
	vm_size_t size;
	vm_prot_t permission;
	memory_object_t pager;
	mach_port_t *entry_handle;
{ return mach_memory_object_memory_entry(host, internal, size, permission, pager, entry_handle); }
#else
(
	host_t host,
	boolean_t internal,
	vm_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_processor_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_processor_info
#if	defined(LINTLIBRARY)
    (host, flavor, out_processor_count, out_processor_info, out_processor_infoCnt)
	host_t host;
	processor_flavor_t flavor;
	natural_t *out_processor_count;
	processor_info_array_t *out_processor_info;
	mach_msg_type_number_t *out_processor_infoCnt;
{ return host_processor_info(host, flavor, out_processor_count, out_processor_info, out_processor_infoCnt); }
#else
(
	host_t host,
	processor_flavor_t flavor,
	natural_t *out_processor_count,
	processor_info_array_t *out_processor_info,
	mach_msg_type_number_t *out_processor_infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_get_clock_service */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_get_clock_service
#if	defined(LINTLIBRARY)
    (host, clock_id, clock_serv)
	host_t host;
	clock_id_t clock_id;
	clock_serv_t *clock_serv;
{ return host_get_clock_service(host, clock_id, clock_serv); }
#else
(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_virtual_physical_table_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_virtual_physical_table_info
#if	defined(LINTLIBRARY)
    (host, info, infoCnt)
	host_t host;
	hash_info_bucket_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return host_virtual_physical_table_info(host, info, infoCnt); }
#else
(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine processor_set_default */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_set_default
#if	defined(LINTLIBRARY)
    (host, default_set)
	host_t host;
	processor_set_name_t *default_set;
{ return processor_set_default(host, default_set); }
#else
(
	host_t host,
	processor_set_name_t *default_set
);
#endif	/* defined(LINTLIBRARY) */

/* Routine processor_set_create */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t processor_set_create
#if	defined(LINTLIBRARY)
    (host, new_set, new_name)
	host_t host;
	processor_set_t *new_set;
	processor_set_name_t *new_name;
{ return processor_set_create(host, new_set, new_name); }
#else
(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_memory_object_memory_entry_64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_memory_object_memory_entry_64
#if	defined(LINTLIBRARY)
    (host, internal, size, permission, pager, entry_handle)
	host_t host;
	boolean_t internal;
	memory_object_size_t size;
	vm_prot_t permission;
	memory_object_t pager;
	mach_port_t *entry_handle;
{ return mach_memory_object_memory_entry_64(host, internal, size, permission, pager, entry_handle); }
#else
(
	host_t host,
	boolean_t internal,
	memory_object_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_statistics */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_statistics
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info_out, host_info_outCnt)
	host_t host_priv;
	host_flavor_t flavor;
	host_info_t host_info_out;
	mach_msg_type_number_t *host_info_outCnt;
{ return host_statistics(host_priv, flavor, host_info_out, host_info_outCnt); }
#else
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_request_notification */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_request_notification
#if	defined(LINTLIBRARY)
    (host, notify_type, notify_port)
	host_t host;
	host_flavor_t notify_type;
	mach_port_t notify_port;
{ return host_request_notification(host, notify_type, notify_port); }
#else
(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_statistics64 */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_statistics64
#if	defined(LINTLIBRARY)
    (host_priv, flavor, host_info64_out, host_info64_outCnt)
	host_t host_priv;
	host_flavor_t flavor;
	host_info64_t host_info64_out;
	mach_msg_type_number_t *host_info64_outCnt;
{ return host_statistics64(host_priv, flavor, host_info64_out, host_info64_outCnt); }
#else
(
	host_t host_priv,
	host_flavor_t flavor,
	host_info64_t host_info64_out,
	mach_msg_type_number_t *host_info64_outCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine mach_zone_info */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t mach_zone_info
#if	defined(LINTLIBRARY)
    (host, names, namesCnt, info, infoCnt)
	host_priv_t host;
	mach_zone_name_array_t *names;
	mach_msg_type_number_t *namesCnt;
	mach_zone_info_array_t *info;
	mach_msg_type_number_t *infoCnt;
{ return mach_zone_info(host, names, namesCnt, info, infoCnt); }
#else
(
	host_priv_t host,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_create_mach_voucher */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_create_mach_voucher
#if	defined(LINTLIBRARY)
    (host, recipes, recipesCnt, voucher)
	host_t host;
	mach_voucher_attr_raw_recipe_array_t recipes;
	mach_msg_type_number_t recipesCnt;
	ipc_voucher_t *voucher;
{ return host_create_mach_voucher(host, recipes, recipesCnt, voucher); }
#else
(
	host_t host,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t recipesCnt,
	ipc_voucher_t *voucher
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_register_mach_voucher_attr_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_register_mach_voucher_attr_manager
#if	defined(LINTLIBRARY)
    (host, attr_manager, default_value, new_key, new_attr_control)
	host_t host;
	mach_voucher_attr_manager_t attr_manager;
	mach_voucher_attr_value_handle_t default_value;
	mach_voucher_attr_key_t *new_key;
	ipc_voucher_attr_control_t *new_attr_control;
{ return host_register_mach_voucher_attr_manager(host, attr_manager, default_value, new_key, new_attr_control); }
#else
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t *new_key,
	ipc_voucher_attr_control_t *new_attr_control
);
#endif	/* defined(LINTLIBRARY) */

/* Routine host_register_well_known_mach_voucher_attr_manager */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t host_register_well_known_mach_voucher_attr_manager
#if	defined(LINTLIBRARY)
    (host, attr_manager, default_value, key, new_attr_control)
	host_t host;
	mach_voucher_attr_manager_t attr_manager;
	mach_voucher_attr_value_handle_t default_value;
	mach_voucher_attr_key_t key;
	ipc_voucher_attr_control_t *new_attr_control;
{ return host_register_well_known_mach_voucher_attr_manager(host, attr_manager, default_value, key, new_attr_control); }
#else
(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t key,
	ipc_voucher_attr_control_t *new_attr_control
);
#endif	/* defined(LINTLIBRARY) */

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
boolean_t mach_host_server(
		mach_msg_header_t *InHeadP,
		mach_msg_header_t *OutHeadP);

#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
mig_routine_t mach_host_server_routine(
		mach_msg_header_t *InHeadP);


/* Description of this subsystem, for use in direct RPC */
extern const struct mach_host_subsystem {
	mig_server_routine_t	server;	/* Server routine */
	mach_msg_id_t	start;	/* Min routine number */
	mach_msg_id_t	end;	/* Max routine number + 1 */
	unsigned int	maxsize;	/* Max msg size */
	vm_address_t	reserved;	/* Reserved */
	struct routine_descriptor	/*Array of routine descriptors */
		routine[25];
} mach_host_subsystem;

/* typedefs for all requests */

#ifndef __Request__mach_host_subsystem__defined
#define __Request__mach_host_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t flavor;
		mach_msg_type_number_t host_info_outCnt;
	} __Request__host_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__host_kernel_version_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__host_page_size_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		vm_size_t size;
		vm_prot_t permission;
	} __Request__mach_memory_object_memory_entry_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		processor_flavor_t flavor;
	} __Request__host_processor_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		clock_id_t clock_id;
	} __Request__host_get_clock_service_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__host_virtual_physical_table_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__processor_set_default_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__processor_set_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t pager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		boolean_t internal;
		memory_object_size_t size;
		vm_prot_t permission;
	} __Request__mach_memory_object_memory_entry_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t flavor;
		mach_msg_type_number_t host_info_outCnt;
	} __Request__host_statistics_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t notify_port;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t notify_type;
	} __Request__host_request_notification_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		host_flavor_t flavor;
		mach_msg_type_number_t host_info64_outCnt;
	} __Request__host_statistics64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
	} __Request__mach_zone_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t recipesCnt;
		uint8_t recipes[5120];
	} __Request__host_create_mach_voucher_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t attr_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_value_handle_t default_value;
	} __Request__host_register_mach_voucher_attr_manager_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t attr_manager;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_value_handle_t default_value;
		mach_voucher_attr_key_t key;
	} __Request__host_register_well_known_mach_voucher_attr_manager_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__mach_host_subsystem__defined */


/* union of all requests */

#ifndef __RequestUnion__mach_host_subsystem__defined
#define __RequestUnion__mach_host_subsystem__defined
union __RequestUnion__mach_host_subsystem {
	__Request__host_info_t Request_host_info;
	__Request__host_kernel_version_t Request_host_kernel_version;
	__Request__host_page_size_t Request_host_page_size;
	__Request__mach_memory_object_memory_entry_t Request_mach_memory_object_memory_entry;
	__Request__host_processor_info_t Request_host_processor_info;
	__Request__host_get_clock_service_t Request_host_get_clock_service;
	__Request__host_virtual_physical_table_info_t Request_host_virtual_physical_table_info;
	__Request__processor_set_default_t Request_processor_set_default;
	__Request__processor_set_create_t Request_processor_set_create;
	__Request__mach_memory_object_memory_entry_64_t Request_mach_memory_object_memory_entry_64;
	__Request__host_statistics_t Request_host_statistics;
	__Request__host_request_notification_t Request_host_request_notification;
	__Request__host_statistics64_t Request_host_statistics64;
	__Request__mach_zone_info_t Request_mach_zone_info;
	__Request__host_create_mach_voucher_t Request_host_create_mach_voucher;
	__Request__host_register_mach_voucher_attr_manager_t Request_host_register_mach_voucher_attr_manager;
	__Request__host_register_well_known_mach_voucher_attr_manager_t Request_host_register_well_known_mach_voucher_attr_manager;
};
#endif /* __RequestUnion__mach_host_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__mach_host_subsystem__defined
#define __Reply__mach_host_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[68];
	} __Reply__host_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t kernel_versionOffset; /* MiG doesn't use it */
		mach_msg_type_number_t kernel_versionCnt;
		char kernel_version[512];
	} __Reply__host_kernel_version_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		vm_size_t out_page_size;
	} __Reply__host_page_size_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
	} __Reply__mach_memory_object_memory_entry_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t out_processor_info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		natural_t out_processor_count;
		mach_msg_type_number_t out_processor_infoCnt;
	} __Reply__host_processor_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t clock_serv;
		/* end of the kernel processed data */
	} __Reply__host_get_clock_service_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t infoCnt;
	} __Reply__host_virtual_physical_table_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t default_set;
		/* end of the kernel processed data */
	} __Reply__processor_set_default_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_set;
		mach_msg_port_descriptor_t new_name;
		/* end of the kernel processed data */
	} __Reply__processor_set_create_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t entry_handle;
		/* end of the kernel processed data */
	} __Reply__mach_memory_object_memory_entry_64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info_outCnt;
		integer_t host_info_out[68];
	} __Reply__host_statistics_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__host_request_notification_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
		mach_msg_type_number_t host_info64_outCnt;
		integer_t host_info64_out[256];
	} __Reply__host_statistics64_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t names;
		mach_msg_ool_descriptor_t info;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_msg_type_number_t namesCnt;
		mach_msg_type_number_t infoCnt;
	} __Reply__mach_zone_info_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t voucher;
		/* end of the kernel processed data */
	} __Reply__host_create_mach_voucher_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_attr_control;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		mach_voucher_attr_key_t new_key;
	} __Reply__host_register_mach_voucher_attr_manager_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_port_descriptor_t new_attr_control;
		/* end of the kernel processed data */
	} __Reply__host_register_well_known_mach_voucher_attr_manager_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__mach_host_subsystem__defined */


/* union of all replies */

#ifndef __ReplyUnion__mach_host_subsystem__defined
#define __ReplyUnion__mach_host_subsystem__defined
union __ReplyUnion__mach_host_subsystem {
	__Reply__host_info_t Reply_host_info;
	__Reply__host_kernel_version_t Reply_host_kernel_version;
	__Reply__host_page_size_t Reply_host_page_size;
	__Reply__mach_memory_object_memory_entry_t Reply_mach_memory_object_memory_entry;
	__Reply__host_processor_info_t Reply_host_processor_info;
	__Reply__host_get_clock_service_t Reply_host_get_clock_service;
	__Reply__host_virtual_physical_table_info_t Reply_host_virtual_physical_table_info;
	__Reply__processor_set_default_t Reply_processor_set_default;
	__Reply__processor_set_create_t Reply_processor_set_create;
	__Reply__mach_memory_object_memory_entry_64_t Reply_mach_memory_object_memory_entry_64;
	__Reply__host_statistics_t Reply_host_statistics;
	__Reply__host_request_notification_t Reply_host_request_notification;
	__Reply__host_statistics64_t Reply_host_statistics64;
	__Reply__mach_zone_info_t Reply_mach_zone_info;
	__Reply__host_create_mach_voucher_t Reply_host_create_mach_voucher;
	__Reply__host_register_mach_voucher_attr_manager_t Reply_host_register_mach_voucher_attr_manager;
	__Reply__host_register_well_known_mach_voucher_attr_manager_t Reply_host_register_well_known_mach_voucher_attr_manager;
};
#endif /* __RequestUnion__mach_host_subsystem__defined */

#ifndef subsystem_to_name_map_mach_host
#define subsystem_to_name_map_mach_host \
    { "host_info", 200 },\
    { "host_kernel_version", 201 },\
    { "host_page_size", 202 },\
    { "mach_memory_object_memory_entry", 203 },\
    { "host_processor_info", 204 },\
    { "host_get_clock_service", 206 },\
    { "host_virtual_physical_table_info", 209 },\
    { "processor_set_default", 213 },\
    { "processor_set_create", 214 },\
    { "mach_memory_object_memory_entry_64", 215 },\
    { "host_statistics", 216 },\
    { "host_request_notification", 217 },\
    { "host_statistics64", 219 },\
    { "mach_zone_info", 220 },\
    { "host_create_mach_voucher", 222 },\
    { "host_register_mach_voucher_attr_manager", 223 },\
    { "host_register_well_known_mach_voucher_attr_manager", 224 }
#endif

#ifdef __AfterMigServerHeader
__AfterMigServerHeader
#endif /* __AfterMigServerHeader */

#endif	 /* _mach_host_server_ */
