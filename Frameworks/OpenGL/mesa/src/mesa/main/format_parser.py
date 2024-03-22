#
# Copyright 2009 VMware, Inc.
# Copyright 2014 Intel Corporation
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import sys

VOID = 'x'
UNSIGNED = 'u'
SIGNED = 's'
FLOAT = 'f'

ARRAY = 'array'
PACKED = 'packed'
OTHER = 'other'

RGB = 'rgb'
SRGB = 'srgb'
YUV = 'yuv'
ZS = 'zs'

VERY_LARGE = 99999999999999999999999

class Channel:
   """Describes a color channel."""

   def __init__(self, type, norm, size):
      self.type = type
      self.norm = norm
      self.size = size
      self.sign = type in (SIGNED, FLOAT)
      self.name = None # Set when the channels are added to the format
      self.shift = -1 # Set when the channels are added to the format
      self.index = -1 # Set when the channels are added to the format

   def __str__(self):
      s = str(self.type)
      if self.norm:
         s += 'n'
      s += str(self.size)
      return s

   def __eq__(self, other):
      if other is None:
         return False

      return self.type == other.type and self.norm == other.norm and self.size == other.size

   def __ne__(self, other):
      return not self.__eq__(other)

   def max(self):
      """Returns the maximum representable number."""
      if self.type == FLOAT:
         return VERY_LARGE
      if self.norm:
         return 1
      if self.type == UNSIGNED:
         return (1 << self.size) - 1
      if self.type == SIGNED:
         return (1 << (self.size - 1)) - 1
      assert False

   def min(self):
      """Returns the minimum representable number."""
      if self.type == FLOAT:
         return -VERY_LARGE
      if self.type == UNSIGNED:
         return 0
      if self.norm:
         return -1
      if self.type == SIGNED:
         return -(1 << (self.size - 1))
      assert False

   def one(self):
      """Returns the value that represents 1.0f."""
      if self.type == UNSIGNED:
         return (1 << self.size) - 1
      if self.type == SIGNED:
         return (1 << (self.size - 1)) - 1
      else:
         return 1

   def datatype(self):
      """Returns the datatype corresponding to a channel type and size"""
      return _get_datatype(self.type, self.size)

