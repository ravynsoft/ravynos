#include <darwintest.h>
#include <sys/types.h>
#include <sys/acl.h>

T_DECL(acl_bad_test, "Tests invalid acl") {

    acl_t acl = acl_from_text("!#acl");
    T_EXPECT_NULL(acl, "Invalid acl detected");
}
