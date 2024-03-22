NIR ALU Instructions
====================

ALU instructions represent simple operations, such as addition, multiplication,
comparison, etc., that take a certain number of arguments and return a result
that only depends on the arguments.  ALU instructions in NIR must be pure in
the sense that they have no side effect and that identical inputs yields an
identical output.  A good rule of thumb is that only things which can be
constant folded should be ALU operations.  If it can't be constant folded, then
it should probably be an intrinsic instead.

Each ALU instruction has an opcode, which is a member of the :c:enum:`nir_op`
enum, that describes what it does as well as how many arguments it takes.
Associated with each opcode is an metadata structure,
:c:struct:`nir_op_info`, which shows how many arguments the opcode takes,
information about data types, and algebraic properties such as associativity
and commutativity. The info structure for each opcode may be accessed through
a global :c:var:`nir_op_infos` array that’s indexed by the opcode.

ALU operations are typeless, meaning that they're only defined to convert
a certain bit-pattern input to another bit-pattern output.  The only concrete
notion of types for a NIR SSA value or register is that each value has a number
of vector components and a bit-size.  How that data is interpreted is entirely
controlled by the opcode.  NIR doesn't have opcodes for ``intBitsToFloat()``
and friends because they are implicit.

Even though ALU operations are typeless, each opcode also has an "ALU type"
metadata for each of the sources and the destination which can be
floating-point, boolean, integer, or unsigned integer.  The ALU type mainly
helps back-ends which want to handle all conversion instructions, for instance,
in a single switch case.  They're also important when a back-end requests the
absolute value, negate, and saturate modifiers (not used by core NIR).  In that
case, modifiers are interpreted with respect to the ALU type on the source or
destination of the instruction.  In addition, if an operation takes a boolean
argument, then the argument may be assumed to be either ``0`` for false or
``~0`` (a.k.a ``-1``) for true even if it is not a 1-bit value.  If an
operation’s result has a boolean type, then it may only produce only ``0`` or ``~0``.

Most of the common ALU ops in NIR operate per-component, meaning that the
operation is defined by what it does on a single scalar value and, when
performed on vectors, it performs the same operation on each component.  Things
like add, multiply, etc. fall into this category.  Per-component operations
naturally scale to as many components as necessary.  Non-per-component ALU ops
are things like :nir:alu-op:`vec4` or :nir:alu-op:`pack_64_2x32` where any
given component in the result value may be a combination of any component in
any source.  These ops have a number of destination components and a number of
components required by each source which is fixed by the opcode.

While most instruction types in NIR require vector sizes to perfectly match on
inputs and outputs, ALU instruction sources have an additional
:c:member:`nir_alu_src.swizzle` field which allows them to act on vectors
which are not the native vector size of the instruction.  This is ideal for
hardware with a native data type of `vec4` but also means that ALU
instructions are often used (and required) for packing/unpacking vectors for
use in other instruction types like intrinsics or texture ops.

.. c:autostruct:: nir_op_info
   :file: src/compiler/nir/nir.h
   :members:

.. c:autovar:: nir_op_infos

.. c:autostruct:: nir_alu_instr
   :members:

.. c:autostruct:: nir_alu_src
   :members:

NIR ALU Opcode Reference:
-------------------------

.. nir:alu-opcodes::
