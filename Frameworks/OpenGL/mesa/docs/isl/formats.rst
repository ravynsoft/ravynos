Surface Formats
===============

A surface format describes the encoding of color information into the actual
data stored in memory.  Surface formats in isl are specified via the
:c:enum:`isl_format` enum.  A complete list of surface formats is included at
the end of this chapter.

In general, a surface format definition consists of two parts: encoding and
layout.

Data Encoding
-------------

There are several different ways that one can encode a number (or vector) into
a binary form, and each makes different trade-offs.  By default, most color
values lie in the range [0, 1], so one of the most common encodings for color
data is unsigned normalized where the range of an unsigned integer of a
particular size is mapped linearly onto the interval [0, 1]. While normalized
is certainly the most common representation for color data, not all data is
color data, and not all values are nicely bounded.  The possible data encodings
are specified by :c:enum:`isl_base_type`:

.. c:autoenum:: isl_base_type
   :file: src/intel/isl/isl.h
   :members:

Data Layout
-----------

The different data layouts fall into two categories: array and packed.  When an
array layout is used, the components are stored sequentially in an array of the
given encoding.  For instance, if the data is encoded in an 8-bit RGBA array
format the data is stored in an array of type :c:type:`uint8_t` where the blue
component of the `i`'th color value is accessed as:

.. code-block:: C

   uint8_t r = ((uint8_t *)data)[i * 4 + 0];
   uint8_t g = ((uint8_t *)data)[i * 4 + 1];
   uint8_t b = ((uint8_t *)data)[i * 4 + 2];
   uint8_t a = ((uint8_t *)data)[i * 4 + 3];

Array formats are popular because of their simplicity.  However, they are
limited to formats where all components have the same size and fit in
a standard C data type.

Packed formats, on the other hand, are encoded with the entire color value
packed into a single 8, 16, or 32-bit value.  The components are specified by
which bits they occupy within that value.  For instance, with the popular
`RGB565` format, each :c:type:`vec3` takes up 16 bits and the
`i`'th color value is accessed as:

.. code-block:: C

   uint8_t r = (*(uint16_t *)data >> 0) & 0x1f;
   uint8_t g = (*(uint16_t *)data >> 5) & 0x3f;
   uint8_t b = (*(uint16_t *)data >> 11) & 0x1f;

Packed formats are useful because they allow you to specify formats with uneven
component sizes such as `RGBA1010102` or where the components are
smaller than 8 bits such as `RGB565` discussed above.  It does,
however, come with the restriction that the entire vector must fit within 8,
16, or 32 bits.

One has to be careful when reasoning about packed formats because it is easy to
get the color order wrong.  With array formats, the channel ordering is usually
implied directly from the format name with `RGBA8888` storing the
formats as in the first example and `BGRA8888` storing them in the BGRA
ordering.  Packed formats, however, are not as simple because some
specifications choose to use a MSB to LSB ordering and others LSB to MSB.  One
must be careful to pay attention to the enum in question in order to avoid
getting them backwards.

From an API perspective, both types of formats are available.  In Vulkan, the
formats that are of the form :c:enumerator:`VK_FORMAT_xxx_PACKEDn` are packed
formats where the entire color fits in `n` bits and formats without the
`_PACKEDn` suffix are array formats.  In GL, if you specify one of the
base types such as :c:enumerator:`GL_FLOAT` you get an array format but if you
specify a packed type such as :c:enumerator:`GL_UNSIGNED_INT_8_8_8_8_REV` you
get a packed format.

The following table provides a summary of the bit orderings of different packed
format specifications.  The bit ordering is relative to the reading of the enum
name from left to right.

=====================  ==============
Component               Left → Right
=====================  ==============
GL                       MSB → LSB
Vulkan                   MSB → LSB
mesa_format              LSB → MSB
Intel surface format     LSB → MSB
=====================  ==============

Understanding sRGB
------------------

The sRGB colorspace is one of the least tractable concepts in the entire world
of surfaces and formats.  Most texture formats are stored in a linear
colorspace where the floating-point value corresponds linearly to intensity
values.  The sRGB color space, on the other hand, is non-linear and provides
greater precision in the lower-intensity (darker) end of the spectrum.  The
relationship between linear and sRGB is governed by the following continuous
bijection:

.. math::

   c_l =
   \begin{cases}
   \frac{c_s}{12.92}                            &\text{if } c_s \le 0.04045 \\\\
   \left(\frac{c_s + 0.055}{1.055}\right)^{2.4} &\text{if } c_s > 0.04045
   \end{cases}

where :math:`c_l` is the linear color and :math:`c_s` is the color in sRGB.
It is important to note that, when an alpha channel is present, the alpha
channel is always stored in the linear colorspace.

