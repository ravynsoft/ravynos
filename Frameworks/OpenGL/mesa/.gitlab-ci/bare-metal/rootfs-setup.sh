#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

rootfs_dst=$1

mkdir -p $rootfs_dst/results

# Set up the init script that brings up the system.
cp $BM/bm-init.sh $rootfs_dst/init
cp $CI_COMMON/init*.sh $rootfs_dst/

date +'%F %T'

# Make JWT token available as file in the bare-metal storage to enable access
# to MinIO
cp "${CI_JOB_JWT_FILE}" "${rootfs_dst}${CI_JOB_JWT_FILE}"

date +'%F %T'

cp $CI_COMMON/capture-devcoredump.sh $rootfs_dst/
cp $CI_COMMON/intel-gpu-freq.sh $rootfs_dst/
cp $CI_COMMON/kdl.sh $rootfs_dst/
cp "$SCRIPTS_DIR/setup-test-env.sh" "$rootfs_dst/"

set +x

# Pass through relevant env vars from the gitlab job to the baremetal init script
echo "Variables passed through:"
"$CI_COMMON"/generate-env.sh | tee $rootfs_dst/set-job-env-vars.sh

set -x

# Add the Mesa drivers we built, and make a consistent symlink to them.
mkdir -p $rootfs_dst/$CI_PROJECT_DIR
rsync -aH --delete $CI_PROJECT_DIR/install/ $rootfs_dst/$CI_PROJECT_DIR/install/

date +'%F %T'
