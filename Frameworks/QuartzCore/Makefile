FRAMEWORK=QuartzCore
.include "../framework.common.mk"

SRCS= \
	CAAnimation.m \
	CAAnimationGroup.m \
	CABase.m \
	CABasicAnimation.m \
	CALayer.m \
	CALayerContext.m \
	CAMediaTimingFunction.m \
	CAPropertyAnimation.m \
	CARenderer.m \
	CATransaction.m \
	CATransactionGroup.m \
	CATransform3D.m \
	CATransition.m \
	CAValueFunction.m \
	CAWindowOpenGLContext.m \
	CIAffineTransform.m \
	CIBloom.m \
	CIBoxBlur.m \
	CIColor.m \
	CIContext.m \
	CIFilter.m \
	CIImage.m \
	CILinearGradient.m \
	CIVector.m

INCS= \
	CAAction.h \
	CAAnimation.h \
	CAAnimationGroup.h \
	CABase.h \
	CABasicAnimation.h \
	CALayer.h \
	CALayerContext.h \
	CAMediaTiming.h \
	CAMediaTimingFunction.h \
	CAPropertyAnimation.h \
	CIBloom.h \
	CARenderer.h \
	CATransaction.h \
	CATransactionGroup.h \
	CATransform3D.h \
	CATransition.h \
	CAValueFunction.h \
	CAWindowOpenGLContext.h \
	CIAffineTransform.h \
	CIBoxBlur.h \
	CIColor.h \
	CIContext.h \
	CIFilter.h \
	CIImage.h \
	CILinearGradient.h \
	CIVector.h \
	CoreAnimation.h \
	CoreImage.h \
	CoreVideo.h \
	QuartzCore.h 

RESOURCES=Info.plist English.lproj
CFLAGS+= ${FMWK_CFLAGS} -I${SRCTOP}/Frameworks \
	-I${OBJTOP}/Frameworks/Foundation/Headers \
        -I${OBJTOP}/Frameworks/AppKit/Headers \
	-F${OBJTOP}/Frameworks/OpenGL -framework OpenGL
LDFLAGS+= ${FMWK_LDFLAGS} \
	-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework \
	-lOpenGL \
        -Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/Current

.include <rvn.framework.mk>

installresources: std_install_hook
