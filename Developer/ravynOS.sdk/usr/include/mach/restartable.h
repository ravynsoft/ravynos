#ifndef	_task_restartable_user_
#define	_task_restartable_user_

/* Module task_restartable */

#include <string.h>
#include <mach/ndr.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/notify.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <mach/mig_errors.h>
#include <mach/port.h>
	
/* BEGIN MIG_STRNCPY_ZEROFILL CODE */

#if defined(__has_include)
#if __has_include(<mach/mig_strncpy_zerofill_support.h>)
#ifndef USING_MIG_STRNCPY_ZEROFILL
#define USING_MIG_STRNCPY_ZEROFILL
#endif
#ifndef __MIG_STRNCPY_ZEROFILL_FORWARD_TYPE_DECLS__
#define __MIG_STRNCPY_ZEROFILL_FORWARD_TYPE_DECLS__
#ifdef __cplusplus
extern "C" {
#endif
	extern int mig_strncpy_zerofill(char *dest, const char *src, int len) __attribute__((weak_import));
#ifdef __cplusplus
}
#endif
#endif /* __MIG_STRNCPY_ZEROFILL_FORWARD_TYPE_DECLS__ */
#endif /* __has_include(<mach/mig_strncpy_zerofill_support.h>) */
#endif /* __has_include */
	
/* END MIG_STRNCPY_ZEROFILL CODE */


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

#ifndef	task_restartable_MSG_COUNT
#define	task_restartable_MSG_COUNT	2
#endif	/* task_restartable_MSG_COUNT */

#include <mach/std_types.h>
#include <mach/mig.h>
#include <mach/mig.h>
#include <mach/mach_types.h>
#include <mach_debug/mach_debug_types.h>
#include <kern/restartable.h>

#ifdef __BeforeMigUserHeader
__BeforeMigUserHeader
#endif /* __BeforeMigUserHeader */

#include <sys/cdefs.h>
__BEGIN_DECLS


/* Routine task_restartable_ranges_register */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_restartable_ranges_register
(
	task_t target_task,
	task_restartable_range_array_t ranges,
	mach_msg_type_number_t rangesCnt
);

/* Routine task_restartable_ranges_synchronize */
#ifdef	mig_external
mig_external
#else
extern
#endif	/* mig_external */
kern_return_t task_restartable_ranges_synchronize
(
	task_t target_task
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

#ifndef __Request__task_restartable_subsystem__defined
#define __Request__task_restartable_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		mach_msg_type_number_t rangesCnt;
		task_restartable_range_t ranges[64];
	} __Request__task_restartable_ranges_register_t __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
	} __Request__task_restartable_ranges_synchronize_t __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif
#endif /* !__Request__task_restartable_subsystem__defined */

/* union of all requests */

#ifndef __RequestUnion__task_restartable_subsystem__defined
#define __RequestUnion__task_restartable_subsystem__defined
union __RequestUnion__task_restartable_subsystem {
	__Request__task_restartable_ranges_register_t Request_task_restartable_ranges_register;
	__Request__task_restartable_ranges_synchronize_t Request_task_restartable_ranges_synchronize;
};
#endif /* !__RequestUnion__task_restartable_subsystem__defined */
/* typedefs for all replies */

#ifndef __Reply__task_restartable_subsystem__defined
#define __Reply__task_restartable_subsystem__defined

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_restartable_ranges_register_t __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif

#ifdef  __MigPackStructs
#pragma pack(push, 4)
#endif
	typedef struct {
		mach_msg_header_t Head;
		NDR_record_t NDR;
		kern_return_t RetCode;
	} __Reply__task_restartable_ranges_synchronize_t __attribute__((unused));
#ifdef  __MigPackStructs
#pragma pack(pop)
#endif
#endif /* !__Reply__task_restartable_subsystem__defined */

/* union of all replies */

#ifndef __ReplyUnion__task_restartable_subsystem__defined
#define __ReplyUnion__task_restartable_subsystem__defined
union __ReplyUnion__task_restartable_subsystem {
	__Reply__task_restartable_ranges_register_t Reply_task_restartable_ranges_register;
	__Reply__task_restartable_ranges_synchronize_t Reply_task_restartable_ranges_synchronize;
};
#endif /* !__RequestUnion__task_restartable_subsystem__defined */

#ifndef subsystem_to_name_map_task_restartable
#define subsystem_to_name_map_task_restartable \
    { "task_restartable_ranges_register", 8000 },\
    { "task_restartable_ranges_synchronize", 8001 }
#endif

#ifdef __AfterMigUserHeader
__AfterMigUserHeader
#endif /* __AfterMigUserHeader */

#endif	 /* _task_restartable_user_ */