The key to understanding sRGB is to think about it starting from the physical
display.  All displays work natively in sRGB.  On older displays, there isn't
so much a conversion operation as a fact of how the hardware works.  All
display hardware has a natural gamma curve required to get from linear to the
signal level required to generate the correct color.  On older CRT displays,
the gamma curve of your average CRT is approximately the sRGB curve.  More
modern display hardware has support for additional gamma curves to try and get
accurate colors but, for the sake of compatibility, everything still operates
in sRGB.  When an image is sent to the X server, X passes the pixels on to the
display verbatim without doing any conversions.  (Fun fact: When dealing with
translucent windows, X blends in the wrong colorspace.)  This means that the
image into which you are rendering will always be interpreted as if it were in
the sRGB colorspace.

When sampling from a texture, the value returned to the shader is in the linear
colorspace.  The conversion from sRGB happens as part of sampling. In OpenGL,
thanks mostly to history, there are various knobs for determining when you
should or should not encode or decode sRGB.  In 2007, :ext:`GL_EXT_texture_sRGB`
added support for sRGB texture formats and was included in OpenGL 2.1.  In
2010, :ext:`GL_EXT_texture_sRGB_decode` added a flag to allow you to disable
texture decoding so that the shader received the data still in the sRGB
colorspace. Then, in 2012, :ext:`GL_ARB_texture_view` came along and made
:ext:`GL_EXT_texture_sRGB_decode` simultaneously obsolete and very confusing.
Now, thanks to the combination of extensions, you can upload a texture as
linear, create an sRGB view of it and ask that sRGB not be decoded.  What
format is it in again?

The situation with render targets is a bit different.  Historically, you got
your render target from the window system (which is always sRGB) and the spec
said nothing whatsoever about encoding.  All render targets were sRGB because
that's how monitors worked and application writers were expected to understand
that their final rendering needed to be in sRGB.  However, with the advent of
:ext:`GL_EXT_framebuffer_object` this was no longer true.  Also, sRGB was causing
problems with blending because GL was blind to the fact that the output was
sRGB and blending was occurring in the wrong colorspace. In 2006, a set of
:ext:`GL_EXT_framebuffer_sRGB` extensions added support (on both the GL and
window-system sides) for detecting whether a particular framebuffer was in sRGB
and instructing GL to do the conversion into the sRGB colorspace as the final
step prior to writing out to the render target.  Enabling sRGB also implied
that blending would occur in the linear colorspace prior to sRGB conversion and
would therefore be more accurate.  When sRGB was added to the OpenGL ES spec in
3.1, they added the query for sRGB but did not add the flag to allow you to
turn it on and off.

In Vulkan, this is all much more straightforward.  Your format is sRGB or it
isn't.  If you have an sRGB image and you don't want sRGB decoding to happen
when you sample from it, you simply create a :c:struct:`VkImageView` that has
the appropriate linear format and the data will be treated as linear and not
converted.  Similarly for render targets, blending always happens in the same
colorspace as the shader output and you determine whether or not you want sRGB
conversion by the format of the :c:struct:`VkImageView` used as the render
target.

Surface Format Introspection API
--------------------------------

ISL provides an API for introspecting the :c:enum:`isl_format` enum and
getting various bits of information about a format.  ISL provides helpers for
introspecting both the data layout of an :c:enum:`isl_format` and the
capabilities of that format for a particular piece of Intel hardware.

Format Layout Introspection
^^^^^^^^^^^^^^^^^^^^^^^^^^^

To get the layout of a given :c:enum:`isl_format`, call
:c:func:`isl_format_get_layout`:

.. c:autofunction:: isl_format_get_layout

.. c:autostruct:: isl_format_layout
   :members:

.. c:autostruct:: isl_channel_layout
   :members:

There are also quite a few helpers for many of the common cases that allow you
to avoid using :c:struct:`isl_format_layout` manually.  There are a lot of
them so we won't include a full list here.  Look at isl.h for more details.

Hardware Format Support Introspection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is provided by means of a table located in isl_format.c.  Looking at the
table directly is often useful for understanding HW support for various
formats.  However, for the purposes of code cleanliness, the table is not
exposed directly and, instead, hardware support information is exposed via
a set of helper functions:

.. c:autofunction:: isl_format_supports_rendering

.. c:autofunction:: isl_format_supports_alpha_blending

.. c:autofunction:: isl_format_supports_sampling

.. c:autofunction:: isl_format_supports_filtering

.. c:autofunction:: isl_format_supports_vertex_fetch

.. c:autofunction:: isl_format_supports_typed_writes
   :file: src/intel/isl/isl_format.c

.. c:autofunction:: isl_format_supports_typed_reads

.. c:autofunction:: isl_format_supports_ccs_d

.. c:autofunction:: isl_format_supports_ccs_e

.. c:autofunction:: isl_format_supports_multisampling

.. c:autofunction:: isl_formats_are_ccs_e_compatible

Surface Format Enums
--------------------

Everything in ISL is done in terms of the :c:enum:`isl_format` enum. However,
for the sake of interacting with other parts of Mesa, we provide a helper for
converting a :c:enum:`pipe_format` to an :c:enum:`isl_format`:

.. c:autofunction:: isl_format_for_pipe_format

The :c:enum:`isl_format` enum is as follows:

.. c:autoenum:: isl_format
   :members:
