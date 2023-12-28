// expression.cc -- expressions in linker scripts for gold

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

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

#include <string>

#include "elfcpp.h"
#include "parameters.h"
#include "symtab.h"
#include "layout.h"
#include "output.h"
#include "script.h"
#include "script-c.h"

namespace gold
{

// This file holds the code which handles linker expressions.

// The dot symbol, which linker scripts refer to simply as ".",
// requires special treatment.  The dot symbol is set several times,
// section addresses will refer to it, output sections will change it,
// and it can be set based on the value of other symbols.  We simplify
// the handling by prohibiting setting the dot symbol to the value of
// a non-absolute symbol.

// When evaluating the value of an expression, we pass in a pointer to
// this struct, so that the expression evaluation can find the
// information it needs.

struct Expression::Expression_eval_info
{
  // The symbol table.
  const Symbol_table* symtab;
  // The layout--we use this to get section information.
  const Layout* layout;
  // Whether to check assertions.
  bool check_assertions;
  // Whether expressions can refer to the dot symbol.  The dot symbol
  // is only available within a SECTIONS clause.
  bool is_dot_available;
  // The current value of the dot symbol.
  uint64_t dot_value;
  // The section in which the dot symbol is defined; this is NULL if
  // it is absolute.
  Output_section* dot_section;
  // Points to where the section of the result should be stored.
  Output_section** result_section_pointer;
  // Pointer to where the alignment of the result should be stored.
  uint64_t* result_alignment_pointer;
  // Pointer to where the type of the symbol on the RHS should be stored.
  elfcpp::STT* type_pointer;
  // Pointer to where the visibility of the symbol on the RHS should be stored.
  elfcpp::STV* vis_pointer;
  // Pointer to where the rest of the symbol's st_other field should be stored.
  unsigned char* nonvis_pointer;
  // Whether the value is valid.  In Symbol_assignment::set_if_absolute, we
  // may be trying to evaluate the address of a section whose address is not
  // yet finalized, and we need to fail the evaluation gracefully.
  bool *is_valid_pointer;
};

// Evaluate an expression.

uint64_t
Expression::eval(const Symbol_table* symtab, const Layout* layout,
		 bool check_assertions)
{
  return this->eval_maybe_dot(symtab, layout, check_assertions, false, 0,
			      NULL, NULL, NULL, NULL, NULL, NULL, false, NULL);
}

// Evaluate an expression which may refer to the dot symbol.

uint64_t
Expression::eval_with_dot(const Symbol_table* symtab, const Layout* layout,
			  bool check_assertions, uint64_t dot_value,
			  Output_section* dot_section,
			  Output_section** result_section_pointer,
			  uint64_t* result_alignment_pointer,
			  bool is_section_dot_assignment)
{
  return this->eval_maybe_dot(symtab, layout, check_assertions, true,
			      dot_value, dot_section, result_section_pointer,
			      result_alignment_pointer, NULL, NULL, NULL,
			      is_section_dot_assignment, NULL);
}

// Evaluate an expression which may or may not refer to the dot
// symbol.

uint64_t
Expression::eval_maybe_dot(const Symbol_table* symtab, const Layout* layout,
			   bool check_assertions, bool is_dot_available,
			   uint64_t dot_value, Output_section* dot_section,
			   Output_section** result_section_pointer,
			   uint64_t* result_alignment_pointer,
			   elfcpp::STT* type_pointer,
			   elfcpp::STV* vis_pointer,
			   unsigned char* nonvis_pointer,
			   bool is_section_dot_assignment,
			   bool* is_valid_pointer)
{
  Expression_eval_info eei;
  eei.symtab = symtab;
  eei.layout = layout;
  eei.check_assertions = check_assertions;
  eei.is_dot_available = is_dot_available;
  eei.dot_value = dot_value;
  eei.dot_section = dot_section;

  // We assume the value is absolute, and only set this to a section
  // if we find a section-relative reference.
  if (result_section_pointer != NULL)
    *result_section_pointer = NULL;
  eei.result_section_pointer = result_section_pointer;

  // For symbol=symbol assignments, we need to track the type, visibility,
  // and remaining st_other bits.
  eei.type_pointer = type_pointer;
  eei.vis_pointer = vis_pointer;
  eei.nonvis_pointer = nonvis_pointer;

  eei.result_alignment_pointer = result_alignment_pointer;

  // Assume the value is valid until we try to evaluate an expression
  // that can't be evaluated yet.
  bool is_valid = true;
  eei.is_valid_pointer = &is_valid;

  uint64_t val = this->value(&eei);

  if (is_valid_pointer != NULL)
    *is_valid_pointer = is_valid;
  else
    gold_assert(is_valid);

  // If this is an assignment to dot within a section, and the value
  // is absolute, treat it as a section-relative offset.
  if (is_section_dot_assignment && *result_section_pointer == NULL)
    {
      gold_assert(dot_section != NULL);
      val += dot_section->address();
      *result_section_pointer = dot_section;
    }
  return val;
}

// A number.

class Integer_expression : public Expression
{
 public:
  Integer_expression(uint64_t val)
    : val_(val)
  { }

