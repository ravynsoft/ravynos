// arm-reloc-property.cc -- ARM relocation property.

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

#include "gold.h"

#include <cstdio>
#include <cstring>
#include <stack>
#include <string>
#include <vector>

#include "elfcpp.h"
#include "arm.h"
#include "arm-reloc-property.h"

namespace gold
{

// Arm_reloc_property::Tree_node methods.

// Parse an S-expression S and build a tree and return the root node.
// Caller is responsible for releasing tree after use.

Arm_reloc_property::Tree_node*
Arm_reloc_property::Tree_node::make_tree(const std::string& s)
{
  std::stack<size_t> size_stack;
  Tree_node_vector node_stack;

  // strtok needs a non-const string pointer.
  char* buffer = new char[s.size() + 1];
  memcpy(buffer, s.data(), s.size());
  buffer[s.size()] = '\0';
  char* token = strtok(buffer, " ");

  while (token != NULL)
    {
      if (strcmp(token, "(") == 0)
	// Remember the node stack position for start of a new internal node.
	size_stack.push(node_stack.size());
      else if (strcmp(token, ")") == 0)
	{
	  // Pop all tree nodes after the previous '(' and use them as
	  // children to build a new internal node.  Push internal node back.
	  size_t current_size = node_stack.size();
	  size_t prev_size = size_stack.top();
	  size_stack.pop();
	  Tree_node* node =
	    new Tree_node(node_stack.begin() + prev_size,
			  node_stack.begin() + current_size); 
	  node_stack.resize(prev_size);
	  node_stack.push_back(node);
	}
      else
	// Just push a leaf node to node_stack.
	node_stack.push_back(new Tree_node(token));

      token = strtok(NULL, " ");
    }

  delete[] buffer;

  // At this point, size_stack should be empty and node_stack should only
  // contain the root node.
  gold_assert(size_stack.empty() && node_stack.size() == 1);
  return node_stack[0];
}

// Arm_reloc_property methods.

// Constructor.

Arm_reloc_property::Arm_reloc_property(
    unsigned int code,
    const char* name,
    Reloc_type rtype,
    bool is_deprecated,
    Reloc_class rclass,
    const std::string& operation,
    bool is_implemented,
    int group_index,
    bool checks_overflow)
  : code_(code), name_(name), reloc_type_(rtype), reloc_class_(rclass),
    group_index_(group_index), size_(0), align_(1),
    relative_address_base_(RAB_NONE), is_deprecated_(is_deprecated),
    is_implemented_(is_implemented), checks_overflow_(checks_overflow),
    uses_got_entry_(false), uses_got_origin_(false), uses_plt_entry_(false),
    uses_thumb_bit_(false), uses_symbol_base_(false), uses_addend_(false),
    uses_symbol_(false)
{
  // Set size and alignment of static and dynamic relocations.
  if (rtype == RT_STATIC)
    {
      switch (rclass)
	{
	case RC_DATA:
	  // Except for R_ARM_ABS16 and R_ARM_ABS8, all static data relocations
	  // have size 4.  All static data relocations have alignment of 1.
	  if (code == elfcpp::R_ARM_ABS8)
	    this->size_ = 1;
	  else if (code == elfcpp::R_ARM_ABS16)
	    this->size_ = 2;
	  else
	    this->size_ = 4;
	  this->align_ = 1;
	  break;
	case RC_MISC:
	  // R_ARM_V4BX should be treated as an ARM relocation.  For all
	  // others, just use defaults.
	  if (code != elfcpp::R_ARM_V4BX)
	    break;
	  // Fall through.
	case RC_ARM:
	  this->size_ = 4;
	  this->align_ = 4;
	  break;
	case RC_THM16:
	  this->size_ = 2;
	  this->align_ = 2;
	  break;
	case RC_THM32:
	  this->size_ = 4;
	  this->align_ = 2;
	  break;
	default:
	  gold_unreachable();
	}
    }
  else if (rtype == RT_DYNAMIC)
    {
      // With the exception of R_ARM_COPY, all dynamic relocations requires
      // that the place being relocated is a word-aligned 32-bit object.
      if (code != elfcpp::R_ARM_COPY)
	{
	  this->size_ = 4;
	  this->align_ = 4;
	}
    }

  // If no relocation operation is specified, we are done.
  if (operation == "NONE")
    return;

  // Extract information from relocation operation.
  Tree_node* root_node = Tree_node::make_tree(operation);
  Tree_node* node = root_node;

  // Check for an expression of the form XXX - YYY.
  if (!node->is_leaf()
      && node->child(0)->is_leaf()
      && node->child(0)->name() == "-")
    {
      struct RAB_table_entry
      {
	Relative_address_base rab;
	const char* name;
      };

      static const RAB_table_entry rab_table[] =
      {
	{ RAB_B_S, "( B S )" },
	{ RAB_DELTA_B_S, "( DELTA_B ( S ) )" },
      	{ RAB_GOT_ORG, "GOT_ORG" },
      	{ RAB_P, "P" },
      	{ RAB_Pa, "Pa" },
      	{ RAB_TLS, "TLS" },
      	{ RAB_tp, "tp" }
      };

      static size_t rab_table_size = sizeof(rab_table) / sizeof(rab_table[0]);
      const std::string rhs(node->child(2)->s_expression());
      for (size_t i = 0; i < rab_table_size; ++i)
	if (rhs == rab_table[i].name)
	  {
	    this->relative_address_base_ = rab_table[i].rab;
	    break;
	  }

      gold_assert(this->relative_address_base_ != RAB_NONE);
      if (this->relative_address_base_ == RAB_B_S)
	this->uses_symbol_base_ = true;
      node = node->child(1);
    }

  // Check for an expression of the form XXX | T.
  if (!node->is_leaf()
      && node->child(0)->is_leaf()
      && node->child(0)->name() == "|")
    {
      gold_assert(node->number_of_children() == 3
		  && node->child(2)->is_leaf()
		  && node->child(2)->name() == "T");
      this->uses_thumb_bit_ = true;
      node = node->child(1);
    }

  // Check for an expression of the form XXX + A.
  if (!node->is_leaf()
      && node->child(0)->is_leaf()
      && node->child(0)->name() == "+")
    {
      gold_assert(node->number_of_children() == 3
		  && node->child(2)->is_leaf()
		  && node->child(2)->name() == "A");
      this->uses_addend_ = true;
      node = node->child(1);
    }

  // Check for an expression of the form XXX(S).
  if (!node->is_leaf() && node->child(0)->is_leaf())
    {
      gold_assert(node->number_of_children() == 2
		  && node->child(1)->is_leaf()
		  && node->child(1)->name() == "S");
      const std::string func(node->child(0)->name());
      if (func == "B")
	this->uses_symbol_base_ = true;
      else if (func == "GOT")
	this->uses_got_entry_ = true;
      else if (func == "PLT")
	this->uses_plt_entry_ = true;
      else if (func == "Module" || func == "DELTA_B")
	// These are used in dynamic relocations.
	;
      else
	gold_unreachable();
      node = node->child(1);
    }

  gold_assert(node->is_leaf() && node->name() == "S");
  this->uses_symbol_ = true;

  delete root_node;
}

// Arm_reloc_property_table methods.

// Constructor.  This processing informations in arm-reloc.def to
// initialize the table.

Arm_reloc_property_table::Arm_reloc_property_table()
{
  // These appear in arm-reloc.def.  Do not rename them.
  Parse_expression A("A"), GOT_ORG("GOT_ORG"), NONE("NONE"), P("P"),
		   Pa("Pa"), S("S"), T("T"), TLS("TLS"), tp("tp");
  const bool Y(true), N(false);

  for (unsigned int i = 0; i < Property_table_size; ++i)
    this->table_[i] = NULL;

#undef RD
#define RD(name, type, deprecated, class, operation, is_implemented, \
	   group_index, checks_oveflow) \
  do \
    { \
      unsigned int code = elfcpp::R_ARM_##name; \
      gold_assert(code < Property_table_size); \
      this->table_[code] = \
	new Arm_reloc_property(elfcpp::R_ARM_##name, "R_ARM_" #name, \
			       Arm_reloc_property::RT_##type, deprecated, \
			       Arm_reloc_property::RC_##class, \
			       (operation).s_expression(), is_implemented, \
			       group_index, checks_oveflow); \
    } \
  while(0);

#include "arm-reloc.def"
#undef RD
}

