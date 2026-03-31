// TEST_CFLAGS -framework Foundation
// need Foundation to get NSObject compatibility additions for class Protocol
// because ARC calls [protocol retain]
/*
TEST_BUILD_OUTPUT
.*protocol_copyPropertyList.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
.*protocol_copyPropertyList.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
.*protocol_copyPropertyList.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
.*protocol_copyPropertyList.m:\d+:\d+: warning: null passed to a callee that requires a non-null argument \[-Wnonnull\]
END
*/

#include "test.h"
#include <string.h>
#include <malloc/malloc.h>
#include <objc/runtime.h>

@protocol SuperProps
@property int prop1;
@property int prop2;
@property(class) int prop1;
@property(class) int prop2;
@end

@protocol SubProps <SuperProps>
@property int prop3;
@property int prop4;
@property(class) int prop3;
@property(class) int prop4;
@end


@protocol FourProps
@property int prop1;
@property int prop2;
@property int prop3;
@property int prop4;

@property(class) int prop1;
@property(class) int prop2;
@property(class) int prop3;
@property(class) int prop4;
@end

@protocol NoProps @end

@protocol OneProp
@property int instanceProp;
@property(class) int classProp;
@end


static int isNamed(objc_property_t p, const char *name)
{
    return (0 == strcmp(name, property_getName(p)));
}

void testfn(objc_property_t *(*copyPropertyList_fn)(Protocol*, unsigned int *), 
            const char *onePropName)
{
    objc_property_t *props;
    unsigned int count;
    Protocol *proto;

    proto = @protocol(SubProps);
    testassert(proto);

    count = 100;
    props = copyPropertyList_fn(proto, &count);
    testassert(props);
    testassert(count == 2);
    testassert((isNamed(props[0], "prop4") && isNamed(props[1], "prop3"))  ||  
               (isNamed(props[0], "prop3") && isNamed(props[1], "prop4")));
    // props[] should be null-terminated
    testassert(props[2] == NULL);
    free(props);

    proto = @protocol(SuperProps);
    testassert(proto);

    count = 100;
    props = copyPropertyList_fn(proto, &count);
    testassert(props);
    testassert(count == 2);
    testassert((isNamed(props[0], "prop1") && isNamed(props[1], "prop2"))  ||  
               (isNamed(props[0], "prop2") && isNamed(props[1], "prop1")));
    // props[] should be null-terminated
    testassert(props[2] == NULL);
    free(props);

    // Check null-termination - this property list block would be 16 bytes
    // if it weren't for the terminator
    proto = @protocol(FourProps);
    testassert(proto);

    count = 100;
    props = copyPropertyList_fn(proto, &count);
    testassert(props);
    testassert(count == 4);
    testassert(malloc_size(props) >= 5 * sizeof(objc_property_t));
    testassert(props[3] != NULL);
    testassert(props[4] == NULL);
    free(props);

    // Check NULL count parameter
    props = copyPropertyList_fn(proto, NULL);
    testassert(props);
    testassert(props[4] == NULL);
    testassert(props[3] != NULL);
    free(props);

    // Check NULL protocol parameter
    count = 100;
    props = copyPropertyList_fn(NULL, &count);
    testassert(!props);
    testassert(count == 0);
    
    // Check NULL protocol and count
    props = copyPropertyList_fn(NULL, NULL);
    testassert(!props);

    // Check protocol with no properties
    proto = @protocol(NoProps);
    testassert(proto);

    count = 100;
    props = copyPropertyList_fn(proto, &count);
    testassert(!props);
    testassert(count == 0);

    // Check instance vs class properties
    proto = @protocol(OneProp);
    testassert(proto);
    
    count = 100;
    props = copyPropertyList_fn(proto, &count);
    testassert(props);
    testassert(count == 1);
    testassert(0 == strcmp(property_getName(props[0]), onePropName));
    free(props);
}

objc_property_t *protocol_copyPropertyList2_YES_YES(Protocol *proto, unsigned int *outCount)
{
    return protocol_copyPropertyList2(proto, outCount, YES, YES);
}

objc_property_t *protocol_copyPropertyList2_YES_NO(Protocol *proto, unsigned int *outCount)
{
    return protocol_copyPropertyList2(proto, outCount, YES, NO);
}

int main()
{
    // protocol_copyPropertyList(...) is identical to 
    // protocol_copyPropertyList2(..., YES, YES)
    testfn(protocol_copyPropertyList, "instanceProp");
    testfn(protocol_copyPropertyList2_YES_YES, "instanceProp");

    // protocol_copyPropertyList2(..., YES, NO) is also identical
    // with the protocol definitions above, except for protocol OneProp.
    testfn(protocol_copyPropertyList2_YES_NO, "classProp");

    // Check non-functionality of optional properties

    unsigned int count;
    objc_property_t *props;

    count = 100;
    props = protocol_copyPropertyList2(@protocol(FourProps), &count, NO, YES);
    testassert(!props);
    testassert(count == 0);

    count = 100;
    props = protocol_copyPropertyList2(@protocol(FourProps), &count, NO, NO);
    testassert(!props);
    testassert(count == 0);

    // Check nil count parameter
    props = protocol_copyPropertyList2(@protocol(FourProps), nil, NO, YES);
    testassert(!props);

    props = protocol_copyPropertyList2(@protocol(FourProps), nil, NO, NO);
    testassert(!props);

    // Check nil protocol parameter
    count = 100;
    props = protocol_copyPropertyList2(nil, &count, NO, YES);
    testassert(!props);
    testassert(count == 0);

    count = 100;
    props = protocol_copyPropertyList2(nil, &count, NO, NO);
    testassert(!props);
    testassert(count == 0);
    
    // Check nil protocol and count
    props = protocol_copyPropertyList2(nil, nil, NO, YES);
    testassert(!props);

    props = protocol_copyPropertyList2(nil, nil, NO, NO);
    testassert(!props);
    

    succeed(__FILE__);
    return 0;
}
