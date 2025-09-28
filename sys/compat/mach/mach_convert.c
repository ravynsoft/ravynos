/*-
 * Copyright (c) 2014-2015, Matthew Macy <mmacy@nextbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Neither the name of Matthew Macy nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>

#include <sys/mach/mach_types.h>
#include <sys/mach/ipc_host.h>
#include <sys/mach/ipc_sync.h>
#include <sys/mach/ipc_tt.h>
#include <sys/mach/ipc/ipc_voucher.h>
#include <sys/mach/task.h>

ipc_port_t
convert_clock_ctrl_to_port(mach_clock_t clock)
{
	return (NULL);
}

ipc_port_t
convert_clock_to_port(mach_clock_t clock)
{
	return (NULL);
}

vm_map_t
convert_port_entry_to_map(ipc_port_t port)
{
	return (NULL);
}


mach_clock_t
convert_port_to_clock(ipc_port_t port)
{
	return (NULL);
}

semaphore_t
convert_port_to_semaphore(ipc_port_t port)
{

	return (NULL);
}

task_name_t
convert_port_to_task_name(ipc_port_t port)
{

	return (NULL);
}

task_suspension_token_t
convert_port_to_task_suspension_token(ipc_port_t port)
{

	return (NULL);
}

ipc_voucher_t
convert_port_to_voucher(ipc_port_t port)
{

	return (NULL);
}

ipc_port_t
convert_semaphore_to_port(semaphore_t sema)
{

	return (NULL);
}

ipc_port_t
convert_task_suspension_token_to_port(task_suspension_token_t token)
{

	return (NULL);
}

ipc_port_t
convert_voucher_attr_control_to_port(ipc_voucher_attr_control_t control)
{

	return (NULL);
}

ipc_port_t
convert_voucher_to_port(ipc_voucher_t voucher)
{

	return (NULL);
}
