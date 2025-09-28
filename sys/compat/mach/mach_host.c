

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/proc.h>

#include <sys/mach/mach_types.h>


#include <sys/mach/host_special_ports.h>
#include <sys/mach/host.h>
#include <sys/mach/mach_host_server.h>

host_data_t	realhost;

int
host_create_mach_voucher(
	host_t host,
	mach_voucher_attr_raw_recipe_array_t recipes,
	mach_msg_type_number_t recipesCnt,
	ipc_voucher_t *voucher
)
UNSUPPORTED;

int
host_get_clock_service(
	host_t host,
	clock_id_t clock_id,
	clock_serv_t *clock_serv
)
UNSUPPORTED;

int
host_info(
	host_t host,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
UNSUPPORTED;

int
host_kernel_version(
	host_t host,
	kernel_version_t kernel_version
)
UNSUPPORTED;

int
host_page_size(
	host_t host,
	vm_size_t *out_page_size
)
UNSUPPORTED;

int
host_processor_info(
	host_t host,
	processor_flavor_t flavor,
	natural_t *out_processor_count,
	processor_info_array_t *out_processor_info,
	mach_msg_type_number_t *out_processor_infoCnt
)
UNSUPPORTED;

int
host_register_mach_voucher_attr_manager(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t *new_key,
	ipc_voucher_attr_control_t *new_attr_control
)
UNSUPPORTED;

int
host_register_well_known_mach_voucher_attr_manager(
	host_t host,
	mach_voucher_attr_manager_t attr_manager,
	mach_voucher_attr_value_handle_t default_value,
	mach_voucher_attr_key_t key,
	ipc_voucher_attr_control_t *new_attr_control
)
UNSUPPORTED;

int
host_request_notification(
	host_t host,
	host_flavor_t notify_type,
	mach_port_t notify_port
)
UNSUPPORTED;

int
host_statistics(
	host_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
UNSUPPORTED;

int
host_statistics64(
	host_t host_priv,
	host_flavor_t flavor,
	host_info64_t host_info64_out,
	mach_msg_type_number_t *host_info64_outCnt
)
UNSUPPORTED;

int
host_virtual_physical_table_info(
	host_t host,
	hash_info_bucket_array_t *info,
	mach_msg_type_number_t *infoCnt
)
UNSUPPORTED;

int
mach_memory_object_memory_entry(
	host_t host,
	boolean_t internal,
	vm_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
)
UNSUPPORTED;

int
mach_memory_object_memory_entry_64(
	host_t host,
	boolean_t internal,
	memory_object_size_t size,
	vm_prot_t permission,
	memory_object_t pager,
	mach_port_t *entry_handle
)
UNSUPPORTED;

int
mach_zone_info(
	host_priv_t host,
	mach_zone_name_array_t *names,
	mach_msg_type_number_t *namesCnt,
	mach_zone_info_array_t *info,
	mach_msg_type_number_t *infoCnt
)
UNSUPPORTED;

int
processor_set_create(
	host_t host,
	processor_set_t *new_set,
	processor_set_name_t *new_name
)
UNSUPPORTED;