  uint64_t
  value(const Expression_eval_info*)
  { return this->val_; }

  void
  print(FILE* f) const
  { fprintf(f, "0x%llx", static_cast<unsigned long long>(this->val_)); }

 private:
  uint64_t val_;
};

extern "C" Expression*
script_exp_integer(uint64_t val)
{
  return new Integer_expression(val);
}

// An expression whose value is the value of a symbol.

class Symbol_expression : public Expression
{
 public:
  Symbol_expression(const char* name, size_t length)
    : name_(name, length)
  { }

  uint64_t
  value(const Expression_eval_info*);

  void
  set_expr_sym_in_real_elf(Symbol_table* symtab) const
  {
    Symbol* sym = symtab->lookup(this->name_.c_str());
    if (sym != NULL)
      sym->set_in_real_elf();
  }

  void
  print(FILE* f) const
  { fprintf(f, "%s", this->name_.c_str()); }

 private:
  std::string name_;
};

uint64_t
Symbol_expression::value(const Expression_eval_info* eei)
{
  Symbol* sym = eei->symtab->lookup(this->name_.c_str());
  if (sym == NULL || !sym->is_defined())
    {
      gold_error(_("undefined symbol '%s' referenced in expression"),
		 this->name_.c_str());
      return 0;
    }

  if (eei->result_section_pointer != NULL)
    *eei->result_section_pointer = sym->output_section();
  if (eei->type_pointer != NULL)
    *eei->type_pointer = sym->type();
  if (eei->vis_pointer != NULL)
    *eei->vis_pointer = sym->visibility();
  if (eei->nonvis_pointer != NULL)
    *eei->nonvis_pointer = sym->nonvis();

  if (parameters->target().get_size() == 32)
    return eei->symtab->get_sized_symbol<32>(sym)->value();
  else if (parameters->target().get_size() == 64)
    return eei->symtab->get_sized_symbol<64>(sym)->value();
  else
    gold_unreachable();
}

// An expression whose value is the value of the special symbol ".".
// This is only valid within a SECTIONS clause.

class Dot_expression : public Expression
{
 public:
  Dot_expression()
  { }

  uint64_t
  value(const Expression_eval_info*);

  void
  print(FILE* f) const
  { fprintf(f, "."); }
};

uint64_t
Dot_expression::value(const Expression_eval_info* eei)
{
  if (!eei->is_dot_available)
    {
      gold_error(_("invalid reference to dot symbol outside of "
		   "SECTIONS clause"));
      return 0;
    }
  if (eei->result_section_pointer != NULL)
    *eei->result_section_pointer = eei->dot_section;
  return eei->dot_value;
}

// A string.  This is either the name of a symbol, or ".".

extern "C" Expression*
script_exp_string(const char* name, size_t length)
{
  if (length == 1 && name[0] == '.')
    return new Dot_expression();
  else
    return new Symbol_expression(name, length);
}

// A unary expression.

class Unary_expression : public Expression
{
 public:
  Unary_expression(Expression* arg)
    : arg_(arg)
  { }

  ~Unary_expression()
  { delete this->arg_; }

 protected:
  uint64_t
  arg_value(const Expression_eval_info* eei,
	    Output_section** arg_section_pointer) const
  {
    return this->arg_->eval_maybe_dot(eei->symtab, eei->layout,
				      eei->check_assertions,
				      eei->is_dot_available,
				      eei->dot_value,
				      eei->dot_section,
				      arg_section_pointer,
				      eei->result_alignment_pointer,
				      NULL,
				      NULL,
				      NULL,
				      false,
				      eei->is_valid_pointer);
  }

  void
  arg_print(FILE* f) const
  { this->arg_->print(f); }

  void
  set_expr_sym_in_real_elf(Symbol_table* symtab) const
  { return this->arg_->set_expr_sym_in_real_elf(symtab); }

