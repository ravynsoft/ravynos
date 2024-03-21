/* aarch64-gen.c -- Generate tables and routines for opcode lookup and
   instruction encoding and decoding.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "libiberty.h"
#include "getopt.h"
#include "opcode/aarch64.h"

#define VERIFIER(x) NULL
#include "aarch64-tbl.h"

static int debug = 0;

/* Structure used in the decoding tree to group a list of aarch64_opcode
   entries.  */

struct opcode_node
{
  aarch64_insn opcode;
  aarch64_insn mask;
  /* Index of the entry in the original table; the top 2 bits help
     determine the table.  */
  unsigned int index;
  struct opcode_node *next;
};

typedef struct opcode_node opcode_node;

/* Head of the list of the opcode_node after read_table.  */
static opcode_node opcode_nodes_head;

/* Node in the decoding tree.  */

struct bittree
{
  unsigned int bitno;
  /* 0, 1, and X (don't care).  */
  struct bittree *bits[2];
  /* List of opcodes; only valid for the leaf node.  */
  opcode_node *list;
};

/* Allocate and initialize an opcode_node.  */
static opcode_node*
new_opcode_node (void)
{
  opcode_node* ent = malloc (sizeof (opcode_node));

  if (!ent)
    abort ();

  ent->opcode = 0;
  ent->mask = 0;
  ent->index = -1;
  ent->next = NULL;

  return ent;
}

/* Multiple tables are supported, although currently only one table is
   in use.  N.B. there are still some functions have the table name
   'aarch64_opcode_table' hard-coded in, e.g. print_find_next_opcode;
   therefore some amount of work needs to be done if the full support
   for multiple tables needs to be enabled.  */
static const struct aarch64_opcode * const aarch64_opcode_tables[] =
{aarch64_opcode_table};

/* Use top 2 bits to indiate which table.  */
static unsigned int
initialize_index (const struct aarch64_opcode* table)
{
  int i;
  const int num_of_tables = sizeof (aarch64_opcode_tables)
    / sizeof (struct aarch64_opcode *);
  for (i = 0; i < num_of_tables; ++i)
    if (table == aarch64_opcode_tables [i])
      break;
  if (i == num_of_tables)
    abort ();
  return (unsigned int)i << 30;
}

static inline const struct aarch64_opcode *
index2table (unsigned int index)
{
  return aarch64_opcode_tables[(index >> 30) & 0x3];
}

static inline unsigned int
real_index (unsigned int index)
{
  return index & ((1 << 30) - 1);
}

/* Given OPCODE_NODE, return the corresponding aarch64_opcode*.  */
static const aarch64_opcode*
get_aarch64_opcode (const opcode_node *opcode_node)
{
  if (opcode_node == NULL)
    return NULL;
  return &index2table (opcode_node->index)[real_index (opcode_node->index)];
}

static void
read_table (const struct aarch64_opcode* table)
{
  const struct aarch64_opcode *ent = table;
  opcode_node **new_ent;
  unsigned int index = initialize_index (table);

  if (!ent->name)
    return;

  new_ent = &opcode_nodes_head.next;

  while (*new_ent)
    new_ent = &(*new_ent)->next;

  do
    {
      /* F_PSEUDO needs to be used together with F_ALIAS to indicate an alias
	 opcode is a programmer friendly pseudo instruction available only in
	 the assembly code (thus will not show up in the disassembly).  */
      assert (!pseudo_opcode_p (ent) || alias_opcode_p (ent));
      /* Skip alias (inc. pseudo) opcode.  */
      if (alias_opcode_p (ent))
	{
	  index++;
	  continue;
	}
      *new_ent = new_opcode_node ();
      (*new_ent)->opcode = ent->opcode;
      (*new_ent)->mask = ent->mask;
      (*new_ent)->index = index++;
      new_ent = &((*new_ent)->next);
    } while ((++ent)->name);
}

static inline void
print_one_opcode_node (opcode_node* ent)
{
  printf ("%s\t%08x\t%08x\t%d\n", get_aarch64_opcode (ent)->name,
	  get_aarch64_opcode (ent)->opcode, get_aarch64_opcode (ent)->mask,
	  (int)real_index (ent->index));
}

/* As an internal debugging utility, print out the list of nodes pointed
   by opcode_nodes_head.  */
static void
print_opcode_nodes (void)
{
  opcode_node* ent = opcode_nodes_head.next;
  printf ("print_opcode_nodes table:\n");
  while (ent)
    {
      print_one_opcode_node (ent);
      ent = ent->next;
    }
}

static struct bittree*
new_bittree_node (void)
{
  struct bittree* node;
  node = malloc (sizeof (struct bittree));
  if (!node)
    abort ();
  node->bitno = -1;
  node->bits[0] = NULL;
  node->bits[1] = NULL;
  return node;
}

