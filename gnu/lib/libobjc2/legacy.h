#pragma once
#include "visibility.h"
#include "ivar.h"
#include "class.h"
#include "category.h"
#include "protocol.h"

PRIVATE Class objc_upgrade_class(struct objc_class_gsv1 *oldClass);
PRIVATE struct objc_category *objc_upgrade_category(struct objc_category_gcc *);

PRIVATE struct objc_class_gsv1* objc_legacy_class_for_class(Class);

PRIVATE struct objc_protocol *objc_upgrade_protocol_gcc(struct objc_protocol_gcc*);
PRIVATE struct objc_protocol *objc_upgrade_protocol_gsv1(struct objc_protocol_gsv1*);

