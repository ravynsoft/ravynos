/*
 * Copyright © 2019 Google, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file lower_precision.cpp
 */

#include "main/macros.h"
#include "main/consts_exts.h"
#include "compiler/glsl_types.h"
#include "ir.h"
#include "ir_builder.h"
#include "ir_optimization.h"
#include "ir_rvalue_visitor.h"
#include "util/half_float.h"
#include "util/set.h"
#include "util/hash_table.h"
#include <vector>

namespace {

class find_precision_visitor : public ir_rvalue_enter_visitor {
public:
   find_precision_visitor(const struct gl_shader_compiler_options *options);
   ~find_precision_visitor();

   virtual void handle_rvalue(ir_rvalue **rvalue);
   virtual ir_visitor_status visit_enter(ir_call *ir);

   ir_function_signature *map_builtin(ir_function_signature *sig);

   /* Set of rvalues that can be lowered. This will be filled in by
    * find_lowerable_rvalues_visitor. Only the root node of a lowerable section
    * will be added to this set.
    */
   struct set *lowerable_rvalues;

   /**
    * A mapping of builtin signature functions to lowered versions. This is
    * filled in lazily when a lowered version is needed.
    */
   struct hash_table *lowered_builtins;
   /**
    * A temporary hash table only used in order to clone functions.
    */
   struct hash_table *clone_ht;

   void *lowered_builtin_mem_ctx;

   const struct gl_shader_compiler_options *options;
};

class find_lowerable_rvalues_visitor : public ir_hierarchical_visitor {
public:
   enum can_lower_state {
      UNKNOWN,
      CANT_LOWER,
      SHOULD_LOWER,
   };

   enum parent_relation {
      /* The parent performs a further operation involving the result from the
       * child and can be lowered along with it.
       */
      COMBINED_OPERATION,
      /* The parent instruction’s operation is independent of the child type so
       * the child should be lowered separately.
       */
      INDEPENDENT_OPERATION,
   };

   struct stack_entry {
      ir_instruction *instr;
      enum can_lower_state state;
      /* List of child rvalues that can be lowered. When this stack entry is
       * popped, if this node itself can’t be lowered than all of the children
       * are root nodes to lower so we will add them to lowerable_rvalues.
       * Otherwise if this node can also be lowered then we won’t add the
       * children because we only want to add the topmost lowerable nodes to
       * lowerable_rvalues and the children will be lowered as part of lowering
       * this node.
       */
      std::vector<ir_instruction *> lowerable_children;
   };

   find_lowerable_rvalues_visitor(struct set *result,
                                  const struct gl_shader_compiler_options *options);

   static void stack_enter(class ir_instruction *ir, void *data);
   static void stack_leave(class ir_instruction *ir, void *data);

   virtual ir_visitor_status visit(ir_constant *ir);
   virtual ir_visitor_status visit(ir_dereference_variable *ir);

   virtual ir_visitor_status visit_enter(ir_dereference_record *ir);
   virtual ir_visitor_status visit_enter(ir_dereference_array *ir);
   virtual ir_visitor_status visit_enter(ir_texture *ir);
   virtual ir_visitor_status visit_enter(ir_expression *ir);

   virtual ir_visitor_status visit_leave(ir_assignment *ir);
   virtual ir_visitor_status visit_leave(ir_call *ir);

   can_lower_state handle_precision(const glsl_type *type,
                                    int precision) const;

   static parent_relation get_parent_relation(ir_instruction *parent,
                                              ir_instruction *child);

   std::vector<stack_entry> stack;
   struct set *lowerable_rvalues;
   const struct gl_shader_compiler_options *options;