/* The largest number of opcode entries that exist at a leaf node of the
   decoding decision tree.  The reason that there can be more than one
   opcode entry is because some opcodes have shared field that is partially
   constrained and thus cannot be fully isolated using the algorithm
   here.  */
static int max_num_opcodes_at_leaf_node = 0;

/* Given a list of opcodes headed by *OPCODE, try to establish one bit that
   is shared by all the opcodes in the list as one of base opcode bits.  If
   such a bit is found, divide the list of the opcodes into two based on the
   value of the bit.

   Store the bit number in BITTREE->BITNO if the division succeeds.  If unable
   to determine such a bit or there is only one opcode in the list, the list
   is decided to be undividable and OPCODE will be assigned to BITTREE->LIST.

   The function recursively call itself until OPCODE is undividable.

   N.B. the nature of this algrithm determines that given any value in the
   32-bit space, the computed decision tree will always be able to find one or
   more opcodes entries for it, regardless whether there is a valid instruction
   defined for this value or not.  In order to detect the undefined values,
   when the caller obtains the opcode entry/entries, it should at least compare
   the bit-wise AND result of the value and the mask with the base opcode
   value; if the two are different, it means that the value is undefined
   (although the value may be still undefined when the comparison is the same,
   in which case call aarch64_opcode_decode to carry out further checks).  */

static void
divide_table_1 (struct bittree *bittree, opcode_node *opcode)
{
  aarch64_insn mask_and;
  opcode_node *ent;
  unsigned int bitno;
  aarch64_insn bitmask;
  opcode_node list0, list1, **ptr0, **ptr1;
  static int depth = 0;

  ++depth;

  if (debug)
    printf ("Enter into depth %d\n", depth);

  assert (opcode != NULL);

  /* Succeed when there is only one opcode left.  */
  if (!opcode->next)
    {
      if (debug)
	{
	  printf ("opcode isolated:\n");
	  print_one_opcode_node (opcode);
	}
      goto divide_table_1_finish;
    }

 divide_table_1_try_again:
  mask_and = -1;
  ent = opcode;
  while (ent)
    {
      mask_and &= ent->mask;
      ent = ent->next;
    }

  if (debug)
    printf ("mask and result: %08x\n", (unsigned int)mask_and);

  /* If no more bit to look into, we have to accept the reality then.  */
  if (!mask_and)
    {
      int i;
      opcode_node *ptr;
      if (debug)
	{
	  ptr = opcode;
	  printf ("Isolated opcode group:\n");
	  do {
	      print_one_opcode_node (ptr);
	      ptr = ptr->next;
	  } while (ptr);
	}
      /* Count the number of opcodes.  */
      for (i = 0, ptr = opcode; ptr; ++i)
	ptr = ptr->next;
      if (i > max_num_opcodes_at_leaf_node)
	max_num_opcodes_at_leaf_node = i;
      goto divide_table_1_finish;
    }

  /* Pick up the right most bit that is 1.  */
  bitno = 0;
  while (!(mask_and & (1 << bitno)))
    ++bitno;
  bitmask = (1 << bitno);

  if (debug)
    printf ("use bit %d\n", bitno);

  /* Record in the bittree.  */
  bittree->bitno = bitno;

  /* Get two new opcode lists; adjust their masks.  */
  list0.next = NULL;
  list1.next = NULL;
  ptr0 = &list0.next;
  ptr1 = &list1.next;
  ent = opcode;
  while (ent)
    {
      if (ent->opcode & bitmask)
	{
	  ent->mask &= (~bitmask);
	  *ptr1 = ent;
	  ent = ent->next;
	  (*ptr1)->next = NULL;
	  ptr1 = &(*ptr1)->next;
	}
      else
	{
	  ent->mask &= (~bitmask);
	  *ptr0 = ent;
	  ent = ent->next;
	  (*ptr0)->next = NULL;
	  ptr0 = &(*ptr0)->next;
	}
    }

  /* If BITNO can NOT divide the opcode group, try next bit.  */
  if (list0.next == NULL)
    {
      opcode = list1.next;
      goto divide_table_1_try_again;
    }
  else if (list1.next == NULL)
    {
      opcode = list0.next;
      goto divide_table_1_try_again;
    }

  /* Further divide.  */
  bittree->bits[0] = new_bittree_node ();
  bittree->bits[1] = new_bittree_node ();
  divide_table_1 (bittree->bits[0], list0.next);
  divide_table_1 (bittree->bits[1], list1.next);

 divide_table_1_finish:
  if (debug)
    printf ("Leave from depth %d\n", depth);
  --depth;

  /* Record the opcode entries on this leaf node.  */
  bittree->list = opcode;

  return;
}

/* Call divide_table_1 to divide the all the opcodes and thus create the
   decoding decision tree.  */
