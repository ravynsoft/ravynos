
.. toctree::
  :maxdepth: 2
  :hidden:

  what-is-libinput
  features
  configuration
  building
  faqs
  reporting-bugs
  troubleshooting
  contributing
  development
  API documentation <@HTTP_DOC_LINK@/api/>


++++++++++++++++++++++++++++++
libinput
++++++++++++++++++++++++++++++

libinput is a library that provides a full input stack for display servers
and other applications that need to handle input devices provided by the
kernel.

libinput provides device detection, event handling and abstraction so
minimize the amount of custom input code the user of libinput need to
provide the common set of functionality that users expect. Input event
processing includes scaling touch coordinates, generating
relative pointer events from touchpads, pointer acceleration, etc.

libinput is not used directly by applications. Think of it more as a device
driver than an application library. See :ref:`what_is_libinput` for more details.

--------------------
Users and Developers
--------------------

Please use the side-bar to nagivate through the various documentation items.

-----------------
API documentation
-----------------

The API documentation is available here:
    https://wayland.freedesktop.org/libinput/doc/latest/api/

.. note:: This documentation is generally only needed by authors of Wayland
	  compositors or other developers dealing with input events directly.

-------
License
-------

libinput is licensed under the MIT license

.. code-block:: none

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions: [...]

See the
`COPYING <https://gitlab.freedesktop.org/libinput/libinput/tree/master/COPYING>`_
file for the full license information.

.....
About
.....
Documentation generated from |git_version|
