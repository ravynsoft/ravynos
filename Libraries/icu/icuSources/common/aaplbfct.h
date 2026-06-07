/**
 ************************************************************************************
 * Copyright (C) 2006-2007,2012 International Business Machines Corporation and others. *
 * All Rights Reserved.                                                             *
 ************************************************************************************
 */

#ifndef AAPLBFCT_H
#define AAPLBFCT_H

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/utext.h"
#include "unicode/uscript.h"
#include "brkeng.h"
#include "dictbe.h"

U_NAMESPACE_BEGIN

class AppleLanguageBreakFactory : public ICULanguageBreakFactory {
 public:

  /**
   * <p>Standard constructor.</p>
   *
   */
  AppleLanguageBreakFactory(UErrorCode &status);

  /**
   * <p>Virtual destructor.</p>
   */
  virtual ~AppleLanguageBreakFactory();

 protected:

  /**
   * <p>Create a DictionaryMatcher for the specified script and break type.</p>
   * @param script An ISO 15924 script code that identifies the dictionary to be
   * created.
   * @param breakType The kind of text break for which a dictionary is 
   * sought.
   * @return A DictionaryMatcher with the desired characteristics, or NULL.
   */
  virtual DictionaryMatcher *loadDictionaryMatcherFor(UScriptCode script);

};

U_NAMESPACE_END

    /* AAPLBFCT_H */
#endif
