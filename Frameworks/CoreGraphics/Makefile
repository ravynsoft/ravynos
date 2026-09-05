SUBDIR =
MK_WERROR = no
FRAMEWORK=CoreGraphics

SRCS= \
	CGAffineTransform.m \
	CGBitmapContext.m \
	CGColor.m \
	CGColorSpace.m \
	CGContext.m \
	CGDataConsumer.m \
	CGDataProvider.m \
	CGDirectDisplay.m \
	CGEvent.m \
	CGFont.m \
	CGFunction.m \
	CGGeometry.m \
	CGGradient.m \
	CGImage.m \
	CGImageDestination.m \
	CGImageProperties.m \
	CGImageSource.m \
	CGLPixelSurface.m \
	CGLayer.m \
	CGPDFContext.m \
	CGPDFDocument.m \
	CGPDFPage.m \
	CGPath.m \
	CGPattern.m \
	CGShading.m

HEADERS= \
	CGAffineTransform.h \
	CGBitmapContext.h \
	CGColor.h \
	CGColorSpace.h \
	CGContext.h \
	CGConversions.h \
	CGDataConsumer.h \
	CGDataProvider.h \
	CGDirectDisplay.h \
	CGError.h \
	CGEvent.h \
	CGFont.h \
	CGFunction.h \
	CGGeometry.h \
	CGGradient.h \
	CGImage.h \
	CGImageDestination.h \
	CGImageProperties.h \
	CGImageSource.h \
	CGLPixelSurface.h \
	CGLayer.h \
	CGPDFContext.h \
	CGPDFDocument.h \
	CGPDFPage.h \
	CGPath.h \
	CGPattern.h \
	CGShading.h \
	CGWindow.h \
	CGWindowLevel.h \
	CoreGraphics.h \
	CoreGraphicsExport.h \
	CoreText.h

RESOURCES=Info.plist
CFLAGS+= -I${.CURDIR:H}/OpenGL -I${.CURDIR:H} -I${.OBJDIR:H} -fblocks \
	-I${.CURDIR}/Foundation/Headers -I${.OBJDIR} \
	-I${ROOT_SOURCE_DIR}/CoreServices -Wno-implicit-function-declaration
LDFLAGS+= -framework Onyx2D -framework CoreFoundation \
	-Wl,-undefined,dynamic_lookup

#-framework OpenGL

all: copy-headers libCoreGraphics.a

copy-headers:
	ln -sf ${.CURDIR:H}/OpenGL/include ${.OBJDIR}/OpenGL

fmwk-install-hook:
.for dir in ${RAVYN_SDKROOT}/System/Library/Frameworks/ ${SYSROOT_DIR}/System/Library/Frameworks/
	mkdir -p ${dir}
	cp -Rf ${.OBJDIR}/${FRAMEWORK}.framework ${dir}
.endfor

.include <rvn.framework.mk>
