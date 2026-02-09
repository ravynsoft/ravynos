/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef	_CFNetDiagnosticsProtocol_user_
#define	_CFNetDiagnosticsProtocol_user_

/* Module CFNetDiagnosticsProtocol */

#include <string.h>
#include <mach/ndr.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/port.h>

#ifdef AUTOTEST
#ifndef FUNCTION_PTR_T
#define FUNCTION_PTR_T
typedef void (*function_ptr_t)(mach_port_t, char *, mach_msg_type_number_t);
typedef struct {
        char            *name;
        function_ptr_t  function;
} function_table_entry;
typedef function_table_entry 	*function_table_t;
#endif /* FUNCTION_PTR_T */
#endif /* AUTOTEST */

#ifndef	CFNetDiagnosticsProtocol_MSG_COUNT
#define	CFNetDiagnosticsProtocol_MSG_COUNT	1
#endif	/* CFNetDiagnosticsProtocol_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mig.h>
#include <mach/mach_types.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* SimpleRoutine passDescriptor */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t _CFNetDiagnosticClient_passDescriptor
(
	mach_port_t server_port,
	int32_t protocol_version,
	vm_address_t descriptor,
	mach_msg_type_number_t descriptorCnt
);

__END_DECLS

/********************** Caution **************************/
/* The following data types should be used to calculate  */
/* maximum message sizes only. The actual message may be */
/* smaller, and the position of the arguments within the */
/* message layout may vary from what is presented here.  */
/* For example, if any of the arguments are variable-    */
/* sized, and less than the maximum is sent, the data    */
/* will be packed tight in the actual message to reduce  */
/* the presence of holes.                                */
/********************** Caution **************************/

/* typedefs for all requests */

#ifndef __Request__CFNetDiagnosticsProtocol_subsystem__defined
#define __Request__CFNetDiagnosticsProtocol_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		/* start of the kernel processed data */
		mach_msg_body_t msgh_body;
		mach_msg_ool_descriptor_t descriptor;
		/* end of the kernel processed data */
		NDR_record_t NDR;
		int32_t protocol_version;
		mach_msg_type_number_t descriptorCnt;
	} __Request__passDescriptor_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Request__CFNetDiagnosticsProtocol_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem__defined
#define __RequestUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem__defined
union __RequestUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem {
	__Request__passDescriptor_t Request__CFNetDiagnosticClient_passDescriptor;
};
#endif /* !__RequestUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__CFNetDiagnosticsProtocol_subsystem__defined
#define __Reply__CFNetDiagnosticsProtocol_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__passDescriptor_t;
#ifdef  __MigPackStructs
#pragma pack()
#endif
#endif /* !__Reply__CFNetDiagnosticsProtocol_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem__defined
#define __ReplyUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem__defined
union __ReplyUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem {
	__Reply__passDescriptor_t Reply__CFNetDiagnosticClient_passDescriptor;
};
#endif /* !__RequestUnion___CFNetDiagnosticClient_CFNetDiagnosticsProtocol_subsystem__defined */

#ifndef subsystem_to_name_map_CFNetDiagnosticsProtocol
#define subsystem_to_name_map_CFNetDiagnosticsProtocol \
    { "passDescriptor", 10000 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _CFNetDiagnosticsProtocol_user_ */
