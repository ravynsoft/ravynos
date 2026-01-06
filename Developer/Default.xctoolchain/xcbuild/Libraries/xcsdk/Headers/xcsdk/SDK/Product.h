/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcsdk_SDK_Product_h
#define __xcsdk_SDK_Product_h

#include <memory>
#include <string>
#include <ext/optional>

namespace libutil { class Filesystem; };
namespace plist { class Dictionary; }

namespace xcsdk { namespace SDK {

class Product {
public:
    typedef std::shared_ptr <Product> shared_ptr;

private:
    ext::optional<std::string> _productName;
    ext::optional<std::string> _productVersion;
    ext::optional<std::string> _productUserVisibleVersion;
    ext::optional<std::string> _productBuildVersion;
    ext::optional<std::string> _productCopyright;

public:
    Product();

public:
    inline ext::optional<std::string> const &name() const
    { return _productName; }
    inline ext::optional<std::string> const &version() const
    { return _productVersion; }
    inline ext::optional<std::string> const &userVisibleVersion() const
    { return _productUserVisibleVersion; }
    inline ext::optional<std::string> const &buildVersion() const
    { return _productBuildVersion; }
    inline ext::optional<std::string> const &copyright() const
    { return _productCopyright; }

public:
    static Product::shared_ptr Open(libutil::Filesystem const *filesystem, std::string const &path);

private:
    bool parse(plist::Dictionary const *dict);
};

} }

#endif  // !__xcsdk_SDK_Product_h
