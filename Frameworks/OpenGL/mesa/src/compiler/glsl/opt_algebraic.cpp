/*
 * Copyright © 2010 Intel Corporation
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
 * \file opt_algebraic.cpp
 *
 * Takes advantage of association, commutivity, and other algebraic
 * properties to simplify expressions.
 */

#include "ir.h"
#include "ir_visitor.h"
#include "ir_rvalue_visitor.h"
#include "ir_optimization.h"
#include "ir_builder.h"
#include "compiler/glsl_types.h"
#include "main/consts_exts.h"

using namespace ir_builder;

namespace {

/**
 * Visitor class for replacing expressions with ir_constant values.
 */

class ir_algebraic_visitor : public ir_rvalue_visitor {
public:
   ir_algebraic_visitor(bool native_integers,
                        const struct gl_shader_compiler_options *options)
      : options(options)
   {
      this->progress = false;
      this->mem_ctx = NULL;
      this->native_integers = native_integers;
   }

   virtual ~ir_algebraic_visitor()
   {
   }

   virtual ir_visitor_status visit_enter(ir_assignment *ir);

   ir_rvalue *handle_expression(ir_expression *ir);
   void handle_rvalue(ir_rvalue **rvalue);
   bool reassociate_constant(ir_expression *ir1,
			     int const_index,
			     ir_constant *constant,
			     ir_expression *ir2);
   void reassociate_operands(ir_expression *ir1,
			     int op1,
			     ir_expression *ir2,
			     int op2);
   ir_rvalue *swizzle_if_required(ir_expression *expr,
				  ir_rvalue *operand);

   const struct gl_shader_compiler_options *options;
   void *mem_ctx;

   bool native_integers;
   bool progress;
};

} /* unnamed namespace */

ir_visitor_status
ir_algebraic_visitor::visit_enter(ir_assignment *ir)
{
   ir_variable *var = ir->lhs->variable_referenced();
   if (var->data.invariant || var->data.precise) {
      /* If we're assigning to an invariant or precise variable, just bail.
       * Most of the algebraic optimizations aren't precision-safe.
       *
       * FINISHME: Find out which optimizations are precision-safe and enable
       * then only for invariant or precise trees.
       */
      return visit_continue_with_parent;
   } else {
      return visit_continue;
   }
}

static inline bool
is_valid_vec_const(ir_constant *ir)
{
   if (ir == NULL)
      return false;

   if (!glsl_type_is_scalar(ir->type) && !glsl_type_is_vector(ir->type))
      return false;

   return true;
}

static inline bool
is_less_than_one(ir_constant *ir)
{
   assert(glsl_type_is_float(ir->type));

   if (!is_valid_vec_const(ir))
      return false;

   unsigned component = 0;
   for (int c = 0; c < ir->type->vector_elements; c++) {
      if (ir->get_float_component(c) < 1.0f)
         component++;
   }

   return (component == ir->type->vector_elements);
}

static inline bool
is_greater_than_zero(ir_constant *ir)
{
   assert(glsl_type_is_float(ir->type));

   if (!is_valid_vec_const(ir))
      return false;

   unsigned component = 0;
   for (int c = 0; c < ir->type->vector_elements; c++) {
      if (ir->get_float_component(c) > 0.0f)
         component++;
   }

   return (component == ir->type->vector_elements);
}

static void
update_type(ir_expression *ir)
{
   if (glsl_type_is_vector(ir->operands[0]->type))
      ir->type = ir->operands[0]->type;
   else
      ir->type = ir->operands[1]->type;
}

void
ir_algebraic_visitor::reassociate_operands(ir_expression *ir1,
					   int op1,
					   ir_expression *ir2,
					   int op2)
{
   ir_rvalue *temp = ir2->operands[op2];
   ir2->operands[op2] = ir1->operands[op1];
   ir1->operands[op1] = temp;

   /* Update the type of ir2.  The type of ir1 won't have changed --
    * base types matched, and at least one of the operands of the 2
    * binops is still a vector if any of them were.
    */
   update_type(ir2);

   this->progress = true;
}

