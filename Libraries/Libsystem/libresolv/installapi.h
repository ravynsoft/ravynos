#include <sys/cdefs.h>
#include <mach/mach.h>
#include <netdb.h>
#include <resolv.h>

// These are declared in Libinfo's headers.  These declarations should eventually move into headers provided by libresolv.

typedef void (*dns_async_callback)(int32_t status, char *buf, uint32_t len, struct sockaddr *from, int fromlen, void *context);
int32_t dns_async_start(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search, dns_async_callback callback, void *context);
int32_t dns_async_send(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search);
int32_t dns_async_receive(mach_port_t p, char **buf, uint32_t *len, struct sockaddr **from, uint32_t *fromlen);
int32_t dns_async_handle_reply(void *msg);
void dns_async_cancel(mach_port_t p);
