/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_StickerSequence_h
#define __xcassets_Asset_StickerSequence_h

#include <xcassets/Asset/Asset.h>
#include <xcassets/StickerDurationType.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class StickerSequence : public Asset {
public:
    class Frame {
    private:
        ext::optional<std::string> _fileName;

    public:
        ext::optional<std::string> const &fileName() const
        { return _fileName; }

    private:
        friend class StickerSequence;
        bool parse(plist::Dictionary const *dict);
    };

private:
    ext::optional<std::vector<Frame>>  _frames;

private:
    ext::optional<std::string>         _accessibilityLabel;
    ext::optional<StickerDurationType> _durationType;
    ext::optional<double>              _duration;
    ext::optional<double>              _repetitions;

private:
    friend class Asset;
    using Asset::Asset;

public:
    ext::optional<std::vector<Frame>> const &frames() const
    { return _frames; }

public:
    ext::optional<StickerDurationType> const &durationType() const
    { return _durationType; }
    ext::optional<double> const &duration() const
    { return _duration; }
    ext::optional<double> const &repetitions() const
    { return _repetitions; }

public:
    static AssetType Type()
    { return AssetType::StickerSequence; }
    virtual AssetType type() const
    { return AssetType::StickerSequence; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("stickersequence"); }

protected:
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_StickerSequence_h
