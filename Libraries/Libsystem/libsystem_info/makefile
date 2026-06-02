MK_WERROR = no
LIB = system_info
SRCS = 	./dns.subproj/herror.c \
	./dns.subproj/res_comp.c \
	./dns.subproj/res_query.c \
	./gen.subproj/getifaddrs.c \
	./gen.subproj/if_indextoname.c \
	./gen.subproj/if_nameindex.c \
	./gen.subproj/if_nametoindex.c \
	./gen.subproj/ip6opt.c \
	./gen.subproj/map_v4v6.c \
	./gen.subproj/rthdr.c \
	./gen.subproj/vars.c \
	./lookup.subproj/cache_module.c \
	./lookup.subproj/ds_module.c \
	./lookup.subproj/file_module.c \
	./lookup.subproj/ils.c \
	./lookup.subproj/kvbuf.c \
	./lookup.subproj/libinfo.c \
	./lookup.subproj/mdns_module.c \
	./lookup.subproj/search_module.c \
	./lookup.subproj/si_data.c \
	./lookup.subproj/si_getaddrinfo.c \
	./lookup.subproj/si_module.c \
	./lookup.subproj/thread_data.c \
	./membership.subproj/membership.c \
	./nis.subproj/getdomainname.c \
	./nis.subproj/innetgr.c \
	./nis.subproj/setdomainname.c \
	./nis.subproj/xdr_domainname.c \
	./nis.subproj/xdr_keydat.c \
	./nis.subproj/xdr_mapname.c \
	./nis.subproj/xdr_peername.c \
	./nis.subproj/xdr_valdat.c \
	./nis.subproj/xdr_ypbind_binding.c \
	./nis.subproj/xdr_ypbind_resp.c \
	./nis.subproj/xdr_ypbind_resptype.c \
	./nis.subproj/xdr_ypbind_setdom.c \
	./nis.subproj/xdr_ypmaplist.c \
	./nis.subproj/xdr_ypreq_key.c \
	./nis.subproj/xdr_ypreq_nokey.c \
	./nis.subproj/xdr_ypresp_all.c \
	./nis.subproj/xdr_ypresp_key_val.c \
	./nis.subproj/xdr_ypresp_maplist.c \
	./nis.subproj/xdr_ypresp_master.c \
	./nis.subproj/xdr_ypresp_order.c \
	./nis.subproj/xdr_ypresp_val.c \
	./nis.subproj/xdr_ypstat.c \
	./nis.subproj/yp_all.c \
	./nis.subproj/yp_bind.c \
	./nis.subproj/yp_first.c \
	./nis.subproj/yp_get_default_domain.c \
	./nis.subproj/yp_maplist.c \
	./nis.subproj/yp_master.c \
	./nis.subproj/yp_order.c \
	./nis.subproj/yperr_string.c \
	./nis.subproj/ypmatch_cache.c \
	./nis.subproj/yppasswdd_xdr.c \
	./nis.subproj/ypprot_err.c \
	./rpc.subproj/auth_none.c \
	./rpc.subproj/auth_unix.c \
	./rpc.subproj/authunix_prot.c \
	./lookup.subproj/muser_module.c \
	./rpc.subproj/bindresvport.c \
	./rpc.subproj/clnt_generic.c \
	./rpc.subproj/clnt_perror.c \
	./rpc.subproj/clnt_raw.c \
	./rpc.subproj/clnt_simple.c \
	./rpc.subproj/clnt_tcp.c \
	./rpc.subproj/clnt_udp.c \
	./rpc.subproj/get_myaddress.c \
	./lookup.subproj/si_compare.c \
	./rpc.subproj/getrpcent.c \
	./rpc.subproj/getrpcport.c \
	./rpc.subproj/pmap_clnt.c \
	./rpc.subproj/pmap_getmaps.c \
	./rpc.subproj/pmap_getport.c \
	./rpc.subproj/pmap_prot.c \
	./rpc.subproj/pmap_prot2.c \
	./rpc.subproj/pmap_rmt.c \
	./rpc.subproj/pmap_wakeup.c \
	./rpc.subproj/rpc_callmsg.c \
	./rpc.subproj/rpc_commondata.c \
	./rpc.subproj/rpc_dtablesize.c \
	./rpc.subproj/rpc_prot.c \
	./rpc.subproj/svc.c \
	./rpc.subproj/svc_auth.c \
	./rpc.subproj/svc_auth_unix.c \
	./rpc.subproj/svc_raw.c \
	./rpc.subproj/svc_run.c \
	./rpc.subproj/svc_simple.c \
	./rpc.subproj/svc_tcp.c \
	./rpc.subproj/svc_udp.c \
	./rpc.subproj/xdr.c \
	./rpc.subproj/xdr_array.c \
	./rpc.subproj/xdr_float.c \
	./rpc.subproj/xdr_mem.c \
	./rpc.subproj/xdr_rec.c \
	./rpc.subproj/xdr_reference.c \
	./rpc.subproj/xdr_sizeof.c \
	./rpc.subproj/xdr_stdio.c \
	./util.subproj/hton.c \
	./util.subproj/rcmd.c \
	./gen.subproj/ether_addr.c \
	./gen.subproj/getifmaddrs.c \
	./lookup.subproj/getnameinfo_link.c \
	./gen.subproj/configuration_profile.c

