#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting
set -e

# If run outside of a deqp-runner invoction (e.g. piglit trace replay), then act
# the same as the first thread in its threadpool.
THREAD=${DEQP_RUNNER_THREAD:-0}

#
# Helper to generate CIDs for virtio-vsock based communication with processes
# running inside crosvm guests.
#
# A CID is a 32-bit Context Identifier to be assigned to a crosvm instance
# and must be unique across the host system. For this purpose, let's take
# the least significant 25 bits from CI_JOB_ID as a base and generate a 7-bit
# prefix number to handle up to 128 concurrent crosvm instances per job runner.
#
# As a result, the following variables are set:
#  - VSOCK_CID: the crosvm unique CID to be passed as a run argument
#
#  - VSOCK_STDOUT, VSOCK_STDERR: the port numbers the guest should accept
#    vsock connections on in order to transfer output messages
#
#  - VM_TEMP_DIR: the temporary directory path used to pass additional
#    context data towards the guest
#
set_vsock_context() {
    [ -n "${CI_JOB_ID}" ] || {
        echo "Missing or unset CI_JOB_ID env variable" >&2
        exit 1
    }

    VM_TEMP_DIR="/tmp-vm.${THREAD}"
    # Clear out any leftover files from a previous run.
    rm -rf $VM_TEMP_DIR
    mkdir $VM_TEMP_DIR || return 1

    VSOCK_CID=$(((CI_JOB_ID & 0x1ffffff) | ((THREAD & 0x7f) << 25)))
    VSOCK_STDOUT=5001
    VSOCK_STDERR=5002

    return 0
}

# The dEQP binary needs to run from the directory it's in
if [ -n "${1##*.sh}" ] && [ -z "${1##*"deqp"*}" ]; then
    DEQP_BIN_DIR=$(dirname "$1")
    export DEQP_BIN_DIR
fi

VM_SOCKET=crosvm-${THREAD}.sock

# Terminate any existing crosvm, if a previous invocation of this shell script
# was terminated due to timeouts.  This "vm stop" may fail if the crosvm died
# without cleaning itself up.
if [ -e $VM_SOCKET ]; then
   crosvm stop $VM_SOCKET || true
   # Wait for socats from that invocation to drain
   sleep 5
   rm -rf $VM_SOCKET || true
fi

set_vsock_context || { echo "Could not generate crosvm vsock CID" >&2; exit 1; }

# Securely pass the current variables to the crosvm environment
echo "Variables passed through:"
SCRIPT_DIR=$(readlink -en "${0%/*}")
${SCRIPT_DIR}/common/generate-env.sh | tee ${VM_TEMP_DIR}/crosvm-env.sh
cp ${SCRIPT_DIR}/setup-test-env.sh ${VM_TEMP_DIR}/setup-test-env.sh

# Set the crosvm-script as the arguments of the current script
echo ". ${VM_TEMP_DIR}/setup-test-env.sh" > ${VM_TEMP_DIR}/crosvm-script.sh
echo "$@" >> ${VM_TEMP_DIR}/crosvm-script.sh

# Setup networking
/usr/sbin/iptables-legacy -w -t nat -A POSTROUTING -o eth0 -j MASQUERADE
echo 1 > /proc/sys/net/ipv4/ip_forward

# Start background processes to receive output from guest
socat -u vsock-connect:${VSOCK_CID}:${VSOCK_STDERR},retry=200,interval=0.1 stderr &
socat -u vsock-connect:${VSOCK_CID}:${VSOCK_STDOUT},retry=200,interval=0.1 stdout &

# Prepare to start crosvm
unset DISPLAY
unset XDG_RUNTIME_DIR

CROSVM_KERN_ARGS="quiet console=null root=my_root rw rootfstype=virtiofs ip=192.168.30.2::192.168.30.1:255.255.255.0:crosvm:eth0"
CROSVM_KERN_ARGS="${CROSVM_KERN_ARGS} init=${SCRIPT_DIR}/crosvm-init.sh -- ${VSOCK_STDOUT} ${VSOCK_STDERR} ${VM_TEMP_DIR}"

[ "${CROSVM_GALLIUM_DRIVER}" = "llvmpipe" ] && \
    CROSVM_LIBGL_ALWAYS_SOFTWARE=true || CROSVM_LIBGL_ALWAYS_SOFTWARE=false

set +e -x

# We aren't testing the host driver here, so we don't need to validate NIR on the host
NIR_DEBUG="novalidate" \
LIBGL_ALWAYS_SOFTWARE=${CROSVM_LIBGL_ALWAYS_SOFTWARE} \
GALLIUM_DRIVER=${CROSVM_GALLIUM_DRIVER} \
VK_ICD_FILENAMES=$CI_PROJECT_DIR/install/share/vulkan/icd.d/${CROSVM_VK_DRIVER}_icd.x86_64.json \
crosvm --no-syslog run \
    --gpu "${CROSVM_GPU_ARGS}" --gpu-render-server "path=/usr/local/libexec/virgl_render_server" \
    -m "${CROSVM_MEMORY:-4096}" -c "${CROSVM_CPU:-2}" --disable-sandbox \
    --shared-dir /:my_root:type=fs:writeback=true:timeout=60:cache=always \
    --net "host-ip=192.168.30.1,netmask=255.255.255.0,mac=AA:BB:CC:00:00:12" \
    -s $VM_SOCKET \
    --cid ${VSOCK_CID} -p "${CROSVM_KERN_ARGS}" \
    /lava-files/${KERNEL_IMAGE_NAME:-bzImage} > ${VM_TEMP_DIR}/crosvm 2>&1

CROSVM_RET=$?

[ ${CROSVM_RET} -eq 0 ] && {
    # The actual return code is the crosvm guest script's exit code
    CROSVM_RET=$(cat ${VM_TEMP_DIR}/exit_code 2>/dev/null)
    # Force error when the guest script's exit code is not available
    CROSVM_RET=${CROSVM_RET:-1}
}

# Show crosvm output on error to help with debugging
[ ${CROSVM_RET} -eq 0 ] || {
    set +x
    echo "Dumping crosvm output.." >&2
    cat ${VM_TEMP_DIR}/crosvm >&2
    set -x
}

exit ${CROSVM_RET}
