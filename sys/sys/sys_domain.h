#ifndef _SYSTEM_DOMAIN_H_
#define _SYSTEM_DOMAIN_H_

#include <sys/cdefs.h>
#include <sys/types.h>

#ifdef _KERNEL
#include <sys/sysctl.h> 
#endif /* _KERNEL */

/* Kernel Events Protocol */ 
#define SYSPROTO_EVENT          1       /* kernel events protocol */

/* Kernel Control Protocol */
#define SYSPROTO_CONTROL        2       /* kernel control protocol */
#define AF_SYS_CONTROL          2       /* corresponding sub address type */

/* System family socket address */
struct sockaddr_sys {
        u_char          ss_len;         /* sizeof(struct sockaddr_sys) */
        u_char          ss_family;      /* AF_SYSTEM */
        u_int16_t       ss_sysaddr;     /* protocol address in AF_SYSTEM */
        u_int32_t       ss_reserved[7]; /* reserved to the protocol use */
};

#ifdef PRIVATE
struct  xsystmgen {
        u_int32_t       xg_len; /* length of this structure */
        u_int32_t       xg_count;       /* number of PCBs at this time */
        u_int64_t       xg_gen; /* generation count at this time */
        u_int64_t       xg_sogen;       /* current socket generation count */
};
#endif /* PRIVATE */

#ifdef _KERNEL

extern struct domain *systemdomain;

SYSCTL_DECL(_net_systm);

/* built in system domain protocols init function */
__BEGIN_DECLS
void kern_event_init(struct domain *);
void kern_control_init(struct domain *);
__END_DECLS
#endif /* _KERNEL */

#endif /* _SYSTEM_DOMAIN_H_ */
