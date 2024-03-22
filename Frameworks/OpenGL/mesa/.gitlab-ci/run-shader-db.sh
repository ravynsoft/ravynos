#!/usr/bin/env bash
set -e

ARTIFACTSDIR=$(pwd)/shader-db
mkdir -p "$ARTIFACTSDIR"
export DRM_SHIM_DEBUG=true

LIBDIR=$(pwd)/install/lib
export LD_LIBRARY_PATH=$LIBDIR

cd /usr/local/shader-db

for driver in freedreno intel v3d vc4; do
    section_start shader-db-${driver} "Running shader-db for $driver"
    env LD_PRELOAD="$LIBDIR/lib${driver}_noop_drm_shim.so" \
        ./run -j"${FDO_CI_CONCURRENT:-4}" ./shaders \
            > "$ARTIFACTSDIR/${driver}-shader-db.txt"
    section_end shader-db-${driver}
done

# Run shader-db over a number of supported chipsets for nouveau
#for chipset in 40 a3 c0 e4 f0 134 162; do
#    section_start shader-db-nouveau-${chipset} "Running shader-db for nouveau - ${chipset}"
#    env LD_PRELOAD="$LIBDIR/libnouveau_noop_drm_shim.so" \
#        NOUVEAU_CHIPSET=${chipset} \
#        ./run -j"${FDO_CI_CONCURRENT:-4}" ./shaders \
#            > "$ARTIFACTSDIR/nouveau-${chipset}-shader-db.txt"
#    section_end shader-db-nouveau-${chipset}
#done

# Run shader-db for r300 (RV370 and RV515)
for chipset in 0x5460 0x7140; do
    section_start shader-db-r300-${chipset} "Running shader-db for r300 - ${chipset}"
    env LD_PRELOAD="$LIBDIR/libradeon_noop_drm_shim.so" \
        RADEON_GPU_ID=${chipset} \
        ./run -j"${FDO_CI_CONCURRENT:-4}" -o r300 ./shaders \
            > "$ARTIFACTSDIR/r300-${chipset}-shader-db.txt"
    section_end shader-db-r300-${chipset}
done
