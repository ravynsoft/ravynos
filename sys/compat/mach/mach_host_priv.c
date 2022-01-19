
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
#include <sys/mach/host_priv_server.h>


int
get_dp_control_port(
	host_priv_t host,
	mach_port_t *contorl_port
)
UNSUPPORTED;

int
host_default_memory_manager(
	host_priv_t host_priv,
	memory_object_default_t *default_manager,
	memory_object_cluster_size_t cluster_size
)
UNSUPPORTED;

int
host_get_UNDServer(
	host_priv_t host,
	UNDServerRef *server
)
UNSUPPORTED;

int
host_get_boot_info(
	host_priv_t host_priv,
	kernel_boot_info_t boot_info
)
UNSUPPORTED;

int
host_get_clock_control(
	host_priv_t host_priv,
	clock_id_t clock_id,
	clock_ctrl_t *clock_ctrl
)
UNSUPPORTED;

int
host_get_special_port(
	host_priv_t host_priv,
	int node,
	int which,
	mach_port_t *port
)
UNSUPPORTED;

int
host_priv_statistics(
	host_priv_t host_priv,
	host_flavor_t flavor,
	host_info_t host_info_out,
	mach_msg_type_number_t *host_info_outCnt
)
UNSUPPORTED;

int
host_processor_set_priv(
	host_priv_t host_priv,
	processor_set_name_t set_name,
	processor_set_t *set
)
UNSUPPORTED;

int
host_processor_sets(
	host_priv_t host_priv,
	processor_set_name_array_t *processor_sets,
	mach_msg_type_number_t *processor_setsCnt
)
UNSUPPORTED;

int
host_processors(
	host_priv_t host_priv,
	processor_array_t *out_processor_list,
	mach_msg_type_number_t *out_processor_listCnt
)
UNSUPPORTED;

int
host_reboot(
	host_priv_t host_priv,
	int options
)
UNSUPPORTED;

int
host_set_UNDServer(
	host_priv_t host,
	UNDServerRef server
)

UNSUPPORTED;

int
host_set_special_port(
	host_priv_t host_priv,
	int which,
	mach_port_t port
)
UNSUPPORTED;

int
kext_request(
	host_priv_t host_priv,
	uint32_t user_log_flags,
	vm_offset_t request_data,
	mach_msg_type_number_t request_dataCnt,
	vm_offset_t *response_data,
	mach_msg_type_number_t *response_dataCnt,
	vm_offset_t *log_data,
	mach_msg_type_number_t *log_dataCnt,
	kern_return_t *op_result
)
UNSUPPORTED;

int
set_dp_control_port(
	host_priv_t host,
	mach_port_t control_port
)
UNSUPPORTED;

int
thread_wire(
	host_priv_t host_priv,
	thread_act_t thread,
	boolean_t wired
)
UNSUPPORTED;
