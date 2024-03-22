.. _vertexelements:

Vertex Elements
===============

This state controls the format of the input attributes contained in
pipe_vertex_buffers. There is one pipe_vertex_element array member for each
input attribute.

Input Formats
-------------

Gallium supports a diverse range of formats for vertex data. Drivers are
guaranteed to support 32-bit floating-point vectors of one to four components.
Additionally, they may support the following formats:

* Integers, signed or unsigned, normalized or non-normalized, 8, 16, or 32
  bits wide
* Floating-point, 16, 32, or 64 bits wide

At this time, support for varied vertex data formats is limited by driver
deficiencies. It is planned to support a single uniform set of formats for all
Gallium drivers at some point.

Rather than attempt to specify every small nuance of behavior, Gallium uses a
very simple set of rules for padding out unspecified components. If an input
uses less than four components, it will be padded out with the constant vector
``(0, 0, 0, 1)``.

Fog, point size, the facing bit, and edgeflags, all are in the standard format
of ``(x, 0, 0, 1)``, and so only the first component of those inputs is used.

Position
%%%%%%%%

Vertex position may be specified with two to four components. Using less than
two components is not allowed.

Colors
%%%%%%

Colors, both front- and back-facing, may omit the alpha component, only using
three components. Using less than three components is not allowed.

Members
-------

src_offset
    The byte offset of the attribute in the buffer given by
    vertex_buffer_index for the first vertex.
instance_divisor
    The instance data rate divisor, used for instancing.
    0 means this is per-vertex data, n means per-instance data used for
    n consecutive instances (n > 0).
vertex_buffer_index
    The vertex buffer this attribute lives in. Several attributes may
    live in the same vertex buffer.
src_format
    The format of the attribute data. One of the PIPE_FORMAT tokens.
