//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <system_error>

// class error_category

// const error_category& system_category();

// XFAIL: suse-linux-enterprise-server-11
// XFAIL: stdlib=apple-libc++ && target={{.+}}-apple-macosx10.{{9|10|11|12}}

#include <system_error>
#include <cassert>
#include <string>
#include <cerrno>

#include "test_macros.h"

void test_message_for_bad_value() {
    errno = E2BIG; // something that message will never generate
    const std::error_category& e_cat1 = std::system_category();
    const std::string msg = e_cat1.message(-1);
    // Exact message format varies by platform.
    LIBCPP_ASSERT(
        msg.rfind("No error information", 0) == 0
        || msg.rfind("Unknown error", 0) == 0     // Musl
        || msg.rfind("Error -1 occurred", 0) == 0 // AIX
    );
    assert(errno == E2BIG);
}

int main(int, char**)
{
    const std::error_category& e_cat1 = std::system_category();
    std::error_condition e_cond = e_cat1.default_error_condition(5);
    assert(e_cond.value() == 5);
    assert(e_cond.category() == std::generic_category());
    e_cond = e_cat1.default_error_condition(5000);
    assert(e_cond.value() == 5000);
    assert(e_cond.category() == std::system_category());
    {
        test_message_for_bad_value();
    }

  return 0;
}