class Swizzle:
   """Describes a swizzle operation.

   A Swizzle is a mapping from one set of channels in one format to the
   channels in another.  Each channel in the destination format is
   associated with one of the following constants:

    * SWIZZLE_X: The first channel in the source format
    * SWIZZLE_Y: The second channel in the source format
    * SWIZZLE_Z: The third channel in the source format
    * SWIZZLE_W: The fourth channel in the source format
    * SWIZZLE_ZERO: The numeric constant 0
    * SWIZZLE_ONE: THe numeric constant 1
    * SWIZZLE_NONE: No data available for this channel

   Sometimes a Swizzle is represented by a 4-character string.  In this
   case, the source channels are represented by the characters "x", "y",
   "z", and "w"; the numeric constants are represented as "0" and "1"; and
   no mapping is represented by "_".  For instance, the map from
   luminance-alpha to rgba is given by "xxxy" because each of the three rgb
   channels maps to the first luminance-alpha channel and the alpha channel
   maps to second luminance-alpha channel.  The mapping from bgr to rgba is
   given by "zyx1" because the first three colors are reversed and alpha is
   always 1.
   """

   __identity_str = 'xyzw01_'

   SWIZZLE_X = 0
   SWIZZLE_Y = 1
   SWIZZLE_Z = 2
   SWIZZLE_W = 3
   SWIZZLE_ZERO = 4
   SWIZZLE_ONE = 5
   SWIZZLE_NONE = 6

   def __init__(self, swizzle):
      """Creates a Swizzle object from a string or array."""
      if isinstance(swizzle, str):
         swizzle = [Swizzle.__identity_str.index(c) for c in swizzle]
      else:
         swizzle = list(swizzle)
         for s in swizzle:
            assert isinstance(s, int) and 0 <= s and s <= Swizzle.SWIZZLE_NONE

      assert len(swizzle) <= 4

      self.__list = swizzle + [Swizzle.SWIZZLE_NONE] * (4 - len(swizzle))
      assert len(self.__list) == 4

   def __iter__(self):
      """Returns an iterator that iterates over this Swizzle.

      The values that the iterator produces are described by the SWIZZLE_*
      constants.
      """
      return self.__list.__iter__()

   def __str__(self):
      """Returns a string representation of this Swizzle."""
      return ''.join(Swizzle.__identity_str[i] for i in self.__list)

   def __getitem__(self, idx):
      """Returns the SWIZZLE_* constant for the given destination channel.

      Valid values for the destination channel include any of the SWIZZLE_*
      constants or any of the following single-character strings: "x", "y",
      "z", "w", "r", "g", "b", "a", "z" "s".
      """

      if isinstance(idx, int):
         assert idx >= Swizzle.SWIZZLE_X and idx <= Swizzle.SWIZZLE_NONE
         if idx <= Swizzle.SWIZZLE_W:
            return self.__list.__getitem__(idx)
         else:
            return idx
      elif isinstance(idx, str):
         if idx in 'xyzw':
            idx = 'xyzw'.find(idx)
         elif idx in 'rgba':
            idx = 'rgba'.find(idx)
         elif idx in 'zs':
            idx = 'zs'.find(idx)
         else:
            assert False
         return self.__list.__getitem__(idx)
      else:
         assert False

   def __mul__(self, other):
      """Returns the composition of this Swizzle with another Swizzle.

      The resulting swizzle is such that, for any valid input to
      __getitem__, (a * b)[i] = a[b[i]].
      """
      assert isinstance(other, Swizzle)
      return Swizzle(self[x] for x in other)

   def inverse(self):
      """Returns a pseudo-inverse of this swizzle.

      Since swizzling isn't necisaraly a bijection, a Swizzle can never
      be truely inverted.  However, the swizzle returned is *almost* the
      inverse of this swizzle in the sense that, for each i in range(3),
      a[a.inverse()[i]] is either i or SWIZZLE_NONE.  If swizzle is just
      a permutation with no channels added or removed, then this
      function returns the actual inverse.

      This "pseudo-inverse" idea can be demonstrated by mapping from
      luminance-alpha to rgba that is given by "xxxy".  To get from rgba
      to lumanence-alpha, we use Swizzle("xxxy").inverse() or "xw__".
      This maps the first component in the lumanence-alpha texture is
      the red component of the rgba image and the second to the alpha
      component, exactly as you would expect.
      """
      rev = [Swizzle.SWIZZLE_NONE] * 4
      for i in range(4):
         for j in range(4):
            if self.__list[j] == i and rev[i] == Swizzle.SWIZZLE_NONE:
               rev[i] = j
      return Swizzle(rev)


