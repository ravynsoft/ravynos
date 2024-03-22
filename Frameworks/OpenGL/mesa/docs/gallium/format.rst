Formats in gallium
==================

Gallium format names mostly follow D3D10 conventions, with some extensions.

Format names like XnYnZnWn have the X component in the lowest-address n bits
and the W component in the highest-address n bits; for B8G8R8A8, byte 0 is
blue and byte 3 is alpha.  Note that platform endianness is not considered
in this definition.  In C::

    struct x8y8z8w8 { uint8_t x, y, z, w; };

Format aliases like XYZWstrq are (s+t+r+q)-bit integers in host endianness,
with the X component in the s least-significant bits of the integer.  In C::

    uint32_t xyzw8888 = (x << 0) | (y << 8) | (z << 16) | (w << 24);

Format suffixes affect the interpretation of the channel:

- ``SINT``:     N bit signed integer [-2^(N-1) ... 2^(N-1) - 1]
- ``SNORM``:    N bit signed integer normalized to [-1 ... 1]
- ``SSCALED``:  N bit signed integer [-2^(N-1) ... 2^(N-1) - 1]
- ``FIXED``:    Signed fixed point integer, (N/2 - 1) bits of mantissa
- ``FLOAT``:    N bit IEEE754 float
- ``NORM``:     Normalized integers, signed or unsigned per channel
- ``UINT``:     N bit unsigned integer [0 ... 2^N - 1]
- ``UNORM``:    N bit unsigned integer normalized to [0 ... 1]
- ``USCALED``:  N bit unsigned integer [0 ... 2^N - 1]

The difference between ``SINT`` and ``SSCALED`` is that the former are pure
integers in shaders, while the latter are floats; likewise for ``UINT`` versus
``USCALED``.

There are two exceptions for ``FLOAT``.  ``R9G9B9E5_FLOAT`` is nine bits
each of red green and blue mantissa, with a shared five bit exponent.
``R11G11B10_FLOAT`` is five bits of exponent and five or six bits of mantissa
for each color channel.

For the ``NORM`` suffix, the signedness of each channel is indicated with an
S or U after the number of channel bits, as in ``R5SG5SB6U_NORM``.

The ``SRGB`` suffix is like ``UNORM`` in range, but in the sRGB colorspace.

Compressed formats are named first by the compression format string (``DXT1``,
``ETC1``, etc), followed by a format-specific subtype.  Refer to the
appropriate compression spec for details.

Formats used in video playback are named by their FOURCC code.

Format names with an embedded underscore are subsampled.  ``R8G8_B8G8`` is a
single 32-bit block of two pixels, where the R and B values are repeated in
both pixels.

Index buffers do not have a natural format in Gallium structures. For purposes
of ``is_format_supported`` queries, the formats ``R8_UINT``, ``R16_UINT``, and
``R32_UINT`` are used with ``PIPE_BIND_INDEX_BUFFER`` for 8-bit, 16-bit, and
32-bit index buffers respectively.

References
----------

DirectX Graphics Infrastructure documentation on DXGI_FORMAT enum:
https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format

FOURCC codes for YUV formats:
http://web.archive.org/web/20220523043110/https://www.fourcc.org/yuv/