 private:
  Expression* arg_;
};

// Handle unary operators.  We use a preprocessor macro as a hack to
// capture the C operator.

#define UNARY_EXPRESSION(NAME, OPERATOR)				\
  class Unary_ ## NAME : public Unary_expression			\
  {									\
  public:								\
    Unary_ ## NAME(Expression* arg)					\
      : Unary_expression(arg)						\
    { }									\
    									\
    uint64_t								\
    value(const Expression_eval_info* eei)				\
    {									\
      Output_section* arg_section;					\
      uint64_t ret = OPERATOR this->arg_value(eei, &arg_section);	\
      if (arg_section != NULL && parameters->options().relocatable())	\
	gold_warning(_("unary " #NAME " applied to section "		\
		       "relative value"));				\
      return ret;							\
    }									\
									\
    void								\
    print(FILE* f) const						\
    {									\
      fprintf(f, "(%s ", #OPERATOR);					\
      this->arg_print(f);						\
      fprintf(f, ")");							\
    }									\
  };									\
									\
  extern "C" Expression*						\
  script_exp_unary_ ## NAME(Expression* arg)				\
  {									\
      return new Unary_ ## NAME(arg);					\
  }

UNARY_EXPRESSION(minus, -)
UNARY_EXPRESSION(logical_not, !)
UNARY_EXPRESSION(bitwise_not, ~)

// A binary expression.

class Binary_expression : public Expression
{
 public:
  Binary_expression(Expression* left, Expression* right)
    : left_(left), right_(right)
  { }

  ~Binary_expression()
  {
    delete this->left_;
    delete this->right_;
  }

 protected:
  uint64_t
  left_value(const Expression_eval_info* eei,
	     Output_section** section_pointer,
	     uint64_t* alignment_pointer) const
  {
    return this->left_->eval_maybe_dot(eei->symtab, eei->layout,
				       eei->check_assertions,
				       eei->is_dot_available,
				       eei->dot_value,
				       eei->dot_section,
				       section_pointer,
				       alignment_pointer,
				       NULL,
				       NULL,
				       NULL,
				       false,
				       eei->is_valid_pointer);
  }

  uint64_t
  right_value(const Expression_eval_info* eei,
	      Output_section** section_pointer,
	      uint64_t* alignment_pointer) const
  {
    return this->right_->eval_maybe_dot(eei->symtab, eei->layout,
					eei->check_assertions,
					eei->is_dot_available,
					eei->dot_value,
					eei->dot_section,
					section_pointer,
					alignment_pointer,
					NULL,
					NULL,
					NULL,
					false,
					eei->is_valid_pointer);
  }

  void
  left_print(FILE* f) const
  { this->left_->print(f); }

  void
  right_print(FILE* f) const
  { this->right_->print(f); }

  // This is a call to function FUNCTION_NAME.  Print it.  This is for
  // debugging.
  void
  print_function(FILE* f, const char* function_name) const
  {
    fprintf(f, "%s(", function_name);
    this->left_print(f);
    fprintf(f, ", ");
    this->right_print(f);
    fprintf(f, ")");
  }

  void
  set_expr_sym_in_real_elf(Symbol_table* symtab) const
  {
    this->left_->set_expr_sym_in_real_elf(symtab);
    this->right_->set_expr_sym_in_real_elf(symtab);
  }

