/*...*/ # /*...*/ version 300
 /*...*/#/*...*/  extension whatever
 /*..*/ # /*..*/  pragma ignored
/**/    #    /**/ line 4
 /*...*/# /*...*/ ifdef NOT_DEFINED
 /*...*/# /*...*/ else
 /*..*/ #/*..*/   endif
 /*...*/# /*...*/ ifndef ALSO_NOT_DEFINED
 /*...*/# /*...*/ else
 /*..*/ #/*..*/   endif
/*...*/ # /*...*/ if 0
 /*...*/#/*...*/  elif 1
 /*..*/ # /*..*/  else
 /**/   # /**/    endif
 /*...*/# /*...*/ define FOO bar
 /*..*/ #/*..*/   define FUNC() baz
 /*..*/ # /*..*/  define FUNC2(a,b) b a
FOO
FUNC()
FUNC2(x,y)


