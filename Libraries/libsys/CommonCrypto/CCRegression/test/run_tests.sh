#!/bin/sh

# run_tests.sh
# Security
#
# Created by Fabrice Gautier on 8/26/10.
# Modified for use with CommonCrypto on 3/10/11
# Copyright 2011 Apple, Inc. All rights reserved.


# Run a command line tool on the sim or the device

CMD=CCRegressions

if [ "${PLATFORM_NAME}" == "iphoneos" ]; then
    INSTALL_DIR=
    SCP_URL=phone:${INSTALL_DIR}
#copy libcommonCrypto.dylib and CCRegressions program to the device, to /tmp
#this will not replace the system CommonCrypto.
#it is assumed that ssh is pre-setup on the device with pubkey authentication
    export RSYNC_PASSWORD=alpine
    echo "run_tests.sh:${LINENO}: note: Copying stuff to device"
    # scp ${CONFIGURATION_BUILD_DIR}/libcommonCrypto.dylib ${SCP_URL}
    scp ${CONFIGURATION_BUILD_DIR}/CCRegressions ${SCP_URL}
    echo "run_tests.sh:${LINENO}: note: Running the test"
    xcrun -sdk "$SDKROOT" PurpleExec --env "DYLD_FRAMEWORK_PATH=${INSTALL_DIR}" --cmd ${INSTALL_DIR}/${CMD}
else
    echo "run_tests.sh:${LINENO}: note: Running test on simulator (${BUILT_PRODUCTS_DIR}/${CMD})"
	export DYLD_ROOT_PATH="${SDKROOT}"
    export DYLD_LIBRARY_PATH="${BUILT_PRODUCTS_DIR}"
    export DYLD_FRAMEWORK_PATH="${BUILT_PRODUCTS_DIR}"
    ${BUILT_PRODUCTS_DIR}/${CMD}
fi


