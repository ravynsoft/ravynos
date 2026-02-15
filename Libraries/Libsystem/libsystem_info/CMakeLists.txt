add_library(libsystem_info SHARED
    copyHeaders
	./dns.subproj/herror.c
	./dns.subproj/res_comp.c
	./dns.subproj/res_query.c
	./gen.subproj/getifaddrs.c
	./gen.subproj/if_indextoname.c
	./gen.subproj/if_nameindex.c
	./gen.subproj/if_nametoindex.c
	./gen.subproj/ip6opt.c
	./gen.subproj/map_v4v6.c
	./gen.subproj/rthdr.c
	./gen.subproj/vars.c
	./lookup.subproj/cache_module.c
	./lookup.subproj/ds_module.c
	./lookup.subproj/file_module.c
	./lookup.subproj/ils.c
	./lookup.subproj/kvbuf.c
	./lookup.subproj/libinfo.c
	./lookup.subproj/mdns_module.c
	./lookup.subproj/search_module.c
	./lookup.subproj/si_data.c
	./lookup.subproj/si_getaddrinfo.c
	./lookup.subproj/si_module.c
	./lookup.subproj/thread_data.c
	./membership.subproj/membership.c
	./nis.subproj/getdomainname.c
	./nis.subproj/innetgr.c
	./nis.subproj/setdomainname.c
	./nis.subproj/xdr_domainname.c
	./nis.subproj/xdr_keydat.c
	./nis.subproj/xdr_mapname.c
	./nis.subproj/xdr_peername.c
	./nis.subproj/xdr_valdat.c
	./nis.subproj/xdr_ypbind_binding.c
	./nis.subproj/xdr_ypbind_resp.c
	./nis.subproj/xdr_ypbind_resptype.c
	./nis.subproj/xdr_ypbind_setdom.c
	./nis.subproj/xdr_ypmaplist.c
	./nis.subproj/xdr_ypreq_key.c
	./nis.subproj/xdr_ypreq_nokey.c
	./nis.subproj/xdr_ypresp_all.c
	./nis.subproj/xdr_ypresp_key_val.c
	./nis.subproj/xdr_ypresp_maplist.c
	./nis.subproj/xdr_ypresp_master.c
	./nis.subproj/xdr_ypresp_order.c
	./nis.subproj/xdr_ypresp_val.c
	./nis.subproj/xdr_ypstat.c
	./nis.subproj/yp_all.c
	./nis.subproj/yp_bind.c
	./nis.subproj/yp_first.c
	./nis.subproj/yp_get_default_domain.c
	./nis.subproj/yp_maplist.c
	./nis.subproj/yp_master.c
	./nis.subproj/yp_order.c
	./nis.subproj/yperr_string.c
	./nis.subproj/ypmatch_cache.c
	./nis.subproj/yppasswdd_xdr.c
	./nis.subproj/ypprot_err.c
	./rpc.subproj/auth_none.c
	./rpc.subproj/auth_unix.c
	./rpc.subproj/authunix_prot.c
	./lookup.subproj/muser_module.c
	./rpc.subproj/bindresvport.c
	./rpc.subproj/clnt_generic.c
	./rpc.subproj/clnt_perror.c
	./rpc.subproj/clnt_raw.c
	./rpc.subproj/clnt_simple.c
	./rpc.subproj/clnt_tcp.c
	./rpc.subproj/clnt_udp.c
	./rpc.subproj/get_myaddress.c
	./lookup.subproj/si_compare.c
	./rpc.subproj/getrpcent.c
	./rpc.subproj/getrpcport.c
	./rpc.subproj/pmap_clnt.c
	./rpc.subproj/pmap_getmaps.c
	./rpc.subproj/pmap_getport.c
	./rpc.subproj/pmap_prot.c
	./rpc.subproj/pmap_prot2.c
	./rpc.subproj/pmap_rmt.c
	./rpc.subproj/pmap_wakeup.c
	./rpc.subproj/rpc_callmsg.c
	./rpc.subproj/rpc_commondata.c
	./rpc.subproj/rpc_dtablesize.c
	./rpc.subproj/rpc_prot.c
	./rpc.subproj/svc.c
	./rpc.subproj/svc_auth.c
	./rpc.subproj/svc_auth_unix.c
	./rpc.subproj/svc_raw.c
	./rpc.subproj/svc_run.c
	./rpc.subproj/svc_simple.c
	./rpc.subproj/svc_tcp.c
	./rpc.subproj/svc_udp.c
	./rpc.subproj/xdr.c
	./rpc.subproj/xdr_array.c
	./rpc.subproj/xdr_float.c
	./rpc.subproj/xdr_mem.c
	./rpc.subproj/xdr_rec.c
	./rpc.subproj/xdr_reference.c
	./rpc.subproj/xdr_sizeof.c
	./rpc.subproj/xdr_stdio.c
	./util.subproj/hton.c
	./util.subproj/rcmd.c
	./gen.subproj/ether_addr.c
	./gen.subproj/getifmaddrs.c
	./lookup.subproj/getnameinfo_link.c
	./gen.subproj/configuration_profile.c
)
set_target_properties(libsystem_info PROPERTIES
    OUTPUT_NAME "system_info" SUFFIX ".dylib" PREFIX "lib" NO_SONAME true)