static struct bittree *
divide_table (void)
{
  struct bittree *bittree = new_bittree_node ();
  divide_table_1 (bittree, opcode_nodes_head.next);
  return bittree;
}

/* Read in all of the tables, create the decoding decision tree and return
   the tree root.  */
static struct bittree *
initialize_decoder_tree (void)
{
  int i;
  const int num_of_tables = (sizeof (aarch64_opcode_tables)
			     / sizeof (struct aarch64_opcode *));
  for (i = 0; i < num_of_tables; ++i)
    read_table (aarch64_opcode_tables [i]);
  if (debug)
    print_opcode_nodes ();
  return divide_table ();
}

static void __attribute__ ((format (printf, 2, 3)))
indented_print (unsigned int indent, const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  printf ("%*s", (int) indent, "");
  vprintf (format, ap);
  va_end (ap);
}

/* N.B. read the comment above divide_table_1 for the reason why the generated
   decision tree function never returns NULL.  */

static void
print_decision_tree_1 (unsigned int indent, struct bittree* bittree)
{
  /* PATTERN is only used to generate comment in the code.  */
  static char pattern[33] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  /* Low bits in PATTERN will be printed first which then look as the high
     bits in comment.  We need to reverse the index to get correct print.  */
  unsigned int msb = sizeof (pattern) - 2;
  assert (bittree != NULL);

  /* Leaf node located.  */
  if (bittree->bits[0] == NULL && bittree->bits[1] == NULL)
    {
      assert (bittree->list != NULL);
      indented_print (indent, "/* 33222222222211111111110000000000\n");
      indented_print (indent, "   10987654321098765432109876543210\n");
      indented_print (indent, "   %s\n", pattern);
      indented_print (indent, "   %s.  */\n",
		      get_aarch64_opcode (bittree->list)->name);
      indented_print (indent, "return %u;\n",
		      real_index (bittree->list->index));
      return;
    }

  /* Walk down the decoder tree.  */
  indented_print (indent, "if (((word >> %d) & 0x1) == 0)\n", bittree->bitno);
  indented_print (indent, "  {\n");
  pattern[msb - bittree->bitno] = '0';
  print_decision_tree_1 (indent + 4, bittree->bits[0]);
  indented_print (indent, "  }\n");
  indented_print (indent, "else\n");
  indented_print (indent, "  {\n");
  pattern[msb - bittree->bitno] = '1';
  print_decision_tree_1 (indent + 4, bittree->bits[1]);
  indented_print (indent, "  }\n");
  pattern[msb - bittree->bitno] = 'x';
}

/* Generate aarch64_opcode_lookup in C code to the standard output.  */

static void
print_decision_tree (struct bittree* bittree)
{
  if (debug)
    printf ("Enter print_decision_tree\n");

  printf ("/* Called by aarch64_opcode_lookup.  */\n\n");

  printf ("static int\n");
  printf ("aarch64_opcode_lookup_1 (uint32_t word)\n");
  printf ("{\n");

  print_decision_tree_1 (2, bittree);

  printf ("}\n\n");


  printf ("/* Lookup opcode WORD in the opcode table.  N.B. all alias\n");
  printf ("   opcodes are ignored here.  */\n\n");

  printf ("const aarch64_opcode *\n");
  printf ("aarch64_opcode_lookup (uint32_t word)\n");
  printf ("{\n");
  printf ("  return aarch64_opcode_table + aarch64_opcode_lookup_1 (word);\n");
  printf ("}\n");
}

static void
print_find_next_opcode_1 (struct bittree* bittree)
{
  assert (bittree != NULL);

  /* Leaf node located.  */
  if (bittree->bits[0] == NULL && bittree->bits[1] == NULL)
    {
      assert (bittree->list != NULL);
      /* Find multiple opcode entries in one leaf node.  */
      if (bittree->list->next != NULL)
	{
	  opcode_node *list = bittree->list;
	  while (list != NULL)
	    {
	      const aarch64_opcode *curr = get_aarch64_opcode (list);
	      const aarch64_opcode *next = get_aarch64_opcode (list->next);

	      printf ("    case %u: ",
		      (unsigned int)(curr - aarch64_opcode_table));
	      if (list->next != NULL)
		{
		  printf ("value = %u; break;\t", real_index (list->next->index));
		  printf ("/* %s --> %s.  */\n", curr->name, next->name);
		}
	      else
		{
		  printf ("return NULL;\t\t");
		  printf ("/* %s --> NULL.  */\n", curr->name);
		}

	      list = list->next;
	    }
	}
      return;
    }

  /* Walk down the decoder tree.  */
  print_find_next_opcode_1 (bittree->bits[0]);
  print_find_next_opcode_1 (bittree->bits[1]);
}

/* Generate aarch64_find_next_opcode in C code to the standard output.  */

