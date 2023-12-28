// attributes.cc -- object attributes for gold

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com>.
// This file contains code adapted from BFD.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include <limits>

#include "attributes.h"
#include "elfcpp.h"
#include "target.h"
#include "parameters.h"
#include "int_encoding.h"

namespace gold
{

// Object_attribute methods.

// Return size of attribute encode in ULEB128.

size_t
Object_attribute::size(int tag) const
{
  // Attributes with default values are not written out.
  if (this->is_default_attribute())
    return 0;

  size_t size = get_length_as_unsigned_LEB_128(tag);
  if (Object_attribute::attribute_type_has_int_value(this->type_))
    size += get_length_as_unsigned_LEB_128(this->int_value_);
  if (Object_attribute::attribute_type_has_string_value(this->type_))
    size += this->string_value_.size() + 1;
  return size;
}

// Whether this has the default value (0/"").

bool
Object_attribute::is_default_attribute() const
{
  if (Object_attribute::attribute_type_has_int_value(this->type_)
      && this->int_value_ != 0)
    return false;
  if (Object_attribute::attribute_type_has_string_value(this->type_)
      && !this->string_value_.empty())
    return false;
  if (Object_attribute::attribute_type_has_no_default(this->type_))
    return false;

  return true;
}

// Whether this matches another Object_attribute OA in merging.
// Two Object_attributes match if they have the same values.

bool
Object_attribute::matches(const Object_attribute& oa) const
{
  return ((this->int_value_ != oa.int_value_)
	  && (this->string_value_ == oa.string_value_));
}

// Write this with TAG to a BUFFER.

void
Object_attribute::write(
    int tag,
    std::vector<unsigned char>* buffer) const
{
  // No need to write default attributes.
  if (this->is_default_attribute())
    return;
  
  // Write tag.
  write_unsigned_LEB_128(buffer, convert_types<uint64_t, int>(tag));

  // Write integer value.
  if (Object_attribute::attribute_type_has_int_value(this->type_))
    write_unsigned_LEB_128(buffer,
			   convert_types<uint64_t, int>(this->int_value_));

  // Write string value.
  if (Object_attribute::attribute_type_has_string_value(this->type_))
    {
      const unsigned char* start =
	reinterpret_cast<const unsigned char*>(this->string_value_.c_str());
      const unsigned char* end = start + this->string_value_.size() + 1;
      buffer->insert(buffer->end(), start, end); 
    }
}

// Vendor_object_attributes methods.

// Copying constructor.

Vendor_object_attributes::Vendor_object_attributes(
    const Vendor_object_attributes& voa)
{
  this->vendor_ = voa.vendor_;

  for (int i = 0; i < NUM_KNOWN_ATTRIBUTES; ++i)
    this->known_attributes_[i] = voa.known_attributes_[i];

  // We do not handle attribute deletion.  So this must be empty.
  gold_assert(this->other_attributes_.empty());

  for (Other_attributes::const_iterator p = voa.other_attributes_.begin();
       p != voa.other_attributes_.end();
       ++p)
    this->other_attributes_[p->first] = new Object_attribute(*(p->second));
}

// Size of this in number of bytes.

size_t
Vendor_object_attributes::size() const
{
  if (this->name() == NULL)
    return 0;

  size_t data_size = 0;
  for (int i = 4; i < NUM_KNOWN_ATTRIBUTES; ++i)
    data_size += this->known_attributes_[i].size(i);

  for (Other_attributes::const_iterator p = this->other_attributes_.begin();
       p != this->other_attributes_.end();
       ++p)
    data_size += p->second->size(p->first);

  // <size> <vendor_name> NUL 0x1 <size>
  return ((data_size != 0
	   || this->vendor_ == Object_attribute::OBJ_ATTR_PROC)
	  ? data_size + strlen(this->name()) + 2 + 2 * 4
	  : 0);
}

// Return a new attribute associated with TAG.

Object_attribute*
Vendor_object_attributes::new_attribute(int tag)
{
  int type = Object_attribute::arg_type(this->vendor_, tag);

  if (tag < NUM_KNOWN_ATTRIBUTES)
    {
      this->known_attributes_[tag].set_type(type);
      return &this->known_attributes_[tag];
    }
  else
    {
      Object_attribute* attr = new Object_attribute();

      // This should be the first time we insert this.
      std::pair<Other_attributes::iterator, bool> ins =
	this->other_attributes_.insert(std::make_pair(tag, attr));
      gold_assert(ins.second);

      attr->set_type(type);
      return attr;
    }
}

// Return an attribute associated with TAG.

Object_attribute*
Vendor_object_attributes::get_attribute(int tag)
{
  if (tag < NUM_KNOWN_ATTRIBUTES)
    return &this->known_attributes_[tag];
  else
    {
      Other_attributes::iterator p =
	this->other_attributes_.find(tag);
      return p != this->other_attributes_.end() ? p->second : NULL;
    }
}

const Object_attribute*
Vendor_object_attributes::get_attribute(int tag) const
{
  if (tag < NUM_KNOWN_ATTRIBUTES)
    return &this->known_attributes_[tag];
  else
    {
      Other_attributes::const_iterator p =
	this->other_attributes_.find(tag);
      return p != this->other_attributes_.end() ? p->second : NULL;
    }
}

// Write attributes to BUFFER.

void
Vendor_object_attributes::write(std::vector<unsigned char>* buffer) const
{
  // Write subsection size.
  size_t voa_size = this->size();
  uint32_t voa_size_as_u32 = convert_types<uint32_t, size_t>(voa_size);
  insert_into_vector<32>(buffer, voa_size_as_u32);

  // Write vendor name.
  const unsigned char* vendor_start =
    reinterpret_cast<const unsigned char*>(this->name());
  size_t vendor_length = strlen(this->name()) + 1;
  const unsigned char* vendor_end = vendor_start + vendor_length;
  buffer->insert(buffer->end(), vendor_start, vendor_end);

  // Write file tag.
  buffer->push_back(Object_attribute::Tag_File);

  // Write attributes size.
  uint32_t attributes_size_as_u32 =
    convert_types<uint32_t, size_t>(voa_size - 4 - vendor_length);
  insert_into_vector<32>(buffer, attributes_size_as_u32);

  // Write known attributes, skipping any defaults.
  for (int i = 4; i < NUM_KNOWN_ATTRIBUTES; ++i)
    {
      // A target may write known attributes in a special order. 
      // Call target hook to remap tags.  Attributes_order is the identity
      // function if no re-ordering is required.
      int tag = parameters->target().attributes_order(i);
      this->known_attributes_[tag].write(tag, buffer);
    }

  // Write other attributes.
  for (Other_attributes::const_iterator q = this->other_attributes_.begin();
       q != this->other_attributes_.end();
       ++q)
    q->second->write(q->first, buffer);
}

// Attributes_section_data methods.

// Compute encoded size of this.

size_t
Attributes_section_data::size() const
{
  size_t data_size = 0;
  for(int vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; ++vendor)
    data_size += this->vendor_object_attributes_[vendor]->size();

  // 'A' <sections for each vendor>
  return data_size != 0 ? data_size + 1 : 0;
}

// Construct an Attributes_section_data object by parsing section contents
// specified by VIEW and SIZE.

Attributes_section_data::Attributes_section_data(
    const unsigned char* view,
    section_size_type size)
{
  for (int vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; ++vendor)
    this->vendor_object_attributes_[vendor] =
      new Vendor_object_attributes(vendor);

  const unsigned char* p = view;
  p = view;
  if (size > 0 && p != NULL && *(p++) == 'A')
    {
      size--;
      while (size > 0)
	{
	  // Size of vendor attributes section.
	  section_size_type section_size =
	    convert_to_section_size_type(read_from_pointer<32>(&p));

	  if (section_size > size)
	    section_size = size;
	  size -= section_size;

	  const char* section_name = reinterpret_cast<const char*>(p);
	  section_size_type section_name_size = strlen(section_name) + 1;
	  section_size -= section_name_size + 4;

	  int vendor;
	  const char* std_section = parameters->target().attributes_vendor();
	  if (std_section != NULL && strcmp(section_name, std_section) == 0)
	    vendor = Object_attribute::OBJ_ATTR_PROC;
	  else if (strcmp(section_name, "gnu") == 0)
	    vendor = Object_attribute::OBJ_ATTR_GNU;
	  else
	    {
	      // Other vendor section.  Ignore it.
	      p += section_name_size + section_size;
	      continue;
	    }
	  p += section_name_size;

	  while (section_size > 0)
	    {
	      const unsigned char* subsection_start = p;

	      // Read vendor subsection index and size.
	      size_t uleb128_len;
	      uint64_t val = read_unsigned_LEB_128(p, &uleb128_len);
	      p += uleb128_len;

	      int tag = convert_types<int, uint64_t>(val);
	      section_size_type subsection_size =
		convert_to_section_size_type(read_from_pointer<32>(&p));
	      section_size -= subsection_size;
	      subsection_size -= (p - subsection_start);

	      const unsigned char* end = p + subsection_size;
	      switch (tag)
		{
		case Object_attribute::Tag_File:
		  while (p < end)
		    {
		      val = read_unsigned_LEB_128(p, &uleb128_len);
		      p += uleb128_len;
		      tag = convert_types<int, uint64_t>(val);
		      Vendor_object_attributes* pvoa =
			this->vendor_object_attributes_[vendor];
		      Object_attribute* attr = pvoa->new_attribute(tag);
		      const char* string_arg;
		      unsigned int int_arg;

		      int type = Object_attribute::arg_type(vendor, tag);
		      switch (type
			      & (Object_attribute::ATTR_TYPE_FLAG_INT_VAL
				 | Object_attribute::ATTR_TYPE_FLAG_STR_VAL))
			{
			case (Object_attribute::ATTR_TYPE_FLAG_INT_VAL
			      | Object_attribute::ATTR_TYPE_FLAG_STR_VAL):
			  val = read_unsigned_LEB_128(p, &uleb128_len);
			  p += uleb128_len;
			  int_arg = convert_types<unsigned int, uint64_t>(val);
			  string_arg = reinterpret_cast<const char *>(p);
			  attr->set_int_value(int_arg);
			  p += strlen(string_arg) + 1;
			  break;
			case Object_attribute::ATTR_TYPE_FLAG_STR_VAL:
			  string_arg = reinterpret_cast<const char *>(p);
			  attr->set_string_value(string_arg);
			  p += strlen(string_arg) + 1;
			  break;
			case Object_attribute::ATTR_TYPE_FLAG_INT_VAL:
			  val = read_unsigned_LEB_128(p, &uleb128_len);
			  p += uleb128_len;
			  int_arg = convert_types<unsigned int, uint64_t>(val);
			  attr->set_int_value(int_arg);
			  break;
			default:
			  gold_unreachable();
			}
		    }
		  break;
		case Object_attribute::Tag_Section:
		case Object_attribute::Tag_Symbol:
		  // Don't have anywhere convenient to attach these.
		  // Fall through for now.
		default:
		  // Ignore things we don't know about.
		  p += subsection_size;
		  subsection_size = 0;
		  break;
		}
	    }
	}
    }
}

// Merge target-independent attributes from another Attribute_section_data
// ASD from an object called NAME into this.

void
Attributes_section_data::merge(
    const char* name,
    const Attributes_section_data* pasd)
{
  // The only common attribute is currently Tag_compatibility,
  // accepted in both processor and "gnu" sections.
  for (int vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; ++vendor)
    {
      // Handle Tag_compatibility.  The tags are only compatible if the flags
      // are identical and, if the flags are '1', the strings are identical.
      // If the flags are non-zero, then we can only use the string "gnu".
      const Object_attribute* in_attr =
	&pasd->known_attributes(vendor)[Object_attribute::Tag_compatibility];
      Object_attribute* out_attr =
	&this->known_attributes(vendor)[Object_attribute::Tag_compatibility];

      if (in_attr->int_value() > 0
	  && in_attr->string_value() != "gnu")
	{
	  gold_error(_("%s: must be processed by '%s' toolchain"),
		     name, in_attr->string_value().c_str());
	  return;
	}

      if (in_attr->int_value() != out_attr->int_value()
	  || in_attr->string_value() != out_attr->string_value())
	{
	  gold_error(_("%s: object tag '%d, %s' is "
		       "incompatible with tag '%d, %s'"),
		     name, in_attr->int_value(),
		     in_attr->string_value().c_str(),
		     out_attr->int_value(),
		     out_attr->string_value().c_str());
	}
    }
}

// Write to a buffer.

void
Attributes_section_data::write(std::vector<unsigned char>* buffer) const
{
  buffer->push_back('A');
  for (int vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; ++vendor)
    if (this->vendor_object_attributes_[vendor]->size() != 0)
      this->vendor_object_attributes_[vendor]->write(buffer);
}

// Methods for Output_attributes_section_data.

// Write attributes section data to file OF.

void
Output_attributes_section_data::do_write(Output_file* of)
{
  off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  std::vector<unsigned char> buffer;
  this->attributes_section_data_.write(&buffer);
  gold_assert(convert_to_section_size_type(buffer.size()) == oview_size);
  memcpy(oview, &buffer.front(), buffer.size());
  of->write_output_view(this->offset(), oview_size, oview);
}

} // End namespace gold.
