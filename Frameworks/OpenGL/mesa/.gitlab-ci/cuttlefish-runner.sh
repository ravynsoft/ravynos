#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

section_start cuttlefish_setup "cuttlefish: setup"
set -xe

export HOME=/cuttlefish
export PATH=$PATH:/cuttlefish/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${CI_PROJECT_DIR}/install/lib/:/cuttlefish/lib64
export EGL_PLATFORM=surfaceless

syslogd

chown root.kvm /dev/kvm

/etc/init.d/cuttlefish-host-resources start

cd /cuttlefish

launch_cvd --verbosity=DEBUG --report_anonymous_usage_stats=n --cpus=8 --memory_mb=8192 --gpu_mode="$ANDROID_GPU_MODE" --daemon --enable_minimal_mode=true --guest_enforce_security=false --use_overlay=false
sleep 1

cd -

adb connect vsock:3:5555
ADB="adb -s vsock:3:5555"

$ADB root
sleep 1
$ADB shell echo Hi from Android
# shellcheck disable=SC2035
$ADB logcat dEQP:D *:S &

# overlay vendor

OV_TMPFS="/data/overlay-remount"
$ADB shell mkdir -p "$OV_TMPFS"
$ADB shell mount -t tmpfs none "$OV_TMPFS"

$ADB shell mkdir -p "$OV_TMPFS/vendor-upper"
$ADB shell mkdir -p "$OV_TMPFS/vendor-work"

opts="lowerdir=/vendor,upperdir=$OV_TMPFS/vendor-upper,workdir=$OV_TMPFS/vendor-work"
$ADB shell mount -t overlay -o "$opts" none /vendor

$ADB shell setenforce 0

# deqp

$ADB push /deqp/modules/egl/deqp-egl-android /data/.
$ADB push /deqp/assets/gl_cts/data/mustpass/egl/aosp_mustpass/3.2.6.x/egl-master.txt /data/.
$ADB push /deqp-runner/deqp-runner /data/.

# download Android Mesa from S3
MESA_ANDROID_ARTIFACT_URL=https://${PIPELINE_ARTIFACTS_BASE}/${S3_ARTIFACT_NAME}.tar.zst
curl -L --retry 4 -f --retry-all-errors --retry-delay 60 -o ${S3_ARTIFACT_NAME}.tar.zst ${MESA_ANDROID_ARTIFACT_URL}
tar -xvf ${S3_ARTIFACT_NAME}.tar.zst
rm "${S3_ARTIFACT_NAME}.tar.zst" &

$ADB push install/all-skips.txt /data/.
$ADB push install/$GPU_VERSION-flakes.txt /data/.
$ADB push install/deqp-$DEQP_SUITE.toml /data/.

# remove 32 bits libs from /vendor/lib

$ADB shell rm /vendor/lib/dri/${ANDROID_DRIVER}_dri.so
$ADB shell rm /vendor/lib/libglapi.so
$ADB shell rm /vendor/lib/egl/libGLES_mesa.so

$ADB shell rm /vendor/lib/egl/libEGL_angle.so
$ADB shell rm /vendor/lib/egl/libEGL_emulation.so
$ADB shell rm /vendor/lib/egl/libGLESv1_CM_angle.so
$ADB shell rm /vendor/lib/egl/libGLESv1_CM_emulation.so
$ADB shell rm /vendor/lib/egl/libGLESv2_angle.so
$ADB shell rm /vendor/lib/egl/libGLESv2_emulation.so

# replace on /vendor/lib64

$ADB push install/lib/dri/${ANDROID_DRIVER}_dri.so /vendor/lib64/dri/${ANDROID_DRIVER}_dri.so
$ADB push install/lib/libglapi.so /vendor/lib64/libglapi.so
$ADB push install/lib/libEGL.so /vendor/lib64/egl/libEGL_mesa.so

$ADB shell rm /vendor/lib64/egl/libEGL_angle.so
$ADB shell rm /vendor/lib64/egl/libEGL_emulation.so
$ADB shell rm /vendor/lib64/egl/libGLESv1_CM_angle.so
$ADB shell rm /vendor/lib64/egl/libGLESv1_CM_emulation.so
$ADB shell rm /vendor/lib64/egl/libGLESv2_angle.so
$ADB shell rm /vendor/lib64/egl/libGLESv2_emulation.so


RESULTS=/data/results
uncollapsed_section_switch cuttlefish_test "cuttlefish: testing"

set +e
$ADB shell "mkdir /data/results; cd /data; ./deqp-runner \
    suite \
    --suite /data/deqp-$DEQP_SUITE.toml \
    --output $RESULTS \
    --skips /data/all-skips.txt $DEQP_SKIPS \
    --flakes /data/$GPU_VERSION-flakes.txt \
    --testlog-to-xml /deqp/executor/testlog-to-xml \
    --fraction-start $CI_NODE_INDEX \
    --fraction $(( CI_NODE_TOTAL * ${DEQP_FRACTION:-1})) \
    --jobs ${FDO_CI_CONCURRENT:-4} \
    $DEQP_RUNNER_OPTIONS"

EXIT_CODE=$?
set -e
section_switch cuttlefish_results "cuttlefish: gathering the results"

$ADB pull $RESULTS results

cp /cuttlefish/cuttlefish/instances/cvd-1/logs/logcat results
cp /cuttlefish/cuttlefish/instances/cvd-1/kernel.log results
cp /cuttlefish/cuttlefish/instances/cvd-1/logs/launcher.log results

section_end cuttlefish_results
exit $EXIT_CODE
