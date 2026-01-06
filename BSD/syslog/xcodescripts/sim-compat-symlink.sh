#!/bin/bash -ex

if [[ "${PLATFORM_NAME}" =~ "simulator" ]]; then
	ln -s libsystem_asl.dylib ${DSTROOT}${INSTALL_PATH}/libsystem_sim_asl.dylib
fi

exit 0