static void
print_find_next_opcode (struct bittree* bittree)
{
  if (debug)
    printf ("Enter print_find_next_opcode\n");

  printf ("\n");
  printf ("const aarch64_opcode *\n");
  printf ("aarch64_find_next_opcode (const aarch64_opcode *opcode)\n");
  printf ("{\n");
  printf ("  /* Use the index as the key to locate the next opcode.  */\n");
  printf ("  int key = opcode - aarch64_opcode_table;\n");
  printf ("  int value;\n");
  printf ("  switch (key)\n");
  printf ("    {\n");

  print_find_next_opcode_1 (bittree);

  printf ("    default: return NULL;\n");
  printf ("    }\n\n");

  printf ("  return aarch64_opcode_table + value;\n");
  printf ("}\n");
}

/* Release the dynamic memory resource allocated for the generation of the
   decoder tree.  */

static void
release_resource_decoder_tree (struct bittree* bittree)
{
  assert (bittree != NULL);

  /* Leaf node located.  */
  if (bittree->bits[0] == NULL && bittree->bits[1] == NULL)
    {
      assert (bittree->list != NULL);
      /* Free opcode_nodes.  */
      opcode_node *list = bittree->list;
      while (list != NULL)
	{
	  opcode_node *next = list->next;
	  free (list);
	  list = next;
	}
      /* Free the tree node.  */
      free (bittree);
      return;
    }

  /* Walk down the decoder tree.  */
  release_resource_decoder_tree (bittree->bits[0]);
  release_resource_decoder_tree (bittree->bits[1]);

  /* Free the tree node.  */
  free (bittree);
}

/* Generate aarch64_find_real_opcode in C code to the standard output.
   TABLE points to the alias info table, while NUM indicates the number of
   entries in the table.  */

static void
print_find_real_opcode (const opcode_node *table, int num)
{
  int i;

  if (debug)
    printf ("Enter print_find_real_opcode\n");

  printf ("\n");
  printf ("const aarch64_opcode *\n");
  printf ("aarch64_find_real_opcode (const aarch64_opcode *opcode)\n");
  printf ("{\n");
  printf ("  /* Use the index as the key to locate the real opcode.  */\n");
  printf ("  int key = opcode - aarch64_opcode_table;\n");
  printf ("  int value;\n");
  printf ("  switch (key)\n");
  printf ("    {\n");

  for (i = 0; i < num; ++i)
    {
      const opcode_node *real = table + i;
      const opcode_node *alias = real->next;
      for (; alias; alias = alias->next)
	printf ("    case %u:\t/* %s */\n", real_index (alias->index),
		get_aarch64_opcode (alias)->name);
      printf ("      value = %u;\t/* --> %s.  */\n", real_index (real->index),
	      get_aarch64_opcode (real)->name);
      printf ("      break;\n");
    }

  printf ("    default: return NULL;\n");
  printf ("    }\n\n");

  printf ("  return aarch64_opcode_table + value;\n");
  printf ("}\n");
}

/* Generate aarch64_find_alias_opcode in C code to the standard output.
   TABLE points to the alias info table, while NUM indicates the number of
   entries in the table.  */

static void
print_find_alias_opcode (const opcode_node *table, int num)
{
  int i;

  if (debug)
    printf ("Enter print_find_alias_opcode\n");

  printf ("\n");
  printf ("const aarch64_opcode *\n");
  printf ("aarch64_find_alias_opcode (const aarch64_opcode *opcode)\n");
  printf ("{\n");
  printf ("  /* Use the index as the key to locate the alias opcode.  */\n");
  printf ("  int key = opcode - aarch64_opcode_table;\n");
  printf ("  int value;\n");
  printf ("  switch (key)\n");
  printf ("    {\n");

  for (i = 0; i < num; ++i)
    {
      const opcode_node *node = table + i;
      assert (node->next);
      printf ("    case %u: value = %u; break;", real_index (node->index),
	      real_index (node->next->index));
      printf ("\t/* %s --> %s.  */\n", get_aarch64_opcode (node)->name,
	      get_aarch64_opcode (node->next)->name);
    }

  printf ("    default: return NULL;\n");
  printf ("    }\n\n");

  printf ("  return aarch64_opcode_table + value;\n");
  printf ("}\n");
}

/* Generate aarch64_find_next_alias_opcode in C code to the standard output.
   TABLE points to the alias info table, while NUM indicates the number of
   entries in the table.  */