 private:
  Expression* left_;
  Expression* right_;
};

// Handle binary operators.  We use a preprocessor macro as a hack to
// capture the C operator.  KEEP_LEFT means that if the left operand
// is section relative and the right operand is not, the result uses
// the same section as the left operand.  KEEP_RIGHT is the same with
// left and right swapped.  IS_DIV means that we need to give an error
// if the right operand is zero.  WARN means that we should warn if
// used on section relative values in a relocatable link.  We always
// warn if used on values in different sections in a relocatable link.

#define BINARY_EXPRESSION(NAME, OPERATOR, KEEP_LEFT, KEEP_RIGHT, IS_DIV, WARN) \
  class Binary_ ## NAME : public Binary_expression			\
  {									\
  public:								\
    Binary_ ## NAME(Expression* left, Expression* right)		\
      : Binary_expression(left, right)					\
    { }									\
									\
    uint64_t								\
    value(const Expression_eval_info* eei)				\
    {									\
      Output_section* left_section;					\
      uint64_t left_alignment = 0;					\
      uint64_t left = this->left_value(eei, &left_section,		\
				       &left_alignment);		\
      Output_section* right_section;					\
      uint64_t right_alignment = 0;					\
      uint64_t right = this->right_value(eei, &right_section,		\
					 &right_alignment);		\
      if (KEEP_RIGHT && left_section == NULL && right_section != NULL)	\
	{								\
	  if (eei->result_section_pointer != NULL)			\
	    *eei->result_section_pointer = right_section;		\
	  if (eei->result_alignment_pointer != NULL			\
	      && right_alignment > *eei->result_alignment_pointer)	\
	    *eei->result_alignment_pointer = right_alignment;		\
	}								\
      else if (KEEP_LEFT						\
	       && left_section != NULL					\
	       && right_section == NULL)				\
	{								\
	  if (eei->result_section_pointer != NULL)			\
	    *eei->result_section_pointer = left_section;		\
	  if (eei->result_alignment_pointer != NULL			\
	      && left_alignment > *eei->result_alignment_pointer)	\
	    *eei->result_alignment_pointer = left_alignment;		\
	}								\
      else if ((WARN || left_section != right_section)			\
	       && (left_section != NULL || right_section != NULL)	\
	       && parameters->options().relocatable())			\
	gold_warning(_("binary " #NAME " applied to section "		\
		       "relative value"));				\
      if (IS_DIV && right == 0)						\
	{								\
	  gold_error(_(#NAME " by zero"));				\
	  return 0;							\
	}								\
      return left OPERATOR right;					\
    }									\
									\
    void								\
    print(FILE* f) const						\
    {									\
      fprintf(f, "(");							\
      this->left_print(f);						\
      fprintf(f, " %s ", #OPERATOR);					\
      this->right_print(f);						\
      fprintf(f, ")");							\
    }									\
  };									\
									\
  extern "C" Expression*						\
  script_exp_binary_ ## NAME(Expression* left, Expression* right)	\
  {									\
    return new Binary_ ## NAME(left, right);				\
  }

BINARY_EXPRESSION(mult, *, false, false, false, true)
BINARY_EXPRESSION(div, /, false, false, true, true)
BINARY_EXPRESSION(mod, %, false, false, true, true)
BINARY_EXPRESSION(add, +, true, true, false, true)
BINARY_EXPRESSION(sub, -, true, false, false, false)
BINARY_EXPRESSION(lshift, <<, false, false, false, true)
BINARY_EXPRESSION(rshift, >>, false, false, false, true)
BINARY_EXPRESSION(eq, ==, false, false, false, false)
BINARY_EXPRESSION(ne, !=, false, false, false, false)
BINARY_EXPRESSION(le, <=, false, false, false, false)
BINARY_EXPRESSION(ge, >=, false, false, false, false)
BINARY_EXPRESSION(lt, <, false, false, false, false)
BINARY_EXPRESSION(gt, >, false, false, false, false)
BINARY_EXPRESSION(bitwise_and, &, true, true, false, true)
BINARY_EXPRESSION(bitwise_xor, ^, true, true, false, true)
BINARY_EXPRESSION(bitwise_or, |, true, true, false, true)
BINARY_EXPRESSION(logical_and, &&, false, false, false, true)
BINARY_EXPRESSION(logical_or, ||, false, false, false, true)

// A trinary expression.

class Trinary_expression : public Expression
{
 public:
  Trinary_expression(Expression* arg1, Expression* arg2, Expression* arg3)
    : arg1_(arg1), arg2_(arg2), arg3_(arg3)
  { }

  ~Trinary_expression()
  {
    delete this->arg1_;
    delete this->arg2_;
    delete this->arg3_;
  }

 protected:
  uint64_t
  arg1_value(const Expression_eval_info* eei,
	     Output_section** section_pointer) const
  {
    return this->arg1_->eval_maybe_dot(eei->symtab, eei->layout,
				       eei->check_assertions,
				       eei->is_dot_available,
				       eei->dot_value,
				       eei->dot_section,
				       section_pointer,
				       NULL,
				       NULL,
				       NULL,
				       NULL,
				       false,
				       eei->is_valid_pointer);
  }

  uint64_t
  arg2_value(const Expression_eval_info* eei,
	     Output_section** section_pointer,
	     uint64_t* alignment_pointer) const
  {
    return this->arg2_->eval_maybe_dot(eei->symtab, eei->layout,
				       eei->check_assertions,
				       eei->is_dot_available,
				       eei->dot_value,
				       eei->dot_section,
				       section_pointer,
				       alignment_pointer,
				       NULL,
				       NULL,
				       NULL,
				       false,
				       eei->is_valid_pointer);
  }

  uint64_t
  arg3_value(const Expression_eval_info* eei,
	     Output_section** section_pointer,
	     uint64_t* alignment_pointer) const
  {
    return this->arg3_->eval_maybe_dot(eei->symtab, eei->layout,
				       eei->check_assertions,
				       eei->is_dot_available,
				       eei->dot_value,
				       eei->dot_section,
				       section_pointer,
				       alignment_pointer,
				       NULL,
				       NULL,
				       NULL,
				       false,
				       eei->is_valid_pointer);
  }

  void
  arg1_print(FILE* f) const
  { this->arg1_->print(f); }

  void
  arg2_print(FILE* f) const
  { this->arg2_->print(f); }

  void
  arg3_print(FILE* f) const
  { this->arg3_->print(f); }

  void
  set_expr_sym_in_real_elf(Symbol_table* symtab) const
  {
    this->arg1_->set_expr_sym_in_real_elf(symtab);
    this->arg2_->set_expr_sym_in_real_elf(symtab);
    this->arg3_->set_expr_sym_in_real_elf(symtab);
  }

 private:
  Expression* arg1_;
  Expression* arg2_;
  Expression* arg3_;
};

// The conditional operator.

class Trinary_cond : public Trinary_expression
{
 public:
  Trinary_cond(Expression* arg1, Expression* arg2, Expression* arg3)
    : Trinary_expression(arg1, arg2, arg3)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    Output_section* arg1_section;
    uint64_t arg1 = this->arg1_value(eei, &arg1_section);
    return (arg1
	    ? this->arg2_value(eei, eei->result_section_pointer,
			       eei->result_alignment_pointer)
	    : this->arg3_value(eei, eei->result_section_pointer,
			       eei->result_alignment_pointer));
  }

  void
  print(FILE* f) const
  {
    fprintf(f, "(");
    this->arg1_print(f);
    fprintf(f, " ? ");
    this->arg2_print(f);
    fprintf(f, " : ");
    this->arg3_print(f);
    fprintf(f, ")");
  }
};

extern "C" Expression*
script_exp_trinary_cond(Expression* arg1, Expression* arg2, Expression* arg3)
{
  return new Trinary_cond(arg1, arg2, arg3);
}

// Max function.

class Max_expression : public Binary_expression
{
 public:
  Max_expression(Expression* left, Expression* right)
    : Binary_expression(left, right)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    Output_section* left_section;
    uint64_t left_alignment;
    uint64_t left = this->left_value(eei, &left_section, &left_alignment);
    Output_section* right_section;
    uint64_t right_alignment;
    uint64_t right = this->right_value(eei, &right_section, &right_alignment);
    if (left_section == right_section)
      {
	if (eei->result_section_pointer != NULL)
	  *eei->result_section_pointer = left_section;
      }
    else if ((left_section != NULL || right_section != NULL)
	     && parameters->options().relocatable())
      gold_warning(_("max applied to section relative value"));
    if (eei->result_alignment_pointer != NULL)
      {
	uint64_t ra = *eei->result_alignment_pointer;
	if (left > right)
	  ra = std::max(ra, left_alignment);
	else if (right > left)
	  ra = std::max(ra, right_alignment);
	else
	  ra = std::max(ra, std::max(left_alignment, right_alignment));
	*eei->result_alignment_pointer = ra;
      }
    return std::max(left, right);
  }

  void
  print(FILE* f) const
  { this->print_function(f, "MAX"); }
};

extern "C" Expression*
script_exp_function_max(Expression* left, Expression* right)
{
  return new Max_expression(left, right);
}

// Min function.

class Min_expression : public Binary_expression
{
 public:
  Min_expression(Expression* left, Expression* right)
    : Binary_expression(left, right)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    Output_section* left_section;
    uint64_t left_alignment;
    uint64_t left = this->left_value(eei, &left_section, &left_alignment);
    Output_section* right_section;
    uint64_t right_alignment;
    uint64_t right = this->right_value(eei, &right_section, &right_alignment);
    if (left_section == right_section)
      {
	if (eei->result_section_pointer != NULL)
	  *eei->result_section_pointer = left_section;
      }
    else if ((left_section != NULL || right_section != NULL)
	     && parameters->options().relocatable())
      gold_warning(_("min applied to section relative value"));
    if (eei->result_alignment_pointer != NULL)
      {
	uint64_t ra = *eei->result_alignment_pointer;
	if (left < right)
	  ra = std::max(ra, left_alignment);
	else if (right < left)
	  ra = std::max(ra, right_alignment);
	else
	  ra = std::max(ra, std::max(left_alignment, right_alignment));
	*eei->result_alignment_pointer = ra;
      }
    return std::min(left, right);
  }

