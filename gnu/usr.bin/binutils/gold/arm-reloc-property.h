// arm-reloc-property.h -- ARM relocation properties   -*- C++ -*-

// Copyright (C) 2010-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com>.

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

#ifndef GOLD_ARM_RELOC_PROPERTY_H
#define GOLD_ARM_RELOC_PROPERTY_H

namespace gold
{
// The Arm_reloc_property class is to store information about a particular
// relocation code.

class Arm_reloc_property
{
 public:
  // Types of relocation codes.
  enum Reloc_type {
    RT_NONE,		// No relocation type.
    RT_STATIC,	// Relocations processed by static linkers.
    RT_DYNAMIC,	// Relocations processed by dynamic linkers.
    RT_PRIVATE,	// Private relocations, not supported by gold.
    RT_OBSOLETE	// Obsolete relocations that should not be used.
  };

  // Classes of relocation codes.
  enum Reloc_class {
    RC_NONE,	// No relocation class.
    RC_DATA,	// Data relocation.
    RC_ARM,	// ARM instruction relocation.
    RC_THM16,	// 16-bit THUMB instruction relocation.
    RC_THM32,	// 32-bit THUMB instruction relocation.
    RC_MISC	// Miscellaneous class.
  };

  // Types of bases of relative addressing relocation codes.
  enum Relative_address_base {
    RAB_NONE,		// Relocation is not relative addressing
    RAB_B_S,		// Address origin of output segment of defining symbol.
    RAB_DELTA_B_S,	// Change of address origin.
    RAB_GOT_ORG,	// Origin of GOT.
    RAB_P,		// Address of the place being relocated.
    RAB_Pa,		// Adjusted address (P & 0xfffffffc).
    RAB_TLS,		// Thread local storage.
    RAB_tp		// Thread pointer.
  };

  // Relocation code represented by this.
  unsigned int
  code() const
  { return this->code_; }

  // Name of the relocation code.
  const std::string&
  name() const
  { return this->name_; }
  
  // Type of relocation code.
  Reloc_type
  reloc_type() const
  { return this->reloc_type_; }

  // Whether this code is deprecated.
  bool
  is_deprecated() const
  { return this->is_deprecated_; }

  // Class of relocation code.
  Reloc_class
  reloc_class() const
  { return this->reloc_class_; }

  // Whether this code is implemented in gold.
  bool
  is_implemented() const
  { return this->is_implemented_; }

  // If code is a group relocation code, return the group number, otherwise -1.
  int
  group_index() const
  { return this->group_index_; }

  // Whether relocation checks for overflow.
  bool
  checks_overflow() const
  { return this->checks_overflow_; }

  // Return size of relocation.
  size_t
  size() const
  { return this->size_; }

  // Return alignment of relocation.
  size_t
  align() const
  { return this->align_; }

  // Whether relocation use a GOT entry.
  bool
  uses_got_entry() const
  { return this->uses_got_entry_; }

  // Whether relocation use a GOT origin.
  bool
  uses_got_origin() const
  { return this->uses_got_origin_; }
  
  // Whether relocation uses the Thumb-bit in a symbol address.
  bool
  uses_thumb_bit() const
  { return this->uses_thumb_bit_; }

  // Whether relocation uses the symbol base.
  bool
  uses_symbol_base() const
  { return this->uses_symbol_base_; }

  // Whether relocation uses the symbol.
  bool
  uses_symbol() const
  { return this->uses_symbol_; }

  // Return the type of relative address base or RAB_NONE if this
  // is not a relative addressing relocation.
  Relative_address_base
  relative_address_base() const
  { return this->relative_address_base_; } 

 protected:
  // These are protected.  We only allow Arm_reloc_property_table to
  // manage Arm_reloc_property. 
  Arm_reloc_property(unsigned int code, const char* name, Reloc_type rtype,
		     bool is_deprecated, Reloc_class rclass,
		     const std::string& operation, bool is_implemented,
		     int group_index, bool checks_overflow);

  friend class Arm_reloc_property_table;
  
 private:
  // Copying is not allowed.
  Arm_reloc_property(const Arm_reloc_property&);
  Arm_reloc_property& operator=(const Arm_reloc_property&);

  // The Tree_node class is used to represent parsed relocation operations. 
  // We look at Trees to extract information about relocation operations.
  class Tree_node
  {
   public:
    typedef std::vector<Tree_node*> Tree_node_vector;

    // Construct a leaf node.
    Tree_node(const char* name)
      : is_leaf_(true), name_(name), children_()
    { }

    // Construct an internal node.  A node owns all its children and is
    // responsible for releasing them at its own destruction.
    Tree_node(Tree_node_vector::const_iterator begin,
	      Tree_node_vector::const_iterator end)
      : is_leaf_(false), name_(), children_()
    {
      for (Tree_node_vector::const_iterator p = begin; p != end; ++p)
	this->children_.push_back(*p);
    }

    ~Tree_node()
    {
      for(size_t i = 0; i <this->children_.size(); ++i)
	delete this->children_[i];
    }

    // Whether this is a leaf node.
    bool
    is_leaf() const
    { return this->is_leaf_; }

    // Return name of this.  This is only valid for a leaf node.
    const std::string&
    name() const
    {
      gold_assert(this->is_leaf_);
      return this->name_;
    }

    // Return the number of children.  This is only valid for a non-leaf node.
    size_t
    number_of_children() const
    {
      gold_assert(!this->is_leaf_);
      return this->children_.size();
    }