class Format:
   """Describes a pixel format."""

   def __init__(self, name, layout, block_width, block_height, block_depth, channels, swizzle, colorspace):
      """Constructs a Format from some metadata and a list of channels.

      The channel objects must be unique to this Format and should not be
      re-used to construct another Format.  This is because certain channel
      information such as shift, offset, and the channel name are set when
      the Format is created and are calculated based on the entire list of
      channels.

      Arguments:
      name -- Name of the format such as 'MESA_FORMAT_A8R8G8B8'
      layout -- One of 'array', 'packed' 'other', or a compressed layout
      block_width -- The block width if the format is compressed, 1 otherwise
      block_height -- The block height if the format is compressed, 1 otherwise
      block_depth -- The block depth if the format is compressed, 1 otherwise
      channels -- A list of Channel objects
      swizzle -- A Swizzle from this format to rgba
      colorspace -- one of 'rgb', 'srgb', 'yuv', or 'zs'
      """
      self.name = name
      self.layout = layout
      self.block_width = block_width
      self.block_height = block_height
      self.block_depth = block_depth
      self.channels = channels
      assert isinstance(swizzle, Swizzle)
      self.swizzle = swizzle
      self.name = name
      assert colorspace in (RGB, SRGB, YUV, ZS)
      self.colorspace = colorspace

      # Name the channels
      chan_names = ['']*4
      if self.colorspace in (RGB, SRGB):
         for (i, s) in enumerate(swizzle):
            if s < 4:
               chan_names[s] += 'rgba'[i]
      elif colorspace == ZS:
         for (i, s) in enumerate(swizzle):
            if s < 4:
               chan_names[s] += 'zs'[i]
      else:
         chan_names = ['x', 'y', 'z', 'w']

      for c, name in zip(self.channels, chan_names):
         assert c.name is None
         if name == 'rgb':
            c.name = 'l'
         elif name == 'rgba':
            c.name = 'i'
         elif name == '':
            c.name = 'x'
         else:
            c.name = name

      # Set indices and offsets
      if self.layout == PACKED:
         shift = 0
         for channel in self.channels:
            assert channel.shift == -1
            channel.shift = shift
            shift += channel.size
      for idx, channel in enumerate(self.channels):
         assert channel.index == -1
         channel.index = idx
      else:
         pass # Shift means nothing here

   def __str__(self):
      return self.name

   def short_name(self):
      """Returns a short name for a format.

      The short name should be suitable to be used as suffix in function
      names.
      """

      name = self.name
      if name.startswith('MESA_FORMAT_'):
         name = name[len('MESA_FORMAT_'):]
      name = name.lower()
      return name

   def block_size(self):
      """Returns the block size (in bits) of the format."""
      size = 0
      for channel in self.channels:
         size += channel.size
      return size

   def num_channels(self):
      """Returns the number of channels in the format."""
      nr_channels = 0
      for channel in self.channels:
         if channel.size:
            nr_channels += 1
      return nr_channels

   def array_element(self):
      """Returns a non-void channel if this format is an array, otherwise None.

      If the returned channel is not None, then this format can be
      considered to be an array of num_channels() channels identical to the
      returned channel.
      """
      if self.layout == ARRAY:
         return self.channels[0]
      elif self.layout == PACKED:
         ref_channel = self.channels[0]
         if ref_channel.type == VOID:
            ref_channel = self.channels[1]
         for channel in self.channels:
            if channel.size == 0 or channel.type == VOID:
               continue
            if channel.size != ref_channel.size or channel.size % 8 != 0:
               return None
            if channel.type != ref_channel.type:
               return None
            if channel.norm != ref_channel.norm:
               return None
         return ref_channel
      else:
         return None

   def is_array(self):
      """Returns true if this format can be considered an array format.

      This function will return true if self.layout == 'array'.  However,
      some formats, such as MESA_FORMAT_A8G8B8R8, can be considered as
      array formats even though they are technically packed.
      """
      return self.array_element() != None

   def is_compressed(self):
      """Returns true if this is a compressed format."""
      return self.block_width != 1 or self.block_height != 1 or self.block_depth != 1

   def is_int(self):
      """Returns true if this format is an integer format.

      See also: is_norm()
      """
      if self.layout not in (ARRAY, PACKED):
         return False
      for channel in self.channels:
         if channel.type not in (VOID, UNSIGNED, SIGNED):
            return False
      return True

   def is_float(self):
      """Returns true if this format is an floating-point format."""
      if self.layout not in (ARRAY, PACKED):
         return False
      for channel in self.channels:
         if channel.type not in (VOID, FLOAT):
            return False
      return True

   def channel_type(self):
      """Returns the type of the channels in this format."""
      _type = VOID
      for c in self.channels:
         if c.type == VOID:
            continue
         if _type == VOID:
            _type = c.type
         assert c.type == _type
      return _type

   def channel_size(self):
      """Returns the size (in bits) of the channels in this format.

      This function should only be called if all of the channels have the
      same size.  This is always the case if is_array() returns true.
      """
      size = None
      for c in self.channels:
         if c.type == VOID:
            continue
         if size is None:
            size = c.size
         assert c.size == size
      return size

   def max_channel_size(self):
      """Returns the size of the largest channel."""
      size = 0
      for c in self.channels:
         if c.type == VOID:
            continue
         size = max(size, c.size)
      return size

   def is_normalized(self):
      """Returns true if this format is normalized.

      While only integer formats can be normalized, not all integer formats
      are normalized.  Normalized integer formats are those where the
      integer value is re-interpreted as a fixed point value in the range
      [0, 1].
      """
      norm = None
      for c in self.channels:
         if c.type == VOID:
            continue
         if norm is None:
            norm = c.norm
         assert c.norm == norm
      return norm

   def has_channel(self, name):
      """Returns true if this format has the given channel."""
      if self.is_compressed():
         # Compressed formats are a bit tricky because the list of channels
         # contains a single channel of type void.  Since we don't have any
         # channel information there, we pull it from the swizzle.
         if str(self.swizzle) == 'xxxx':
            return name == 'i'
         elif str(self.swizzle)[0:3] in ('xxx', 'yyy'):
            if name == 'l':
               return True
            elif name == 'a':
               return self.swizzle['a'] <= Swizzle.SWIZZLE_W
            else:
               return False
         elif name in 'rgba':
            return self.swizzle[name] <= Swizzle.SWIZZLE_W
         else:
            return False
      else:
         for channel in self.channels:
            if channel.name == name:
               return True
         return False

   def get_channel(self, name):
      """Returns the channel with the given name if it exists."""
      for channel in self.channels:
         if channel.name == name:
            return channel
      return None

   def datatype(self):
      """Returns the datatype corresponding to a format's channel type and size"""
      if self.layout == PACKED:
         if self.block_size() == 8:
            return 'uint8_t'
         if self.block_size() == 16:
            return 'uint16_t'
         if self.block_size() == 32:
            return 'uint32_t'
         else:
            assert False
      else:
         return _get_datatype(self.channel_type(), self.channel_size())

