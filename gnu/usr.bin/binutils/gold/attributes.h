// attributes.h -- object attributes for gold   -*- C++ -*-

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

// Handle object attributes.

#ifndef GOLD_ATTRIBUTES_H
#define GOLD_ATTRIBUTES_H

#include <map>

#include "parameters.h"
#include "target.h"
#include "output.h"
#include "reduced_debug_output.h"

namespace gold
{

// Object attribute values.  The attribute tag is not stored in this object.

class Object_attribute
{
 public:
  // The value of an object attribute.  The type indicates whether the
  // attribute holds and integer, a string, or both.  It can also indicate that
  // there can be no default (i.e. all values must be written to file, even
  // zero).
  enum
  {
    ATTR_TYPE_FLAG_INT_VAL = (1 << 0),
    ATTR_TYPE_FLAG_STR_VAL = (1 << 1),
    ATTR_TYPE_FLAG_NO_DEFAULT = (1 << 2)
  };

  // Object attributes may either be defined by the processor ABI, index
  // OBJ_ATTR_PROC in the *_obj_attributes arrays, or be GNU-specific
  // (and possibly also processor-specific), index OBJ_ATTR_GNU.
  enum
  {
    OBJ_ATTR_PROC,
    OBJ_ATTR_GNU,
    OBJ_ATTR_FIRST = OBJ_ATTR_PROC,
    OBJ_ATTR_LAST = OBJ_ATTR_GNU
  };

  // The following object attribute tags are taken as generic, for all
  // targets and for "gnu" where there is no target standard. 
  enum
  {
    Tag_NULL = 0,
    Tag_File = 1,
    Tag_Section = 2,
    Tag_Symbol = 3,
    Tag_compatibility = 32
  };

  Object_attribute()
   : type_(0), int_value_(0), string_value_()
  { }

  // Copying constructor.  We need to implement this to copy the string value.
  Object_attribute(const Object_attribute& oa)
   : type_(oa.type_), int_value_(oa.int_value_), string_value_(oa.string_value_)
  { }

  ~Object_attribute()
  { }

  // Assignment operator.  We need to implement this to copy the string value.
  Object_attribute&
  operator=(const Object_attribute& source)
  {
    this->type_ = source.type_;
    this->int_value_ = source.int_value_;
    this->string_value_ = source.string_value_;
    return *this;
  }

  // Return attribute type.
  int
  type() const
  { return this->type_; }

  // Set attribute type.
  void
  set_type(int type)
  { this->type_ = type; }

  // Return integer value.
  unsigned int
  int_value() const
  { return this->int_value_; }

  // Set integer value.
  void
  set_int_value(unsigned int i)
  { this->int_value_ = i; }

  // Return string value.
  const std::string&
  string_value() const
  { return this->string_value_; }

  // Set string value.
  void
  set_string_value(const std::string& s)
  { this->string_value_ = s; }

  void
  set_string_value(const char* s)
  { this->string_value_ = s; }

  // Whether attribute type has integer value.
  static bool
  attribute_type_has_int_value(int type)
  { return (type & ATTR_TYPE_FLAG_INT_VAL) != 0; }

  // Whether attribute type has string value.
  static bool
  attribute_type_has_string_value(int type)
  { return (type & ATTR_TYPE_FLAG_STR_VAL) != 0; }

  // Whether attribute type has no default value.
  static bool
  attribute_type_has_no_default(int type)
  { return (type & ATTR_TYPE_FLAG_NO_DEFAULT) != 0; }

  // Whether this has default value (0/"").
  bool
  is_default_attribute() const;

  // Return ULEB128 encoded size of tag and attribute.  
  size_t
  size(int tag) const;

  // Whether this matches another object attribute in merging.
  bool
  matches(const Object_attribute& oa) const;
  
  // Write to attribute with tag to BUFFER.
  void
  write(int tag, std::vector<unsigned char>* buffer) const;

  // Determine what arguments an attribute tag takes.
  static int
  arg_type(int vendor, int tag)
  {
    switch (vendor)
      {
      case OBJ_ATTR_PROC:
	return parameters->target().attribute_arg_type(tag);
      case OBJ_ATTR_GNU:
	return Object_attribute::gnu_arg_type(tag);
      default:
	gold_unreachable();
     }
  }

 private:
  // Determine whether a GNU object attribute tag takes an integer, a
  // string or both.  */
  static int
  gnu_arg_type(int tag)
  {
    // Except for Tag_compatibility, for GNU attributes we follow the
    // same rule ARM ones > 32 follow: odd-numbered tags take strings
    // and even-numbered tags take integers.  In addition, tag & 2 is
    // nonzero for architecture-independent tags and zero for
    // architecture-dependent ones.
    if (tag == Object_attribute::Tag_compatibility)
      return ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL;
    else
      return (tag & 1) != 0 ? ATTR_TYPE_FLAG_STR_VAL : ATTR_TYPE_FLAG_INT_VAL;
  }

  // Attribute type.
  int type_;
  // Integer value.
  int int_value_;
  // String value.
  std::string string_value_;
};

// This class contains attributes of a particular vendor.

class Vendor_object_attributes
{
 public:
  // The maximum number of known object attributes for any target.
  static const int NUM_KNOWN_ATTRIBUTES = 71;

  Vendor_object_attributes(int vendor)
    : vendor_(vendor), other_attributes_() 
  { }

  // Copying constructor.
  Vendor_object_attributes(const Vendor_object_attributes&);

  ~Vendor_object_attributes()
  {
    for (Other_attributes::iterator p = this->other_attributes_.begin();
	 p != this->other_attributes_.end();
	 ++p)
      delete p->second;
  }