/**
 * Reassociates a constant down a tree of adds or multiplies.
 *
 * Consider (2 * (a * (b * 0.5))).  We want to end up with a * b.
 */
bool
ir_algebraic_visitor::reassociate_constant(ir_expression *ir1, int const_index,
					   ir_constant *constant,
					   ir_expression *ir2)
{
   if (!ir2 || ir1->operation != ir2->operation)
      return false;

   /* Don't want to even think about matrices. */
   if (glsl_type_is_matrix(ir1->operands[0]->type) ||
       glsl_type_is_matrix(ir1->operands[1]->type) ||
       glsl_type_is_matrix(ir2->operands[0]->type) ||
       glsl_type_is_matrix(ir2->operands[1]->type))
      return false;

   void *mem_ctx = ralloc_parent(ir2);

   ir_constant *ir2_const[2];
   ir2_const[0] = ir2->operands[0]->constant_expression_value(mem_ctx);
   ir2_const[1] = ir2->operands[1]->constant_expression_value(mem_ctx);

   if (ir2_const[0] && ir2_const[1])
      return false;

   if (ir2_const[0]) {
      reassociate_operands(ir1, const_index, ir2, 1);
      return true;
   } else if (ir2_const[1]) {
      reassociate_operands(ir1, const_index, ir2, 0);
      return true;
   }

   if (reassociate_constant(ir1, const_index, constant,
			    ir2->operands[0]->as_expression())) {
      update_type(ir2);
      return true;
   }

   if (reassociate_constant(ir1, const_index, constant,
			    ir2->operands[1]->as_expression())) {
      update_type(ir2);
      return true;
   }

   return false;
}

/* When eliminating an expression and just returning one of its operands,
 * we may need to swizzle that operand out to a vector if the expression was
 * vector type.
 */
ir_rvalue *
ir_algebraic_visitor::swizzle_if_required(ir_expression *expr,
					  ir_rvalue *operand)
{
   if (glsl_type_is_vector(expr->type) && glsl_type_is_scalar(operand->type)) {
      return new(mem_ctx) ir_swizzle(operand, 0, 0, 0, 0,
				     expr->type->vector_elements);
   } else
      return operand;
}