def _get_datatype(type, size):
   if type == FLOAT:
      if size == 32:
         return 'float'
      elif size == 16:
         return 'uint16_t'
      else:
         assert False
   elif type == UNSIGNED:
      if size <= 8:
         return 'uint8_t'
      elif size <= 16:
         return 'uint16_t'
      elif size <= 32:
         return 'uint32_t'
      else:
         assert False
   elif type == SIGNED:
      if size <= 8:
         return 'int8_t'
      elif size <= 16:
         return 'int16_t'
      elif size <= 32:
         return 'int32_t'
      else:
         assert False
   else:
      assert False

def _parse_channels(fields):
   channels = []
   for field in fields:
      if not field:
         continue

      type = field[0] if field[0] else 'x'

      if field[1] == 'n':
         norm = True
         size = int(field[2:])
      else:
         norm = False
         size = int(field[1:])

      channel = Channel(type, norm, size)
      channels.append(channel)

   return channels

def parse(filename):
   """Parse a format description in CSV format.

   This function parses the given CSV file and returns an iterable of
   channels."""

   with open(filename) as stream:
      for line in stream:
         try:
            comment = line.index('#')
         except ValueError:
            pass
         else:
            line = line[:comment]
         line = line.strip()
         if not line:
            continue

         fields = [field.strip() for field in line.split(',')]

         name = fields[0]
         layout = fields[1]
         block_width = int(fields[2])
         block_height = int(fields[3])
         block_depth = int(fields[4])
         colorspace = fields[10]

         try:
            swizzle = Swizzle(fields[9])
         except:
            sys.exit("error parsing swizzle for format " + name)

         channels = _parse_channels(fields[5:9])

         yield Format(name, layout, block_width, block_height, block_depth, channels, swizzle, colorspace)
