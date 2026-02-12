execute_process(
    COMMAND env SRCROOT=${CMAKE_CURRENT_SOURCE_DIR}
                DSTROOT=${RAVYN_SDKROOT}
                OBJROOT=${CMAKE_CURRENT_BINARY_DIR}
                RPCGEN=${TOOLS}/rpcgen
                sh ${CMAKE_CURRENT_SOURCE_DIR}/xcodescripts/install_rpcsvc.sh)