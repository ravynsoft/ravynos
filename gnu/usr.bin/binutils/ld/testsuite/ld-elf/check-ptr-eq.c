extern void abort (void);

/* Since GCC 5 folds symbol address comparison, assuming each symbol has
   different address,  &foo == &bar is always false for GCC 5.  Use
   check_ptr_eq to check if two functions are the same.  */

void 
check_ptr_eq (void (*f1) (void), void (*f2) (void))
{
  if (f1 != f2)
    abort ();
}