set(CFLAGS -DPRIVATE -D__APPLE__ -fblocks -fno-common -DDISPATCH_USE_DTRACE=0 -DINET6
    -D__DARWIN_NON_CANCELABLE -DCONFIG_MAC -DSYNTH_ROOTFS # -DDS_AVAILABLE -- zoe 2/11/26 Put this back once we have DirectoryService
    --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${CMAKE_CURRENT_SOURCE_DIR}/Libinfo
    -I${CMAKE_CURRENT_SOURCE_DIR}/gen.subproj
    -I${CMAKE_CURRENT_SOURCE_DIR}/nis.subproj
    -I${CMAKE_CURRENT_SOURCE_DIR}/lookup.subproj
    -I${CMAKE_CURRENT_SOURCE_DIR}/membership.subproj
    -I${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch
    -I${CMAKE_CURRENT_SOURCE_DIR}/../libsystem_notify
    -I${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/private
    -I${CMAKE_CURRENT_SOURCE_DIR}/../libresolv
    -I${CMAKE_CURRENT_SOURCE_DIR}/../../openbsm
    -I${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared
    -I${ROOT_SOURCE_DIR}/BSD/usr.sbin/configd/dnsinfo
    -I${ROOT_SOURCE_DIR}/BSD/sbin/launchd
    -I${RAVYN_SDKROOT}/usr/include/arpa
    -I${RAVYN_SDKROOT}/usr/local/include/kernel
    -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
    -I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
)

set(MIG_FLAGS ${CFLAGS} -D__MigTypeCheck)

target_compile_options(libsystem_info PRIVATE
    ${CFLAGS} -Wno-int-conversion -Wno-implicit-int
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_link_options(libsystem_info PRIVATE
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -umbrella System
    -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system
    -Wl,-all_load -Wl,-current_version,${PROD_VERSION} -Wl,-compatibility_version,1
    -Wl,-install_name,/usr/lib/system/libsystem_info.dylib -lsystem_c -lxpc
    -lsystem_blocks -lsystem_malloc -lsystem_platform -lsystem_kernel
    -mmacos-version-min=${MACOS_VERSION_MIN} -Wl,-flat_namespace #-Wl,-undefined,warning
    )

add_dependencies(libsystem_info libsystem_darwin dispatch)
add_custom_command(TARGET libsystem_info POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_info> ${RAVYN_SDKROOT}/usr/lib/system/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libsystem_info> ${SYSROOT_DIR}/usr/lib/system/
)

add_custom_command(OUTPUT copyHeaders
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
        env DSTROOT=${RAVYN_SDKROOT} SRCROOT=${CMAKE_CURRENT_SOURCE_DIR}
            sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/install_files.sh)