// Return a string describing a relocation code that fails to get a
// relocation property in get_implemented_static_reloc_property().

std::string
Arm_reloc_property_table::reloc_name_in_error_message(unsigned int code)
{
  gold_assert(code < Property_table_size);

  const Arm_reloc_property* arp = this->table_[code];

  if (arp == NULL)
    {
      char buffer[100];
      sprintf(buffer, _("invalid reloc %u"), code);
      return std::string(buffer);
    }
  
  // gold only implements static relocation codes.
  Arm_reloc_property::Reloc_type reloc_type = arp->reloc_type();
  gold_assert(reloc_type == Arm_reloc_property::RT_STATIC
	      || !arp->is_implemented());

  const char* prefix = NULL;
  switch (reloc_type)
    {
    case Arm_reloc_property::RT_STATIC:
      prefix = arp->is_implemented() ? _("reloc ") : _("unimplemented reloc ");
      break;
    case Arm_reloc_property::RT_DYNAMIC:
      prefix = _("dynamic reloc ");
      break;
    case Arm_reloc_property::RT_PRIVATE:
      prefix = _("private reloc ");
      break;
    case Arm_reloc_property::RT_OBSOLETE:
      prefix = _("obsolete reloc ");
      break;
    default:
      gold_unreachable();
    }
  return std::string(prefix) + arp->name();
}

} // End namespace gold.
