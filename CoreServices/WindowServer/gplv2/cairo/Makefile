MK_AUTO_OBJ=    yes
MK_WERROR=	no
WARNS=	        1
LIB=		cairo
SHLIB_NAME=	cairo
PACKAGE= 	cairo

build: ${LIB}

.PATH: ${.CURDIR}/src
SRCS= \
	cairo-analysis-surface.c \
	cairo-arc.c \
	cairo-array.c \
	cairo-atomic.c \
	cairo-base64-stream.c \
	cairo-base85-stream.c \
	cairo-bentley-ottmann-rectangular.c \
	cairo-bentley-ottmann-rectilinear.c \
	cairo-bentley-ottmann.c \
	cairo-botor-scan-converter.c \
	cairo-boxes-intersect.c \
	cairo-boxes.c \
	cairo-cache.c \
	cairo-clip-boxes.c \
	cairo-clip-polygon.c \
	cairo-clip-region.c \
	cairo-clip-surface.c \
	cairo-clip-tor-scan-converter.c \
	cairo-clip.c \
	cairo-color.c \
	cairo-composite-rectangles.c \
	cairo-compositor.c \
	cairo-contour.c \
	cairo-damage.c \
	cairo-debug.c \
	cairo-default-context.c \
	cairo-device.c \
	cairo-error.c \
	cairo-fallback-compositor.c \
	cairo-fixed.c \
	cairo-font-face-twin-data.c \
	cairo-font-face-twin.c \
	cairo-font-face.c \
	cairo-font-options.c \
	cairo-freed-pool.c \
	cairo-freelist.c \
	cairo-gstate.c \
	cairo-hash.c \
	cairo-hull.c \
	cairo-image-compositor.c \
	cairo-image-info.c \
	cairo-image-source.c \
	cairo-image-surface.c \
	cairo-line.c \
	cairo-lzw.c \
	cairo-mask-compositor.c \
	cairo-matrix.c \
	cairo-mempool.c \
	cairo-mesh-pattern-rasterizer.c \
	cairo-misc.c \
	cairo-mono-scan-converter.c \
	cairo-mutex.c \
	cairo-no-compositor.c \
	cairo-observer.c \
	cairo-output-stream.c \
	cairo-paginated-surface.c \
	cairo-path-bounds.c \
	cairo-path-fill.c \
	cairo-path-fixed.c \
	cairo-path-in-fill.c \
	cairo-path-stroke-boxes.c \
	cairo-path-stroke-polygon.c \
	cairo-path-stroke-traps.c \
	cairo-path-stroke-tristrip.c \
	cairo-path-stroke.c \
	cairo-path.c \
	cairo-pattern.c \
	cairo-pen.c \
	cairo-polygon-intersect.c \
	cairo-polygon-reduce.c \
	cairo-polygon.c \
	cairo-raster-source-pattern.c \
	cairo-recording-surface.c \
	cairo-rectangle.c \
	cairo-rectangular-scan-converter.c \
	cairo-region.c \
	cairo-rtree.c \
	cairo-scaled-font.c \
	cairo-shape-mask-compositor.c \
	cairo-slope.c \
	cairo-spans-compositor.c \
	cairo-spans.c \
	cairo-spline.c \
	cairo-stroke-dash.c \
	cairo-stroke-style.c \
	cairo-surface-clipper.c \
	cairo-surface-fallback.c \
	cairo-surface-observer.c \
	cairo-surface-offset.c \
	cairo-surface-snapshot.c \
	cairo-surface-subsurface.c \
	cairo-surface-wrapper.c \
	cairo-surface.c \
	cairo-time.c \
	cairo-tor-scan-converter.c \
	cairo-tor22-scan-converter.c \
	cairo-toy-font-face.c \
	cairo-traps-compositor.c \
	cairo-traps.c \
	cairo-tristrip.c \
	cairo-unicode.c \
	cairo-user-font.c \
	cairo-version.c \
	cairo-wideint.c \
	cairo.c \
	cairo-cff-subset.c \
	cairo-scaled-font-subsets.c \
	cairo-truetype-subset.c \
	cairo-type1-fallback.c \
	cairo-type1-glyph-names.c \
	cairo-type1-subset.c \
	cairo-type3-glyph-surface.c \
	cairo-pdf-operators.c \
	cairo-pdf-shading.c \
	cairo-tag-attributes.c \
	cairo-tag-stack.c \
	cairo-deflate-stream.c \
	cairo-png.c \
	cairo-svg-surface.c \
	cairo-script-surface.c \
	cairo-ft-font.c \
	cairo-colr-glyph-render.c \
	cairo-svg-glyph-render.c \
	cairo-ps-surface.c \
	cairo-pdf-surface.c \
	cairo-pdf-interchange.c

