add_executable(host_rpcgen)
target_sources(host_rpcgen PRIVATE rpc_clntout.c rpc_cout.c rpc_hout.c rpc_main.c
    rpc_parse.c rpc_sample.c rpc_scan.c rpc_svcout.c rpc_tblout.c rpc_util.c)
target_compile_options(host_rpcgen PRIVATE "-D__dead2=" "-D__unused=")
target_include_directories(host_rpcgen PRIVATE ../include)