ir_rvalue *
ir_algebraic_visitor::handle_expression(ir_expression *ir)
{
   ir_constant *op_const[4] = {NULL, NULL, NULL, NULL};
   ir_expression *op_expr[4] = {NULL, NULL, NULL, NULL};

   if (ir->operation == ir_binop_mul &&
       glsl_type_is_matrix(ir->operands[0]->type) &&
       glsl_type_is_vector(ir->operands[1]->type)) {
      ir_expression *matrix_mul = ir->operands[0]->as_expression();

      if (matrix_mul && matrix_mul->operation == ir_binop_mul &&
         glsl_type_is_matrix(matrix_mul->operands[0]->type) &&
         glsl_type_is_matrix(matrix_mul->operands[1]->type)) {

         return mul(matrix_mul->operands[0],
                    mul(matrix_mul->operands[1], ir->operands[1]));
      }
   }

   assert(ir->num_operands <= 4);
   for (unsigned i = 0; i < ir->num_operands; i++) {
      if (glsl_type_is_matrix(ir->operands[i]->type))
	 return ir;

      op_const[i] =
         ir->operands[i]->constant_expression_value(ralloc_parent(ir));
      op_expr[i] = ir->operands[i]->as_expression();
   }

   if (this->mem_ctx == NULL)
      this->mem_ctx = ralloc_parent(ir);

   switch (ir->operation) {
   case ir_binop_add:
      /* Reassociate addition of constants so that we can do constant
       * folding.
       */
      if (op_const[0] && !op_const[1])
	 reassociate_constant(ir, 0, op_const[0], op_expr[1]);
      if (op_const[1] && !op_const[0])
	 reassociate_constant(ir, 1, op_const[1], op_expr[0]);

      break;

   case ir_binop_mul:
      /* Reassociate multiplication of constants so that we can do
       * constant folding.
       */
      if (op_const[0] && !op_const[1])
	 reassociate_constant(ir, 0, op_const[0], op_expr[1]);
      if (op_const[1] && !op_const[0])
	 reassociate_constant(ir, 1, op_const[1], op_expr[0]);
      break;

   case ir_binop_min:
   case ir_binop_max:
      if (!glsl_type_is_float(ir->type))
         break;

      /* Replace min(max) operations and its commutative combinations with
       * a saturate operation
       */
      for (int op = 0; op < 2; op++) {
         ir_expression *inner_expr = op_expr[op];
         ir_constant *outer_const = op_const[1 - op];
         ir_expression_operation op_cond = (ir->operation == ir_binop_max) ?
            ir_binop_min : ir_binop_max;

         if (!inner_expr || !outer_const || (inner_expr->operation != op_cond))
            continue;

         /* One of these has to be a constant */
         if (!inner_expr->operands[0]->as_constant() &&
             !inner_expr->operands[1]->as_constant())
            break;

         /* Found a min(max) combination. Now try to see if its operands
          * meet our conditions that we can do just a single saturate operation
          */
         for (int minmax_op = 0; minmax_op < 2; minmax_op++) {
            ir_rvalue *x = inner_expr->operands[minmax_op];
            ir_rvalue *y = inner_expr->operands[1 - minmax_op];

            ir_constant *inner_const = y->as_constant();
            if (!inner_const)
               continue;

            /* min(max(x, 0.0), 1.0) is sat(x) */
            if (ir->operation == ir_binop_min &&
                inner_const->is_zero() &&
                outer_const->is_one())
               return saturate(x);

            /* max(min(x, 1.0), 0.0) is sat(x) */
            if (ir->operation == ir_binop_max &&
                inner_const->is_one() &&
                outer_const->is_zero())
               return saturate(x);

            /* min(max(x, 0.0), b) where b < 1.0 is sat(min(x, b)) */
            if (ir->operation == ir_binop_min &&
                inner_const->is_zero() &&
                is_less_than_one(outer_const))
               return saturate(expr(ir_binop_min, x, outer_const));

            /* max(min(x, b), 0.0) where b < 1.0 is sat(min(x, b)) */
            if (ir->operation == ir_binop_max &&
                is_less_than_one(inner_const) &&
                outer_const->is_zero())
               return saturate(expr(ir_binop_min, x, inner_const));

            /* max(min(x, 1.0), b) where b > 0.0 is sat(max(x, b)) */
            if (ir->operation == ir_binop_max &&
                inner_const->is_one() &&
                is_greater_than_zero(outer_const))
               return saturate(expr(ir_binop_max, x, outer_const));

            /* min(max(x, b), 1.0) where b > 0.0 is sat(max(x, b)) */
            if (ir->operation == ir_binop_min &&
                is_greater_than_zero(inner_const) &&
                outer_const->is_one())
               return saturate(expr(ir_binop_max, x, inner_const));
         }
      }

      break;

   /* Remove interpolateAt* instructions for demoted inputs. They are
    * assigned a constant expression to facilitate this.
    */
   case ir_unop_interpolate_at_centroid:
   case ir_binop_interpolate_at_offset:
   case ir_binop_interpolate_at_sample:
      if (op_const[0])
         return ir->operands[0];
      break;

   default:
      break;
   }

   return ir;
}

void
ir_algebraic_visitor::handle_rvalue(ir_rvalue **rvalue)
{
   if (!*rvalue)
      return;

   ir_expression *expr = (*rvalue)->as_expression();
   if (!expr || expr->operation == ir_quadop_vector)
      return;

   ir_rvalue *new_rvalue = handle_expression(expr);
   if (new_rvalue == *rvalue)
      return;

   /* If the expr used to be some vec OP scalar returning a vector, and the
    * optimization gave us back a scalar, we still need to turn it into a
    * vector.
    */
   *rvalue = swizzle_if_required(expr, new_rvalue);

   this->progress = true;
}

bool
do_algebraic(exec_list *instructions, bool native_integers,
             const struct gl_shader_compiler_options *options)
{
   ir_algebraic_visitor v(native_integers, options);

   visit_list_elements(&v, instructions);

   return v.progress;
}
