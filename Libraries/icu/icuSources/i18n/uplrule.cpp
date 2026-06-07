/*
*****************************************************************************************
* Copyright (C) 2010-2011 Apple Inc. All Rights Reserved.
*****************************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/upluralrules.h"
#include "unicode/uplrule.h"

U_NAMESPACE_USE

U_CAPI UPluralRules* U_EXPORT2
uplrule_open(const char *locale,
              UErrorCode *status)
{
    return uplrules_open(locale, status);
}

U_CAPI void U_EXPORT2
uplrule_close(UPluralRules *plrules)
{
    uplrules_close(plrules);
}

U_CAPI int32_t U_EXPORT2
uplrule_select(const UPluralRules *plrules,
               int32_t number,
               UChar *keyword, int32_t capacity,
               UErrorCode *status)
{
    return uplrules_select(plrules, number, keyword, capacity, status);
}

U_CAPI int32_t U_EXPORT2
uplrule_selectDouble(const UPluralRules *plrules,
                     double number,
                     UChar *keyword, int32_t capacity,
                     UErrorCode *status)
{
    return uplrules_select(plrules, number, keyword, capacity, status);
}

#endif /* #if !UCONFIG_NO_FORMATTING */