  void
  print(FILE* f) const
  { this->print_function(f, "MIN"); }
};

extern "C" Expression*
script_exp_function_min(Expression* left, Expression* right)
{
  return new Min_expression(left, right);
}

// Class Section_expression.  This is a parent class used for
// functions which take the name of an output section.

class Section_expression : public Expression
{
 public:
  Section_expression(const char* section_name, size_t section_name_len)
    : section_name_(section_name, section_name_len)
  { }

  uint64_t
  value(const Expression_eval_info*);

  void
  print(FILE* f) const
  { fprintf(f, "%s(%s)", this->function_name(), this->section_name_.c_str()); }

 protected:
  // The child class must implement this.
  virtual uint64_t
  value_from_output_section(const Expression_eval_info*,
			    Output_section*) = 0;

  // The child class must implement this.
  virtual uint64_t
  value_from_script_output_section(uint64_t address, uint64_t load_address,
                                   uint64_t addralign, uint64_t size) = 0;

  // The child class must implement this.
  virtual const char*
  function_name() const = 0;

 private:
  std::string section_name_;
};

uint64_t
Section_expression::value(const Expression_eval_info* eei)
{
  const char* section_name = this->section_name_.c_str();
  Output_section* os = eei->layout->find_output_section(section_name);
  if (os != NULL)
    return this->value_from_output_section(eei, os);

  uint64_t address;
  uint64_t load_address;
  uint64_t addralign;
  uint64_t size;
  const Script_options* ss = eei->layout->script_options();
  if (ss->saw_sections_clause())
    {
      if (ss->script_sections()->get_output_section_info(section_name,
                                                         &address,
                                                         &load_address,
                                                         &addralign,
                                                         &size))
        return this->value_from_script_output_section(address, load_address,
                                                      addralign, size);
    }

  gold_error("%s called on nonexistent output section '%s'",
             this->function_name(), section_name);
  return 0;
}

// ABSOLUTE function.

class Absolute_expression : public Unary_expression
{
 public:
  Absolute_expression(Expression* arg)
    : Unary_expression(arg)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    uint64_t ret = this->arg_value(eei, NULL);
    // Force the value to be absolute.
    if (eei->result_section_pointer != NULL)
      *eei->result_section_pointer = NULL;
    return ret;
  }

  void
  print(FILE* f) const
  {
    fprintf(f, "ABSOLUTE(");
    this->arg_print(f);
    fprintf(f, ")");
  }
};

extern "C" Expression*
script_exp_function_absolute(Expression* arg)
{
  return new Absolute_expression(arg);
}

// ALIGN function.

class Align_expression : public Binary_expression
{
 public:
  Align_expression(Expression* left, Expression* right)
    : Binary_expression(left, right)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    Output_section* align_section;
    uint64_t align = this->right_value(eei, &align_section, NULL);
    if (align_section != NULL
	&& parameters->options().relocatable())
      gold_warning(_("aligning to section relative value"));

    if (eei->result_alignment_pointer != NULL
	&& align > *eei->result_alignment_pointer)
      {
	uint64_t a = align;
	while ((a & (a - 1)) != 0)
	  a &= a - 1;
	*eei->result_alignment_pointer = a;
      }

    uint64_t value = this->left_value(eei, eei->result_section_pointer, NULL);
    if (align <= 1)
      return value;
    return ((value + align - 1) / align) * align;
  }

  void
  print(FILE* f) const
  { this->print_function(f, "ALIGN"); }
};

extern "C" Expression*
script_exp_function_align(Expression* left, Expression* right)
{
  return new Align_expression(left, right);
}

// ASSERT function.

class Assert_expression : public Unary_expression
{
 public:
  Assert_expression(Expression* arg, const char* message, size_t length)
    : Unary_expression(arg), message_(message, length)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    uint64_t value = this->arg_value(eei, eei->result_section_pointer);
    if (!value && eei->check_assertions)
      gold_error("%s", this->message_.c_str());
    return value;
  }

  void
  print(FILE* f) const
  {
    fprintf(f, "ASSERT(");
    this->arg_print(f);
    fprintf(f, ", %s)", this->message_.c_str());
  }

 private:
  std::string message_;
};

extern "C" Expression*
script_exp_function_assert(Expression* expr, const char* message,
			   size_t length)
{
  return new Assert_expression(expr, message, length);
}

// ADDR function.

class Addr_expression : public Section_expression
{
 public:
  Addr_expression(const char* section_name, size_t section_name_len)
    : Section_expression(section_name, section_name_len)
  { }

 protected:
  uint64_t
  value_from_output_section(const Expression_eval_info* eei,
			    Output_section* os)
  {
    if (eei->result_section_pointer != NULL)
      *eei->result_section_pointer = os;
    if (os->is_address_valid())
      return os->address();
    *eei->is_valid_pointer = false;
    return 0;
  }

  uint64_t
  value_from_script_output_section(uint64_t address, uint64_t, uint64_t,
                                   uint64_t)
  { return address; }

  const char*
  function_name() const
  { return "ADDR"; }
};

extern "C" Expression*
script_exp_function_addr(const char* section_name, size_t section_name_len)
{
  return new Addr_expression(section_name, section_name_len);
}

// ALIGNOF.

class Alignof_expression : public Section_expression
{
 public:
  Alignof_expression(const char* section_name, size_t section_name_len)
    : Section_expression(section_name, section_name_len)
  { }

 protected:
  uint64_t
  value_from_output_section(const Expression_eval_info*,
			    Output_section* os)
  { return os->addralign(); }

  uint64_t
  value_from_script_output_section(uint64_t, uint64_t, uint64_t addralign,
                                   uint64_t)
  { return addralign; }

  const char*
  function_name() const
  { return "ALIGNOF"; }
};

extern "C" Expression*
script_exp_function_alignof(const char* section_name, size_t section_name_len)
{
  return new Alignof_expression(section_name, section_name_len);
}

// CONSTANT.  It would be nice if we could simply evaluate this
// immediately and return an Integer_expression, but unfortunately we
// don't know the target.

class Constant_expression : public Expression
{
 public:
  Constant_expression(const char* name, size_t length);

  uint64_t
  value(const Expression_eval_info*);

  void
  print(FILE* f) const;

 private:
  enum Constant_function
  {
    CONSTANT_MAXPAGESIZE,
    CONSTANT_COMMONPAGESIZE
  };

  Constant_function function_;
};

Constant_expression::Constant_expression(const char* name, size_t length)
{
  if (length == 11 && strncmp(name, "MAXPAGESIZE", length) == 0)
    this->function_ = CONSTANT_MAXPAGESIZE;
  else if (length == 14 && strncmp(name, "COMMONPAGESIZE", length) == 0)
    this->function_ = CONSTANT_COMMONPAGESIZE;
  else
    {
      std::string s(name, length);
      gold_error(_("unknown constant %s"), s.c_str());
      this->function_ = CONSTANT_MAXPAGESIZE;
    }
}

uint64_t
Constant_expression::value(const Expression_eval_info*)
{
  switch (this->function_)
    {
    case CONSTANT_MAXPAGESIZE:
      return parameters->target().abi_pagesize();
    case CONSTANT_COMMONPAGESIZE:
      return parameters->target().common_pagesize();
    default:
      gold_unreachable();
    }
}

void
Constant_expression::print(FILE* f) const
{
  const char* name;
  switch (this->function_)
    {
    case CONSTANT_MAXPAGESIZE:
      name = "MAXPAGESIZE";
      break;
    case CONSTANT_COMMONPAGESIZE:
      name = "COMMONPAGESIZE";
      break;
    default:
      gold_unreachable();
    }
  fprintf(f, "CONSTANT(%s)", name);
}
  
extern "C" Expression*
script_exp_function_constant(const char* name, size_t length)
{
  return new Constant_expression(name, length);
}

// DATA_SEGMENT_ALIGN.  FIXME: we don't implement this; we always fall
// back to the general case.

extern "C" Expression*
script_exp_function_data_segment_align(Expression* left, Expression*)
{
  Expression* e1 = script_exp_function_align(script_exp_string(".", 1), left);
  Expression* e2 = script_exp_binary_sub(left, script_exp_integer(1));
  Expression* e3 = script_exp_binary_bitwise_and(script_exp_string(".", 1),
						 e2);
  return script_exp_binary_add(e1, e3);
}

// DATA_SEGMENT_RELRO.  FIXME: This is not implemented.

extern "C" Expression*
script_exp_function_data_segment_relro_end(Expression*, Expression* right)
{
  return right;
}

// DATA_SEGMENT_END.  FIXME: This is not implemented.

extern "C" Expression*
script_exp_function_data_segment_end(Expression* val)
{
  return val;
}

// DEFINED function.

class Defined_expression : public Expression
{
 public:
  Defined_expression(const char* symbol_name, size_t symbol_name_len)
    : symbol_name_(symbol_name, symbol_name_len)
  { }

  uint64_t
  value(const Expression_eval_info* eei)
  {
    Symbol* sym = eei->symtab->lookup(this->symbol_name_.c_str());
    return sym != NULL && sym->is_defined();
  }

  void
  print(FILE* f) const
  { fprintf(f, "DEFINED(%s)", this->symbol_name_.c_str()); }

 private:
  std::string symbol_name_;
};

extern "C" Expression*
script_exp_function_defined(const char* symbol_name, size_t symbol_name_len)
{
  return new Defined_expression(symbol_name, symbol_name_len);
}

