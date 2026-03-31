add_library(CrashReporterClient STATIC CRClient.c CRClient.cpp)
set_target_properties(CrashReporterClient PROPERTIES
    OUTPUT_NAME "CrashReporterClient" PREFIX "lib" SUFFIX ".a")
target_compile_options(CrashReporterClient PRIVATE --sysroot=${RAVYN_SDKROOT})
add_custom_command(TARGET CrashReporterClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CrashReporterClient>
        ${RAVYN_SDKROOT}/usr/lib/system/)
add_dependencies(CrashReporterClient libkernel) # just for build ordering