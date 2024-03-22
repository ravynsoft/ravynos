ISASPEC - XML Based ISA Specification
=====================================

isaspec provides a mechanism to describe an instruction set in XML, and
generate a disassembler and assembler.  The intention is
to describe the instruction set more formally than hand-coded assembler
and disassembler, and better decouple the shader compiler from the
underlying instruction encoding to simplify dealing with instruction
encoding differences between generations of GPU.

Benefits of a formal ISA description, compared to hand-coded assemblers
and disassemblers, include easier detection of new bit combinations that
were not seen before in previous generations due to more rigorous
description of bits that are expect to be '0' or '1' or 'x' (dontcare)
and verification that different encodings don't have conflicting bits
(i.e. that the specification cannot result in more than one valid
interpretation of any bit pattern).

The isaspec tool and XML schema are intended to be generic (not specific
to ir3), although there are currently a couple limitations due to short-
cuts taken to get things up and running (which are mostly not inherent to
the XML schema, and should not be too difficult to remove from the py and
decode/disasm utility):

* Maximum "field" size is 64b
* Fixed instruction size

Often times, especially when new functionality is added in later gens
while retaining (or at least mostly retaining) backwards compatibility
with encodings used in earlier generations, the actual encoding can be
rather messy to describe.  To support this, isaspec provides many flexible
mechanism, such as conditional overrides and derived fields.  This not
only allows for describing an irregular instruction encoding, but also
allows matching an existing disasm syntax (which might not have been
design around the idea of disassembly based on a formal ISA description).

Bitsets
-------

The fundamental concept of matching a bit-pattern to an instruction
decoding/encoding is the concept of a hierarchical tree of bitsets.
This is intended to match how the HW decodes instructions, where certain
bits describe the instruction (and sub-encoding, and so on), and other
bits describe various operands to the instruction.

Bitsets can also be used recursively as the type of a field described
in another bitset.

The leaves of the tree of instruction bitsets represent every possible
instruction.  Deciding which instruction a bitpattern is amounts to:

.. code-block:: c

   m = (val & bitsets[n]->mask) & ~bitsets[n]->dontcare;

   if (m == bitsets[n]->match) {
      /* we've found the instruction description */
   }

For example, the starting point to decode an ir3 instruction is a 64b
bitset:

.. code-block:: xml

   <bitset name="#instruction" size="64">
   	<doc>
   		Encoding of an ir3 instruction.  All instructions are 64b.
   	</doc>
   </bitset>

In the first level of instruction encoding hierarchy, the high three bits
group things into instruction "categories":

.. code-block:: xml

   <bitset name="#instruction-cat2" extends="#instruction">
   	<field name="DST" low="32" high="39" type="#reg-gpr"/>
   	<field name="REPEAT" low="40" high="41" type="#rptN"/>
   	<field name="SAT" pos="42" type="bool" display="(sat)"/>
   	<field name="SS" pos="44" type="bool" display="(ss)"/>
   	<field name="UL" pos="45" type="bool" display="(ul)"/>
   	<field name="DST_CONV" pos="46" type="bool">
   		<doc>
   			Destination register is opposite precision as source, i.e.
   			if {FULL} is true then destination is half precision, and
   			visa versa.
   		</doc>
   	</field>
   	<derived name="DST_HALF" expr="#dest-half" type="bool" display="h"/>
   	<field name="EI" pos="47" type="bool" display="(ei)"/>
   	<field name="FULL" pos="52" type="bool">
   		<doc>Full precision source registers</doc>
   	</field>
   	<field name="JP" pos="59" type="bool" display="(jp)"/>
   	<field name="SY" pos="60" type="bool" display="(sy)"/>
   	<pattern low="61" high="63">010</pattern>  <!-- cat2 -->
   	<!--
   		NOTE, both SRC1_R and SRC2_R are defined at this level because
   		SRC2_R is still a valid bit for (nopN) (REPEAT==0) for cat2
   		instructions with only a single src
   	 -->
   	<field name="SRC1_R" pos="43" type="bool" display="(r)"/>
   	<field name="SRC2_R" pos="51" type="bool" display="(r)"/>
   	<derived name="ZERO" expr="#zero" type="bool" display=""/>
   </bitset>

The ``<pattern>`` elements are the part(s) that determine which leaf-node
bitset matches against a given bit pattern.  The leaf node's match/mask/
dontcare bitmasks are a combination of those defined at the leaf node and
recursively each parent bitclass.

For example, cat2 instructions (ALU instructions with up to two src
registers) can have either one or two source registers:

.. code-block:: xml

   <bitset name="#instruction-cat2-1src" extends="#instruction-cat2">
   	<override expr="#cat2-cat3-nop-encoding">
   		<display>
   			{SY}{SS}{JP}{SAT}(nop{NOP}) {UL}{NAME} {EI}{DST_HALF}{DST}, {SRC1}
   		</display>
   		<derived name="NOP" expr="#cat2-cat3-nop-value" type="uint"/>
   		<field name="SRC1" low="0" high="15" type="#multisrc">
   			<param name="ZERO" as="SRC_R"/>
   			<param name="FULL"/>
   		</field>
   	</override>
   	<display>
   		{SY}{SS}{JP}{SAT}{REPEAT}{UL}{NAME} {EI}{DST_HALF}{DST}, {SRC1}
   	</display>
   	<pattern low="16" high="31">xxxxxxxxxxxxxxxx</pattern>
   	<pattern low="48" high="50">xxx</pattern>  <!-- COND -->
   	<field name="SRC1" low="0" high="15" type="#multisrc">
   		<param name="SRC1_R" as="SRC_R"/>
   		<param name="FULL"/>
   	</field>
   </bitset>
   
   <bitset name="absneg.f" extends="#instruction-cat2-1src">
   	<pattern low="53" high="58">000110</pattern>
   </bitset>

