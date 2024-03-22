=====================
Adreno Five Microcode
=====================

.. contents::

.. _afuc-introduction:

Introduction
============

Adreno GPUs prior to 6xx use two micro-controllers to parse the command-stream,
setup the hardware for draws (or compute jobs), and do various GPU
housekeeping.  They are relatively simple (basically glorified
register writers) and basically all their state is in a collection
of registers.  Ie. there is no stack, and no memory assigned to
them; any global state like which bank of context registers is to
be used in the next draw is stored in a register.

The setup is similar to radeon, in fact Adreno 2xx thru 4xx used
basically the same instruction set as r600.  There is a "PFP"
(Prefetch Parser) and "ME" (Micro Engine, also confusingly referred
to as "PM4").  These make up the "CP" ("Command Parser").  The
PFP runs ahead of the ME, with some PM4 packets handled entirely
in the PFP.  Between the PFP and ME is a FIFO ("MEQ").  In the
generations prior to Adreno 5xx, the PFP and ME had different
instruction sets.

Starting with Adreno 5xx, a new microcontroller with a unified
instruction set was introduced, although the overall architecture
and purpose of the two microcontrollers remains the same.

For lack of a better name, this new instruction set is called
"Adreno Five MicroCode" or "afuc".  (No idea what Qualcomm calls
it internally).

With Adreno 6xx, the separate PFP and ME are replaced with a single
SQE microcontroller using the same instruction set as 5xx.

Starting with Adreno 660, another processor called LPAC (Low Priority
Asynchronous Compute) is introduced which is a slightly cut-down copy of the
SQE used to execute background compute tasks. Unlike on 5xx, the firmware is
bundled together with the main SQE firmware, and the SQE is responsible for
booting LPAC. On 7xx, to implement concurrent binning the SQE is split into two
processors called BR and BV. Again, the firmware for all three is bundled
together and BR is responsible for booting both BV and LPAC.

.. _afuc-overview:

Instruction Set Overview
========================

The afuc instruction set is heavily inspired by MIPS, but not exactly
compatible.

Registers
=========

Similar to MIPS, there are 32 registers, and some are special purpose. ``$00``
is the same as ``$zero`` on MIPS, it reads 0 and writes are discarded.

Registers are displayed in the current disassembly with a hexadecimal
numbering, e.g. ``$0a`` is encoded as 10.

The ABI used when processing packets is that ``$01`` contains the current PM4
header, registers from ``$02`` up to ``$11`` are temporaries and may be freely
clobbered by the packet handler, while ``$12`` and above are used to store
global state like the IB level and next visible draw (for draw skipping).

Unlike in MIPS, there is a special small hardware-managed stack and special
instructions ``call``/``ret`` which use it. The stack only contains return
addresses, there is no "stack frame" to spill values to. As a result, ``$sp``,
``$fp``, and ``$ra`` don't exist as on MIPS. Instead the last 3 registers are
used to :ref:`afuc-read<read>` from various queues and
:ref:`afuc-reg-writes<write GPU registers>`. In addition there is a ``$rem``
register which normally contains the number of words remaining in the packet
but can also be used as a normal register in combination with the rep prefix.

.. _afuc-alu:

ALU Instructions
================

The following instructions are available:

- ``add``   - add
- ``addhi`` - add + carry (for upper 32b of 64b value)
- ``sub``   - subtract
- ``subhi`` - subtract + carry (for upper 32b of 64b value)
- ``and``   - bitwise AND
- ``or``    - bitwise OR
- ``xor``   - bitwise XOR
- ``not``   - bitwise NOT (no src1)
- ``shl``   - shift-left
- ``ushr``  - unsigned shift-right
- ``ishr``  - signed shift-right
- ``rot``   - rotate-left (like shift-left with wrap-around)
- ``mul8``  - multiply low 8b of two src
- ``min``   - minimum
- ``max``   - maximum
- ``cmp``  - compare two values

Similar to MIPS, The ALU instructions can take either two src registers, or a
src plus 16b immediate as 2nd src, ex::

  add $dst, $src, 0x1234   ; src2 is immed
  add $dst, $src1, $src2   ; src2 is reg

The ``not`` instruction only takes a single source::

  not $dst, $src
  not $dst, 0x1234

