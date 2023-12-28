.text
.align 2

.local ifunc
.type ifunc, @gnu_indirect_function
.set ifunc, resolver

resolver:
  la.local $a0, impl
  jr $ra

impl:
  li.w $a0, 42
  jr $ra

.global test
.type test, @function
test:
  move $s0, $ra
  bl ifunc
  xori $a0, $a0, 42
  jr $s0

.data
.global ptr
.type ptr, @object
ptr:
  .dword test