.PATH: ${.OBJDIR}
SRCS += notify_ipcUser.c
SLF = /System/Library/Frameworks

# FIXME: zoe 2/10/26 - Should generate notify_probes.h from .d with dtrace

# -DDS_AVAILABLE -- zoe 2/11/26 Put this back once we have DirectoryService
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-DPRIVATE -D__APPLE__ -fblocks -fno-common -DDISPATCH_USE_DTRACE=0 \
	-DINET6 -D__DARWIN_NON_CANCELABLE -DCONFIG_MAC -DSYNTH_ROOTFS \
	-DTARGET_OS_BRIDGE=0 -Wno-int-conversion -Wno-implicit-int \
	-I${.CURDIR} \
	-I${.CURDIR}/Libinfo \
	-I${.CURDIR}/gen.subproj \
	-I${.CURDIR}/nis.subproj \
	-I${.CURDIR}/lookup.subproj \
	-I${.CURDIR}/membership.subproj \
	-I${.CURDIR}/../liblaunch \
	-I${.CURDIR}/../libsystem_notify \
	-I${.CURDIR}/../libdispatch/private \
	-I${.CURDIR}/../libresolv \
	-I${.CURDIR}/../../openbsm \
	-I${.CURDIR}/../libsystem_c/locale \
	-I${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared \
	-I${ROOT_SOURCE_DIR}/BSD/usr.sbin/configd/dnsinfo \
	-I${ROOT_SOURCE_DIR}/BSD/sbin/launchd \
	-I${RAVYN_SDKROOT}/usr/include/arpa \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders

MIG_FLAGS = ${CFLAGS} -D__MigTypeCheck

LDFLAGS = -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -umbrella System \
    -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system \
    -Wl,-current_version,${PROD_VERSION} -Wl,-compatibility_version,1 \
    -Wl,-install_name,/usr/lib/system/libsystem_info.dylib -lsystem_c -lxpc \
    -lsystem_blocks -lsystem_malloc -lsystem_platform -lsystem_kernel \
    -lsystem_trace -lsystem_dnssd -lsystem_notify -ldispatch -ldyld \
    -lsystem_pthread -mmacos-version-min=${MACOS_VERSION_MIN}


all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/notify_ipc.h ${.OBJDIR}/notify_ipcUser.c \
${.OBJDIR}/notify_ipcServer.c: ${.CURDIR}/../libsystem_notify/notify_ipc.defs
	${MIG} ${MIG_FLAGS} -novouchers -header notify_ipc.h \
		${.CURDIR}/../libsystem_notify/notify_ipc.defs

copyheaders:
	cd ${.CURDIR}; DSTROOT=${RAVYN_SDKROOT} SRCROOT=${.CURDIR} \
		sh ./xcodescripts/install_files.sh

lib${LIB}.a: copyheaders

${.OBJDIR}/lib${LIB}.dylib: ${.OBJDIR}/notify_ipcUser.c lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
		-Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>