One departure from MIPS is that there is a special immediate-form ``mov``
instruction that can shift the 16-bit immediate by a given amount::

   mov $dst, 0x1234 << 2

This replaces ``lui`` on MIPS (just use a shift of 16) while also allowing the
quick construction of small bitfields, which comes in handy in various places.

.. _afuc-alu-cmp:

The ``cmp`` instruction returns:

- ``0x00`` if src1 > src2
- ``0x2b`` if src1 == src2
- ``0x1e`` if src1 < src2

See explanation in :ref:`afuc-branch`


.. _afuc-branch:

Branch Instructions
===================

The following branch/jump instructions are available:

- ``brne`` - branch if not equal (or bit not set)
- ``breq`` - branch if equal (or bit set)
- ``jump`` - unconditional jump

Both ``brne`` and ``breq`` have two forms, comparing the src register
against either a small immediate (up to 5 bits) or a specific bit::

  breq $src, b3, #somelabel  ; branch if src & (1 << 3)
  breq $src, 0x3, #somelabel ; branch if src == 3

The branch instructions are encoded with a 16b relative offset.
Since ``$00`` always reads back zero, it can be used to construct
an unconditional relative jump.

The :ref:`cmp <afuc-alu-cmp>` instruction can be paired with the
bit-test variants of ``brne``/``breq`` to implement gt/ge/lt/le,
due to the bit pattern it returns, for example::

  cmp $04, $02, $03
  breq $04, b1, #somelabel

will branch if ``$02`` is less than or equal to ``$03``.

Delay slots
-----------

Branch instructions have a delay slot so the following instruction is always
executed regardless of whether branch is taken or not. Unlike MIPS, a branch in
the delay slot is legal as long as the original branch and the branch in its
delay slot are never both taken. Because jump tables are awkward and slow due
to the lack of memory caching, this is often exploited to create dense
sequences of branches to implement switch-case constructs::

   breq $02, 0x1, #foo
   breq $02, 0x2, #bar
   breq $02, 0x3, #baz
   ...
   nop
   jump #default

Another common use of a branch in a delay slot is a double-jump (jump to one
location if a condition is true, and another location if false). In MIPS this
requires two delay slots::

   beq $t0, 0x1, #foo
   nop ; beq delay slot
   b #bar
   nop ; b delay slot

In afuc this only requires a delay slot for the second branch::

   breq $02, 0x1, #foo
   brne $02, 0x1, #bar
   nop

Note that for the second branch we had to use a conditional branch with the
opposite condition instead of an unconditional branch as in the MIPS example,
to guarantee that at most one is ever taken.

.. _afuc-call:

Call/Return
===========

Simple subroutines can be implemented with ``call``/``ret``.  The
jump instruction encodes a fixed offset from the SQE instruction base.

  TODO not sure how many levels deep function calls can be nested.
  There isn't really a stack.  Definitely seems to be multiple
  levels of fxn call, see in PFP: CP_CONTEXT_SWITCH_YIELD -> f13 ->
  f22.

.. _afuc-nop:

NOPs
====

Afuc has a special NOP encoding where the low 24 bits are ignored by the
processor. On a5xx the high 8 bits are ``00``, on a6xx they are ``01``
(probably to make sure that 0 is not a legal instruction, increasing the
chances of halting immediately when something is misconfigured). This is used
sometimes to create a "payload" that is ignored when executed. For example, the
first 2 instructions of the firmware typically contain the firmware ID and
version followed by the packet handling table offset encoded as NOPs. They are
skipped when executed but they are later read as data by the bootstrap routine.

.. _afuc-control:

Control Registers
=================

Control registers are a special register space that can only be read/written
directly by CP through ``cread``/``cwrite`` instructions::

- ``cread $dst, [$off + addr], flags``
- ``cread $dst, [$off + addr]!, flags``
- ``cwrite $src, [$off + addr], flags``
- ``cwrite $src, [$off + addr]!, flags``