   void pop_stack_entry();
   void add_lowerable_children(const stack_entry &entry);
};

class lower_precision_visitor : public ir_rvalue_visitor {
public:
   virtual void handle_rvalue(ir_rvalue **rvalue);
   virtual ir_visitor_status visit_enter(ir_dereference_array *);
   virtual ir_visitor_status visit_enter(ir_dereference_record *);
   virtual ir_visitor_status visit_enter(ir_call *ir);
   virtual ir_visitor_status visit_enter(ir_texture *ir);
   virtual ir_visitor_status visit_leave(ir_expression *);
};

static bool
can_lower_type(const struct gl_shader_compiler_options *options,
               const glsl_type *type)
{
   /* Don’t lower any expressions involving non-float types except bool and
    * texture samplers. This will rule out operations that change the type such
    * as conversion to ints. Instead it will end up lowering the arguments
    * instead and adding a final conversion to float32. We want to handle
    * boolean types so that it will do comparisons as 16-bit.
    */

   switch (glsl_without_array(type)->base_type) {
   /* TODO: should we do anything for these two with regard to Int16 vs FP16
    * support?
    */
   case GLSL_TYPE_BOOL:
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_IMAGE:
      return true;

   case GLSL_TYPE_FLOAT:
      return options->LowerPrecisionFloat16;

   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
      return options->LowerPrecisionInt16;

   default:
      return false;
   }
}

find_lowerable_rvalues_visitor::find_lowerable_rvalues_visitor(struct set *res,
                                 const struct gl_shader_compiler_options *opts)
{
   lowerable_rvalues = res;
   options = opts;
   callback_enter = stack_enter;
   callback_leave = stack_leave;
   data_enter = this;
   data_leave = this;
}

void
find_lowerable_rvalues_visitor::stack_enter(class ir_instruction *ir,
                                            void *data)
{
   find_lowerable_rvalues_visitor *state =
      (find_lowerable_rvalues_visitor *) data;

   /* Add a new stack entry for this instruction */
   stack_entry entry;

   entry.instr = ir;
   entry.state = state->in_assignee ? CANT_LOWER : UNKNOWN;

   state->stack.push_back(entry);
}

void
find_lowerable_rvalues_visitor::add_lowerable_children(const stack_entry &entry)
{
   /* We can’t lower this node so if there were any pending children then they
    * are all root lowerable nodes and we should add them to the set.
    */
   for (auto &it : entry.lowerable_children)
      _mesa_set_add(lowerable_rvalues, it);
}

void
find_lowerable_rvalues_visitor::pop_stack_entry()
{
   const stack_entry &entry = stack.back();

   if (stack.size() >= 2) {
      /* Combine this state into the parent state, unless the parent operation
       * doesn’t have any relation to the child operations
       */
      stack_entry &parent = stack.end()[-2];
      parent_relation rel = get_parent_relation(parent.instr, entry.instr);

      if (rel == COMBINED_OPERATION) {
         switch (entry.state) {
         case CANT_LOWER:
            parent.state = CANT_LOWER;
            break;
         case SHOULD_LOWER:
            if (parent.state == UNKNOWN)
               parent.state = SHOULD_LOWER;
            break;
         case UNKNOWN:
            break;
         }
      }
   }

   if (entry.state == SHOULD_LOWER) {
      ir_rvalue *rv = entry.instr->as_rvalue();

      if (rv == NULL) {
         add_lowerable_children(entry);
      } else if (stack.size() >= 2) {
         stack_entry &parent = stack.end()[-2];

         switch (get_parent_relation(parent.instr, rv)) {
         case COMBINED_OPERATION:
            /* We only want to add the toplevel lowerable instructions to the
             * lowerable set. Therefore if there is a parent then instead of
             * adding this instruction to the set we will queue depending on
             * the result of the parent instruction.
             */
            parent.lowerable_children.push_back(entry.instr);
            break;
         case INDEPENDENT_OPERATION:
            _mesa_set_add(lowerable_rvalues, rv);
            break;
         }
      } else {
         /* This is a toplevel node so add it directly to the lowerable
          * set.
          */
         _mesa_set_add(lowerable_rvalues, rv);
      }
   } else if (entry.state == CANT_LOWER) {
      add_lowerable_children(entry);
   }

   stack.pop_back();
}

void
find_lowerable_rvalues_visitor::stack_leave(class ir_instruction *ir,
                                            void *data)
{
   find_lowerable_rvalues_visitor *state =
      (find_lowerable_rvalues_visitor *) data;

   state->pop_stack_entry();
}

enum find_lowerable_rvalues_visitor::can_lower_state
find_lowerable_rvalues_visitor::handle_precision(const glsl_type *type,
                                                 int precision) const
{
   if (!can_lower_type(options, type))
      return CANT_LOWER;

   switch (precision) {
   case GLSL_PRECISION_NONE:
      return UNKNOWN;
   case GLSL_PRECISION_HIGH:
      return CANT_LOWER;
   case GLSL_PRECISION_MEDIUM:
   case GLSL_PRECISION_LOW:
      return SHOULD_LOWER;
   }

   return CANT_LOWER;
}

enum find_lowerable_rvalues_visitor::parent_relation
find_lowerable_rvalues_visitor::get_parent_relation(ir_instruction *parent,
                                                    ir_instruction *child)
{
   /* If the parent is a dereference instruction then the only child could be
    * for example an array dereference and that should be lowered independently
    * of the parent.
    */
   if (parent->as_dereference())
      return INDEPENDENT_OPERATION;

   /* The precision of texture sampling depend on the precision of the sampler.
    * The rest of the arguments don’t matter so we can treat it as an
    * independent operation.
    */
   if (parent->as_texture())
      return INDEPENDENT_OPERATION;

   return COMBINED_OPERATION;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit(ir_constant *ir)
{
   stack_enter(ir, this);

   if (!can_lower_type(options, ir->type))
      stack.back().state = CANT_LOWER;

   stack_leave(ir, this);

   return visit_continue;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit(ir_dereference_variable *ir)
{
   stack_enter(ir, this);

   if (stack.back().state == UNKNOWN)
      stack.back().state = handle_precision(ir->type, ir->precision());

   stack_leave(ir, this);

   return visit_continue;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit_enter(ir_dereference_record *ir)
{
   ir_hierarchical_visitor::visit_enter(ir);

   if (stack.back().state == UNKNOWN)
      stack.back().state = handle_precision(ir->type, ir->precision());

   return visit_continue;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit_enter(ir_dereference_array *ir)
{
   ir_hierarchical_visitor::visit_enter(ir);

   if (stack.back().state == UNKNOWN)
      stack.back().state = handle_precision(ir->type, ir->precision());

   return visit_continue;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit_enter(ir_texture *ir)
{
   ir_hierarchical_visitor::visit_enter(ir);

   /* The precision of the sample value depends on the precision of the
    * sampler.
    */
   stack.back().state = handle_precision(ir->type,
                                         ir->sampler->precision());
   return visit_continue;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit_enter(ir_expression *ir)
{
   ir_hierarchical_visitor::visit_enter(ir);

   if (!can_lower_type(options, ir->type))
      stack.back().state = CANT_LOWER;

   /* Don't lower precision for derivative calculations */
   if (!options->LowerPrecisionDerivatives &&
       (ir->operation == ir_unop_dFdx ||
        ir->operation == ir_unop_dFdx_coarse ||
        ir->operation == ir_unop_dFdx_fine ||
        ir->operation == ir_unop_dFdy ||
        ir->operation == ir_unop_dFdy_coarse ||
        ir->operation == ir_unop_dFdy_fine)) {
      stack.back().state = CANT_LOWER;
   }

   return visit_continue;
}

static unsigned
handle_call(ir_call *ir, const struct set *lowerable_rvalues)
{
   /* The intrinsic call is inside the wrapper imageLoad function that will
    * be inlined. We have to handle both of them.
    */
   if (ir->callee->intrinsic_id == ir_intrinsic_image_load ||
       (ir->callee->is_builtin() &&
        !strcmp(ir->callee_name(), "imageLoad"))) {
      ir_rvalue *param = (ir_rvalue*)ir->actual_parameters.get_head();
      ir_variable *resource = param->variable_referenced();

      assert(ir->callee->return_precision == GLSL_PRECISION_HIGH);
      assert(glsl_type_is_image(glsl_without_array(resource->type)));

      /* GLSL ES 3.20 requires that images have a precision modifier, but if
       * you set one, it doesn't do anything, because all intrinsics are
       * defined with highp. This seems to be a spec bug.
       *
       * In theory we could set the return value to mediump if the image
       * format has a lower precision. This appears to be the most sensible
       * thing to do.
       */
      const struct util_format_description *desc =
         util_format_description(resource->data.image_format);
      int i =
         util_format_get_first_non_void_channel(resource->data.image_format);
      bool mediump;

      assert(i >= 0);

      if (desc->channel[i].pure_integer ||
          desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT)
         mediump = desc->channel[i].size <= 16;
      else
         mediump = desc->channel[i].size <= 10; /* unorm/snorm */

      return mediump ? GLSL_PRECISION_MEDIUM : GLSL_PRECISION_HIGH;
   }

   /* Return the declared precision for user-defined functions. */
   if (!ir->callee->is_builtin() || ir->callee->return_precision != GLSL_PRECISION_NONE)
      return ir->callee->return_precision;

   /* Handle special calls. */
   if (ir->callee->is_builtin() && ir->actual_parameters.length()) {
      ir_rvalue *param = (ir_rvalue*)ir->actual_parameters.get_head();
      ir_variable *var = param->variable_referenced();

      /* Handle builtin wrappers around ir_texture opcodes. These wrappers will
       * be inlined by lower_precision() if we return true here, so that we can
       * get to ir_texture later and do proper lowering.
       *
       * We should lower the type of the return value if the sampler type
       * uses lower precision. The function parameters don't matter.
       */
      if (var && glsl_type_is_sampler(glsl_without_array(var->type))) {
         /* textureGatherOffsets always takes a highp array of constants. As
          * per the discussion https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/16547#note_1393704
          * trying to lower the precision results in segfault later on
          * in the compiler as textureGatherOffsets will end up being passed
          * a temp when its expecting a constant as required by the spec.
          */
         if (!strcmp(ir->callee_name(), "textureGatherOffsets"))
            return GLSL_PRECISION_HIGH;

         return var->data.precision;
      }
   }

   if (ir->callee->return_precision != GLSL_PRECISION_NONE)
      return ir->callee->return_precision;

   if (/* Parameters are always implicitly promoted to highp: */
       !strcmp(ir->callee_name(), "floatBitsToInt") ||
       !strcmp(ir->callee_name(), "floatBitsToUint") ||
       !strcmp(ir->callee_name(), "intBitsToFloat") ||
       !strcmp(ir->callee_name(), "uintBitsToFloat"))
      return GLSL_PRECISION_HIGH;

   /* Number of parameters to check if they are lowerable. */
   unsigned check_parameters = ir->actual_parameters.length();

   /* "For the interpolateAt* functions, the call will return a precision
    *  qualification matching the precision of the interpolant argument to the
    *  function call."
    *
    * and
    *
    * "The precision qualification of the value returned from bitfieldExtract()
    *  matches the precision qualification of the call's input argument
    *  “value”."
    */
   if (!strcmp(ir->callee_name(), "interpolateAtOffset") ||
       !strcmp(ir->callee_name(), "interpolateAtSample") ||
       !strcmp(ir->callee_name(), "bitfieldExtract")) {
      check_parameters = 1;
   } else if (!strcmp(ir->callee_name(), "bitfieldInsert")) {
      /* "The precision qualification of the value returned from bitfieldInsert
       * matches the highest precision qualification of the call's input
       * arguments “base” and “insert”."
       */
      check_parameters = 2;
   }

   /* If the call is to a builtin, then the function won’t have a return
    * precision and we should determine it from the precision of the arguments.
    */
   foreach_in_list(ir_rvalue, param, &ir->actual_parameters) {
      if (!check_parameters)
         break;

      if (!param->as_constant() &&
          _mesa_set_search(lowerable_rvalues, param) == NULL)
         return GLSL_PRECISION_HIGH;

      --check_parameters;
   }

   return GLSL_PRECISION_MEDIUM;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit_leave(ir_call *ir)
{
   ir_hierarchical_visitor::visit_leave(ir);

   /* Special case for handling temporary variables generated by the compiler
    * for function calls. If we assign to one of these using a function call
    * that has a lowerable return type then we can assume the temporary
    * variable should have a medium precision too.
    */

   /* Do nothing if the return type is void. */
   if (!ir->return_deref)
      return visit_continue;

   ir_variable *var = ir->return_deref->variable_referenced();

   assert(var->data.mode == ir_var_temporary);

   unsigned return_precision = handle_call(ir, lowerable_rvalues);

   can_lower_state lower_state =
      handle_precision(var->type, return_precision);

   if (lower_state == SHOULD_LOWER) {
      /* Function calls always write to a temporary return value in the caller,
       * which has no other users.  That temp may start with the precision of
       * the function's signature, but if we're inferring the precision of an
       * unqualified builtin operation (particularly the imageLoad overrides!)
       * then we need to update it.
       */
      var->data.precision = GLSL_PRECISION_MEDIUM;
   } else {
      var->data.precision = GLSL_PRECISION_HIGH;
   }

   return visit_continue;
}

ir_visitor_status
find_lowerable_rvalues_visitor::visit_leave(ir_assignment *ir)
{
   ir_hierarchical_visitor::visit_leave(ir);

   /* Special case for handling temporary variables generated by the compiler.
    * If we assign to one of these using a lowered precision then we can assume
    * the temporary variable should have a medium precision too.
    */
   ir_variable *var = ir->lhs->variable_referenced();

   if (var->data.mode == ir_var_temporary) {
      if (_mesa_set_search(lowerable_rvalues, ir->rhs)) {
         /* Only override the precision if this is the first assignment. For
          * temporaries such as the ones generated for the ?: operator there
          * can be multiple assignments with different precisions. This way we
          * get the highest precision of all of the assignments.
          */
         if (var->data.precision == GLSL_PRECISION_NONE)
            var->data.precision = GLSL_PRECISION_MEDIUM;
      } else if (!ir->rhs->as_constant()) {
         var->data.precision = GLSL_PRECISION_HIGH;
      }
   }

   return visit_continue;
}

void
find_lowerable_rvalues(const struct gl_shader_compiler_options *options,
                       exec_list *instructions,
                       struct set *result)
{
   find_lowerable_rvalues_visitor v(result, options);

   visit_list_elements(&v, instructions);

   assert(v.stack.empty());
}

static const glsl_type *
convert_type(bool up, const glsl_type *type)
{
   if (glsl_type_is_array(type)) {
      return glsl_array_type(convert_type(up, type->fields.array),
                             glsl_array_size(type),
                             type->explicit_stride);
   }

   glsl_base_type new_base_type;

   if (up) {
      switch (type->base_type) {
      case GLSL_TYPE_FLOAT16:
         new_base_type = GLSL_TYPE_FLOAT;
         break;
      case GLSL_TYPE_INT16:
         new_base_type = GLSL_TYPE_INT;
         break;
      case GLSL_TYPE_UINT16:
         new_base_type = GLSL_TYPE_UINT;
         break;
      default:
         unreachable("invalid type");
         return NULL;
      }
   } else {
      switch (type->base_type) {
      case GLSL_TYPE_FLOAT:
         new_base_type = GLSL_TYPE_FLOAT16;
         break;
      case GLSL_TYPE_INT:
         new_base_type = GLSL_TYPE_INT16;
         break;
      case GLSL_TYPE_UINT:
         new_base_type = GLSL_TYPE_UINT16;
         break;
      default:
         unreachable("invalid type");
         return NULL;
      }
   }

   return glsl_simple_explicit_type(new_base_type,
                                    type->vector_elements,
                                    type->matrix_columns,
                                    type->explicit_stride,
                                    type->interface_row_major,
                                    0 /* explicit_alignment */);
}

static const glsl_type *
lower_glsl_type(const glsl_type *type)
{
   return convert_type(false, type);
}

static ir_rvalue *
convert_precision(bool up, ir_rvalue *ir)
{
   unsigned op;

   if (up) {
      switch (ir->type->base_type) {
      case GLSL_TYPE_FLOAT16:
         op = ir_unop_f162f;
         break;
      case GLSL_TYPE_INT16:
         op = ir_unop_i2i;
         break;
      case GLSL_TYPE_UINT16:
         op = ir_unop_u2u;
         break;
      default:
         unreachable("invalid type");
         return NULL;
      }
   } else {
      switch (ir->type->base_type) {
      case GLSL_TYPE_FLOAT:
         op = ir_unop_f2fmp;
         break;
      case GLSL_TYPE_INT:
         op = ir_unop_i2imp;
         break;
      case GLSL_TYPE_UINT:
         op = ir_unop_u2ump;
         break;
      default:
         unreachable("invalid type");
         return NULL;
      }
   }

   const glsl_type *desired_type = convert_type(up, ir->type);
   void *mem_ctx = ralloc_parent(ir);
   return new(mem_ctx) ir_expression(op, desired_type, ir, NULL);
}

void
lower_precision_visitor::handle_rvalue(ir_rvalue **rvalue)
{
   ir_rvalue *ir = *rvalue;

   if (ir == NULL)
      return;

   if (ir->as_dereference()) {
      if (!glsl_type_is_boolean(ir->type))
         *rvalue = convert_precision(false, ir);
   } else if (glsl_type_is_32bit(ir->type)) {
      ir->type = lower_glsl_type(ir->type);

      ir_constant *const_ir = ir->as_constant();

      if (const_ir) {
         ir_constant_data value;

         if (ir->type->base_type == GLSL_TYPE_FLOAT16) {
            for (unsigned i = 0; i < ARRAY_SIZE(value.f16); i++)
               value.f16[i] = _mesa_float_to_half(const_ir->value.f[i]);
         } else if (ir->type->base_type == GLSL_TYPE_INT16) {
            for (unsigned i = 0; i < ARRAY_SIZE(value.i16); i++)
               value.i16[i] = const_ir->value.i[i];
         } else if (ir->type->base_type == GLSL_TYPE_UINT16) {
            for (unsigned i = 0; i < ARRAY_SIZE(value.u16); i++)
               value.u16[i] = const_ir->value.u[i];
         } else {
            unreachable("invalid type");
         }

         const_ir->value = value;
      }
   }
}

ir_visitor_status
lower_precision_visitor::visit_enter(ir_dereference_record *ir)
{
   /* We don’t want to lower the variable */
   return visit_continue_with_parent;
}

ir_visitor_status
lower_precision_visitor::visit_enter(ir_dereference_array *ir)
{
   /* We don’t want to convert the array index or the variable. If the array
    * index itself is lowerable that will be handled separately.
    */
   return visit_continue_with_parent;
}

ir_visitor_status
lower_precision_visitor::visit_enter(ir_call *ir)
{
   /* We don’t want to convert the arguments. These will be handled separately.
    */
   return visit_continue_with_parent;
}

ir_visitor_status
lower_precision_visitor::visit_enter(ir_texture *ir)
{
   /* We don’t want to convert the arguments. These will be handled separately.
    */
   return visit_continue_with_parent;
}

ir_visitor_status
lower_precision_visitor::visit_leave(ir_expression *ir)
{
   ir_rvalue_visitor::visit_leave(ir);

   /* If the expression is a conversion operation to or from bool then fix the
    * operation.
    */
   switch (ir->operation) {
   case ir_unop_b2f:
      ir->operation = ir_unop_b2f16;
      break;
   case ir_unop_f2b:
      ir->operation = ir_unop_f162b;
      break;
   case ir_unop_b2i:
   case ir_unop_i2b:
      /* Nothing to do - they both support int16. */
      break;
   default:
      break;
   }

   return visit_continue;
}

void
find_precision_visitor::handle_rvalue(ir_rvalue **rvalue)
{
   /* Checking the precision of rvalue can be lowered first throughout
    * find_lowerable_rvalues_visitor.
    * Once it found the precision of rvalue can be lowered, then we can
    * add conversion f2fmp, etc. through lower_precision_visitor.
    */
   if (*rvalue == NULL)
      return;

   struct set_entry *entry = _mesa_set_search(lowerable_rvalues, *rvalue);

   if (!entry)
      return;

   _mesa_set_remove(lowerable_rvalues, entry);

   /* If the entire expression is just a variable dereference then trying to
    * lower it will just directly add pointless to and from conversions without
    * any actual operation in-between. Although these will eventually get
    * optimised out, avoiding generating them here also avoids breaking inout
    * parameters to functions.
    */
   if ((*rvalue)->as_dereference())
      return;

   lower_precision_visitor v;

   (*rvalue)->accept(&v);
   v.handle_rvalue(rvalue);

   /* We don’t need to add the final conversion if the final type has been
    * converted to bool
    */
   if ((*rvalue)->type->base_type != GLSL_TYPE_BOOL) {
      *rvalue = convert_precision(true, *rvalue);
   }
}

ir_visitor_status
find_precision_visitor::visit_enter(ir_call *ir)
{
   ir_rvalue_enter_visitor::visit_enter(ir);

   ir_variable *return_var =
      ir->return_deref ? ir->return_deref->variable_referenced() : NULL;

   /* Don't do anything for image_load here. We have only changed the return
    * value to mediump/lowp, so that following instructions can use reduced
    * precision.
    *
    * The return value type of the intrinsic itself isn't changed here, but
    * can be changed in NIR if all users use the *2*mp opcode.
    */
   if (ir->callee->intrinsic_id == ir_intrinsic_image_load)
      return visit_continue;

   /* If this is a call to a builtin and the find_lowerable_rvalues_visitor
    * overrode the precision of the temporary return variable, then we can
    * replace the builtin implementation with a lowered version.
    */

   if (!ir->callee->is_builtin() ||
       ir->callee->is_intrinsic() ||
       return_var == NULL ||
       (return_var->data.precision != GLSL_PRECISION_MEDIUM &&
        return_var->data.precision != GLSL_PRECISION_LOW))
      return visit_continue;

   ir->callee = map_builtin(ir->callee);
   ir->generate_inline(ir);
   ir->remove();

   return visit_continue_with_parent;
}

ir_function_signature *
find_precision_visitor::map_builtin(ir_function_signature *sig)
{
   if (lowered_builtins == NULL) {
      lowered_builtins = _mesa_pointer_hash_table_create(NULL);
      clone_ht =_mesa_pointer_hash_table_create(NULL);
      lowered_builtin_mem_ctx = ralloc_context(NULL);
   } else {
      struct hash_entry *entry = _mesa_hash_table_search(lowered_builtins, sig);
      if (entry)
         return (ir_function_signature *) entry->data;
   }

   ir_function_signature *lowered_sig =
      sig->clone(lowered_builtin_mem_ctx, clone_ht);

   /* If we're lowering the output precision of the function, then also lower
    * the precision of its inputs unless they have a specific qualifier.  The
    * exception is bitCount, which doesn't declare its arguments highp but
    * should not be lowering the args to mediump just because the output is
    * lowp.
    */
   if (strcmp(sig->function_name(), "bitCount") != 0) {
      foreach_in_list(ir_variable, param, &lowered_sig->parameters) {
         /* Demote the precision of unqualified function arguments. */
         if (param->data.precision == GLSL_PRECISION_NONE)
            param->data.precision = GLSL_PRECISION_MEDIUM;
      }
   }

   lower_precision(options, &lowered_sig->body);

   _mesa_hash_table_clear(clone_ht, NULL);

   _mesa_hash_table_insert(lowered_builtins, sig, lowered_sig);

   return lowered_sig;
}

find_precision_visitor::find_precision_visitor(const struct gl_shader_compiler_options *options)
   : lowerable_rvalues(_mesa_pointer_set_create(NULL)),
     lowered_builtins(NULL),
     clone_ht(NULL),
     lowered_builtin_mem_ctx(NULL),
     options(options)
{
}

find_precision_visitor::~find_precision_visitor()
{
   _mesa_set_destroy(lowerable_rvalues, NULL);

   if (lowered_builtins) {
      _mesa_hash_table_destroy(lowered_builtins, NULL);
      _mesa_hash_table_destroy(clone_ht, NULL);
      ralloc_free(lowered_builtin_mem_ctx);
   }
}

/* Lowering opcodes to 16 bits is not enough for programs with control flow
 * (and the ?: operator, which is represented by if-then-else in the IR),
 * because temporary variables, which are used for passing values between
 * code blocks, are not lowered, resulting in 32-bit phis in NIR.
 *
 * First change the variable types to 16 bits, then change all ir_dereference
 * types to 16 bits.
 */
class lower_variables_visitor : public ir_rvalue_enter_visitor {
public:
   lower_variables_visitor(const struct gl_shader_compiler_options *options)
      : options(options) {
      lower_vars = _mesa_pointer_set_create(NULL);
   }

   virtual ~lower_variables_visitor()
   {
      _mesa_set_destroy(lower_vars, NULL);
   }

   virtual ir_visitor_status visit(ir_variable *var);
   virtual ir_visitor_status visit_enter(ir_assignment *ir);
   virtual ir_visitor_status visit_enter(ir_return *ir);
   virtual ir_visitor_status visit_enter(ir_call *ir);
   virtual void handle_rvalue(ir_rvalue **rvalue);

   void fix_types_in_deref_chain(ir_dereference *ir);
   void convert_split_assignment(ir_dereference *lhs, ir_rvalue *rhs,
                                 bool insert_before);

   const struct gl_shader_compiler_options *options;
   set *lower_vars;
};

static void
lower_constant(ir_constant *ir)
{
   if (glsl_type_is_array(ir->type)) {
      for (int i = 0; i < glsl_array_size(ir->type); i++)
         lower_constant(ir->get_array_element(i));

      ir->type = lower_glsl_type(ir->type);
      return;
   }

   ir->type = lower_glsl_type(ir->type);
   ir_constant_data value;

   if (ir->type->base_type == GLSL_TYPE_FLOAT16) {
      for (unsigned i = 0; i < ARRAY_SIZE(value.f16); i++)
         value.f16[i] = _mesa_float_to_half(ir->value.f[i]);
   } else if (ir->type->base_type == GLSL_TYPE_INT16) {
      for (unsigned i = 0; i < ARRAY_SIZE(value.i16); i++)
         value.i16[i] = ir->value.i[i];
   } else if (ir->type->base_type == GLSL_TYPE_UINT16) {
      for (unsigned i = 0; i < ARRAY_SIZE(value.u16); i++)
         value.u16[i] = ir->value.u[i];
   } else {
      unreachable("invalid type");
   }

   ir->value = value;
}

ir_visitor_status
lower_variables_visitor::visit(ir_variable *var)
{
   if ((var->data.mode != ir_var_temporary &&
        var->data.mode != ir_var_auto &&
        /* Lower uniforms but not UBOs. */
        (var->data.mode != ir_var_uniform ||
         var->is_in_buffer_block() ||
         !(options->LowerPrecisionFloat16Uniforms &&
           glsl_without_array(var->type)->base_type == GLSL_TYPE_FLOAT))) ||
       !glsl_type_is_32bit(glsl_without_array(var->type)) ||
       (var->data.precision != GLSL_PRECISION_MEDIUM &&
        var->data.precision != GLSL_PRECISION_LOW) ||
       !can_lower_type(options, var->type))
      return visit_continue;

   /* Lower constant initializers. */
   if (var->constant_value &&
       var->type == var->constant_value->type) {
      if (!options->LowerPrecisionConstants)
         return visit_continue;
      var->constant_value =
         var->constant_value->clone(ralloc_parent(var), NULL);
      lower_constant(var->constant_value);
   }

   if (var->constant_initializer &&
       var->type == var->constant_initializer->type) {
      if (!options->LowerPrecisionConstants)
         return visit_continue;
      var->constant_initializer =
         var->constant_initializer->clone(ralloc_parent(var), NULL);
      lower_constant(var->constant_initializer);
   }

   var->type = lower_glsl_type(var->type);
   _mesa_set_add(lower_vars, var);

   return visit_continue;
}

void
lower_variables_visitor::fix_types_in_deref_chain(ir_dereference *ir)
{
   assert(glsl_type_is_32bit(glsl_without_array(ir->type)));
   assert(_mesa_set_search(lower_vars, ir->variable_referenced()));

   /* Fix the type in the dereference node. */
   ir->type = lower_glsl_type(ir->type);

   /* If it's an array, fix the types in the whole dereference chain. */
   for (ir_dereference_array *deref_array = ir->as_dereference_array();
        deref_array;
        deref_array = deref_array->array->as_dereference_array()) {
      assert(glsl_type_is_32bit(glsl_without_array(deref_array->array->type)));
      deref_array->array->type = lower_glsl_type(deref_array->array->type);
   }
}

void
lower_variables_visitor::convert_split_assignment(ir_dereference *lhs,
                                                  ir_rvalue *rhs,
                                                  bool insert_before)
{
   void *mem_ctx = ralloc_parent(lhs);

   if (glsl_type_is_array(lhs->type)) {
      for (unsigned i = 0; i < lhs->type->length; i++) {
         ir_dereference *l, *r;

         l = new(mem_ctx) ir_dereference_array(lhs->clone(mem_ctx, NULL),
                                               new(mem_ctx) ir_constant(i));
         r = new(mem_ctx) ir_dereference_array(rhs->clone(mem_ctx, NULL),
                                               new(mem_ctx) ir_constant(i));
         convert_split_assignment(l, r, insert_before);
      }
      return;
   }

   assert(glsl_type_is_16bit(lhs->type) || glsl_type_is_32bit(lhs->type));
   assert(glsl_type_is_16bit(rhs->type) || glsl_type_is_32bit(rhs->type));
   assert(glsl_type_is_16bit(lhs->type) != glsl_type_is_16bit(rhs->type));

   ir_assignment *assign =
      new(mem_ctx) ir_assignment(lhs, convert_precision(glsl_type_is_32bit(lhs->type), rhs));

   if (insert_before)
      base_ir->insert_before(assign);
   else
      base_ir->insert_after(assign);
}

ir_visitor_status
lower_variables_visitor::visit_enter(ir_assignment *ir)
{
   ir_dereference *lhs = ir->lhs;
   ir_variable *var = lhs->variable_referenced();
   ir_dereference *rhs_deref = ir->rhs->as_dereference();
   ir_variable *rhs_var = rhs_deref ? rhs_deref->variable_referenced() : NULL;
   ir_constant *rhs_const = ir->rhs->as_constant();

   /* Legalize array assignments between lowered and non-lowered variables. */
   if (glsl_type_is_array(lhs->type) &&
       (rhs_var || rhs_const) &&
       (!rhs_var ||
        (var &&
         glsl_type_is_16bit(glsl_without_array(var->type)) !=
         glsl_type_is_16bit(glsl_without_array(rhs_var->type)))) &&
       (!rhs_const ||
        (var &&
         glsl_type_is_16bit(glsl_without_array(var->type)) &&
         glsl_type_is_32bit(glsl_without_array(rhs_const->type))))) {
      assert(glsl_type_is_array(ir->rhs->type));

      /* Fix array assignments from lowered to non-lowered. */
      if (rhs_var && _mesa_set_search(lower_vars, rhs_var)) {
         fix_types_in_deref_chain(rhs_deref);
         /* Convert to 32 bits for LHS. */
         convert_split_assignment(lhs, rhs_deref, true);
         ir->remove();
         return visit_continue;
      }

      /* Fix array assignments from non-lowered to lowered. */
      if (var &&
          _mesa_set_search(lower_vars, var) &&
          glsl_type_is_32bit(glsl_without_array(ir->rhs->type))) {
         fix_types_in_deref_chain(lhs);
         /* Convert to 16 bits for LHS. */
         convert_split_assignment(lhs, ir->rhs, true);
         ir->remove();
         return visit_continue;
      }
   }

   /* Fix assignment types. */
   if (var &&
       _mesa_set_search(lower_vars, var)) {
      /* Fix the LHS type. */
      if (glsl_type_is_32bit(glsl_without_array(lhs->type)))
         fix_types_in_deref_chain(lhs);

      /* Fix the RHS type if it's a lowered variable. */
      if (rhs_var &&
          _mesa_set_search(lower_vars, rhs_var) &&
          glsl_type_is_32bit(glsl_without_array(rhs_deref->type)))
         fix_types_in_deref_chain(rhs_deref);

      /* Fix the RHS type if it's a non-array expression. */
      if (glsl_type_is_32bit(ir->rhs->type)) {
         ir_expression *expr = ir->rhs->as_expression();

         /* Convert the RHS to the LHS type. */
         if (expr &&
             (expr->operation == ir_unop_f162f ||
              expr->operation == ir_unop_i2i ||
              expr->operation == ir_unop_u2u) &&
             glsl_type_is_16bit(expr->operands[0]->type)) {
            /* If there is an "up" conversion, just remove it.
             * This is optional. We could as well execute the else statement and
             * let NIR eliminate the up+down conversions.
             */
            ir->rhs = expr->operands[0];
         } else {
            /* Add a "down" conversion operation to fix the type of RHS. */
            ir->rhs = convert_precision(false, ir->rhs);
         }
      }
   }

   return ir_rvalue_enter_visitor::visit_enter(ir);
}

ir_visitor_status
lower_variables_visitor::visit_enter(ir_return *ir)
{
   void *mem_ctx = ralloc_parent(ir);

   ir_dereference *deref = ir->value ? ir->value->as_dereference() : NULL;
   if (deref) {
      ir_variable *var = deref->variable_referenced();

      /* Fix the type of the return value. */
      if (var &&
          _mesa_set_search(lower_vars, var) &&
          glsl_type_is_32bit(glsl_without_array(deref->type))) {
         /* Create a 32-bit temporary variable. */
         ir_variable *new_var =
            new(mem_ctx) ir_variable(deref->type, "lowerp", ir_var_temporary);
         base_ir->insert_before(new_var);

         /* Fix types in dereferences. */
         fix_types_in_deref_chain(deref);

         /* Convert to 32 bits for the return value. */
         convert_split_assignment(new(mem_ctx) ir_dereference_variable(new_var),
                                  deref, true);
         ir->value = new(mem_ctx) ir_dereference_variable(new_var);
      }
   }

   return ir_rvalue_enter_visitor::visit_enter(ir);
}

void lower_variables_visitor::handle_rvalue(ir_rvalue **rvalue)
{
   ir_rvalue *ir = *rvalue;

   if (in_assignee || ir == NULL)
      return;

   ir_expression *expr = ir->as_expression();
   ir_dereference *expr_op0_deref = expr ? expr->operands[0]->as_dereference() : NULL;

   /* Remove f2fmp(float16). Same for int16 and uint16. */
   if (expr &&
       expr_op0_deref &&
       (expr->operation == ir_unop_f2fmp ||
        expr->operation == ir_unop_i2imp ||
        expr->operation == ir_unop_u2ump ||
        expr->operation == ir_unop_f2f16 ||
        expr->operation == ir_unop_i2i ||
        expr->operation == ir_unop_u2u) &&
       glsl_type_is_16bit(glsl_without_array(expr->type)) &&
       glsl_type_is_32bit(glsl_without_array(expr_op0_deref->type)) &&
       expr_op0_deref->variable_referenced() &&
       _mesa_set_search(lower_vars, expr_op0_deref->variable_referenced())) {
      fix_types_in_deref_chain(expr_op0_deref);

      /* Remove f2fmp/i2imp/u2ump. */
      *rvalue = expr_op0_deref;
      return;
   }

   ir_dereference *deref = ir->as_dereference();

   if (deref) {
      ir_variable *var = deref->variable_referenced();

      /* var can be NULL if we are dereferencing ir_constant. */
      if (var &&
          _mesa_set_search(lower_vars, var) &&
          glsl_type_is_32bit(glsl_without_array(deref->type))) {
         void *mem_ctx = ralloc_parent(ir);

         /* Create a 32-bit temporary variable. */
         ir_variable *new_var =
            new(mem_ctx) ir_variable(deref->type, "lowerp", ir_var_temporary);
         base_ir->insert_before(new_var);

         /* Fix types in dereferences. */
         fix_types_in_deref_chain(deref);

         /* Convert to 32 bits for the rvalue. */
         convert_split_assignment(new(mem_ctx) ir_dereference_variable(new_var),
                                  deref, true);
         *rvalue = new(mem_ctx) ir_dereference_variable(new_var);
      }
   }
}

ir_visitor_status
lower_variables_visitor::visit_enter(ir_call *ir)
{
   void *mem_ctx = ralloc_parent(ir);

   /* We can't pass 16-bit variables as 32-bit inout/out parameters. */
   foreach_two_lists(formal_node, &ir->callee->parameters,
                     actual_node, &ir->actual_parameters) {
      ir_dereference *param_deref =
         ((ir_rvalue *)actual_node)->as_dereference();
      ir_variable *param = (ir_variable *)formal_node;

      if (!param_deref)
            continue;

      ir_variable *var = param_deref->variable_referenced();

      /* var can be NULL if we are dereferencing ir_constant. */
      if (var &&
          _mesa_set_search(lower_vars, var) &&
          glsl_type_is_32bit(glsl_without_array(param->type))) {
         fix_types_in_deref_chain(param_deref);

         /* Create a 32-bit temporary variable for the parameter. */
         ir_variable *new_var =
            new(mem_ctx) ir_variable(param->type, "lowerp", ir_var_temporary);
         base_ir->insert_before(new_var);

         /* Replace the parameter. */
         actual_node->replace_with(new(mem_ctx) ir_dereference_variable(new_var));

         if (param->data.mode == ir_var_function_in ||
             param->data.mode == ir_var_function_inout) {
            /* Convert to 32 bits for passing in. */
            convert_split_assignment(new(mem_ctx) ir_dereference_variable(new_var),
                                     param_deref->clone(mem_ctx, NULL), true);
         }
         if (param->data.mode == ir_var_function_out ||
             param->data.mode == ir_var_function_inout) {
            /* Convert to 16 bits after returning. */
            convert_split_assignment(param_deref,
                                     new(mem_ctx) ir_dereference_variable(new_var),
                                     false);
         }
      }
   }

   /* Fix the type of return value dereferencies. */
   ir_dereference_variable *ret_deref = ir->return_deref;
   ir_variable *ret_var = ret_deref ? ret_deref->variable_referenced() : NULL;

   if (ret_var &&
       _mesa_set_search(lower_vars, ret_var) &&
       glsl_type_is_32bit(glsl_without_array(ret_deref->type))) {
      /* Create a 32-bit temporary variable. */
      ir_variable *new_var =
         new(mem_ctx) ir_variable(ir->callee->return_type, "lowerp",
                                  ir_var_temporary);
      base_ir->insert_before(new_var);

      /* Replace the return variable. */
      ret_deref->var = new_var;

      /* Convert to 16 bits after returning. */
      convert_split_assignment(new(mem_ctx) ir_dereference_variable(ret_var),
                               new(mem_ctx) ir_dereference_variable(new_var),
                               false);
   }

   return ir_rvalue_enter_visitor::visit_enter(ir);
}

}

void
lower_precision(const struct gl_shader_compiler_options *options,
                exec_list *instructions)
{
   find_precision_visitor v(options);
   find_lowerable_rvalues(options, instructions, v.lowerable_rvalues);
   visit_list_elements(&v, instructions);

   lower_variables_visitor vars(options);
   visit_list_elements(&vars, instructions);
}