static void
print_find_next_alias_opcode (const opcode_node *table, int num)
{
  int i;

  if (debug)
    printf ("Enter print_find_next_alias_opcode\n");

  printf ("\n");
  printf ("const aarch64_opcode *\n");
  printf ("aarch64_find_next_alias_opcode (const aarch64_opcode *opcode)\n");
  printf ("{\n");
  printf ("  /* Use the index as the key to locate the next opcode.  */\n");
  printf ("  int key = opcode - aarch64_opcode_table;\n");
  printf ("  int value;\n");
  printf ("  switch (key)\n");
  printf ("    {\n");

  for (i = 0; i < num; ++i)
    {
      const opcode_node *node = table + i;
      assert (node->next);
      if (node->next->next == NULL)
	continue;
      while (node->next->next)
	{
	  printf ("    case %u: value = %u; break;", real_index (node->next->index),
		 real_index (node->next->next->index));
	  printf ("\t/* %s --> %s.  */\n",
		  get_aarch64_opcode (node->next)->name,
		  get_aarch64_opcode (node->next->next)->name);
	  node = node->next;
	}
    }

  printf ("    default: return NULL;\n");
  printf ("    }\n\n");

  printf ("  return aarch64_opcode_table + value;\n");
  printf ("}\n");
}

/* Given OPCODE, establish and return a link list of alias nodes in the
   preferred order.  */

opcode_node *
find_alias_opcode (const aarch64_opcode *opcode)
{
  int i;
  /* Assume maximum of 32 disassemble preference candidates.  */
  const int max_num_aliases = 32;
  const aarch64_opcode *ent;
  const aarch64_opcode *preferred[max_num_aliases + 1];
  opcode_node head, **next;

  assert (opcode_has_alias (opcode));

  i = 0;
  if (opcode->name != NULL)
    preferred[i++] = opcode;
  ent = aarch64_opcode_table;
  while (ent->name != NULL)
    {
      /* The mask of an alias opcode must be equal to or a super-set (i.e.
	 more constrained) of that of the aliased opcode; so is the base
	 opcode value.  */
      if (alias_opcode_p (ent)
	  && (ent->mask & opcode->mask) == opcode->mask
	  && (opcode->mask & ent->opcode) == (opcode->mask & opcode->opcode))
	{
	  assert (i < max_num_aliases);
	  preferred[i++] = ent;
	  if (debug)
	    printf ("found %s for %s.", ent->name, opcode->name);
	}
      ++ent;
    }

  if (debug)
    {
      int m;
      printf ("un-orderd list: ");
      for (m = 0; m < i; ++m)
	printf ("%s, ", preferred[m]->name);
      printf ("\n");
    }

  /* There must be at least one alias.  */
  assert (i >= 1);

  /* Sort preferred array according to the priority (from the lowest to the
     highest.  */
  if (i > 1)
    {
      int j, k;
      for (j = 0; j < i - 1; ++j)
	{
	  for (k = 0; k < i - 1 - j; ++k)
	    {
	      const aarch64_opcode *t;
	      t = preferred [k+1];
	      if (opcode_priority (t) < opcode_priority (preferred [k]))
		{
		  preferred [k+1] = preferred [k];
		  preferred [k] = t;
		}
	    }
	}
    }

  if (debug)
    {
      int m;
      printf ("orderd list: ");
      for (m = 0; m < i; ++m)
	printf ("%s, ", preferred[m]->name);
      printf ("\n");
    }

  /* Create a link-list of opcode_node with disassemble preference from
     higher to lower.  */
  next = &head.next;
  --i;
  while (i >= 0)
    {
      const aarch64_opcode *alias = preferred [i];
      opcode_node *node = new_opcode_node ();

      if (debug)
	printf ("add %s.\n", alias->name);

      node->index = alias - aarch64_opcode_table;
      *next = node;
      next = &node->next;

      --i;
    }
  *next = NULL;

  return head.next;
}

/* Create and return alias information.
   Return the address of the created alias info table; return the number
   of table entries in *NUM_PTR.  */

opcode_node *
create_alias_info (int *num_ptr)
{
  int i, num;
  opcode_node *ret;
  const aarch64_opcode *ent;

  /* Calculate the total number of opcodes that have alias.  */
  num = 0;
  ent = aarch64_opcode_table;
  while (ent->name != NULL)
    {
      if (opcode_has_alias (ent))
	{
	  /* Assert the alias relationship be flat-structured to keep
	     algorithms simple; not allow F_ALIAS and F_HAS_ALIAS both
	     specified.  */
	  assert (!alias_opcode_p (ent));
	  ++num;
	}
      ++ent;
    }
  assert (num_ptr);
  *num_ptr = num;

  /* The array of real opcodes that have alias(es).  */
  ret = malloc (sizeof (opcode_node) * num);

  /* For each opcode, establish a list of alias nodes in a preferred
     order.  */
  for (i = 0, ent = aarch64_opcode_table; i < num; ++i, ++ent)
    {
      opcode_node *node = ret + i;
      while (ent->name != NULL && !opcode_has_alias (ent))
	++ent;
      assert (ent->name != NULL);
      node->index = ent - aarch64_opcode_table;
      node->next = find_alias_opcode (ent);
      assert (node->next);
    }
  assert (i == num);

  return ret;
}

/* Release the dynamic memory resource allocated for the generation of the
   alias information.  */