  // Size of this in number of bytes.
  size_t
  size() const;
  
  // Name of this written vendor subsection.
  const char*
  name() const
  {
    return (this->vendor_ == Object_attribute::OBJ_ATTR_PROC
	    ? parameters->target().attributes_vendor()
	    : "gnu");
  }

  // Return an array of known attributes.
  Object_attribute*
  known_attributes()
  { return &this->known_attributes_[0]; }

  const Object_attribute*
  known_attributes() const
  { return &this->known_attributes_[0]; }

  typedef std::map<int, Object_attribute*> Other_attributes;

  // Return attributes other than the known ones.
  Other_attributes*
  other_attributes()
  { return &this->other_attributes_; }

  const Other_attributes*
  other_attributes() const
  { return &this->other_attributes_; }

  // Return a new attribute associated with TAG.
  Object_attribute*
  new_attribute(int tag);

  // Get an attribute
  Object_attribute*
  get_attribute(int tag);

  const Object_attribute*
  get_attribute(int tag) const;

  // Write to BUFFER.
  void
  write(std::vector<unsigned char>* buffer) const;

 private:
  // Vendor of the object attributes.
  int vendor_;
  // Attributes with known tags.  There are store in an array for fast
  // access.
  Object_attribute known_attributes_[NUM_KNOWN_ATTRIBUTES];
  // Attributes with known tags.  There are stored in a sorted container.
  Other_attributes other_attributes_;
};

// This class contains contents of an attributes section.

class Attributes_section_data
{
 public:
  // Construct an Attributes_section_data object by parsing section contents
  // in VIEW of SIZE.
  Attributes_section_data(const unsigned char* view, section_size_type size);

  // Copying constructor.
  Attributes_section_data(const Attributes_section_data& asd)
  {
    for (int vendor = Object_attribute::OBJ_ATTR_FIRST;
	 vendor <= Object_attribute::OBJ_ATTR_LAST;
	 ++vendor)
      this->vendor_object_attributes_[vendor] =
	new Vendor_object_attributes(*asd.vendor_object_attributes_[vendor]);
  }
  
  ~Attributes_section_data()
  {
    for (int vendor = Object_attribute::OBJ_ATTR_FIRST;
	 vendor <= Object_attribute::OBJ_ATTR_LAST;
	 ++vendor)
      delete this->vendor_object_attributes_[vendor];
  }
 
  // Return the size of this as number of bytes.
  size_t
  size() const;

  // Return an array of known attributes.
  Object_attribute*
  known_attributes(int vendor)
  {
    gold_assert(vendor >= OBJ_ATTR_FIRST && vendor <= OBJ_ATTR_LAST);
    return this->vendor_object_attributes_[vendor]->known_attributes();
  }

  const Object_attribute*
  known_attributes(int vendor) const
  {
    gold_assert(vendor >= OBJ_ATTR_FIRST && vendor <= OBJ_ATTR_LAST);
    return this->vendor_object_attributes_[vendor]->known_attributes();
  }

  // Return the other attributes.
  Vendor_object_attributes::Other_attributes*
  other_attributes(int vendor)
  {
    gold_assert(vendor >= OBJ_ATTR_FIRST && vendor <= OBJ_ATTR_LAST);
    return this->vendor_object_attributes_[vendor]->other_attributes();
  }

  // Return the other attributes.
  const Vendor_object_attributes::Other_attributes*
  other_attributes(int vendor) const
  {
    gold_assert(vendor >= OBJ_ATTR_FIRST && vendor <= OBJ_ATTR_LAST);
    return this->vendor_object_attributes_[vendor]->other_attributes();
  }

  // Return an attribute.
  Object_attribute*
  get_attribute(int vendor, int tag)
  {
    gold_assert(vendor >= OBJ_ATTR_FIRST && vendor <= OBJ_ATTR_LAST);
    return this->vendor_object_attributes_[vendor]->get_attribute(tag);
  }
  
  const Object_attribute*
  get_attribute(int vendor, int tag) const
  {
    gold_assert(vendor >= OBJ_ATTR_FIRST && vendor <= OBJ_ATTR_LAST);
    return this->vendor_object_attributes_[vendor]->get_attribute(tag);
  }
  
  // Merge target-independent attributes from another Attributes_section_data
  // of an object called NAME.
  void
  merge(const char* name, const Attributes_section_data* pasd);

  // Write to byte stream in an unsigned char vector.
  void
  write(std::vector<unsigned char>*) const;

 private:
  // For convenience.
  static const int OBJ_ATTR_FIRST = Object_attribute::OBJ_ATTR_FIRST;
  static const int OBJ_ATTR_LAST = Object_attribute::OBJ_ATTR_LAST;

  // Vendor object attributes.
  Vendor_object_attributes* vendor_object_attributes_[OBJ_ATTR_LAST+1];
};

// This class is used for writing out an Attribute_section_data.

class Output_attributes_section_data : public Output_section_data
{
 public:
  Output_attributes_section_data(const Attributes_section_data& asd)
    : Output_section_data(1), attributes_section_data_(asd)
  { }

 protected:
  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** attributes")); }

  // Write the data to the output file.
  void
  do_write(Output_file*);
  
  // Set final data size.
  void
  set_final_data_size()
  { this->set_data_size(attributes_section_data_.size()); }

 private:
  // Attributes_section_data corresponding to this.
  const Attributes_section_data& attributes_section_data_;
};

} // End namespace gold.

#endif	// !defined(GOLD_ATTRIBUTES_H)