Control registers ``0x000`` to ``0x0ff`` are private registers used to control
the CP, for example to indicate where to read from memory or (normal)
registers.  ``0x100`` to ``0x17f`` are a private scratch space used by the
firmware however it wants, for example as an ad-hoc stack to spill registers
when calling a function or to store the scratch used in ``CP_SCRATCH_TO_*``
packets. Starting with the introduction of LPAC, ``0x200`` to ``0x27f`` are a
shared scratch space used to communicate between processors and on a7xx they
can also be written on event completion to implement so-called "on-chip
timestamps".

In cases where no offset is needed, ``$00`` is frequently used as the offset.

The addressing mode with ``!`` is a pre-increment mode that writes the final
address ``$off + addr`` to ``$off``.

For example, the following sequences sets::

  ; load CP_INDIRECT_BUFFER parameters from cmdstream:
  mov $02, $data   ; low 32b of IB target address
  mov $03, $data   ; high 32b of IB target
  mov $04, $data   ; IB size in dwords

  ; sanity check # of dwords:
  breq $04, 0x0, #l23

  ; this seems something to do with figuring out whether
  ; we are going from RB->IB1 or IB1->IB2 (ie. so the
  ; below cwrite instructions update either
  ; CP_IB1_BASE_LO/HI/BUFSIZE or CP_IB2_BASE_LO/HI/BUFSIZE
  and $05, $18, 0x0003
  shl $05, $05, 0x0002

  ; update CP_IBn_BASE_LO/HI/BUFSIZE:
  cwrite $02, [$05 + 0x0b0], 0x8
  cwrite $03, [$05 + 0x0b1], 0x8
  cwrite $04, [$05 + 0x0b2], 0x8

Unlike normal GPU registers, writing control registers seems to always take
effect immediately; if writing a control register triggers some complex
operation that the firmware needs to wait for, then it typically uses a
spinloop with another control register to wait for it to finish.

Control registers are documented in ``adreno_control_regs.xml``. The
disassembler will try to recognize an immediate address as a known control
register and print it, for example this sequence similar to the above sequence
but on a6xx::

  and $05, $12, 0x0003
  shl $05, $05, 0x0002
  cwrite $0e, [$05 + @IB1_BASE], 0x0
  cwrite $0b, [$05 + @IB1_BASE+0x1], 0x0
  cwrite $04, [$05 + @IB1_DWORDS], 0x0

.. _afuc-sqe-regs:

SQE Registers
=============

Starting with a6xx, the state of the SQE processor itself can be accessed
through ``sread``/``swrite`` instructions that work identically to
``cread``/``cwrite``. For example, this includes the state of the
``call``/``ret`` stack. This is mainly used during the preemption routine but
it's also used to set the entrypoint for preemption.

.. _afuc-read:

Reading Memory and Registers
============================

The CP accesses memory directly with no caching. This means that except for
very small amounts of data accessed rarely, ``load`` and ``store`` are very
slow. Instead, ME/PFP and later SQE read memory through various queues. Reading
registers also use a queue, likely because burst reading several registers at
once is faster than reading them one-by-one and reading does not complete
immediately. Queueing up a read involves writing a (address, length) pair to a
control register, and data is read from the queue using one of three special
registers:

- ``$data`` reads the next PM4 packet word. This comes from the RB, IB1, IB2,
  or SDS (Set Draw State) queue, controlled by ``@IB_LEVEL``. It also
  decrements ``$rem`` if it isn't already decremented by a rep prefix.
- ``$memdata`` reads the next word from a memory read buffer (MRB) setup by
  writing ``@MEM_READ_ADDR``/``@MEM_READ_DWORDS``. It's used by things like
  ``CP_MEMCPY`` and reading indirect draw parameters in ``CP_DRAW_INDIRECT``.
- ``$regdata`` reads from a register read buffer (RRB) setup by
  ``@REG_READ_ADDR``/``@REG_READ_DWORDS``.

RB, IB1, IB2, SDS, and MRB make up the Read-Only Queue or ROQ, in addition to
the Visibility Stream Decoder (VSD) which is setup via a similar control
register pair but is read by a fixed-function parser that the CP accesses via a
few control registers.

.. _afuc-reg-writes:

Writing Registers
=================

The same special registers, when used as a destination, can be used to
write GPU registers on ME. Because they have a totally different function when
used as a destination, they use different names:

- ``$addr`` sets the address and disables ``CP_PROTECT`` address checking.
- ``$usraddr`` sets the address and checks it against the ``CP_PROTECT`` access
  table. It's used for addresses specified by the PM4 packet stream instead of
  internally.
- ``$data`` writes the register and auto-increments the address.

for example, to write::

  mov $addr, CP_SCRATCH_REG[0x2] ; set register to write
  mov $data, $03                 ; CP_SCRATCH_REG[0x2]
  mov $data, $04                 ; CP_SCRATCH_REG[0x3]
  ...

subsequent writes to ``$data`` will increment the address of the register
to write, so a sequence of consecutive registers can be written. On a5xx ME,
this will directly write the register, on a6xx SQE this will instead determine
which cluster(s) the register belongs to and push the write onto the
appropriate per-cluster queue(s) letting the SQE run ahead of the GPU.

When bit 18 of ``$addr`` is set, the auto-incrementing is disabled. This is
often used with :ref:`afuc-mem-writes <NRT_DATA>`.

On a5xx ME, ``$regdata`` can also be used to directly read a register::

  mov $addr, CP_SCRATCH_REG[0x2]
  mov $03, $regdata
  mov $04, $regdata

This does not exist on a6xx because register reads are not synchronized against
writes any more.

Many registers that are updated frequently have two banks, so they can be
updated without stalling for previous draw to finish.  On a5xx, these banks are
arranged so bit 11 is zero for bank 0 and 1 for bank 1.  The ME fw (at
least the version I'm looking at) stores this in ``$17``, so to update these
registers from ME::

  or $addr, $17, VFD_INDEX_OFFSET
  mov $data, $03
  ...

On a6xx this is handled transparently to the SQE, and the bank to use is stored
separately in the cluster queue.

Registers can also be written directly, skipping the queue, by writing
``@REG_WRITE_ADDR``/``@REG_WRITE``. This is used on a6xx for certain frontend
registers that have their own queues and on a5xx is used by the PFP::

  mov $0c, CP_SCRATCH_REG[0x7]
  mov $02, 0x789a   ; value
  cwrite $0c, [$00 + @REG_WRITE_ADDR], 0x8
  cwrite $02, [$00 + @REG_WRITE], 0x8

Like with the ``$addr``/``$data`` approach, the destination register address
increments on each write to ``@REG_WRITE``.

.. _afuc-pipe-regs:

Pipe Registers
--------------

This yet another private register space, triggered by writing to the high 8
bits of ``$addr`` and then writing ``$data`` normally. Some pipe registers like
``WAIT_MEM_WRITES`` or ``WAIT_GPU_IDLE`` have no data and a write is triggered
immediately when ``$addr`` is written, for example in ``CP_WAIT_MEM_WRITES``::

  mov $addr, 0x0084 << 24 ; |WAIT_MEM_WRITES

The pipe register is decoded here by the disassembler in a comment.

The main difference of pipe registers from control registers are:

- They are always write-only.
- On a6xx they are pipelined together with normal register writes, on a5xx they
  are written from ME like normal registers.
- Writing them can take an arbitrary amount of time, so they can be used to
  wait for some condition without spinning.

In short, they behave more like normal registers but are not expected to be
read/written by anything other than CP. Over time more and more GPU registers
not touched by the kernel driver have been converted to pipe registers.

.. _afuc-mem-writes:

Writing Memory
==============

Writing memory is done by writing GPU registers:

- ``CP_ME_NRT_ADDR_LO``/``_HI`` - write to set the address to read or write
- ``CP_ME_NRT_DATA`` - write to trigger write to address in ``CP_ME_NRT_ADDR``.

The address register increments with successive writes.

On a5xx, this seems to be only used by ME.  If PFP were also using it, they would
race with each other.  It can also be used for reads, primarily small reads.

Memory Write example::

  ; store 64b value in $04+$05 to 64b address in $02+$03
  mov $addr, CP_ME_NRT_ADDR_LO
  mov $data, $02
  mov $data, $03
  mov $addr, CP_ME_NRT_DATA
  mov $data, $04
  mov $data, $05

Memory Read example::

  ; load 64b value from address in $02+$03 into $04+$05
  mov $addr, CP_ME_NRT_ADDR_LO
  mov $data, $02
  mov $data, $03
  mov $04, $addr
  mov $05, $addr

On a6xx ``CP_ME_NRT_ADDR`` and ``CP_ME_NRT_DATA`` have been replaced by
:ref:`afuc-pipe-regs <pipe registers>` and they can only be used for writes but
it otherwise works similarly.

Load and Store Instructions
===========================

a6xx adds ``load`` and ``store`` instruction that work similarly to ``cread``
and ``cwrite``. Because the address is 64-bits but registers are 32-bit, the
high 32 bits come from the ``@LOAD_STORE_HI``
:ref:`afuc-control <control register>`. They are mostly used by the context
switch routine and even then very sparingly, before the memory read/write queue
state is saved while it is being restored.

Modifiers
=========

There are two modifiers that enable more compact and efficient implementations
of common patterns:

.. _afuc-rep:

Repeat
------

``(rep)`` repeats the same instruction ``$rem`` times. More precisely, it
decrements ``$rem`` after the instruction executes if it wasn't already
decremented from a read from ``$data`` and re-executes the instruction until
``$rem`` is 0.  It can be used with ALU instructions and control instructions.
Usually it is used in conjunction with ``$data`` to read the rest of the packet
in one instruction, but it can also be used freestanding, for example this
snippet clears the control register scratch space::

  mov $rem, 0x0080 ; clear 0x80 registers
  mov $03, 0x00ff ; start at 0xff + 1 = 0x100
  (rep)cwrite $00, [$03 + 0x001], 0x4

Note the use of pre-increment mode, so that the first execution clears
``0x100`` and updates ``$03`` to ``0x100``, the second execution clears
``0x101`` and updates ``$03`` to ``0x101``, and so on.

.. _afuc-xmov:

eXtra Moves
-----------

``(xmovN)`` is an optimization which lets the firmware read multiple words from
a queue in the same cycle. Conceptually, it adds "extra" mov instructions to be
executed after a given ALU instruction, although in practice they are likely
executed in parallel. ``(xmov1)`` adds up to 1 move, ``(xmov2)`` adds up to 2,
and ``(xmov3)`` adds up to 3. The actual number of moves added is the minimum
of the number in the instruction and ``$rem``, so a ``(xmov3)`` instruction
behaves like a ``(xmov1)`` instruction if ``$rem = 1``. Given an instruction::

  (xmovN) alu $dst, $src1, $src2

or a 1-source instruction::

  (xmovN) alu $dst, $src2

then we compute the number of extra moves ``M = min(N, $rem)``. If ``M = 1``,
then we add::

  mov $data, $src2

If ``M = 2``, then we add::

  mov $data, $src2
  mov $data, $src2

Finally, as a special case explained below, if ``M = 3`` then we add::

  mov $data, $src2
  mov $dst, $src2 ; !!!
  mov $data, $src2

If ``$dst`` is not one of the "special" registers ``$data``, ``$addr``,
``$usraddr``, then ``$data`` is replaced by ``$00`` in all destinations, i.e.
the results of the subsequent moves are discarded.

The purpose of the ``M = 3`` special case is mostly to efficiently implement
``CP_CONTEXT_REG_BUNCH``. This is the entire implementation of
``CP_CONTEXT_REG_BUNCH``, which is essentially just one instruction::

  CP_CONTEXT_REG_BUNCH:
  (rep)(xmov3)mov $usraddr, $data
  waitin
  mov $01, $data

If there are 4 or more words remaining in the packet, that is if there are at
least two more registers to write, then (ignoring the ``(rep)`` for a moment)
the instruction expands to::

  mov $usraddr, $data
  mov $data, $data
  mov $usraddr, $data
  mov $data, $data

This is likely all executed in a single cycle, allowing us to write 2 registers
per cycle.

``(xmov1)`` can be also added to ``(rep)mov $data, $data``, which is a common
pattern to write the rest of the packet to successive registers, to write up to
2 registers per cycle as well. The firmware does not use ``(xmov3)``, however,
so 2 registers per cycle is likely a hardware limitation.

Although ``(xmovN)`` is often used in combination with ``(rep)``, it doesn't
have to be. For example, ``(xmov1)mov $data, $data`` moves the next 2 packet
words to 2 successive registers.

.. _afuc-sds:

Set Draw State
--------------

``(sdsN)`` is a modifier for ``cwrite`` used to accelerate
``CP_SET_DRAW_STATE``. For each draw state group to update,
``CP_SET_DRAW_STATE`` needs to copy 3 words from the packet containing the
group to update, metadata, and base address plus size.  Using the ``(sds2)``
modifier as well as ``(rep)``, this can be accomplished in a single
instruction::

  (rep)(sds2)cwrite $data, [$00 + @DRAW_STATE_SET_HDR]

The first word containing the header is written to ``@DRAW_STATE_SET_HDR``, and
the second and third words containing the draw state base come from reading the
source again twice and are written directly to the draw state RAM.

In testing with other control registers, ``(sdsN)`` causes the source to be
read ``N`` extra times and then thrown away. Only when used in combination with
``@DRAW_STATE_SET_HDR`` do the extra source reads have an effect.

Packet Table
============

The core of the microprocessor's job is to parse each packet header and jump to
its handler. This is done through a ``waitin`` instruction which waits for the
packet header to become available and then parses the header and jumps to the
handler using a jump table. However it does *not* actually consume the header.
Like any branch instruction, it has a delay slot, and by convention this delay
slot always contains a ``mov $01, $data`` instruction. This consumes the same
header that ``waitin`` parsed and puts it in ``$01`` so that the packet header
is available in ``$01`` in the next packet. Thus all packet handlers end with
this sequence::

  waitin
  mov $01, $data

The jump table itself is initialized by the SQE in the bootstrap routine at the
beginning of the firmware. Amongst other tasks, it reads the offset of the jump
table from the NOP payload at the beginning, then uses a jump table embedded at
the end of the firmware to set it up by writing the ``@PACKET_TABLE_WRITE``
control register.  After everything is setup, it does the ``waitin`` sequence
to start handling the first packet (which should be ``CP_ME_INIT``).

Example Packet
==============

Let's examine an implementation of ``CP_MEM_WRITE``::

  CP_MEM_WRITE:
  mov $addr, 0x00a0 << 24 ; |NRT_ADDR

First, we setup the register to write to, which is the ``NRT_ADDR``
:ref:`afuc-pipe-regs <pipe register>`. It turns out that the low 2 bits of
``NRT_ADDR`` are a flag which when 1 disables auto-incrementing ``NRT_ADDR``
when ``NRT_DATA`` is written, but we don't want this behavior so we have to
make sure they are clear::

  or $02, $data, 0x0003 ; reading $data reads the next PM4 word
  xor $data, $02, 0x0003 ; writing $data writes the register, which is NRT_ADDR

Writing ``$data`` auto-increments ``$addr``, so now the next write is to
``0xa1`` or ``NRT_ADDR+1`` (``NRT_ADDR`` is a 64-bit register)::

  mov $data, $data

Now, we have to write ``NRT_DATA``. We want to repeatedly write the same
register, without having to fight the auto-increment by resetting ``$addr``
each time, which is where the bit 18 that disables auto-increment comes in
handy::

  mov $addr, 0xa204 << 16 ; |NRT_DATA

Finally, we have to repeatedly copy the remaining PM4 packet data to the
``NRT_DATA`` register, which we can do in one instruction with
:ref:`afuc-rep <(rep)>`. Furthermore we can use :ref:`afuc-xmov <(xmov1)>` to
squeeze out some more performance::

  (rep)(xmov1)mov $data, $data

At the end is the standard go-to-next-packet sequence::

  waitin
  mov $01, $data

A6XX NOTES
==========

The ``$14`` register holds global flags set by:

  CP_SKIP_IB2_ENABLE_LOCAL - b8
  CP_SKIP_IB2_ENABLE_GLOBAL - b9
  CP_SET_MARKER
    MODE=GMEM - sets b15
    MODE=BLIT2D - clears b15, b12, b7
  CP_SET_MODE - b29+b30
  CP_SET_VISIBILITY_OVERRIDE - b11, b21, b30?
  CP_SET_DRAW_STATE - checks b29+b30

  CP_COND_REG_EXEC - checks b10, which should be predicate flag?
