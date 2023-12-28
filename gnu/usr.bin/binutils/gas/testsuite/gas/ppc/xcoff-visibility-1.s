# Tests every possible visibility using XCOFF format.
# Ensure that the visibility field is left empty if no
# visibility is provided.
# Ensure that only the last visibility is taken into
# account when several are provided.

# Csect visibility
  .globl globl_novisibility[RW]
  .csect globl_novisibility[RW]
  .globl globl_internal[RW], internal
  .csect globl_internal[RW]
  .globl globl_hidden[RW], hidden
  .csect globl_hidden[RW]
  .globl globl_protected[RW], protected
  .csect globl_protected[RW]
  .globl globl_exported[RW], exported
  .csect globl_exported[RW]
  .globl globl_dual[RW], exported
  .globl globl_dual[RW], internal
  .csect globl_dual[RW]

# Weak csect visibility
  .weak weak_novisibility[RW]
  .csect weak_novisibility[RW]
  .weak weak_internal[RW], internal
  .csect weak_internal[RW]
  .weak weak_hidden[RW], hidden
  .csect weak_hidden[RW]
  .weak weak_protected[RW], protected
  .csect weak_protected[RW]
  .weak weak_exported[RW], exported
  .csect weak_exported[RW]
  .weak weak_dual[RW], exported
  .weak weak_dual[RW], internal
  .csect weak_dual[RW]

# Comm visibility
  .comm comm_novisibility[RW], 8, 4
  .comm comm_internal[RW], 8, 4, internal
  .comm comm_hidden[RW], 8, 4, hidden
  .comm comm_protected[RW], 8, 4, protected
  .comm comm_exported[RW], 8, 4, exported

# Extern visibility
  .extern extern_novisibility[RW]
  .extern extern_internal[RW], internal
  .extern extern_hidden[RW], hidden
  .extern extern_protected[RW], protected
  .extern extern_exported[RW], exported
  .extern extern_dual[RW], exported
  .extern extern_dual[RW], internal

# Label visibility
  .csect .text[PR]
  .globl l_novisibility
l_novisibility:
  blr

  .globl l_internal, internal
l_internal:
  blr

  .globl l_hidden, hidden
l_hidden:
  blr

  .globl l_protected, protected
l_protected:
  blr

  .globl l_exported, exported
l_exported:
  blr

  .globl l_dual, exported
  .globl l_dual, internal
l_dual:
  blr
