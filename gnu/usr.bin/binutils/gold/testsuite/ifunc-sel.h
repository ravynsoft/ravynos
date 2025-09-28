/* Used by the elf ifunc tests.  */
#ifndef ELF_IFUNC_SEL_H
#define ELF_IFUNC_SEL_H 1

extern int global;

static inline __attribute__ ((always_inline)) void *
ifunc_sel (int (*f1) (void), int (*f2) (void), int (*f3) (void))
{
#ifdef __powerpc__
  /* When generating PIC, powerpc gcc loads the address of "global"
     from the GOT, but the GOT may not have been relocated.
     Similarly, "f1", "f2" and "f3" may be loaded from non-relocated
     GOT entries.

     There is NO WAY to make this ill conceived IFUNC misfeature
     reliably work on targets that use a GOT for function or variable
     addresses, short of implementing two passes over relocations in
     ld.so, with ifunc relocations being applied after all other
     relocations, globally.

     Cheat.  Don't use the GOT.  Rely on this function being inlined
     and calculate all variable and function addresses relative to pc.
     Using the 'X' constraint is risky, but that's the only way to
     make the asm here see the function names for %4, %5 and %6.
     Sadly, powerpc64 gcc doesn't accept use of %3 here with 'X' for
     some reason, so we expand it ourselves.  */
  register void *ret __asm__ ("r3");
  void *temp1, *temp2;
  __asm__ ("mflr %1\n\t"
	   "bcl 20,31,1f\n"
	   "1:\tmflr %2\n\t"
	   "mtlr %1\n\t"
	   "addis %1,%2,global-1b@ha\n\t"
	   "lwz %1,global-1b@l(%1)\n\t"
	   "addis %0,%2,%4-1b@ha\n\t"
	   "addi %0,%0,%4-1b@l\n\t"
	   "cmpwi %1,1\n\t"
	   "beqlr\n\t"
	   "addis %0,%2,%5-1b@ha\n\t"
	   "addi %0,%0,%5-1b@l\n\t"
	   "cmpwi %1,-1\n\t"
	   "beqlr\n\t"
	   "addis %0,%2,%6-1b@ha\n\t"
	   "addi %0,%0,%6-1b@l"
	   : "=&b" (ret), "=&b" (temp1), "=&b" (temp2)
	   : "X" (&global), "X" (f1), "X" (f2), "X" (f3));
  return ret;
#else
  switch (global)
    {
    case 1:
      return f1;
    case -1:
      return f2;
    default:
      return f3;
    }
#endif
}

static inline __attribute__ ((always_inline)) void *
ifunc_one (int (*f1) (void))
{
#ifdef __powerpc__
  /* As above, PIC may use an unrelocated GOT entry for f1.

     Case study: ifuncmain6pie's shared library, ifuncmod6.so, wants
     the address of "foo" in function get_foo().  So there is a GOT
     entry for "foo" in ifuncmod6.so.  ld.so relocates ifuncmod6.so
     *before* ifuncmain6pie, and on finding "foo" to be STT_GNU_IFUNC,
     calls this function with f1 set to "one".  But the address of
     "one" is loaded from ifuncmain6pie's GOT, which hasn't been
     relocated yet.

     Cheat as for ifunc-sel.  */
  register void *ret __asm__ ("r3");
  void *temp;
  __asm__ ("mflr %1\n\t"
	   "bcl 20,31,1f\n"
	   "1:\tmflr %0\n\t"
	   "mtlr %1\n\t"
	   "addis %0,%0,%2-1b@ha\n\t"
	   "addi %0,%0,%2-1b@l"
	   : "=&b" (ret), "=&r" (temp)
	   : "X" (f1));
  return ret;
#else
  return f1;
#endif
}
#endif