INCS= \
	cairo.h \
	cairo-version.h \
	cairo-deprecated.h \
	cairo-ps.h \
	cairo-pdf.h \
	cairo-svg.h \
	cairo-ft.h

CFLAGS+= --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
	 -I${.CURDIR} -DCAIRO_COMPILATION -D_GNU_SOURCE \
	 -D_REENTRANT -pthread -DCAIRO_HAS_DLSYM=1 -DHAVE_ZLIB=1 \
	 -DCAIRO_HAS_SVG_SURFACE=1 -DCAIRO_HAS_PNG_FUNCTIONS=1 \
 	 -DCAIRO_HAS_FC_FONT=1 -DCAIRO_HAS_FT_FONT=1 \
	 -DCAIRO_HAS_PS_SURFACE=1 -DCAIRO_HAS_PDF_SURFACE=1 \
	 -DCAIRO_HAS_SCRIPT_SURFACE=1 -DCAIRO_HAS_INTERPRETER=1 \
	 -DCAIRO_HAS_IMAGE_SURFACE=1 -DHAS_PIXMAN_GLYPHS=1 \
	 -DCAIRO_HAS_USER_FONT=1 -DCAIRO_HAS_MIME_SURFACE=1 \
	 -DCAIRO_HAS_RECORDING_SURFACE=1 -DHAVE_STRNDUP=1 \
	 -DCAIRO_HAS_OBSERVER_SURFACE=1 -DCAIRO_HAS_PTHREAD=1 \
	 -DHAVE_CXX11_ATOMIC_PRIMITIVES=1 -DHAVE_SYS_STAT_H=1 \
	 -DHAVE_MKDIR=1 -DHAVE_ALARM -DHAVE_BUILTIN_CLZ \
         -DHAVE_FEDIVBYZERO -DHAVE_FEENABLEEXCEPT -DHAVE_FENV_H \
         -DHAVE_GCC_VECTOR_EXTENSIONS -DHAVE_GETPAGESIZE \
         -DHAVE_GETTIMEOFDAY -DHAVE_MMAP -DHAVE_MPROTECT \
         -DHAVE_POSIX_MEMALIGN -DHAVE_PTHREADS -DHAVE_SIGACTION \
         -DHAVE_SYS_MMAN_H -DHAVE_UNISTD_H -DHAVE_STDINT_H \
         -DTOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR -DHAVE_SIGNAL_H \
         -DTLS=__thread -DASM_LEADING_UNDERSCORE -DHAVE_FLOAT128 \
	 -DSIZEOF_LONG=8 -DSIZEOF_VOID_P=8 -DSIZEOF_INT=4 \
	 -DSIZEOF_LONG_LONG=8 -DSIZEOF_SIZE_T=8 -DHAVE_CTIME_R \
	 -DHAVE_SYS_WAIT_H -DHAVE_VASPRINTF -DHAVE_DLFCN_H \
	 -DHAVE_FCNTL_H -DHAVE_SYS_SOCKET_H -DHAVE_SYS_UN_H \
	 -DHAVE_DLSYM -DHAVE_UINT64_T -DHAVE_FORK -DHAVE_WAITPID \
	 -DHAVE_LINK -DHAVE_BYTESWAP_H -DCAIRO_FEATURES_H \
	 -I${MAKEOBJDIR} \
	 -I${SRCTOP}/Frameworks/AppKit/fontconfig \
	 -I${SRCTOP}/Frameworks/Onyx2D/freetype/include
LDFLAGS+= -Wl,-Bsymbolic-functions -L${OBJTOP}/tmp/usr/lib \
	  -L${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework/Resources \
	  -L${OBJTOP}/Frameworks/AppKit/fontconfig/src/.libs \
	  -lpng -lfontconfig -lfreetype -lpixman-1 -lpthread -ldl -lz \
	  -Wl,-R/System/Library/Frameworks/AppKit.framework/Resources \
	  -Wl,-R/System/Library/Frameworks/Onyx2D.framework/Resources

cairo-features.h:
	echo >${.TARGET}

config.h:
	echo >${.TARGET}

make-obj-dirs:
	mkdir -p ${MAKEOBJDIR}
.for subdir in ${SUBDIR}
	mkdir -p ${MAKEOBJDIR}/${subdir}
.endfor

.include <bsd.lib.mk>

${OBJS}: config.h cairo-features.h

