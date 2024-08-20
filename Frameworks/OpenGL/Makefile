FRAMEWORK=OpenGL
SUBDIR+=        libpciaccess
SUBDIR+=        libdrm 
SUBDIR+=        wayland .WAIT
SUBDIR+=        mesa

.include "../framework.common.mk"

MK_INSTALLLIB= no
SRCS= OpenGL.m
INCS= \
	include/CGLCurrent.h \
	include/CGLTypes.h \
	include/gl.h \
	include/glext.h \
	include/glplatform.h \
	include/OpenGL.h \
        include/egl.h \
        include/eglext.h \
        include/eglextchromium.h \
        include/eglexternalplatform.h \
        include/eglexternalplatformversion.h \
        include/eglmesaext.h \
        include/eglplatform.h \
        include/khrplatform.h
RESOURCES=      Info.plist

LINK_SUBDIR+= mesa/src/egl
LINK_SUBDIR+= mesa/src/mapi/es2api
LINK_SUBDIR+= mesa/src/mapi/shared-glapi
LINK_SUBDIR+= mesa/src/gbm
LINK_SUBDIR+= mesa/src/util
LINK_SUBDIR+= mesa/src/loader
LINK_SUBDIR+= mesa/src/compiler
LINK_SUBDIR+= mesa/src/mesa
LINK_SUBDIR+= mesa/src/gallium/drivers/softpipe
LINK_SUBDIR+= mesa/src/gallium/auxiliary/libgallium.a.p
LINK_SUBDIR+= mesa/src/gallium/auxiliary/libgalliumvl.a.p
LINK_SUBDIR+= mesa/src/gallium/auxiliary/libgalliumvlwinsys.a.p

#LINK_SUBDIR+= mesa/src/gallium/winsys/drm
#LINK_SUBDIR+= mesa/src/gallium/auxiliary/winsys
#LINK_SUBDIR+= mesa/src/virtio
#LINK_SUBDIR+= mesa/src/amd/common/libamd_common.a.p
#LINK_SUBDIR+= mesa/src/amd/addrlib
#LINK_SUBDIR+= mesa/src/amd/compiler
#LINK_SUBDIR+= mesa/src/amd/vpelib
#LINK_SUBDIR+= mesa/src/intel/blorp
#LINK_SUBDIR+= mesa/src/intel/common
#LINK_SUBDIR+= mesa/src/intel/compiler
#LINK_SUBDIR+= mesa/src/intel/dev
#LINK_SUBDIR+= mesa/src/intel/ds
#LINK_SUBDIR+= mesa/src/intel/genxml
#LINK_SUBDIR+= mesa/src/intel/isl/libisl.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver200.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver125.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver120.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver110.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver40.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver50.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver60.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver70.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver75.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver80.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_per_hw_ver90.a.p
#LINK_SUBDIR+= mesa/src/intel/isl/libisl_tiled_memcpy.a.p
#LINK_SUBDIR+= mesa/src/intel/perf
#LINK_SUBDIR+= mesa/src/gallium/auxiliary/drivers
#LINK_SUBDIR+= mesa/src/gallium/auxiliary/frontends/xa
#LINK_SUBDIR+= mesa/src/gallium/auxiliary/targets
#LINK_SUBDIR+= mesa/src/gallium/auxiliary/pipe-loader
#LINK_SUBDIR+= mesa/src/gallium/auxiliary/target-helpers
#LINK_SUBDIR+= mesa/src/gallium/drivers/crocus
#LINK_SUBDIR+= mesa/src/gallium/drivers/i915
#LINK_SUBDIR+= mesa/src/gallium/drivers/nouveau
#LINK_SUBDIR+= mesa/src/gallium/drivers/r300
#LINK_SUBDIR+= mesa/src/gallium/drivers/r600
#LINK_SUBDIR+= mesa/src/gallium/drivers/svga
#LINK_SUBDIR+= mesa/src/gallium/drivers/virgl
#LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx10.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx103.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx11.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx115.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx7.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx8.a.p
##LINK_SUBDIR+= mesa/src/gallium/drivers/radeonsi/libradeonsi_gfx9.a.p
#LINK_SUBDIR+= mesa/src/gallium/targets/dri
#LINK_SUBDIR+= mesa/src/gallium/frontends/dri/libdri.a.p

CFLAGS+= ${FMWK_CFLAGS} -I${SRCTOP}/Frameworks -I${.CURDIR}/include 
CFLAGS+= -I${SRCTOP}/Frameworks/Foundation/Headers
CFLAGS+= -I${.CURDIR}/libpciaccess/include
LDFLAGS+= -Llibdrm -Llibpciaccess -Lwayland/src \
	  -lwayland -ldrm -lpciaccess -lstdc++ -lstdthreads \
	  -Wl,--no-as-needed -Wl,-R\$$ORIGIN
LD_LIBRARY_PATH+= ${OBJTOP}/tmp/usr/lib
.export LD_LIBRARY_PATH

_libinstall_hook:
	echo ${FRAMEWORK} ${.TARGET}
.for i in \
          src/mapi/es1api/libGLESv1_CM.so.1 \
	  src/gallium/targets/dri/i915_dri.so \
	  src/gallium/targets/dri/iris_dri.so \
	  src/gallium/targets/dri/kms_swrast_dri.so \
	  src/gallium/targets/dri/libgallium_dri.so \
	  src/gallium/targets/dri/swrast_dri.so \
	  src/gallium/targets/dri/vmwgfx_dri.so
	rm -rf mesa/${i}*.p
	cp -Rfv mesa/${i}* ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
	ln -sfv ${i:T} ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/${i:T:R}
.endfor
	cp -fv libpciaccess/libpciaccess.so.0 ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
	cp -fv libOpenGL.so ${FRAMEWORK}.framework/Versions/Current/
.for inc in ${INCS}
	cp -fv ${.CURDIR}/${inc} ${FRAMEWORK}.framework/Headers/
.endfor


.include <rvn.framework.mk>

installresources: std_install_hook
_libinstall: _libinstall_hook