// LOADADDR function

class Loadaddr_expression : public Section_expression
{
 public:
  Loadaddr_expression(const char* section_name, size_t section_name_len)
    : Section_expression(section_name, section_name_len)
  { }

 protected:
  uint64_t
  value_from_output_section(const Expression_eval_info* eei,
			    Output_section* os)
  {
    if (os->has_load_address())
      return os->load_address();
    else
      {
	if (eei->result_section_pointer != NULL)
	  *eei->result_section_pointer = os;
	return os->address();
      }
  }

  uint64_t
  value_from_script_output_section(uint64_t, uint64_t load_address, uint64_t,
                                   uint64_t)
  { return load_address; }

  const char*
  function_name() const
  { return "LOADADDR"; }
};

extern "C" Expression*
script_exp_function_loadaddr(const char* section_name, size_t section_name_len)
{
  return new Loadaddr_expression(section_name, section_name_len);
}

// SIZEOF function

class Sizeof_expression : public Section_expression
{
 public:
  Sizeof_expression(const char* section_name, size_t section_name_len)
    : Section_expression(section_name, section_name_len)
  { }

 protected:
  uint64_t
  value_from_output_section(const Expression_eval_info*,
			    Output_section* os)
  {
    // We can not use data_size here, as the size of the section may
    // not have been finalized.  Instead we get whatever the current
    // size is.  This will work correctly for backward references in
    // linker scripts.
    return os->current_data_size();
  }

  uint64_t
  value_from_script_output_section(uint64_t, uint64_t, uint64_t,
                                   uint64_t size)
  { return size; }

  const char*
  function_name() const
  { return "SIZEOF"; }
};

extern "C" Expression*
script_exp_function_sizeof(const char* section_name, size_t section_name_len)
{
  return new Sizeof_expression(section_name, section_name_len);
}

// SIZEOF_HEADERS.

class Sizeof_headers_expression : public Expression
{
 public:
  Sizeof_headers_expression()
  { }

  uint64_t
  value(const Expression_eval_info*);

  void
  print(FILE* f) const
  { fprintf(f, "SIZEOF_HEADERS"); }
};

uint64_t
Sizeof_headers_expression::value(const Expression_eval_info* eei)
{
  unsigned int ehdr_size;
  unsigned int phdr_size;
  if (parameters->target().get_size() == 32)
    {
      ehdr_size = elfcpp::Elf_sizes<32>::ehdr_size;
      phdr_size = elfcpp::Elf_sizes<32>::phdr_size;
    }
  else if (parameters->target().get_size() == 64)
    {
      ehdr_size = elfcpp::Elf_sizes<64>::ehdr_size;
      phdr_size = elfcpp::Elf_sizes<64>::phdr_size;
    }
  else
    gold_unreachable();

  return ehdr_size + phdr_size * eei->layout->expected_segment_count();
}

extern "C" Expression*
script_exp_function_sizeof_headers()
{
  return new Sizeof_headers_expression();
}

// SEGMENT_START.

class Segment_start_expression : public Unary_expression
{
 public:
  Segment_start_expression(const char* segment_name, size_t segment_name_len,
			   Expression* default_value)
    : Unary_expression(default_value),
      segment_name_(segment_name, segment_name_len)
  { }

  uint64_t
  value(const Expression_eval_info*);

  void
  print(FILE* f) const
  {
    fprintf(f, "SEGMENT_START(\"%s\", ", this->segment_name_.c_str());
    this->arg_print(f);
    fprintf(f, ")");
  }

 private:
  std::string segment_name_;
};

uint64_t
Segment_start_expression::value(const Expression_eval_info* eei)
{
  // Check for command line overrides.
  if (parameters->options().user_set_Ttext()
      && this->segment_name_ == ".text")
    return parameters->options().Ttext();
  else if (parameters->options().user_set_Tdata()
	   && this->segment_name_ == ".data")
    return parameters->options().Tdata();
  else if (parameters->options().user_set_Tbss()
	   && this->segment_name_ == ".bss")
    return parameters->options().Tbss();
  else
    {
      uint64_t ret = this->arg_value(eei, NULL);
      // Force the value to be absolute.
      if (eei->result_section_pointer != NULL)
        *eei->result_section_pointer = NULL;
      return ret;
    }
}

extern "C" Expression*
script_exp_function_segment_start(const char* segment_name,
				  size_t segment_name_len,
				  Expression* default_value)
{
  return new Segment_start_expression(segment_name, segment_name_len,
				      default_value);
}

} // End namespace gold.
