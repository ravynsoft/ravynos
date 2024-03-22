Conformance Testing
===================

Mesa as a project does not get certified conformant by Khronos for the
APIs it implements.  Rather, individual driver teams run the
conformance tests and submit their results on a set of hardware on a
particular operating system.  The canonical list is at Khronos' list
of `conformant
products <https://www.khronos.org/conformance/adopters/conformant-products/>`_
and you can find some reports there by searching for "Mesa",
"Raspbian" and "RADV" for example.

Submitting conformance results to Khronos
-----------------------------------------

If your driver team is associated with an organization that is a
Khronos member and has submitted conformance for your API on another
software stack (likely you're a hardware company), it will probably be
easiest to submit your conformance through them.

If you are an individual developer or your organization hasn't
submitted results for the given API yet, X.Org is a member through
Software in the Public Interest, and they can help submit your
conformance results to get added to the list of conformant products.
You should probably coordinate with board@foundation.x.org for your
first submission.