In this example, ``absneg.f`` is a concrete cat2 instruction (leaf node of
the bitset inheritance tree) which has a single src register.  At the
``#instruction-cat2-1src`` level, bits that are used for the 2nd src arg
and condition code (for cat2 instructions which use a condition code) are
defined as 'x' (dontcare), which matches our understanding of the hardware
(but also lets the disassembler flag cases where '1' bits show up in places
we don't expect, which may signal a new instruction (sub)encoding).

You'll notice that ``SRC1`` refers back to a different bitset hierarchy
that describes various different src register encoding (used for cat2 and
cat4 instructions), i.e. GPR vs CONST vs relative GPR/CONST.  For fields
which have bitset types, parameters can be "passed" in via ``<param>``
elements, which can be referred to by the display template string, and/or
expressions.  For example, this helps to deal with cases where other fields
outside of that bitset control the encoding/decoding, such as in the
``#multisrc`` example:

.. code-block:: xml

   <bitset name="#multisrc" size="16">
   	<doc>
   		Encoding for instruction source which can be GPR/CONST/IMMED
   		or relative GPR/CONST.
   	</doc>
   </bitset>

   ...

   <bitset name="#multisrc-gpr" extends="#multisrc">
   	<display>
   		{ABSNEG}{SRC_R}{HALF}{SRC}
   	</display>
   	<derived name="HALF" expr="#multisrc-half" type="bool" display="h"/>
   	<field name="SRC" low="0" high="7" type="#reg-gpr"/>
   	<pattern low="8" high="13">000000</pattern>
   	<field name="ABSNEG" low="14" high="15" type="#absneg"/>
   </bitset>

At some level in the bitset inheritance hierarchy, there is expected to be a
``<display>`` element specifying a template string used during bitset
decoding.  The display template consists of references to fields (which may
be derived fields) specified as ``{FIELDNAME}`` and other characters
which are just echoed through to the resulting decoded bitset.

The special field reference ``{NAME}`` prints the name of the bitset. This is
often useful when the ``<display>`` element is at a higher level than the
leaves of the hierarchy, for example a whole class of similar instructions that
only differ in opcode.

Sometimes there may be multiple variants of an instruction that must be
different bitsets, for example because they are so different that they must
derive from different bitsets, but they have the same name. Because bitset
names must be unique in the encoder, this can be a problem, but this can worked
around with the ``displayname`` attribute on the ``bitset`` which changes how
``{NAME}`` is displayed but not the name used in the encoder. ``displayname``
is only useful for leaf bitsets.

It is possible to define a line column alignment value per field to influence
the visual output. It needs to be specified as ``{FIELDNAME:align=xx}``.

The ``<override>`` element will be described in the next section, but it
provides for both different decoded instruction syntax/mnemonics (when
simply providing a different display template string) as well as instruction
encoding where different ranges of bits have a different meaning based on
some other bitfield (or combination of bitfields).  In this example it is
used to cover the cases where ``SRCn_R`` has a different meaning and a
different disassembly syntax depending on whether ``REPEAT`` equals zero.

The ``<template>`` element can be used to represent a placeholder for a more
complex ``<display>`` substring.

Overrides
---------

In many cases, a bitset is not convenient for describing the expected
disasm syntax, and/or interpretation of some range of bits differs based
on some other field or combination of fields.  These *could* be modeled
as different derived bitsets, at the expense of a combinatorial explosion
of the size of the bitset inheritance tree.  For example, *every* cat2
(and cat3) instruction has both a ``(nopN)`` interpretation in addition to
the ``(rptN`)`` interpretation.

An ``<override>`` in a bitset allows to redefine the display string, and/or
field definitions from the default case.  If the override's expr(ession)
evaluates to non-zero, ``<display>``, ``<field>``, and ``<derived>``
elements take precedence over what is defined in the top-level of the
bitset (i.e. the default case).

Expressions
-----------

Both ``<override>`` and ``<derived>`` fields make use of ``<expr>`` elements,
either defined inline, or defined and named at the top level and referred to
by name in multiple other places.  An expression is a simple 'C' expression
which can reference fields (including other derived fields) with the same
``{FIELDNAME}`` syntax as display template strings.  For example:

.. code-block:: xml

   <expr name="#cat2-cat3-nop-encoding">
   	(({SRC1_R} != 0) || ({SRC2_R} != 0)) &amp;&amp; ({REPEAT} == 0)
   </expr>

In the case of ``<override>`` elements, the override applies if the expression
evaluates to non-zero.  In the case of ``<derived>`` fields, the expression
evaluates to the value of the derived field.

Branching
---------

isaspec supports a few special field types for printing branch destinations. If
``isaspec_decode_options::branch_labels`` is true, a pre-pass over the program
to be disassembled determines which instructions are branch destinations and
then they are printed when disassembling, in addition to printing the name of
the destination when printing the field itself.

There are two different types, which affect how the destination is computed. If
the field type is ``branch``, then the field is interpreted as a signed offset
from the current instruction. If the type is ``absbranch``, then it is
interpreted as an offset from the first instruction to be disassembled. In
either case, the offset is multiplied by the instruction size.

For example, here is what a signed-offset unconditional jump instruction might
look like:

.. code-block:: xml

   <bitset name="jump" extends="#instruction">
      <display>
         jump #{OFFSET}
      </display>
      <pattern low="26" high="31">110010</pattern> <!-- opcode goes here -->
      <field name="OFFSET" low="0" high="25" type="branch"/>
   </bitset>

This would produce a disassembly like ``jump #l42`` if the destination is 42
instructions after the start of the disassembly. The destination would be
preceded by a line with just ``l42:``.

``branch`` and ``absbranch`` fields can additionally have a ``call="true"``
attribute. For now, this just changes the disassembly. In particular the label
prefix is changed to ``fxn`` and an extra empty line before the destination is
added to visually seperate the disassembly into functions. So, for example, a
call instruction defined like this:

.. code-block:: xml

   <bitset name="call" extends="#instruction">
      <display>
         call #{OFFSET}
      </display>
      <pattern low="26" high="31">110010</pattern> <!-- opcode goes here -->
      <field name="OFFSET" low="0" high="25" type="branch" call="true"/>
   </bitset>

will disassemble to ``call #fxn42``.

Finally, users with special knowledge about where execution may start can define
"entrypoints" when disassembling which are printed like function call
destinations, with an extra empty line, but with an arbitrary user-defined
name. Names that are ``fxn`` or ``l`` followed by a number are discouraged
because they may clash with automatically-generated names.

Encoding
--------

To facilitate instruction encoding, ``<encode>`` elements can be provided
to teach the generated instruction packing code how to map from data structures
representing the IR to fields.  For example:

.. code-block:: xml

   <bitset name="#instruction" size="64">
   	<doc>
   		Encoding of an ir3 instruction.  All instructions are 64b.
   	</doc>
   	<gen min="300"/>
   	<encode type="struct ir3_instruction *" case-prefix="OPC_">
   		<!--
   			Define mapping from encode src to individual fields,
   			which are common across all instruction categories
   			at the root instruction level
   
   			Not all of these apply to all instructions, but we
   			can define mappings here for anything that is used
   			in more than one instruction category.  For things
   			that are specific to a single instruction category,
   			mappings should be defined at that level instead.
   		 -->
   		<map name="DST">src->regs[0]</map>
   		<map name="SRC1">src->regs[1]</map>
   		<map name="SRC2">src->regs[2]</map>
   		<map name="SRC3">src->regs[3]</map>
   		<map name="REPEAT">src->repeat</map>
   		<map name="SS">!!(src->flags &amp; IR3_INSTR_SS)</map>
   		<map name="JP">!!(src->flags &amp; IR3_INSTR_JP)</map>
   		<map name="SY">!!(src->flags &amp; IR3_INSTR_SY)</map>
   		<map name="UL">!!(src->flags &amp; IR3_INSTR_UL)</map>
   		<map name="EQ">0</map>  <!-- We don't use this (yet) -->
   		<map name="SAT">!!(src->flags &amp; IR3_INSTR_SAT)</map>
   	</encode>
   </bitset>

The ``type`` attribute specifies that the input to encoding an instruction
is a ``struct ir3_instruction *``.  In the case of bitset hierarchies with
multiple possible leaf nodes, a ``case-prefix`` attribute should be supplied
along with a function that maps the bitset encode source to an enum value
with the specified prefix prepended to uppercase'd leaf node name.  I.e. in
this case, "add.f" becomes ``OPC_ADD_F``.

Individual ``<map>`` elements teach the encoder how to map from the encode
source to fields in the encoded instruction.
