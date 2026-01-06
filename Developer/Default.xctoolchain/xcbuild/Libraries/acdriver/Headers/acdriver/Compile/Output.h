/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __acdriver_Compile_Output_h
#define __acdriver_Compile_Output_h

#include <acdriver/NonStandard.h>
#include <plist/Dictionary.h>
#include <car/Writer.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <ext/optional>

namespace libutil { class Filesystem; }
namespace xcassets { namespace Asset { class Asset; } }

namespace acdriver {
namespace Compile {

/*
 * Output for asset catalog compilation.
 */
class Output {
public:
    enum class Format {
        /*
         * Write out a compiled asset catalog.
         */
        Compiled,
        /*
         * Copy the assets into a folder.
         */
        Folder,
    };

private:
    std::string                        _root;
    Format                             _format;

private:
    ext::optional<std::string>         _appIcon;
    ext::optional<std::string>         _launchImage;
    NonStandard::ImageTypeSet          _allowedNonStandardImageTypes;

private:
    ext::optional<car::Writer>         _car;
    std::vector<std::pair<std::string, std::string>> _copies;
    std::unique_ptr<plist::Dictionary> _additionalInfo;

private:
    std::vector<std::string>           _inputs;
    std::vector<std::string>           _outputs;

public:
    Output(
        std::string const &root,
        Format format,
        ext::optional<std::string> const &appIcon,
        ext::optional<std::string> const &launchImage,
        NonStandard::ImageTypeSet const &allowedNonStandardImageTypes = NonStandard::ImageTypeSet());

public:
    /*
     * The directory the assets should be compiled into.
     */
    std::string const &root() const
    { return _root; }

    /*
     * The output format for the compiled assets.
     */
    Format format() const
    { return _format; }

public:
    /*
     * The name of the app icon to use.
     */
    ext::optional<std::string> const &appIcon()
    { return _appIcon; }

    /*
     * The name of the launch image to use.
     */
    ext::optional<std::string> const &launchImage()
    { return _launchImage; }

    /*
     * List of non-standard image formats that are allowed into an image set
     * when non-standard behavior are enabled.
     */
    NonStandard::ImageTypeSet const &allowedNonStandardImageTypes() const
    { return _allowedNonStandardImageTypes; }

public:
    /*
     * If the format is compiled, the compiled catalog writer.
     */
    ext::optional<car::Writer> const &car() const
    { return _car; }
    ext::optional<car::Writer> &car()
    { return _car; }

    /*
     * Files to copy into the output.
     */
    std::vector<std::pair<std::string, std::string>> const &copies() const
    { return _copies; }
    std::vector<std::pair<std::string, std::string>> &copies()
    { return _copies; }

    /*
     * Additional Info.plist entries to include.
     */
    plist::Dictionary const *additionalInfo() const
    { return _additionalInfo.get(); }
    plist::Dictionary *additionalInfo()
    { return _additionalInfo.get(); }

public:
    /*
     * Files that were read in as input.
     */
    std::vector<std::string> const &inputs() const
    { return _inputs; }
    std::vector<std::string> &inputs()
    { return _inputs; }

    /*
     * Files that are written as outputs.
     */
    std::vector<std::string> const &outputs() const
    { return _outputs; }
    std::vector<std::string> &outputs()
    { return _outputs; }

public:
    /*
     * The identifier of an asset for use in results.
     */
    static std::string AssetReference(xcassets::Asset::Asset const *asset);
};

}
}

#endif // !__acdriver_Compile_Output_h