    // Return the i-th child of this.  This is only valid for a non-leaf node.
    Tree_node*
    child(size_t i) const
    {
      gold_assert(!this->is_leaf_ && i < this->children_.size());
      return this->children_[i];
    }

    // Parse an S-expression string and build a tree and return the root node.
    // Caller is responsible for releasing tree after use.
    static Tree_node*
    make_tree(const std::string&);

    // Convert a tree back to an S-expression string.
    std::string
    s_expression() const
    {
      if (this->is_leaf_)
	return this->name_;

      // Concatenate S-expressions of children. Enclose them with
      // a pair of parentheses and use space as token delimiters.
      std::string s("(");
      for(size_t i = 0; i <this->children_.size(); ++i)
	s = s + " " + this->children_[i]->s_expression();
      return s + " )";
    }

   private:
    // Whether this is a leaf node.
    bool is_leaf_;
    // Name of this if this is a leaf node.
    std::string name_;
    // Children of this if this a non-leaf node.
    Tree_node_vector children_;
  };

  // Relocation code.
  unsigned int code_;
  // Relocation name.
  std::string name_;
  // Type of relocation.
  Reloc_type reloc_type_;
  // Class of relocation.
  Reloc_class reloc_class_;
  // Group index (0, 1, or 2) if this is a group relocation or -1 otherwise.
  int group_index_; 
  // Size of relocation.
  size_t size_;
  // Alignment of relocation.
  size_t align_;
  // Relative address base.
  Relative_address_base relative_address_base_;
  // Whether this is deprecated.
  bool is_deprecated_ : 1;
  // Whether this is implemented in gold.
  bool is_implemented_ : 1;
  // Whether this checks overflow.
  bool checks_overflow_ : 1;
  // Whether this uses a GOT entry.
  bool uses_got_entry_ : 1;
  // Whether this uses a GOT origin.
  bool uses_got_origin_ : 1;
  // Whether this uses a PLT entry.
  bool uses_plt_entry_ : 1;
  // Whether this uses the THUMB bit in symbol address.
  bool uses_thumb_bit_ : 1;
  // Whether this uses the symbol base.
  bool uses_symbol_base_ : 1;
  // Whether this uses an addend.
  bool uses_addend_ : 1;
  // Whether this uses the symbol.
  bool uses_symbol_ : 1;
};

// Arm_reloc_property_table.  This table is used for looking up properties
// of relocation types.  The table entries are initialized using information
// from arm-reloc.def.

class Arm_reloc_property_table
{
 public:
  Arm_reloc_property_table();

  // Return an Arm_reloc_property object for CODE if it is a valid relocation
  // code or NULL otherwise.
  const Arm_reloc_property*
  get_reloc_property(unsigned int code) const
  {
    gold_assert(code < Property_table_size);
    return this->table_[code];
  }

  // Like get_reloc_property but only return non-NULL if relocation code is
  // static and implemented.
  const Arm_reloc_property*
  get_implemented_static_reloc_property(unsigned int code) const
  {
    gold_assert(code < Property_table_size);
    const Arm_reloc_property* arp = this->table_[code];
    return ((arp != NULL
	     && (arp->reloc_type() == Arm_reloc_property::RT_STATIC)
	     && arp->is_implemented())
	    ? arp
	    : NULL);
  }
  
  // Return a string describing the relocation code that is not
  // an implemented static reloc code.
  std::string
  reloc_name_in_error_message(unsigned int code);

 private:
  // Copying is not allowed.
  Arm_reloc_property_table(const Arm_reloc_property_table&);
  Arm_reloc_property_table& operator=(const Arm_reloc_property_table&);

  // The Parse_expression class is used to convert relocation operations in
  // arm-reloc.def into S-expression strings, which are parsed again to
  // build actual expression trees.  We do not build the expression trees
  // directly because the parser for operations in arm-reloc.def is simpler
  // this way.  Conversion from S-expressions to trees is simple.
  class Parse_expression
  {
   public:
    // Construction a Parse_expression with an S-expression string.
    Parse_expression(const std::string& s_expression)
      : s_expression_(s_expression)
    { }

    // Value of this expression as an S-expression string.
    const std::string&
    s_expression() const
    { return this->s_expression_; }

    // We want to overload operators used in relocation operations so
    // that we can execute operations in arm-reloc.def to generate
    // S-expressions directly.
#define DEF_OPERATOR_OVERLOAD(op) \
    Parse_expression \
    operator op (const Parse_expression& e) \
    { \
      return Parse_expression("( " #op " " + this->s_expression_ + " " + \
			      e.s_expression_ + " )"); \
    }

    // Operator appearing in relocation operations in arm-reloc.def.
    DEF_OPERATOR_OVERLOAD(+)
    DEF_OPERATOR_OVERLOAD(-)
    DEF_OPERATOR_OVERLOAD(|)
    
   private:
    // This represented as an S-expression string.
    std::string s_expression_;
  };

#define DEF_RELOC_FUNC(name) \
  static Parse_expression \
  (name)(const Parse_expression& arg) \
  { return Parse_expression("( " #name " " + arg.s_expression() + " )"); }

  // Functions appearing in relocation operations in arm-reloc.def.
  DEF_RELOC_FUNC(B)
  DEF_RELOC_FUNC(DELTA_B)
  DEF_RELOC_FUNC(GOT)
  DEF_RELOC_FUNC(Module)
  DEF_RELOC_FUNC(PLT)

  static const unsigned int Property_table_size = 256;

  // The property table.
  Arm_reloc_property* table_[Property_table_size];
};

} // End namespace gold.

#endif // !defined(GOLD_ARM_RELOC_PROPERTY_H)