void
release_resource_alias_info (opcode_node *alias_info, int num)
{
  int i = 0;
  opcode_node *node = alias_info;

  /* Free opcode_node list.  */
  for (; i < num; ++i, ++node)
    {
      opcode_node *list = node->next;
      do
	{
	  opcode_node *next = list->next;
	  free (list);
	  list = next;
	} while (list != NULL);
    }

  /* Free opcode_node array.  */
  free (alias_info);
}

/* As a debugging utility, print out the result of the table division, although
   it is not doing much this moment.  */
static void
print_divide_result (const struct bittree *bittree ATTRIBUTE_UNUSED)
{
  printf ("max_num_opcodes_at_leaf_node: %d\n", max_num_opcodes_at_leaf_node);
  return;
}

/* Structure to help generate the operand table.  */
struct operand
{
  const char *class;
  const char *inserter;
  const char *extractor;
  const char *str;
  const char *flags;
  const char *fields;
  const char *desc;
  unsigned processed : 1;
  unsigned has_inserter : 1;
  unsigned has_extractor : 1;
};

typedef struct operand operand;

#ifdef X
#undef X
#endif

#ifdef Y
#undef Y
#endif

#ifdef F
#undef F
#endif

/* Get the operand information in strings.  */

static operand operands[] =
{
    {"NIL", "0", "0", "", "0", "{0}", "<none>", 0, 0, 0},
#define F(...)	#__VA_ARGS__
#define X(a,b,c,d,e,f,g)	\
    {#a, #b, #c, d, #e, "{"f"}", g, 0, 0, 0},
#define Y(a,b,d,e,f,g)		\
    {#a, "ins_"#b, "ext_"#b, d, #e, "{"f"}", g, 0, 0, 0},
    AARCH64_OPERANDS
    {"NIL", "0", "0", "", "0", "{0}", "DUMMY", 0, 0, 0},
};

#undef F
#undef X

static void
process_operand_table (void)
{
  int i;
  operand *opnd;
  const int num = sizeof (operands) / sizeof (operand);

  for (i = 0, opnd = operands; i < num; ++i, ++opnd)
    {
      opnd->has_inserter = opnd->inserter[0] != '0';
      opnd->has_extractor = opnd->extractor[0] != '0';
    }
}

/* Generate aarch64_operands in C to the standard output.  */

static void
print_operand_table (void)
{
  int i;
  operand *opnd;
  const int num = sizeof (operands) / sizeof (operand);

  if (debug)
    printf ("Enter print_operand_table\n");

  printf ("\n");
  printf ("const struct aarch64_operand aarch64_operands[] =\n");
  printf ("{\n");

  for (i = 0, opnd = operands; i < num; ++i, ++opnd)
    {
      char flags[256];
      flags[0] = '\0';
      if (opnd->flags[0] != '0')
	sprintf (flags, "%s", opnd->flags);
      if (opnd->has_inserter)
	{
	  if (flags[0] != '\0')
	    strcat (flags, " | ");
	  strcat (flags, "OPD_F_HAS_INSERTER");
	}
      if (opnd->has_extractor)
	{
	  if (flags[0] != '\0')
	    strcat (flags, " | ");
	  strcat (flags, "OPD_F_HAS_EXTRACTOR");
	}
      if (flags[0] == '\0')
	{
	  flags[0] = '0';
	  flags[1] = '\0';
	}
    printf ("  {AARCH64_OPND_CLASS_%s, \"%s\", %s, %s, \"%s\"},\n",
	    opnd->class, opnd->str, flags, opnd->fields, opnd->desc);
    }
  printf ("};\n");
}

/* Generate aarch64_insert_operand in C to the standard output.  */

static void
print_operand_inserter (void)
{
  int i;
  operand *opnd;
  const int num = sizeof (operands) / sizeof (operand);

  if (debug)
    printf ("Enter print_operand_inserter\n");

  printf ("\n");
  printf ("bool\n");
  printf ("aarch64_insert_operand (const aarch64_operand *self,\n\
			   const aarch64_opnd_info *info,\n\
			   aarch64_insn *code, const aarch64_inst *inst,\n\
			   aarch64_operand_error *errors)\n");
  printf ("{\n");
  printf ("  /* Use the index as the key.  */\n");
  printf ("  int key = self - aarch64_operands;\n");
  printf ("  switch (key)\n");
  printf ("    {\n");

  for (i = 0, opnd = operands; i < num; ++i, ++opnd)
    opnd->processed = 0;

  for (i = 0, opnd = operands; i < num; ++i, ++opnd)
    {
      if (!opnd->processed && opnd->has_inserter)
	{
	  int j = i + 1;
	  const int len = strlen (opnd->inserter);
	  operand *opnd2 = opnd + 1;
	  printf ("    case %u:\n", (unsigned int)(opnd - operands));
	  opnd->processed = 1;
	  for (; j < num; ++j, ++opnd2)
	    {
	      if (!opnd2->processed
		  && opnd2->has_inserter
		  && len == strlen (opnd2->inserter)
		  && strncmp (opnd->inserter, opnd2->inserter, len) == 0)
		{
		  printf ("    case %u:\n", (unsigned int)(opnd2 - operands));
		  opnd2->processed = 1;
		}
	    }
	  printf ("      return aarch64_%s (self, info, code, inst, errors);\n",
		  opnd->inserter);
	}
    }

  printf ("    default: assert (0); abort ();\n");
  printf ("    }\n");
  printf ("}\n");
}

/* Generate aarch64_extract_operand in C to the standard output.  */

static void
print_operand_extractor (void)
{
  int i;
  operand *opnd;
  const int num = sizeof (operands) / sizeof (operand);

  if (debug)
    printf ("Enter print_operand_extractor\n");

  printf ("\n");
  printf ("bool\n");
  printf ("aarch64_extract_operand (const aarch64_operand *self,\n\
			   aarch64_opnd_info *info,\n\
			   aarch64_insn code, const aarch64_inst *inst,\n\
			   aarch64_operand_error *errors)\n");
  printf ("{\n");
  printf ("  /* Use the index as the key.  */\n");
  printf ("  int key = self - aarch64_operands;\n");
  printf ("  switch (key)\n");
  printf ("    {\n");

  for (i = 0, opnd = operands; i < num; ++i, ++opnd)
    opnd->processed = 0;

  for (i = 0, opnd = operands; i < num; ++i, ++opnd)
    {
      if (!opnd->processed && opnd->has_extractor)
	{
	  int j = i + 1;
	  const int len = strlen (opnd->extractor);
	  operand *opnd2 = opnd + 1;
	  printf ("    case %u:\n", (unsigned int)(opnd - operands));
	  opnd->processed = 1;
	  for (; j < num; ++j, ++opnd2)
	    {
	      if (!opnd2->processed
		  && opnd2->has_extractor
		  && len == strlen (opnd2->extractor)
		  && strncmp (opnd->extractor, opnd2->extractor, len) == 0)
		{
		  printf ("    case %u:\n", (unsigned int)(opnd2 - operands));
		  opnd2->processed = 1;
		}
	    }
	  printf ("      return aarch64_%s (self, info, code, inst, errors);\n",
		  opnd->extractor);
	}
    }

  printf ("    default: assert (0); abort ();\n");
  printf ("    }\n");
  printf ("}\n");
}

/* Table indexed by opcode enumerator stores the index of the corresponding
   opcode entry in aarch64_opcode_table.  */
static unsigned op_enum_table [OP_TOTAL_NUM];

/* Print out the routine which, given the opcode enumerator, returns the
   corresponding opcode entry pointer.  */

static void
print_get_opcode (void)
{
  int i;
  const int num = OP_TOTAL_NUM;
  const aarch64_opcode *opcode;

  if (debug)
    printf ("Enter print_get_opcode\n");

  /* Fill in the internal table.  */
  opcode = aarch64_opcode_table;
  while (opcode->name != NULL)
    {
      if (opcode->op != OP_NIL)
	{
	  /* Assert opcode enumerator be unique, in other words, no shared by
	     different opcodes.  */
	  if (op_enum_table[opcode->op] != 0)
	    {
	      fprintf (stderr, "Opcode %u is shared by different %s and %s.\n",
		       opcode->op,
		       aarch64_opcode_table[op_enum_table[opcode->op]].name,
		       opcode->name);
	      assert (0);
	      abort ();
	    }
	  assert (opcode->op < OP_TOTAL_NUM);
	  op_enum_table[opcode->op] = opcode - aarch64_opcode_table;
	}
      ++opcode;
    }

  /* Print the table.  */
  printf ("\n");
  printf ("/* Indexed by an enum aarch64_op enumerator, the value is the offset of\n\
   the corresponding aarch64_opcode entry in the aarch64_opcode_table.  */\n\n");
  printf ("static const unsigned op_enum_table [] =\n");
  printf ("{\n");
  for (i = 0; i < num; ++i)
    printf ("  %u,\n", op_enum_table[i]);
  printf ("};\n");

  /* Print the function.  */
  printf ("\n");
  printf ("/* Given the opcode enumerator OP, return the pointer to the corresponding\n");
  printf ("   opcode entry.  */\n");
  printf ("\n");
  printf ("const aarch64_opcode *\n");
  printf ("aarch64_get_opcode (enum aarch64_op op)\n");
  printf ("{\n");
  printf ("  return aarch64_opcode_table + op_enum_table[op];\n");
  printf ("}\n");
}

/* Print out the content of an opcode table (not in use).  */
static void ATTRIBUTE_UNUSED
print_table (struct aarch64_opcode* table)
{
  struct aarch64_opcode *ent = table;
  do
    {
      printf ("%s\t%08x\t%08x\n", ent->name, (unsigned int)ent->opcode,
	      (unsigned int)ent->mask);
    } while ((++ent)->name);
}

static const char * program_name = NULL;

/* Program options.  */
struct option long_options[] =
{
  {"debug",   no_argument,       NULL, 'd'},
  {"version", no_argument,       NULL, 'V'},
  {"help",    no_argument,       NULL, 'h'},
  {"gen-opc", no_argument,       NULL, 'c'},
  {"gen-asm", no_argument,       NULL, 'a'},
  {"gen-dis", no_argument,       NULL, 's'},
  {0,         no_argument,       NULL, 0}
};

static void
print_version (void)
{
  printf ("%s: version 1.0\n", program_name);
  xexit (0);
}

static void
usage (FILE * stream, int status)
{
  fprintf (stream, "Usage: %s [-V | --version] [-d | --debug] [--help]\n",
	   program_name);
  fprintf (stream, "\t[ [-c | --gen-opc] | [-a | --gen-asm] | [-s | --gen-dis] ]\n");
  xexit (status);
}

int
main (int argc, char **argv)
{
  extern int chdir (char *);
  int c;
  int gen_opcode_p = 0;
  int gen_assembler_p = 0;
  int gen_disassembler_p = 0;

  program_name = *argv;
  xmalloc_set_program_name (program_name);

  while ((c = getopt_long (argc, argv, "vVdhacs", long_options, 0)) != EOF)
    switch (c)
      {
      case 'V':
      case 'v':
	print_version ();
	break;
      case 'd':
	debug = 1;
	break;
      case 'h':
      case '?':
	usage (stderr, 0);
	break;
      case 'c':
	gen_opcode_p = 1;
	break;
      case 'a':
	gen_assembler_p = 1;
	break;
      case 's':
	gen_disassembler_p = 1;
	break;
      default:
      case 0:
	break;
      }

  if (argc == 1 || optind != argc)
    usage (stdout, 1);

  if (gen_opcode_p + gen_assembler_p + gen_disassembler_p > 1)
    {
      printf ("Please specify only one of the following options\n\
	      [-c | --gen-opc] [-a | --gen-asm] [-s | --gen-dis]\n");
      xexit (2);
    }

  struct bittree *decoder_tree;

  decoder_tree = initialize_decoder_tree ();
  if (debug)
    print_divide_result (decoder_tree);

  printf ("/* This file is automatically generated by aarch64-gen.  Do not edit!  */\n");
  printf ("/* Copyright (C) 2012-2023 Free Software Foundation, Inc.\n\
   Contributed by ARM Ltd.\n\
\n\
   This file is part of the GNU opcodes library.\n\
\n\
   This library is free software; you can redistribute it and/or modify\n\
   it under the terms of the GNU General Public License as published by\n\
   the Free Software Foundation; either version 3, or (at your option)\n\
   any later version.\n\
\n\
   It is distributed in the hope that it will be useful, but WITHOUT\n\
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n\
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public\n\
   License for more details.\n\
\n\
   You should have received a copy of the GNU General Public License\n\
   along with this program; see the file COPYING3. If not,\n\
   see <http://www.gnu.org/licenses/>.  */\n");

  printf ("\n");
  printf ("#include \"sysdep.h\"\n");
  if (gen_opcode_p)
    printf ("#include \"aarch64-opc.h\"\n");
  if (gen_assembler_p)
    printf ("#include \"aarch64-asm.h\"\n");
  if (gen_disassembler_p)
    printf ("#include \"aarch64-dis.h\"\n");
  printf ("\n");

  /* Generate opcode entry lookup for the disassembler.  */
  if (gen_disassembler_p)
    {
      print_decision_tree (decoder_tree);
      print_find_next_opcode (decoder_tree);
      release_resource_decoder_tree (decoder_tree);
    }

  /* Generate alias opcode handling for the assembler or the disassembler.  */
  if (gen_assembler_p || gen_disassembler_p)
    {
      int num;
      opcode_node *alias_info = create_alias_info (&num);

      if (gen_assembler_p)
	print_find_real_opcode (alias_info, num);

      if (gen_disassembler_p)
	{
	  print_find_alias_opcode (alias_info, num);
	  print_find_next_alias_opcode (alias_info, num);
	}

      release_resource_alias_info (alias_info, num);
    }

  /* Generate operand table.  */
  process_operand_table ();

  if (gen_assembler_p)
    print_operand_inserter ();

  if (gen_disassembler_p)
    print_operand_extractor ();

  if (gen_opcode_p)
    print_operand_table ();

  /* Generate utility to return aarch64_opcode entry given an enumerator.  */
  if (gen_opcode_p)
    print_get_opcode ();

  exit (0);
}
