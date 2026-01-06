/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_TemplateRenderingIntent_h
#define __xcassets_TemplateRenderingIntent_h

#include <ext/optional>
#include <string>

namespace xcassets {

/*
 * The way an image is rendered.
 */
enum class TemplateRenderingIntent {
    Original,
    Template,
};

class TemplateRenderingIntents {
private:
    TemplateRenderingIntents();
    ~TemplateRenderingIntents();

public:
    /*
     * Parse a matching template rendering intent from a string, if valid.
     */
    static ext::optional<TemplateRenderingIntent> Parse(std::string const &value);

    /*
     * Convert an template rendering intent to a string.
     */
    static std::string String(TemplateRenderingIntent templateRenderingIntent);
};

}

#endif // !__xcassets_TemplateRenderingIntent_h

