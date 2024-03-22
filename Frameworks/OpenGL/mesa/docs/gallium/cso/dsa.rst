.. _depth-stencil-alpha:

Depth, Stencil, & Alpha
=======================

These three states control the depth, stencil, and alpha tests, used to
discard fragments that have passed through the fragment shader.

Traditionally, these three tests have been clumped together in hardware, so
they are all stored in one structure.

During actual execution, the order of operations done on fragments is always:

* Alpha
* Stencil
* Depth

Depth Members
-------------

enabled
    Whether the depth test is enabled.
writemask
    Whether the depth buffer receives depth writes.
func
    The depth test function. One of PIPE_FUNC.

Stencil Members
---------------

enabled
    Whether the stencil test is enabled. For the second stencil, whether the
    two-sided stencil is enabled. If two-sided stencil is disabled, the other
    fields for the second array member are not valid.
func
    The stencil test function. One of PIPE_FUNC.
valuemask
    Stencil test value mask; this is ANDed with the value in the stencil
    buffer and the reference value before doing the stencil comparison test.
writemask
    Stencil test writemask; this controls which bits of the stencil buffer
    are written.
fail_op
    The operation to carry out if the stencil test fails. One of
    PIPE_STENCIL_OP.
zfail_op
    The operation to carry out if the stencil test passes but the depth test
    fails. One of PIPE_STENCIL_OP.
zpass_op
    The operation to carry out if the stencil test and depth test both pass.
    One of PIPE_STENCIL_OP.

Alpha Members
-------------

enabled
    Whether the alpha test is enabled.
func
    The alpha test function. One of PIPE_FUNC.
ref_value
    Alpha test reference value; used for certain functions.
