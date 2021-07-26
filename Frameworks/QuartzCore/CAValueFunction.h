#import <Foundation/NSObject.h>
#import <QuartzCore/CABase.h>

CA_EXPORT NSString *const kCAValueFunctionTranslate;
CA_EXPORT NSString *const kCAValueFunctionTranslateX;
CA_EXPORT NSString *const kCAValueFunctionTranslateY;
CA_EXPORT NSString *const kCAValueFunctionTranslateZ;

CA_EXPORT NSString *const kCAValueFunctionScale;
CA_EXPORT NSString *const kCAValueFunctionScaleX;
CA_EXPORT NSString *const kCAValueFunctionScaleY;
CA_EXPORT NSString *const kCAValueFunctionScaleZ;

CA_EXPORT NSString *const kCAValueFunctionRotateX;
CA_EXPORT NSString *const kCAValueFunctionRotateY;
CA_EXPORT NSString *const kCAValueFunctionRotateZ;

@interface CAValueFunction : NSObject {
    NSString *_name;
}

+ functionWithName:(NSString *)name;

@property(readonly) NSString *name;

@end
