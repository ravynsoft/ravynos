/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/SDK/Product.h>
#include <libutil/Filesystem.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Format/Any.h>
#include <plist/Keys/Unpack.h>

using xcsdk::SDK::Product;
using libutil::Filesystem;

Product::
Product()
{
}

bool Product::
parse(plist::Dictionary const *dict)
{
    std::unordered_set<std::string> seen;
    auto unpack = plist::Keys::Unpack("Product", dict, &seen);

    auto PN   = unpack.cast <plist::String> ("ProductName");
    auto PV   = unpack.cast <plist::String> ("ProductVersion");
    auto PUVV = unpack.cast <plist::String> ("ProductUserVisibleVersion");
    auto PBV  = unpack.cast <plist::String> ("ProductBuildVersion");
    auto PC   = unpack.cast <plist::String> ("ProductCopyright");

    if (!unpack.complete(true)) {
        fprintf(stderr, "%s", unpack.errorText().c_str());
    }

    if (PN != nullptr) {
        _productName = PN->value();
    }

    if (PV != nullptr) {
        _productVersion = PV->value();
    }

    if (PUVV != nullptr) {
        _productUserVisibleVersion = PUVV->value();
    }

    if (PBV != nullptr) {
        _productBuildVersion = PBV->value();
    }

    if (PC != nullptr) {
        _productCopyright = PC->value();
    }

    return true;
}

Product::shared_ptr Product::
Open(Filesystem const *filesystem, std::string const &path)
{
    if (path.empty()) {
        return nullptr;
    }

    /*
     * Load information.
     */
    std::string settingsFileName = path + "/System/Library/CoreServices/SystemVersion.plist";
    if (!filesystem->isReadable(settingsFileName)) {
        return nullptr;
    }

    std::string realPath = filesystem->resolvePath(settingsFileName);
    if (realPath.empty()) {
        return nullptr;
    }

    std::vector<uint8_t> contents;
    if (!filesystem->read(&contents, settingsFileName)) {
        return nullptr;
    }

    /*
     * Parse property list.
     */
    auto result = plist::Format::Any::Deserialize(contents);
    if (result.first == nullptr) {
        return nullptr;
    }

    plist::Dictionary *plist = plist::CastTo<plist::Dictionary>(result.first.get());
    if (plist == nullptr) {
        return nullptr;
    }

    /*
     * Parse the dictionary and create the object.
     */
    auto product = std::make_shared <Product> ();
    if (!product->parse(plist)) {
        return nullptr;
    }

    return product;
}
