: BEGIN{die "You meant to run regen/embed.pl"} # Stop early if fed to perl.
:
: WARNING:  The meanings of some flags have been changed as of v5.31.0
:
: This file is known to be processed by regen/embed.pl, autodoc.pl,
: makedef.pl, Devel::PPPort, and porting/diag.t.
:
: This file contains entries for various functions and macros defined by perl.
: Each entry includes the name, parameters, and various attributes about it.
: In most functions listed here, the name is a short name, and the function's
: real name is the short one, prefixed by either 'Perl_' (for publicly visible
: functions) or 'S_' (for internal-to-a-file static ones).  In many instances a
: macro is defined that is the name in this file, and which expands to call the
: real (full) name, with any appropriate thread context paramaters, thus hiding
: that detail from the typical code.
:
: Most macros (as opposed to function) listed here are the complete full name.
:
: All non-static functions defined by perl need to be listed in this file.
: embed.pl uses the entries here to construct:
:   1) proto.h to declare to the compiler the function interfaces; and
:   2) embed.h to create short name macros
:
: Static functions internal to a file need not appear here, but there is
: benefit to declaring them here:
:   1)	It generally handles the thread context parameter invisibly making it
:	trivial to add or remove needing thread context passed;
:   2)  It defines a PERL_ARGS_ASSERT_foo macro, which can save you debugging
:	time;
:   3)  It is is automatically known to Devel::PPPort, making it quicker to
:	later find out when it came into existence.  For example
:	    perl ppport.h --api-info=/edit_distance/
:	yields
:		Supported at least since perl-5.23.8, with or without ppport.h.
:
: Lines in this file are of the form:
:    flags|return_type|name|arg1|arg2|...|argN
:
: 'flags' is a string of single letters.  Most of the flags are meaningful only
: to embed.pl; some only to autodoc.pl, and others only to makedef.pl.  The
: comments here mostly don't include how Devel::PPPort or diag.t use them:
:
: A function taking no parameters will have no 'arg' elements.
: A line may be continued onto the next by ending it with a backslash.
: Leading and trailing whitespace will be ignored in each component.
:
: Most entries here have a macro created with the entry name.  This presents
: name space collision potentials which haven't been well thought out, but are
: now documented here.  In practice this has rarely been an issue.  At least,
: with a macro, the XS author can #undef it, unlike a function.
:
: The default without flags is to declare a function for internal perl-core use
: only.  The short name is visible only when the PERL_CORE symbol is defined.
: On some platforms, such as Linux and Darwin, all non-static functions
: are currently externally visible.  Because of this, and also for programs
: that embed perl, most non-static functions should have the 'p' flag to avoid
: namespace clashes.
:
: There are several advantages to using a macro instead of the full Perl_foo or
: S_foo form: it hides the need to know if the called function requires a
: thread context parameter or not, and the code using it is more readable
: because of fewer parameters being visible.  And if there is some bug in it
: that gets fixed in a later release, ppport.h can be changed to automatically
: backport the fixed version to modules.  The only disadvantage khw can think
: of is the namespace pollution one.
:
: Since we don't require a C compiler to support variadic macros (C99), the
: macros can't be generated in such situations.
:
: WARNING: Any macro created in a header file is visible to XS code, unless
: care is taken to wrap it within something like #ifdef PERL_CORE..#endif.
: This has had to be done with things like MAX and MIN, but nearly everything
: else has been created without regard to the namespace pollution problem.
:
: Here's what else you need to know about using this file with regards to name
: space pollution:
:
: The A flag is used to make a function and its short name visible everywhere
:	     on all platforms.  This should be used to make it part of Perl's
:	     API contract with XS developers.  The documentation for these is
:	     usually placed in perlapi.  If no documentation exists, that fact
:	     is also noted in perlapi.
:
: The C flag is used instead for functions and their short names that need to
:            be accessible everywhere, typically because they are called from a
:            publicly available macro or inline function, but they are not for
:            public use by themselves.  The documentation for these is placed
:            in perlintern.  If no documentation exists, that fact is also
:            noted in perlintern.
:
:            These really need the 'p' flag to avoid name space collisions.
:
:	     Some of these have been constructed so that the wrapper macro
:	     names begin with an underscore to lessen the chances of a name
:	     collision.  However, this is contrary to the C standard, and those
:	     should be changed.
:
: The E flag is used instead for a function and its short name that is supposed
:            to be used only in the core, and in extensions compiled with the
:            PERL_EXT symbol defined.  Again, on some platforms, the function
:            will be visible everywhere, so one of the 'p' or 'S' flags is
:            generally needed.  Also note that an XS writer can always cheat
:            and pretend to be an extension by #defining PERL_EXT.
:
: The X flag is similar to the C flag in that the function (whose entry better
:	     have the 'p' flag) is accessible everywhere on all platforms.
:	     However the short name macro that normally gets generated is
:	     suppressed outside the core.  (Except it is also visible in
:	     PERL_EXT extensions if the E flag is also specified.)  This flag
:	     is used for functions that are called from a public macro, the
:	     name of which isn't derived from the function name.  You'll have
:	     to write the macro yourself, and from within it, refer to the
:	     function in its full 'Perl_' form with any necessary thread
:	     context parameter.
:
: Just below is a description of the relevant parts of the automatic
: documentation generation system which heavily involves this file.  Below that
: is a description of all the flags used in this file.
:
: Scattered around the perl source are lines of the form:
:
:   =for apidoc name
:   =for apidoc_item name
:
: followed by pod for that function.  The purpose of these lines and the text
: that immediately follows them is to furnish documentation for functions
: and macros listed here in embed.fnc.  The lines tend to be placed near the
: source for the item they describe.  autodoc.pl is run as part of the standard
: build process to extract this documentation and build perlapi.pod from the
: elements that are in the API (flagged as A in this file), and perlintern.pod
: from the other elements.
:
: 'name' in the apidoc line corresponds to an item listed in this file, so that
: the signature and flags need only be specified once, here, and automatically
: they get placed into the generated pod.
:
: 'apidoc_item' is used for subsidiary entries, which share the same pod as the
: plain apidoc one does.  Thus the documentation for functions which do
: essentially the same thing, but with minor differences can all be placed in
: the same entry.  The apidoc_item lines must all come after the apidoc line
: and before the pod for the entry.
:
: The entries in this file that have corresponding '=for apidoc' entries must
: have the 'd' flag set in this file.
:
: In C files, the apidoc lines are inside comment blocks.  They may also be
: placed in pod files.  In those, the =for causes lines from there until the
: next line beginning with an '=' to not be considered part of that pod.
:
: The 'h' flag is used to hide (suppress) the pod associated with =apidoc lines
: from being placed in the generated perlapi or perlintern.  There are several
: reasons you might want to do this, given in the 'h' flag description below,
: but one is for the case where the =apidoc occurs in a file that contains
: regular pod.  Without that flag, the associated pod will be placed in both
: it, and perlapi or perlintern.  That may be what you want, but it gives you
: the flexibility to choose that, or instead have just a link to the source pod
: inserted in perlapi or perlintern.  This allows single-source browsing for
: someone; they don't have to scan multiple pods trying to find something
: suitable.
:
: There are also lines of this form scattered around the perl
: source:
:
:   =for apidoc_section Section Name
:   =head1 Section Name
:
: These aren't tied to this embed.fnc file, and so are documented in autodoc.pl.
:
: What goes into the documentation of a particular function ends with the next
: line that begins with an '='.  In particular, an '=cut' line ends that
: documentation without introducing something new.
:
: Various macros and other elements aren't listed here in embed.fnc.  They are
: documented in the same manner, but since they don't have this file to get
: information from, the defining lines have the syntax and meaning they do in
: this file, so it can be specified:
:
:   =for apidoc flags|return_type|name|arg1|arg2|...|argN
:   =for apidoc_item flags|return_type|name|arg1|arg2|...|argN
:
: The 'name' in any such line must not be the same as any in this file (i.e.,
: no redundant definitions), and one of the flags on the apidoc lines must be
: 'm' or 'y', indicating it is not a function.
:
: All but the name field of an apidoc_item line are optional, and if empty,
: inherits from the controlling plain apidoc line.   The flags field is
: generally empty, and in fact, the only flags it can have are ones directly
: related to its display.  For example it might have the T flag to indicate no
: thread context parameter is used, whereas the apidoc entry does have a thread
: context.  Here is an example:
:
: =for apidoc    Am|char*      |SvPV       |SV* sv|STRLEN len
: =for apidoc_item |const char*|SvPV_const |SV* sv|STRLEN len
: =for apidoc_item |char*      |SvPV_nolen |SV* sv
:
: Since these are macros, the arguments need not be legal C parameters.  To
: indicate this to downstream software that inspects these lines, there are a
: few conventions.  An example would be:
:
:   =for apidoc Am|void|Newxc|void* ptr|int nitems|type|cast
:
: In this example, a real call of Newxc, 'type' would be specified as something
: like 'int' or 'char', and 'cast' by perhaps 'struct foo'.
:
: The complete list of conventions is:
:  type     the argument names a type
:  cast     the argument names a type which the macro casts to
:  SP       the argument is the stack pointer, SP
:  block    the argument is a C brace-enclosed block
:  number   the argument is a C numeric constant, like 3
:  token    the argument is a generic C preprocessor token, like abc
:  "string" the argument is a literal C double-quoted string; what's important
:	    here are the quotes; for clarity, you can say whatever you want
:	    inside them
:
: Unlike other arguments, none of these is of the form 'int name'.  There is no
: name.
:
: If any argument or return value is not one of the above, and isn't legal C
: language, the entry still can be specified, using the 'u' flag.
:
: 'return_type' in these lines can be empty, unlike in this file:
:
: =for apidoc Amnu||START_EXTERN_C
:
: Devel::PPPort also looks at both this file and the '=for apidoc' lines.  In
: part it is to construct lists of elements that are or are not backported.
:
: makedef.pl uses this file for constructing the export list which lists the
: symbols that should be available on all platforms.
:
: porting/diag.t checks some things for consistency based on this file.
:
: The remainder of these introductory comments detail all the possible flags:
:
:   A  Both long and short names are accessible fully everywhere (usually part
:      of the public API).  If the function is not part of the public API,
:      instead use C, E, or X.
:
:         add entry to the list of symbols available on all platforms
:	    unless e or m are also specified;
:         any doc entry goes in perlapi.pod rather than perlintern.pod.  If
:	    there isn't a doc entry, autodoc.pl lists this in perlapi as
:	    existing and being undocumented; unless x is also specified, in
:	    which case it simply isn't listed.
:         makes the short name defined for everywhere, not just for
:	    PERL_CORE/PERL_EXT
:
:   a  Allocates memory a la malloc/calloc.  Also implies "R".  This flag
:      should only be on a function which returns 'empty' memory which has no
:      other pointers to it, and which does not contain any pointers to other
:      things. So for example realloc() can't be 'a'.
:
:         proto.h: add __attribute__malloc__
:
:   b  Binary backward compatibility.  This is used for functions which are
:      kept only to not have to change legacy applications that call them.  If
:      there are no such legacy applications in a Perl installation for all
:      functions flagged with this, the installation can run Configure with the
:      -Accflags='-DNO_MATHOMS' parameter to not even compile them.
:
:      Sometimes the function has been subsumed by a more general one (say, by
:      adding a flags parameter), and a macro exists with the original short
:      name API, and it calls the new function, bypassing this one, and the
:      original 'Perl_' form is being deprecated.  In this case also specify
:      the 'M' flag.
:
:      Without the M flag, these functions should be deprecated, and it is an
:      error to not also specify the 'D' flag.
:
:      The 'b' functions are normally moved to mathoms.c, but if circumstances
:      dictate otherwise, they can be anywhere, provided the whole function is
:      wrapped with
:	    #ifndef NO_MATHOMS
:	    ...
:	    #endif
:
:      Note that this flag no longer automatically adds a 'Perl_' prefix to the
:      name.  Additionally specify 'p' to do that.
:
:      This flag effectively causes nothing to happen if the perl interpreter
:      is compiled with -DNO_MATHOMS (which causes any functions with this flag
:      to not be compiled); otherwise these happen:
:         add entry to the list of symbols available on all platforms;
:         create PERL_ARGS_ASSERT_foo;
:	  add embed.h entry (unless overridden by the 'M' or 'o' flags)
:
:   C  Intended for core use only.  This indicates to XS writers that they
:      shouldn't be using this function.  Devel::PPPort informs them of this,
:      for example.  Some functions have to be accessible everywhere even if
:      they are not intended for public use.  An example is helper functions
:      that are called from inline ones that are publicly available.
:
:         add entry to the list of symbols available on all platforms
:	    unless e or m are also specified;
:         any doc entry goes in perlintern.pod rather than perlapi.pod.  If
:	    there isn't a doc entry, autodoc.pl lists this in perlintern as
:	    existing and being undocumented
:         makes the short name defined for everywhere, not just for
:	    PERL_CORE/PERL_EXT
:
:   D  Function is deprecated:
:
:         proto.h: add __attribute__deprecated__
:         autodoc.pl adds a note to this effect in the doc entry
:
:   d  Function has documentation (somewhere) in the source:
:
:         enables 'no docs for foo" warning in autodoc.pl if the documentation
:         isn't found.
:
:   E  Visible to extensions included in the Perl core:
:
:         in embed.h, change "#ifdef PERL_CORE"
:         into               "#if defined(PERL_CORE) || defined(PERL_EXT)"
:
:       To be usable from dynamically loaded extensions, either:
:	  1) it must be static to its containing file ("i" or "S" flag); or
:         2) be combined with the "X" flag.
:
:   e  Not exported
:
:         suppress entry in the list of symbols available on all platforms
:
:   f  Function takes a format string. If the function name =~ qr/strftime/
:      then it is assumed to take a strftime-style format string as the 1st
:      arg; otherwise it's assumed to take a printf style format string, not
:      necessarily the 1st arg.  All the arguments following it (including
:      possibly '...') are assumed to be for the format.
:
:         embed.h: any entry in here is suppressed because of varargs
:         proto.h: add __attribute__format__ (or ...null_ok__)
:
:   F  Function has a '...' parameter, but don't assume it is a format.  This
:      is to make sure that new functions with formats can't be added without
:      considering if they are format functions or not.  A reason to use this
:      flag even on a format function is if the format would generate
:	    error: format string argument is not a string type
:
:   G  Suppress empty PERL_ARGS_ASSERT_foo macro.  Normally such a macro is
:      generated for all entries for functions 'foo' in this file.  If there is
:      a pointer argument to 'foo', it needs to be declared in this file as
:      either NN or NULLOK, and the function definition must call its
:      corresponding PERL_ARGS_ASSERT_foo macro (a porting test ensures this)
:      which asserts at runtime (under DEBUGGING builds) that NN arguments are
:      not NULL.  If there aren't NN arguments, use of this macro is optional.
:      Rarely, a function will define its own PERL_ARGS_ASSERT_foo macro, and
:      in those cases, adding this flag to its entry in this file will suppress
:      the normal one.  It is not possible to suppress the generated macro if
:      it isn't optional, that is, if there is at least one NN argument.
:
:         proto.h: PERL_ARGS_ASSERT macro is not defined unless the function
:		   has NN arguments
:
:   h  Hide any documentation that would normally go into perlapi or
:      perlintern.  This is typically used when the documentation is actually
:      in another pod.  If you don't use the 'h', that documentation is
:      displayed in both places; with the flag, it stays in the pod, and a
:      link to that pod is instead placed in perlapi or perlintern.  This
:      allows one to browse perlapi or perlintern and see all the potentially
:      relevant elements.  A good example is perlapio.  It has documentation
:      about PerlIO functions with other text giving context.  There's no point
:      in writing a second entry for perlapi, but it would be good if someone
:      browsing perlapi knew about it.  By adding '=for apidoc' lines in
:      perlapio, the appropriate text could be simply copied into perlapi if
:      deemed appropriate, or just a link added there when the 'h' flag is
:      specified.
:      This flag is useful for symbolic names for flags.  A single =for apidoc
:      line can be added to the pod where the meaning is discussed, and perlapi
:      will list the name, with a link to the pod.  Another use would be if
:      there are a bunch of macros which follow a common paradigm in their
:      naming, so rather than having an entry for each slight variation, there
:      is an overarching one.  This flag is useful for downstream programs,
:      such as Devel::PPPort.
:
:   i  inline static.  This is used for functions that the compiler is being
:      requested to inline.  If the function is in a header file its
:      definition will be visible (unless guarded by #if..#endif) to all
:      XS code.  (A typical guard will be that it is being included in a
:      particular C file(s) or in the perl core.)  Therefore, all
:      non-guarded functions should also have the 'p' flag specified to avoid
:      polluting the XS code name space.  Otherwise an error is generated if
:      the 'S' flag is not also specified.
:
:         proto.h: function is declared as PERL_STATIC_INLINE
:
:   I  This flag works exactly the same as 'i' but it also adds
:      __attribute__((always_inline)) or __forceinline if either of them is
:      supported by the compiler.
:
:         proto.h: function is declared as PERL_STATIC_FORCE_INLINE and
:                  __attribute__always_inline__ is added
:
:   m  Implemented as a macro; there is no function associated with this name,
:      and hence no long Perl_ or S_ name.  However, if the macro name itself
:      begins with 'Perl_', autodoc.pl will show a thread context parameter
:      unless the 'T' flag is specified.
:
:         suppress proto.h entry (actually, not suppressed, but commented out)
:         suppress entry in the list of exported symbols available on all platforms
:         suppress embed.h entry, as the implementation should furnish the macro
:
:   M  The implementation is furnishing its own macro instead of relying on the
:      default short name macro that simply expands to call the real name
:      function.  This is used if the parameters need to be cast from what the
:      caller has, or if there is a macro that bypasses this function (whose
:      long name is being retained for backward compatibility for those who
:      call it with that name).  An example is when a new function is created
:      with an extra parameter and a wrapper macro is added that has the old
:      API, but calls the new one with the exta parameter set to a default.
:
:      This flag requires the 'p' flag to be specified, as there would be no
:      need to do this if the function weren't publicly accessible before.
:
:      The entry is processed based on the other flags, but the:
:         embed.h entry is suppressed
:
:   N  The name in the entry isn't strictly a name
:
:      Normally, the name of the function or macro must contain all \w
:      characters, and a warning is raised otherwise.  This flag suppresses
:      that warning, so that weird things can be documented
:
:   n  Has no arguments.  Perhaps a better name would have been '0'. (used only
:      in =for apidoc entries)
:
:      The macro (it can't be a function) is used without any parameters nor
:      empty parentheses.
:
:      Perhaps a better name for this flag would have been '0'.  The reason the
:      flag was not changed to that from 'n', is if D:P were to be regenerated
:      on an older perl, it still would use the new embed.fnc shipped with it,
:      but would be using the flags from the older perl source code.
:
:
:   O  Has a perl_ compatibility macro.
:
:      The really OLD name for API funcs.
:
:      autodoc.pl adds a note that the perl_ form of this function is
:      deprecated.
:
:   o  Has no Perl_foo or S_foo compatibility macro:
:
:	This is used for whatever reason to force the function to be called
:	with the long name.  Perhaps there is a varargs issue.  Use the 'M'
:	flag instead for wrapper macros, and legacy-only functions should
:	also use 'b'.
:
:         embed.h: suppress "#define foo Perl_foo"
:
:      autodoc.pl adds a note that this function must be explicitly called as
:      Perl_$name, and with an aTHX_ parameter unless the 'T' flag is also
:      specified.

:      mnemonic: 'omit' generated macro
:
:   P  Pure function:
:
:	A pure function has no effects except the return value, and the return
:       value depends only on params and/or globals.  This is a hint to the
:	compiler that it can optimize calls to this function out of common
:	subexpressions.  Consequently if this flag is wrongly specified, it can
:	lead to subtle bugs that vary by platform, compiler, compiler version,
:	and optimization level.  Also, a future commit could easily change a
:	currently-pure function without even noticing this flag.  So it should
:	be used sparingly, only for functions that are unlikely to ever become
:	not pure by future commits.  It should not be used for static
:	functions, as the compiler already has the information needed to make
:	the 'pure' determination and doesn't need any hint; so it doesn't add
:	value in those cases, and could be dangerous if it causes the compiler
:	to skip doing its own checks.  It should not be used on functions that
:	touch SVs, as those can trigger unexpected magic.  Also implies "R":
:
:         proto.h: add __attribute__pure__
:
:   p  Function in source code has a Perl_ prefix:
:
:         proto.h: function is declared as Perl_foo rather than foo
:         embed.h: "#define foo Perl_foo" entries added
:
:   R  Return value must not be ignored (also implied by 'a' and 'P' flags):
:
:	gcc has a bug (which they claim is a feature) in which casting the
:       result of one of these to (void) doesn't silence the warning that the
:	result is ignored.  (Perl has a workaround for this bug, see
:       PERL_UNUSED_RESULT docs)
:
:        proto.h: add __attribute__warn_unused_result__
:
:   r  Function never returns:
:
:        proto.h: add __attribute__noreturn__
:
:   S  Static function: function in source code has a S_ prefix:
:
:         proto.h: function is declared as S_foo rather than foo,
:                STATIC is added to declaration;
:         embed.h: "#define foo S_foo" entries added
:
:   s  autodoc.pl adds a terminating semi-colon to the usage example in the
:      documentation.
:
:   T  Has no implicit interpreter/thread context argument:
:
:         suppress the pTHX part of "foo(pTHX...)" in proto.h;
:         In the PERL_IMPLICIT_SYS branch of embed.h, generates
:             "#define foo Perl_foo",      rather than
:             "#define foo(a,b,c) Perl_foo(aTHX_ a,b,c)
:
:   u  The macro's (it has to be a macro) return value or parameters are
:      unorthodox, and aren't in the list above of recognized weird ones.   For
:      example, they aren't C parameters, or the macro expands to something
:      that isn't a symbol.
:
:      For example, the expansion of STR_WITH_LEN is a comma separated pair of
:      values, so would have this flag; or some macros take preprocessor
:      tokens, so would have this flag.
:
:      This also is used for entries that require processing for use, such as
:      being compiled by xsubpp.  This flag is an indication to downstream
:      tools, such as Devel::PPPort, that this requires special handling.
:
:   U  autodoc.pl will not output a usage example
:
:   W  Add a comma_pDEPTH argument to function prototypes, and a comma_aDEPTH
:      argument to the function calls. This means that under DEBUGGING
:      a depth argument is added to the functions, which is used for
:      example by the regex engine for debugging and trace output.
:      A non DEBUGGING build will not pass the unused argument.
:      Currently restricted to functions with at least one argument.
:
:   X  Explicitly exported:
:
:         add entry to the list of symbols available on all platforms, unless e
:	    or m
:
:      This is often used for private functions that are used by public
:      macros.  In those cases the macros must use the long form of the
:      name (Perl_blah(aTHX_ ...)).
:
:   x  Experimental, may change:
:
:         any doc entry is marked that it may change.  Also used to suppress
:	  making a perlapi doc entry if it would just be a placeholder.
:
:   y  Typedef.  The element names a type rather than being a macro
:
: In this file, pointer parameters that must not be passed NULLs should be
: prefixed with NN.
:
: And, pointer parameters that may be NULL should be prefixed with NULLOK.
: This has no effect on output yet.  It's a notation for the maintainers to
: know "I have defined whether NULL is OK or not" rather than having neither
: NULL or NULLOK, which is ambiguous.
:
: Individual flags may be separated by non-tab whitespace.

CipRTX	|char *	|mortal_getenv	|NN const char * str

#if defined(PERL_IMPLICIT_SYS)
ATo	|PerlInterpreter*|perl_alloc_using \
				|NN struct IPerlMem *ipM \
				|NN struct IPerlMem *ipMS \
				|NN struct IPerlMem *ipMP \
				|NN struct IPerlEnv *ipE \
				|NN struct IPerlStdIO *ipStd \
				|NN struct IPerlLIO *ipLIO \
				|NN struct IPerlDir *ipD \
				|NN struct IPerlSock *ipS \
				|NN struct IPerlProc *ipP
#endif
ATod	|PerlInterpreter*	|perl_alloc
ATod	|void	|perl_construct	|NN PerlInterpreter *my_perl
ATod	|int	|perl_destruct	|NN PerlInterpreter *my_perl
ATod	|void	|perl_free	|NN PerlInterpreter *my_perl
ATod	|int	|perl_run	|NN PerlInterpreter *my_perl
ATod	|int	|perl_parse	|NN PerlInterpreter *my_perl|XSINIT_t xsinit \
				|int argc|NULLOK char** argv|NULLOK char** env
CTpR	|bool	|doing_taint	|int argc|NULLOK char** argv|NULLOK char** env
#if defined(USE_ITHREADS)
ATod	|PerlInterpreter*|perl_clone|NN PerlInterpreter *proto_perl|UV flags
#  if defined(PERL_IMPLICIT_SYS)
ATo	|PerlInterpreter*|perl_clone_using \
				|NN PerlInterpreter *proto_perl \
				|UV flags \
				|NN struct IPerlMem* ipM \
				|NN struct IPerlMem* ipMS \
				|NN struct IPerlMem* ipMP \
				|NN struct IPerlEnv* ipE \
				|NN struct IPerlStdIO* ipStd \
				|NN struct IPerlLIO* ipLIO \
				|NN struct IPerlDir* ipD \
				|NN struct IPerlSock* ipS \
				|NN struct IPerlProc* ipP
#  endif
#endif

AaTophd	|Malloc_t|malloc	|MEM_SIZE nbytes
AaTophd	|Malloc_t|calloc	|MEM_SIZE elements|MEM_SIZE size
ARTophd	|Malloc_t|realloc	|Malloc_t where|MEM_SIZE nbytes
ATop	|Free_t	|mfree		|Malloc_t where
#if defined(MYMALLOC)
TpR	|MEM_SIZE|malloced_size	|NN void *p
TpR	|MEM_SIZE|malloc_good_size	|size_t nbytes
#endif
#if defined(PERL_IN_MALLOC_C)
ST	|int	|adjust_size_and_find_bucket	|NN size_t *nbytes_p
#endif

ATpR	|void*	|get_context
ATp	|void	|set_context	|NN void *t

XEop	|bool	|try_amagic_bin	|int method|int flags
XEop	|bool	|try_amagic_un	|int method|int flags
Ap	|SV*	|amagic_call	|NN SV* left|NN SV* right|int method|int dir
Ap	|SV *	|amagic_deref_call|NN SV *ref|int method
p	|bool	|amagic_is_enabled|int method
Ap	|int	|Gv_AMupdate	|NN HV* stash|bool destructing
ApR	|CV*	|gv_handler	|NULLOK HV* stash|I32 id
Apd	|OP*	|op_append_elem	|I32 optype|NULLOK OP* first|NULLOK OP* last
Apd	|OP*	|op_append_list	|I32 optype|NULLOK OP* first|NULLOK OP* last
Apd	|OP*	|op_linklist	|NN OP *o
Apd	|OP*	|op_prepend_elem|I32 optype|NULLOK OP* first|NULLOK OP* last
: FIXME - this is only called by pp_chown. They should be merged.
p	|I32	|apply		|I32 type|NN SV** mark|NN SV** sp
Apx	|void	|apply_attrs_string|NN const char *stashpv|NN CV *cv|NN const char *attrstr|STRLEN len
Apd	|void	|av_clear	|NN AV *av
Apd	|SV*	|av_delete	|NN AV *av|SSize_t key|I32 flags
ApdR	|bool	|av_exists	|NN AV *av|SSize_t key
Apd	|void	|av_extend	|NN AV *av|SSize_t key
p	|void	|av_extend_guts	|NULLOK AV *av|SSize_t key \
				|NN SSize_t *maxp \
				|NN SV ***allocp|NN SV ***arrayp
ApdR	|SV**	|av_fetch	|NN AV *av|SSize_t key|I32 lval
CipdR	|SV**	|av_fetch_simple|NN AV *av|SSize_t key|I32 lval
Apd	|void	|av_fill	|NN AV *av|SSize_t fill
ApdR	|SSize_t|av_len		|NN AV *av
ApdR	|AV*	|av_make	|SSize_t size|NN SV **strp
ApdR	|AV*	|av_new_alloc	|SSize_t size|bool zeroflag
p	|SV*	|av_nonelem	|NN AV *av|SSize_t ix
Apd	|SV*	|av_pop		|NN AV *av
Apdoe	|void	|av_create_and_push|NN AV **const avp|NN SV *const val
Apd	|void	|av_push	|NN AV *av|NN SV *val
: Used in scope.c, and by Data::Alias
EXp	|void	|av_reify	|NN AV *av
ApdR	|SV*	|av_shift	|NN AV *av
Apd	|SV**	|av_store	|NN AV *av|SSize_t key|NULLOK SV *val
Cipd	|SV**	|av_store_simple|NN AV *av|SSize_t key|NULLOK SV *val
AmdR	|SSize_t|av_top_index	|NN AV *av
AidRp	|Size_t	|av_count	|NN AV *av
AmdR	|SSize_t|av_tindex	|NN AV *av
Apd	|void	|av_undef	|NN AV *av
Apdoe	|SV**	|av_create_and_unshift_one|NN AV **const avp|NN SV *const val
Apd	|void	|av_unshift	|NN AV *av|SSize_t num
Cpo	|SV**	|av_arylen_p	|NN AV *av
Cpo	|IV*	|av_iter_p	|NN AV *av
#if defined(PERL_IN_AV_C)
S	|MAGIC*	|get_aux_mg	|NN AV *av
#endif
: Used in perly.y
pR	|OP*	|bind_match	|I32 type|NN OP *left|NN OP *right
: Used in perly.y
ApdR	|OP*	|block_end	|I32 floor|NULLOK OP* seq
ApR	|U8	|block_gimme
: Used in perly.y
ApdR	|int	|block_start	|int full
Aodxp	|void	|blockhook_register |NN BHK *hk
p	|void	|boot_core_builtin
: Used in perl.c
p	|void	|boot_core_UNIVERSAL
: Used in perl.c
p	|void	|boot_core_PerlIO
Ap	|void	|call_list	|I32 oldscope|NN AV *paramList
Apd	|const PERL_CONTEXT *	|caller_cx|I32 level \
				|NULLOK const PERL_CONTEXT **dbcxp
: Used in several source files
pR	|bool	|cando		|Mode_t mode|bool effective|NN const Stat_t* statbufp
CpRT	|U32	|cast_ulong	|NV f
CpRT	|I32	|cast_i32	|NV f
CpRT	|IV	|cast_iv	|NV f
CpRT	|UV	|cast_uv	|NV f
#if !defined(HAS_TRUNCATE) && !defined(HAS_CHSIZE) && defined(F_FREESP)
ApR	|I32	|my_chsize	|int fd|Off_t length
#endif
p	|const COP*|closest_cop	|NN const COP *cop|NULLOK const OP *o \
				|NULLOK const OP *curop|bool opnext
: Used in perly.y
ApdR	|OP*	|op_convert_list	|I32 optype|I32 flags|NULLOK OP* o
: Used in op.c and perl.c
px	|void	|create_eval_scope|NULLOK OP *retop|U32 flags
Aprd	|void	|croak_sv	|NN SV *baseex
: croak()'s first parm can be NULL.  Otherwise, mod_perl breaks.
Afprd	|void	|croak		|NULLOK const char* pat|...
Aprd	|void	|vcroak		|NULLOK const char* pat|NULLOK va_list* args
ATprd	|void	|croak_no_modify
ATprd	|void	|croak_xs_usage	|NN const CV *const cv \
				|NN const char *const params
Tpr	|void	|croak_no_mem
TprX	|void	|croak_popstack
fTrp	|void	|croak_caller|NULLOK const char* pat|...
fTpre	|void	|noperl_die|NN const char* pat|...
#if defined(WIN32)
Tore	|void	|win32_croak_not_implemented|NN const char * fname
#endif
#if defined(MULTIPLICITY)
AdfTrp	|void	|croak_nocontext|NULLOK const char* pat|...
AdfTrp	|OP*    |die_nocontext  |NULLOK const char* pat|...
AfTp	|void	|deb_nocontext	|NN const char* pat|...
AdfTp	|char*	|form_nocontext	|NN const char* pat|...
AdFTp	|void	|load_module_nocontext|U32 flags|NN SV* name|NULLOK SV* ver|...
AdfTp	|SV*	|mess_nocontext	|NN const char* pat|...
AdfTp	|void	|warn_nocontext	|NN const char* pat|...
AdfTp	|void	|warner_nocontext|U32 err|NN const char* pat|...
AdfTp	|SV*	|newSVpvf_nocontext|NN const char *const pat|...
AdfTp	|void	|sv_catpvf_nocontext|NN SV *const sv|NN const char *const pat|...
AdfTp	|void	|sv_setpvf_nocontext|NN SV *const sv|NN const char *const pat|...
AdfTp	|void	|sv_catpvf_mg_nocontext|NN SV *const sv|NN const char *const pat|...
AdfTp	|void	|sv_setpvf_mg_nocontext|NN SV *const sv|NN const char *const pat|...
AbfTpD	|int	|fprintf_nocontext|NN PerlIO *stream|NN const char *format|...
AbfTpD	|int	|printf_nocontext|NN const char *format|...
#endif
: Used in pp.c
pd	|SV *	|core_prototype	|NULLOK SV *sv|NN const char *name \
				|const int code|NULLOK int * const opnum
: Used in gv.c
p	|OP *	|coresub_op	|NN SV *const coreargssv|const int code \
				|const int opnum
: Used in sv.c
ExXp	|void	|cv_ckproto_len_flags	|NN const CV* cv|NULLOK const GV* gv\
				|NULLOK const char* p|const STRLEN len \
                                |const U32 flags
: Used in pp.c and pp_sys.c
ApdR	|SV*	|gv_const_sv	|NN GV* gv
ApdRT	|SV*	|cv_const_sv	|NULLOK const CV *const cv
pRT	|SV*	|cv_const_sv_or_av|NULLOK const CV *const cv
Apd	|SV *	|cv_name	|NN CV *cv|NULLOK SV *sv|U32 flags
Apd	|void	|cv_undef	|NN CV* cv
p	|void	|cv_undef_flags	|NN CV* cv|U32 flags
pd	|void	|cv_forget_slab	|NULLOK CV *cv
Cp	|void	|cx_dump	|NN PERL_CONTEXT* cx
AiMpd	|GV *	|CvGV		|NN CV *sv
AiMTp	|I32 *	|CvDEPTH	|NN const CV * const sv
Aphd	|SV*	|filter_add	|NULLOK filter_t funcp|NULLOK SV* datasv
Ap	|void	|filter_del	|NN filter_t funcp
ApRhd	|I32	|filter_read	|int idx|NN SV *buf_sv|int maxlen
ApPR	|char**	|get_op_descs
ApPR	|char**	|get_op_names
: FIXME discussion on p5p
pPR	|const char*	|get_no_modify
: FIXME discussion on p5p
pPR	|U32*	|get_opargs
ApPR	|PPADDR_t*|get_ppaddr
: Used by CXINC, which appears to be in widespread use
CpR	|I32	|cxinc
Afp	|void	|deb		|NN const char* pat|...
Ap	|void	|vdeb		|NN const char* pat|NULLOK va_list* args
Ap	|void	|debprofdump
EXp	|SV*	|multideref_stringify	|NN const OP* o|NULLOK CV *cv
EXp	|SV*	|multiconcat_stringify	|NN const OP* o
Ap	|I32	|debop		|NN const OP* o
Ap	|I32	|debstack
Ap	|I32	|debstackptrs
pR	|SV *	|defelem_target	|NN SV *sv|NULLOK MAGIC *mg
ATpd	|char*	|delimcpy|NN char* to|NN const char* to_end		\
			 |NN const char* from|NN const char* from_end	\
			 |const int delim|NN I32* retlen
EXTpd	|char*	|delimcpy_no_escape|NN char* to|NN const char* to_end	\
				   |NN const char* from			\
				   |NN const char* from_end		\
				   |const int delim|NN I32* retlen
: Used in op.c, perl.c
px	|void	|delete_eval_scope
Aprd	|OP*    |die_sv         |NN SV *baseex
Afrpd	|OP*    |die            |NULLOK const char* pat|...
: Used in util.c
pr	|void	|die_unwind	|NN SV* msv
Ap	|void	|dounwind	|I32 cxix
: FIXME
pMb	|bool|do_aexec	|NULLOK SV* really|NN SV** mark|NN SV** sp
: Used in pp_sys.c
p	|bool|do_aexec5	|NULLOK SV* really|NN SV** mark|NN SV** sp|int fd|int do_report
AbpD	|int	|do_binmode	|NN PerlIO *fp|int iotype|int mode
: Used in pp.c
Ap	|bool	|do_close	|NULLOK GV* gv|bool not_implicit
: Defined in doio.c, used only in pp_sys.c
p	|bool	|do_eof		|NN GV* gv

#ifdef PERL_DEFAULT_DO_EXEC3_IMPLEMENTATION
pM	|bool|do_exec	|NN const char* cmd
#else
p	|bool|do_exec	|NN const char* cmd
#endif

#if defined(WIN32) || defined(VMS)
Ap	|int	|do_aspawn	|NULLOK SV* really|NN SV** mark|NN SV** sp
Ap	|int	|do_spawn	|NN char* cmd
Ap	|int	|do_spawn_nowait|NN char* cmd
#endif
#if !defined(WIN32)
p	|bool|do_exec3	|NN const char *incmd|int fd|int do_report
#endif
#if defined(PERL_IN_DOIO_C)
S	|void	|exec_failed	|NN const char *cmd|int fd|int do_report
S	|bool	|argvout_final	|NN MAGIC *mg|NN IO *io|bool not_implicit
#endif
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
: Defined in doio.c, used only in pp_sys.c
p	|I32	|do_ipcctl	|I32 optype|NN SV** mark|NN SV** sp
: Defined in doio.c, used only in pp_sys.c
p	|I32	|do_ipcget	|I32 optype|NN SV** mark|NN SV** sp
: Defined in doio.c, used only in pp_sys.c
p	|I32	|do_msgrcv	|NN SV** mark|NN SV** sp
: Defined in doio.c, used only in pp_sys.c
p	|I32	|do_msgsnd	|NN SV** mark|NN SV** sp
: Defined in doio.c, used only in pp_sys.c
p	|I32	|do_semop	|NN SV** mark|NN SV** sp
: Defined in doio.c, used only in pp_sys.c
p	|I32	|do_shmio	|I32 optype|NN SV** mark|NN SV** sp
#endif
Ap	|void	|do_join	|NN SV *sv|NN SV *delim|NN SV **mark|NN SV **sp
: Used in pp.c and pp_hot.c, prototype generated by regen/opcode.pl
: p	|OP*	|do_kv
: used in pp.c, pp_hot.c
pR	|I32	|do_ncmp	|NN SV *const left|NN SV *const right
ApMb	|bool	|do_open	|NN GV* gv|NN const char* name|I32 len|int as_raw \
				|int rawmode|int rawperm|NULLOK PerlIO* supplied_fp
AbpD	|bool	|do_open9	|NN GV *gv|NN const char *name|I32 len|int as_raw \
				|int rawmode|int rawperm|NULLOK PerlIO *supplied_fp \
				|NN SV *svs|I32 num
pT	|void	|setfd_cloexec|int fd
pT	|void	|setfd_inhexec|int fd
p	|void	|setfd_cloexec_for_nonsysfd|int fd
p	|void	|setfd_inhexec_for_sysfd|int fd
p	|void	|setfd_cloexec_or_inhexec_by_sysfdness|int fd
pR	|int	|PerlLIO_dup_cloexec|int oldfd
p	|int	|PerlLIO_dup2_cloexec|int oldfd|int newfd
pR	|int	|PerlLIO_open_cloexec|NN const char *file|int flag
pR	|int	|PerlLIO_open3_cloexec|NN const char *file|int flag|int perm
pToR	|int	|my_mkstemp_cloexec|NN char *templte
pToR	|int	|my_mkostemp_cloexec|NN char *templte|int flags
#ifdef HAS_PIPE
pR	|int	|PerlProc_pipe_cloexec|NN int *pipefd
#endif
#ifdef HAS_SOCKET
pR	|int	|PerlSock_socket_cloexec|int domain|int type|int protocol
pR	|int	|PerlSock_accept_cloexec|int listenfd \
				|NULLOK struct sockaddr *addr \
				|NULLOK Sock_size_t *addrlen
#endif
#if defined (HAS_SOCKETPAIR) || \
    (defined (HAS_SOCKET) && defined(SOCK_DGRAM) && \
	defined(AF_INET) && defined(PF_INET))
pR	|int	|PerlSock_socketpair_cloexec|int domain|int type|int protocol \
				|NN int *pairfd
#endif
#if defined(PERL_IN_DOIO_C)
S	|IO *	|openn_setup    |NN GV *gv|NN char *mode|NN PerlIO **saveifp \
				|NN PerlIO **saveofp|NN int *savefd \
                                |NN char *savetype
S	|bool	|openn_cleanup	|NN GV *gv|NN IO *io|NULLOK PerlIO *fp \
				|NN char *mode|NN const char *oname \
                                |NULLOK PerlIO *saveifp|NULLOK PerlIO *saveofp \
                                |int savefd|char savetype|int writing \
                                |bool was_fdopen|NULLOK const char *type \
                                |NULLOK Stat_t *statbufp
#endif
Ap	|bool	|do_openn	|NN GV *gv|NN const char *oname|I32 len \
				|int as_raw|int rawmode|int rawperm \
				|NULLOK PerlIO *supplied_fp|NULLOK SV **svp \
				|I32 num
xp	|bool	|do_open_raw	|NN GV *gv|NN const char *oname|STRLEN len \
				|int rawmode|int rawperm|NULLOK Stat_t *statbufp
xp	|bool	|do_open6	|NN GV *gv|NN const char *oname|STRLEN len \
				|NULLOK PerlIO *supplied_fp|NULLOK SV **svp \
				|U32 num
: Used in pp_hot.c and pp_sys.c
p	|bool	|do_print	|NULLOK SV* sv|NN PerlIO* fp
: Used in pp_sys.c
pR	|OP*	|do_readline
: Defined in doio.c, used only in pp_sys.c
p	|bool	|do_seek	|NULLOK GV* gv|Off_t pos|int whence
Ap	|void	|do_sprintf	|NN SV* sv|SSize_t len|NN SV** sarg
: Defined in doio.c, used only in pp_sys.c
p	|Off_t	|do_sysseek	|NN GV* gv|Off_t pos|int whence
: Defined in doio.c, used only in pp_sys.c
pR	|Off_t	|do_tell	|NN GV* gv
: Defined in doop.c, used only in pp.c
p	|Size_t	|do_trans	|NN SV* sv
: Used in my.c and pp.c
p	|UV	|do_vecget	|NN SV* sv|STRLEN offset|int size
: Defined in doop.c, used only in mg.c (with /* XXX slurp this routine */)
p	|void	|do_vecset	|NN SV* sv
: Defined in doop.c, used only in pp.c
p	|void	|do_vop		|I32 optype|NN SV* sv|NN SV* left|NN SV* right
: Used in perly.y
p	|OP*	|dofile		|NN OP* term|I32 force_builtin
ApR	|U8	|dowantarray
Adp	|void	|dump_all
p	|void	|dump_all_perl	|bool justperl
Ap	|void	|dump_eval
Ap	|void	|dump_form	|NN const GV* gv
Ap	|void	|gv_dump	|NULLOK GV* gv
Apd	|OPclass|op_class	|NULLOK const OP *o
Apd	|void	|op_dump	|NN const OP *o
Ap	|void	|pmop_dump	|NULLOK PMOP* pm
Apd	|void	|dump_packsubs	|NN const HV* stash
p	|void	|dump_packsubs_perl	|NN const HV* stash|bool justperl
Ap	|void	|dump_sub	|NN const GV* gv
p	|void	|dump_sub_perl	|NN const GV* gv|bool justperl
Apd	|void	|fbm_compile	|NN SV* sv|U32 flags
ApdR	|char*	|fbm_instr	|NN unsigned char* big|NN unsigned char* bigend \
				|NN SV* littlestr|U32 flags
pEXTR	|const char *|cntrl_to_mnemonic|const U8 c
p	|CV *	|find_lexical_cv|PADOFFSET off
: Defined in util.c, used only in perl.c
p	|char*	|find_script	|NN const char *scriptname|bool dosearch \
				|NULLOK const char *const *const search_ext|I32 flags
#if defined(PERL_IN_OP_C)
S	|OP*	|force_list	|NULLOK OP* arg|bool nullit
i	|OP*	|op_integerize	|NN OP *o
i	|OP*	|op_std_init	|NN OP *o
#if defined(USE_ITHREADS)
i	|void	|op_relocate_sv	|NN SV** svp|NN PADOFFSET* targp
#endif
i	|OP*	|newMETHOP_internal	|I32 type|I32 flags|NULLOK OP* dynamic_meth \
					|NULLOK SV* const_meth
: FIXME
S	|OP*	|fold_constants	|NN OP * const o
Sd	|OP*	|traverse_op_tree|NN OP* top|NN OP* o
#endif
Afpd	|char*	|form		|NN const char* pat|...
Adp	|char*	|vform		|NN const char* pat|NULLOK va_list* args
Cp	|void	|free_tmps
#if defined(PERL_IN_OP_C)
S	|void	|gen_constant_list|NULLOK OP* o
#endif
#if !defined(HAS_GETENV_LEN)
: Used in hv.c
p	|char*	|getenv_len	|NN const char *env_elem|NN unsigned long *len
#endif
: Used in pp_ctl.c and pp_hot.c
poe	|void	|get_db_sub	|NULLOK SV **svp|NN CV *cv
Ap	|void	|gp_free	|NULLOK GV* gv
Ap	|GP*	|gp_ref		|NULLOK GP* gp
Ap	|GV*	|gv_add_by_type	|NULLOK GV *gv|svtype type
ApMb	|GV*	|gv_AVadd	|NULLOK GV *gv
ApMb	|GV*	|gv_HVadd	|NULLOK GV *gv
ApMb	|GV*	|gv_IOadd	|NULLOK GV* gv
AdmR	|GV*	|gv_autoload4	|NULLOK HV* stash|NN const char* name \
				|STRLEN len|I32 method
ApR	|GV*	|gv_autoload_sv	|NULLOK HV* stash|NN SV* namesv|U32 flags
ApR	|GV*	|gv_autoload_pv	|NULLOK HV* stash|NN const char* namepv \
                                |U32 flags
ApR	|GV*	|gv_autoload_pvn	|NULLOK HV* stash|NN const char* name \
                                        |STRLEN len|U32 flags
Cp	|void	|gv_check	|NN HV* stash
AbpD	|void	|gv_efullname	|NN SV* sv|NN const GV* gv
ApMb	|void	|gv_efullname3	|NN SV* sv|NN const GV* gv|NULLOK const char* prefix
Ap	|void	|gv_efullname4	|NN SV* sv|NN const GV* gv|NULLOK const char* prefix|bool keepmain
Adp	|GV*	|gv_fetchfile	|NN const char* name
Adp	|GV*	|gv_fetchfile_flags|NN const char *const name|const STRLEN len\
				|const U32 flags
Amd	|GV*	|gv_fetchmeth	|NULLOK HV* stash|NN const char* name \
				|STRLEN len|I32 level
Apd	|GV*	|gv_fetchmeth_sv	|NULLOK HV* stash|NN SV* namesv|I32 level|U32 flags
Apd	|GV*	|gv_fetchmeth_pv	|NULLOK HV* stash|NN const char* name \
                                        |I32 level|U32 flags
Apd	|GV*	|gv_fetchmeth_pvn	|NULLOK HV* stash|NN const char* name \
                                        |STRLEN len|I32 level|U32 flags
Amd	|GV*	|gv_fetchmeth_autoload	|NULLOK HV* stash \
					|NN const char* name|STRLEN len \
					|I32 level
Apd	|GV*	|gv_fetchmeth_sv_autoload	|NULLOK HV* stash|NN SV* namesv|I32 level|U32 flags
Apd	|GV*	|gv_fetchmeth_pv_autoload	|NULLOK HV* stash|NN const char* name \
                                        |I32 level|U32 flags
Apd	|GV*	|gv_fetchmeth_pvn_autoload	|NULLOK HV* stash|NN const char* name \
                                        |STRLEN len|I32 level|U32 flags
ApdMb	|GV*	|gv_fetchmethod	|NN HV* stash|NN const char* name
Apd	|GV*	|gv_fetchmethod_autoload|NN HV* stash|NN const char* name \
				|I32 autoload
Apx	|GV*	|gv_fetchmethod_sv_flags|NN HV* stash|NN SV* namesv|U32 flags
Apx	|GV*	|gv_fetchmethod_pv_flags|NN HV* stash|NN const char* name \
				|U32 flags
Apx	|GV*	|gv_fetchmethod_pvn_flags|NN HV* stash|NN const char* name \
				|const STRLEN len|U32 flags
Adp	|GV*	|gv_fetchpv	|NN const char *nambeg|I32 flags|const svtype sv_type
AbpD	|void	|gv_fullname	|NN SV* sv|NN const GV* gv
ApMb	|void	|gv_fullname3	|NN SV* sv|NN const GV* gv|NULLOK const char* prefix
Ap	|void	|gv_fullname4	|NN SV* sv|NN const GV* gv|NULLOK const char* prefix|bool keepmain
: Used in scope.c
pxoe	|GP *	|newGP		|NN GV *const gv
pX	|void	|cvgv_set	|NN CV* cv|NULLOK GV* gv
poX	|GV *	|cvgv_from_hek	|NN CV* cv
pX	|void	|cvstash_set	|NN CV* cv|NULLOK HV* stash
Amd	|void	|gv_init	|NN GV* gv|NULLOK HV* stash \
                                |NN const char* name|STRLEN len|int multi
Apd	|void	|gv_init_sv	|NN GV* gv|NULLOK HV* stash|NN SV* namesv|U32 flags
Apd	|void	|gv_init_pv	|NN GV* gv|NULLOK HV* stash|NN const char* name \
                                |U32 flags
Apd	|void	|gv_init_pvn	|NN GV* gv|NULLOK HV* stash|NN const char* name \
                                |STRLEN len|U32 flags
Ap	|void	|gv_name_set	|NN GV* gv|NN const char *name|U32 len|U32 flags
pe	|GV *	|gv_override	|NN const char * const name \
				|const STRLEN len
Xxpd	|void	|gv_try_downgrade|NN GV* gv
p	|void	|gv_setref	|NN SV *const dsv|NN SV *const ssv
Apd	|HV*	|gv_stashpv	|NN const char* name|I32 flags
Apd	|HV*	|gv_stashpvn	|NN const char* name|U32 namelen|I32 flags
#if defined(PERL_IN_GV_C) || defined(PERL_IN_UNIVERSAL_C)
EpGd	|HV*	|gv_stashsvpvn_cached|SV *namesv|const char* name|U32 namelen|I32 flags
#endif
#if defined(PERL_IN_GV_C)
i	|HV*	|gv_stashpvn_internal	|NN const char* name|U32 namelen|I32 flags
i	|GV*	|gv_fetchmeth_internal	|NULLOK HV* stash|NULLOK SV* meth|NULLOK const char* name \
					|STRLEN len|I32 level|U32 flags
#endif
Apd	|HV*	|gv_stashsv	|NN SV* sv|I32 flags
po	|struct xpvhv_aux*|hv_auxalloc|NN HV *hv
Apd	|void	|hv_clear	|NULLOK HV *hv
: used in SAVEHINTS() and op.c
ApdR	|HV *	|hv_copy_hints_hv|NULLOK HV *const ohv
Ap	|void	|hv_delayfree_ent|NN HV *hv|NULLOK HE *entry
AbMdp	|SV*	|hv_delete	|NULLOK HV *hv|NN const char *key|I32 klen \
				|I32 flags
AbMdp	|SV*	|hv_delete_ent	|NULLOK HV *hv|NN SV *keysv|I32 flags|U32 hash
AbMdRp	|bool	|hv_exists	|NULLOK HV *hv|NN const char *key|I32 klen
AbMdRp	|bool	|hv_exists_ent	|NULLOK HV *hv|NN SV *keysv|U32 hash
AbMdp	|SV**	|hv_fetch	|NULLOK HV *hv|NN const char *key|I32 klen \
				|I32 lval
AbMdp	|HE*	|hv_fetch_ent	|NULLOK HV *hv|NN SV *keysv|I32 lval|U32 hash
Cp	|void*	|hv_common	|NULLOK HV *hv|NULLOK SV *keysv \
				|NULLOK const char* key|STRLEN klen|int flags \
				|int action|NULLOK SV *val|U32 hash
Cp	|void*	|hv_common_key_len|NULLOK HV *hv|NN const char *key \
				|I32 klen_i32|const int action|NULLOK SV *val \
				|const U32 hash
Apod	|STRLEN	|hv_fill	|NN HV *const hv
Ap	|void	|hv_free_ent	|NN HV *hv|NULLOK HE *entry
Apd	|I32	|hv_iterinit	|NN HV *hv
ApdR	|char*	|hv_iterkey	|NN HE* entry|NN I32* retlen
ApdR	|SV*	|hv_iterkeysv	|NN HE* entry
ApdRbM	|HE*	|hv_iternext	|NN HV *hv
ApdR	|SV*	|hv_iternextsv	|NN HV *hv|NN char **key|NN I32 *retlen
ApxdR	|HE*	|hv_iternext_flags|NN HV *hv|I32 flags
ApdR	|SV*	|hv_iterval	|NN HV *hv|NN HE *entry
Ap	|void	|hv_ksplit	|NN HV *hv|IV newmax
ApdbM	|void	|hv_magic	|NN HV *hv|NULLOK GV *gv|int how
#if defined(PERL_IN_HV_C)
S	|SV *	|refcounted_he_value	|NN const struct refcounted_he *he
#endif
Xpd	|HV *	|refcounted_he_chain_2hv|NULLOK const struct refcounted_he *c|U32 flags
Xpd	|SV *	|refcounted_he_fetch_pvn|NULLOK const struct refcounted_he *chain \
				|NN const char *keypv|STRLEN keylen|U32 hash|U32 flags
Xpd	|SV *	|refcounted_he_fetch_pv|NULLOK const struct refcounted_he *chain \
				|NN const char *key|U32 hash|U32 flags
Xpd	|SV *	|refcounted_he_fetch_sv|NULLOK const struct refcounted_he *chain \
				|NN SV *key|U32 hash|U32 flags
Xpd	|struct refcounted_he *|refcounted_he_new_pvn \
				|NULLOK struct refcounted_he *parent \
				|NN const char *keypv|STRLEN keylen \
				|U32 hash|NULLOK SV *value|U32 flags
Xpd	|struct refcounted_he *|refcounted_he_new_pv \
				|NULLOK struct refcounted_he *parent \
				|NN const char *key \
				|U32 hash|NULLOK SV *value|U32 flags
Xpd	|struct refcounted_he *|refcounted_he_new_sv \
				|NULLOK struct refcounted_he *parent \
				|NN SV *key \
				|U32 hash|NULLOK SV *value|U32 flags
Xpd	|void	|refcounted_he_free|NULLOK struct refcounted_he *he
Xpd	|struct refcounted_he *|refcounted_he_inc|NULLOK struct refcounted_he *he
ApbMd	|SV**	|hv_store	|NULLOK HV *hv|NULLOK const char *key \
				|I32 klen|NULLOK SV *val|U32 hash
ApbMd	|HE*	|hv_store_ent	|NULLOK HV *hv|NULLOK SV *key|NULLOK SV *val\
				|U32 hash
ApbMx	|SV**	|hv_store_flags	|NULLOK HV *hv|NULLOK const char *key \
				|I32 klen|NULLOK SV *val|U32 hash|int flags
Amd	|void	|hv_undef	|NULLOK HV *hv
poX	|void	|hv_undef_flags	|NULLOK HV *hv|U32 flags
AdmP	|I32	|ibcmp		|NN const char* a|NN const char* b|I32 len
AdiTp	|I32	|foldEQ		|NN const char* a|NN const char* b|I32 len
AdmP	|I32	|ibcmp_locale	|NN const char* a|NN const char* b|I32 len
AiTpd	|I32	|foldEQ_locale	|NN const char* a|NN const char* b|I32 len
Adm	|I32	|ibcmp_utf8	|NN const char *s1|NULLOK char **pe1|UV l1 \
				|bool u1|NN const char *s2|NULLOK char **pe2 \
				|UV l2|bool u2
Amd	|I32	|foldEQ_utf8	|NN const char *s1|NULLOK char **pe1|UV l1 \
				|bool u1|NN const char *s2|NULLOK char **pe2 \
				|UV l2|bool u2
Cp	|I32	|foldEQ_utf8_flags |NN const char *s1|NULLOK char **pe1|UV l1 \
				|bool u1|NN const char *s2|NULLOK char **pe2 \
				|UV l2|bool u2|U32 flags
CiTp	|I32	|foldEQ_latin1	|NN const char* a|NN const char* b|I32 len
#if defined(PERL_IN_DOIO_C)
SR	|bool	|ingroup	|Gid_t testgid|bool effective
#endif
: Used in toke.c
p	|void	|init_argv_symbols|int argc|NN char **argv
: Used in pp_ctl.c
po	|void	|init_dbargs
: Used in mg.c
p	|void	|init_debugger
Ap	|void	|init_stacks
Ap	|void	|init_tm	|NN struct tm *ptm
: Used in perly.y
AbMTpPRd|char*	|instr		|NN const char* big|NN const char* little
: Used in sv.c
p	|bool	|io_close	|NN IO* io|NULLOK GV *gv \
				|bool not_implicit|bool warn_on_fail
: Used in perly.y
pR	|OP*	|invert		|NULLOK OP* cmd
pR	|OP*	|cmpchain_start	|I32 type|NULLOK OP* left \
				|NULLOK OP* right
pR	|OP*	|cmpchain_extend|I32 type|NN OP* ch|NULLOK OP* right
pR	|OP*	|cmpchain_finish|NN OP* ch
ApR	|I32	|is_lvalue_sub
: Used in cop.h
XopR	|I32	|was_lvalue_sub
CpRTP	|STRLEN	|is_utf8_char_helper_|NN const U8 * const s|NN const U8 * e|const U32 flags
CpRTP	|Size_t	|is_utf8_FF_helper_|NN const U8 * const s0		    \
				|NN const U8 * const e			    \
				|const bool require_partial
Cp	|UV	|to_uni_upper	|UV c|NN U8 *p|NN STRLEN *lenp
Cp	|UV	|to_uni_title	|UV c|NN U8 *p|NN STRLEN *lenp
p	|void	|init_uniprops
#ifdef PERL_IN_UTF8_C
STR	|U8	|to_lower_latin1|const U8 c|NULLOK U8 *p|NULLOK STRLEN *lenp  \
		|const char dummy
#endif
#if defined(PERL_IN_UTF8_C) || defined(PERL_IN_PP_C)
p	|UV	|_to_upper_title_latin1|const U8 c|NN U8 *p|NN STRLEN *lenp|const char S_or_s
#endif
Cp	|UV	|to_uni_lower	|UV c|NN U8 *p|NN STRLEN *lenp
Cm	|UV	|to_uni_fold	|UV c|NN U8 *p|NN STRLEN *lenp
Cp	|UV	|_to_uni_fold_flags|UV c|NN U8 *p|NN STRLEN *lenp|U8 flags
CpR	|bool	|_is_uni_perl_idcont|UV c
CpR	|bool	|_is_uni_perl_idstart|UV c
ATdmoR	|bool	|is_utf8_invariant_string|NN const U8* const s		    \
		|STRLEN len
ATidRp	|bool	|is_utf8_invariant_string_loc|NN const U8* const s	    \
		|STRLEN len						    \
		|NULLOK const U8 ** ep
CTiRp	|unsigned|single_1bit_pos32|U32 word
CTiRp	|unsigned|lsbit_pos32|U32 word
CTiRp	|unsigned|msbit_pos32|U32 word
#ifdef U64TYPE	/* HAS_QUAD undefined outside of core */
CTiRp	|unsigned|single_1bit_pos64|U64 word
CTiRp	|unsigned|lsbit_pos64|U64 word
CTiRp	|unsigned|msbit_pos64|U64 word
#endif
#ifndef EBCDIC
CTiRp	|unsigned int|variant_byte_number|PERL_UINTMAX_T word
#endif
#if defined(PERL_CORE) || defined(PERL_EXT)
EiTRd	|Size_t	|variant_under_utf8_count|NN const U8* const s		    \
		|NN const U8* const e
#endif
AmTdRP	|bool	|is_ascii_string|NN const U8* const s|STRLEN len
AmTdRP	|bool	|is_invariant_string|NN const U8* const s|STRLEN len
#if defined(PERL_CORE) || defined (PERL_EXT)
EXTidRp	|bool	|is_utf8_non_invariant_string|NN const U8* const s	    \
		|STRLEN len
#endif
AbTpdD	|STRLEN	|is_utf8_char	|NN const U8 *s
AbMTpd	|STRLEN	|is_utf8_char_buf|NN const U8 *buf|NN const U8 *buf_end
ATidRp	|Size_t	|isUTF8_CHAR|NN const U8 * const s0			    \
			    |NN const U8 * const e
ATidRp	|Size_t	|isUTF8_CHAR_flags|NN const U8 * const s0		    \
			    |NN const U8 * const e			    \
			    |const U32 flags
ATidRp	|Size_t	|isSTRICT_UTF8_CHAR |NN const U8 * const s0		    \
				    |NN const U8 * const e
ATidRp	|Size_t	|isC9_STRICT_UTF8_CHAR |NN const U8 * const s0		    \
				       |NN const U8 * const e
ATmdR	|bool	|is_utf8_string	|NN const U8 *s|STRLEN len
ATidRp	|bool	|is_utf8_string_flags					    \
		|NN const U8 *s|STRLEN len|const U32 flags
ATmdR	|bool	|is_strict_utf8_string|NN const U8 *s|STRLEN len
ATmdR	|bool	|is_c9strict_utf8_string|NN const U8 *s|STRLEN len
ATpdMb	|bool	|is_utf8_string_loc					    \
		|NN const U8 *s|const STRLEN len|NN const U8 **ep
ATdm	|bool	|is_utf8_string_loc_flags				    \
		|NN const U8 *s|STRLEN len|NN const U8 **ep		    \
		|const U32 flags
ATdm	|bool	|is_strict_utf8_string_loc				    \
		|NN const U8 *s|STRLEN len|NN const U8 **ep
ATdm	|bool	|is_c9strict_utf8_string_loc				    \
		|NN const U8 *s|STRLEN len|NN const U8 **ep
ATipd	|bool	|is_utf8_string_loclen					    \
		|NN const U8 *s|STRLEN len|NULLOK const U8 **ep		    \
		|NULLOK STRLEN *el
ATidp	|bool	|is_utf8_string_loclen_flags				    \
		|NN const U8 *s|STRLEN len|NULLOK const U8 **ep		    \
		|NULLOK STRLEN *el|const U32 flags
ATidp	|bool	|is_strict_utf8_string_loclen				    \
		|NN const U8 *s|STRLEN len|NULLOK const U8 **ep	    \
		|NULLOK STRLEN *el
ATidp	|bool	|is_c9strict_utf8_string_loclen				    \
		|NN const U8 *s|STRLEN len|NULLOK const U8 **ep	    \
		|NULLOK STRLEN *el
AmTd	|bool	|is_utf8_fixed_width_buf_flags				    \
		|NN const U8 * const s|STRLEN len|const U32 flags
AmTd	|bool	|is_utf8_fixed_width_buf_loc_flags			    \
		|NN const U8 * const s|STRLEN len			    \
		|NULLOK const U8 **ep|const U32 flags
ATidp	|bool	|is_utf8_fixed_width_buf_loclen_flags			    \
		|NN const U8 * const s|STRLEN len			    \
		|NULLOK const U8 **ep|NULLOK STRLEN *el|const U32 flags
AmTdP	|bool	|is_utf8_valid_partial_char				    \
		|NN const U8 * const s0|NN const U8 * const e
ATidRp	|bool	|is_utf8_valid_partial_char_flags			    \
		|NN const U8 * const s0|NN const U8 * const e|const U32 flags
CpR     |bool   |_is_uni_FOO|const U8 classnum|const UV c
CpR     |bool   |_is_utf8_FOO|const U8 classnum|NN const U8 *p     \
		|NN const U8 * const e
CpR     |bool   |_is_utf8_perl_idcont|NN const U8 *p|NN const U8 * const e
CpR     |bool   |_is_utf8_perl_idstart|NN const U8 *p|NN const U8 * const e

#if defined(PERL_CORE) || defined(PERL_EXT)
EXdpR	|bool	|isSCRIPT_RUN	|NN const U8 *s|NN const U8 *send   \
				|const bool utf8_target
#endif
: Used in perly.y
p	|OP*	|jmaybe		|NN OP *o
: Used in pp.c
pP	|I32	|keyword	|NN const char *name|I32 len|bool all_keywords
#if defined(PERL_IN_OP_C)
S	|void	|inplace_aassign	|NN OP* o
#endif
Ap	|void	|leave_scope	|I32 base
p	|void	|notify_parser_that_changed_to_utf8
: Public lexer API
Axpd	|void	|lex_start	|NULLOK SV* line|NULLOK PerlIO *rsfp|U32 flags
Axpd	|bool	|lex_bufutf8
Axpd	|char*	|lex_grow_linestr|STRLEN len
Axpd	|void	|lex_stuff_pvn	|NN const char* pv|STRLEN len|U32 flags
Axpd	|void	|lex_stuff_pv	|NN const char* pv|U32 flags
Axpd	|void	|lex_stuff_sv	|NN SV* sv|U32 flags
Axpd	|void	|lex_unstuff	|NN char* ptr
Axpd	|void	|lex_read_to	|NN char* ptr
Axpd	|void	|lex_discard_to	|NN char* ptr
Axpd	|bool	|lex_next_chunk	|U32 flags
Axpd	|I32	|lex_peek_unichar|U32 flags
Axpd	|I32	|lex_read_unichar|U32 flags
Axpd	|void	|lex_read_space	|U32 flags
: Public parser API
Axpd	|OP*	|parse_arithexpr|U32 flags
Axpd	|OP*	|parse_termexpr	|U32 flags
Axpd	|OP*	|parse_listexpr	|U32 flags
Axpd	|OP*	|parse_fullexpr	|U32 flags
Axpd	|OP*	|parse_block	|U32 flags
Axpd	|OP*	|parse_barestmt	|U32 flags
Axpd	|SV*	|parse_label	|U32 flags
Axpd	|OP*	|parse_fullstmt	|U32 flags
Axpd	|OP*	|parse_stmtseq	|U32 flags
Axpd	|OP*	|parse_subsignature|U32 flags
: Used in various files
Apd	|void	|op_null	|NN OP* o
: FIXME. Used by Data::Alias
EXp	|void	|op_clear	|NN OP* o
Ap	|void	|op_refcnt_lock
Ap	|void	|op_refcnt_unlock
ApdT	|OP*	|op_sibling_splice|NULLOK OP *parent|NULLOK OP *start \
		|int del_count|NULLOK OP* insert
ApdT	|OP*	|op_parent|NN OP *o
#if defined(PERL_IN_OP_C)
S	|OP*	|listkids	|NULLOK OP* o
#endif
p	|OP*	|list		|NULLOK OP* o
AFpd	|void	|load_module|U32 flags|NN SV* name|NULLOK SV* ver|...
Adp	|void	|vload_module|U32 flags|NN SV* name|NULLOK SV* ver|NULLOK va_list* args
: Used in perly.y
p	|OP*	|localize	|NN OP *o|I32 lex
ApdR	|I32	|looks_like_number|NN SV *const sv
AMpd	|UV	|grok_hex	|NN const char* start|NN STRLEN* len_p|NN I32* flags|NULLOK NV *result
Apd	|int	|grok_infnan	|NN const char** sp|NN const char *send
Apd	|int	|grok_number	|NN const char *pv|STRLEN len|NULLOK UV *valuep
Apd	|int	|grok_number_flags|NN const char *pv|STRLEN len|NULLOK UV *valuep|U32 flags
ApdR	|bool	|grok_numeric_radix|NN const char **sp|NN const char *send
ApMd	|UV	|grok_oct	|NN const char* start|NN STRLEN* len_p|NN I32* flags|NULLOK NV *result
ApMd	|UV	|grok_bin	|NN const char* start|NN STRLEN* len_p|NN I32* flags|NULLOK NV *result
Cp	|UV	|grok_bin_oct_hex|NN const char* start			    \
				 |NN STRLEN* len_p			    \
				 |NN I32* flags				    \
			         |NULLOK NV *result			    \
				 |const unsigned shift			    \
				 |const U8 lookup_bit			    \
				 |const char prefix
#ifdef PERL_IN_NUMERIC_C
S	|void	|output_non_portable|const U8 shift
#endif
EXpdT	|bool	|grok_atoUV	|NN const char* pv|NN UV* valptr|NULLOK const char** endptr
: These are all indirectly referenced by globals.c. This is somewhat annoying.
p	|int	|magic_clearenv	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_clear_all_env|NN SV* sv|NN MAGIC* mg
dp	|int	|magic_clearhint|NN SV* sv|NN MAGIC* mg
dp	|int	|magic_clearhints|NN SV* sv|NN MAGIC* mg
p	|int	|magic_clearisa	|NULLOK SV* sv|NN MAGIC* mg
p	|int	|magic_clearpack|NN SV* sv|NN MAGIC* mg
p	|int	|magic_clearsig	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_copycallchecker|NN SV* sv|NN MAGIC *mg|NN SV *nsv \
				      |NULLOK const char *name|I32 namlen
p	|int	|magic_existspack|NN SV* sv|NN const MAGIC* mg
p	|int	|magic_freeovrld|NN SV* sv|NN MAGIC* mg
p	|int	|magic_get	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getarylen|NN SV* sv|NN const MAGIC* mg
p	|int	|magic_getdefelem|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getdebugvar|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getnkeys	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getpack	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getpos	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getsig	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getsubstr|NN SV* sv|NN MAGIC* mg
p	|int	|magic_gettaint	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getuvar	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_getvec	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_nextpack	|NN SV *sv|NN MAGIC *mg|NN SV *key
p	|U32	|magic_regdata_cnt|NN SV* sv|NN MAGIC* mg
p	|int	|magic_regdatum_get|NN SV* sv|NN MAGIC* mg
:removing noreturn to silence a warning for this function resulted in no
:change to the interpreter DLL image under VS 2003 -O1 -GL 32 bits only because
:this is used in a magic vtable, do not use this on conventionally called funcs
#ifdef _MSC_VER
p	|int	|magic_regdatum_set|NN SV* sv|NN MAGIC* mg
#else
pr	|int	|magic_regdatum_set|NN SV* sv|NN MAGIC* mg
#endif
p	|int	|magic_set	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setarylen|NN SV* sv|NN MAGIC* mg
p	|int	|magic_cleararylen_p|NN SV* sv|NN MAGIC* mg
p	|int	|magic_freearylen_p|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setdbline|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setdebugvar|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setdefelem|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setnonelem|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setenv	|NN SV* sv|NN MAGIC* mg
dp	|int	|magic_sethint	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setisa	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setlvref	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setmglob	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_freemglob|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setnkeys	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setpack	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setpos	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setregexp|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setsigall|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setsig	|NULLOK SV* sv|NN MAGIC* mg
p	|int	|magic_setsubstr|NN SV* sv|NN MAGIC* mg
p	|int	|magic_settaint	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setuvar	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setvec	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_setutf8	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_freeutf8	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_set_all_env|NN SV* sv|NN MAGIC* mg
p	|U32	|magic_sizepack	|NN SV* sv|NN MAGIC* mg
p	|int	|magic_wipepack	|NN SV* sv|NN MAGIC* mg
Fpod	|SV*	|magic_methcall	|NN SV *sv|NN const MAGIC *mg \
				|NN SV *meth|U32 flags \
				|U32 argc|...
Ap	|I32 *	|markstack_grow
#if defined(USE_LOCALE_COLLATE)
p	|int	|magic_setcollxfrm|NN SV* sv|NN MAGIC* mg
p	|int	|magic_freecollxfrm|NN SV* sv|NN MAGIC* mg
pbD	|char*	|mem_collxfrm	|NN const char* input_string|STRLEN len|NN STRLEN* xlen
: Defined in locale.c, used only in sv.c
#   if defined(PERL_IN_LOCALE_C) || defined(PERL_IN_SV_C) || defined(PERL_IN_MATHOMS_C)
p	|char*	|_mem_collxfrm	|NN const char* input_string	\
				|STRLEN len			\
				|NN STRLEN* xlen		\
				|bool utf8
#   endif
#endif
Afpd	|SV*	|mess		|NN const char* pat|...
Apd	|SV*	|mess_sv	|NN SV* basemsg|bool consume
Apd	|SV*	|vmess		|NN const char* pat|NULLOK va_list* args
: FIXME - either make it public, or stop exporting it. (Data::Alias uses this)
: Used in gv.c, op.c, toke.c
EXp	|void	|qerror		|NN SV* err
Apd	|void	|sortsv		|NULLOK SV** array|size_t num_elts|NN SVCOMPARE_t cmp
Apd	|void	|sortsv_flags	|NULLOK SV** array|size_t num_elts|NN SVCOMPARE_t cmp|U32 flags
Apd	|int	|mg_clear	|NN SV* sv
Apd	|int	|mg_copy	|NN SV *sv|NN SV *nsv|NULLOK const char *key \
				|I32 klen
: Defined in mg.c, used only in scope.c
pd	|void	|mg_localize	|NN SV* sv|NN SV* nsv|bool setmagic
Apd	|SV*	|sv_string_from_errnum|int errnum|NULLOK SV* tgtsv
ApdRT	|MAGIC*	|mg_find	|NULLOK const SV* sv|int type
ApdRT	|MAGIC*	|mg_findext	|NULLOK const SV* sv|int type|NULLOK const MGVTBL *vtbl
: exported for re.pm
EXpR	|MAGIC*	|mg_find_mglob	|NN SV* sv
Apd	|int	|mg_free	|NN SV* sv
Apd	|void	|mg_free_type	|NN SV* sv|int how
Apd	|void	|mg_freeext	|NN SV* sv|int how|NULLOK const MGVTBL *vtbl
Apd	|int	|mg_get		|NN SV* sv
ApdD	|U32	|mg_length	|NN SV* sv
ApdT	|void	|mg_magical	|NN SV* sv
Apd	|int	|mg_set		|NN SV* sv
Ap	|I32	|mg_size	|NN SV* sv
AdpT	|void	|mini_mktime	|NN struct tm *ptm
Axmd	|OP*	|op_lvalue	|NULLOK OP* o|I32 type
poX	|OP*	|op_lvalue_flags|NULLOK OP* o|I32 type|U32 flags
pd	|void	|finalize_optree		|NN OP* o
pd	|void	|optimize_optree|NN OP* o
#if defined(PERL_IN_OP_C)
S	|void	|optimize_op	|NN OP* o
S	|void	|finalize_op	|NN OP* o
S	|void	|move_proto_attr|NN OP **proto|NN OP **attrs \
				|NN const GV *name|bool curstash
#endif
: Used in op.c and pp_sys.c
p	|int	|mode_from_discipline|NULLOK const char* s|STRLEN len
Cp	|const char*	|moreswitches	|NN const char* s
Apd	|NV	|my_atof	|NN const char *s
ATdpR	|NV	|my_strtod	|NN const char * const s|NULLOK char ** e
Aprd	|void	|my_exit	|U32 status
Apr	|void	|my_failure_exit
Ap	|I32	|my_fflush_all
ATp	|Pid_t	|my_fork
ATp	|void	|atfork_lock
ATp	|void	|atfork_unlock
m	|I32	|my_lstat
pX	|I32	|my_lstat_flags	|NULLOK const U32 flags
#if ! defined(HAS_MEMRCHR) && (defined(PERL_CORE) || defined(PERL_EXT))
EeiT	|void *	|my_memrchr	|NN const char * s|const char c|const STRLEN len
#endif
#if !defined(PERL_IMPLICIT_SYS)
Ap	|I32	|my_pclose	|NULLOK PerlIO* ptr
Ap	|PerlIO*|my_popen	|NN const char* cmd|NN const char* mode
#endif
Ap	|PerlIO*|my_popen_list	|NN const char* mode|int n|NN SV ** args
Apd	|void	|my_setenv	|NULLOK const char* nam|NULLOK const char* val
m	|I32	|my_stat
pX	|I32	|my_stat_flags	|NULLOK const U32 flags
Adfp	|char *	|my_strftime	|NN const char *fmt|int sec|int min|int hour|int mday|int mon|int year|int wday|int yday|int isdst
: Used in pp_ctl.c
p	|void	|my_unexec
CbDTPR	|UV	|NATIVE_TO_NEED	|const UV enc|const UV ch
CbDTPR	|UV	|ASCII_TO_NEED	|const UV enc|const UV ch
ApR	|OP*	|newANONLIST	|NULLOK OP* o
ApR	|OP*	|newANONHASH	|NULLOK OP* o
Ap	|OP*	|newANONSUB	|I32 floor|NULLOK OP* proto|NULLOK OP* block
ApdR	|OP*	|newASSIGNOP	|I32 flags|NULLOK OP* left|I32 optype|NULLOK OP* right
ApdR	|OP*	|newCONDOP	|I32 flags|NN OP* first|NULLOK OP* trueop|NULLOK OP* falseop
Apd	|CV*	|newCONSTSUB	|NULLOK HV* stash|NULLOK const char* name|NULLOK SV* sv
Apd	|CV*	|newCONSTSUB_flags|NULLOK HV* stash \
				  |NULLOK const char* name|STRLEN len \
				  |U32 flags|NULLOK SV* sv
Ap	|void	|newFORM	|I32 floor|NULLOK OP* o|NULLOK OP* block
ApdR	|OP*	|newFOROP	|I32 flags|NULLOK OP* sv|NN OP* expr|NULLOK OP* block|NULLOK OP* cont
ApdR	|OP*	|newGIVENOP	|NN OP* cond|NN OP* block|PADOFFSET defsv_off
ApdR	|OP*	|newLOGOP	|I32 optype|I32 flags|NN OP *first|NN OP *other
px	|LOGOP*	|alloc_LOGOP	|I32 type|NULLOK OP *first|NULLOK OP *other
ApdR	|OP*	|newLOOPEX	|I32 type|NN OP* label
ApdR	|OP*	|newLOOPOP	|I32 flags|I32 debuggable|NULLOK OP* expr|NULLOK OP* block
ApdR	|OP*	|newNULLLIST
ApdR	|OP*	|newOP		|I32 optype|I32 flags
Ap	|void	|newPROG	|NN OP* o
ApdR	|OP*	|newRANGE	|I32 flags|NN OP* left|NN OP* right
ApdR	|OP*	|newSLICEOP	|I32 flags|NULLOK OP* subscript|NULLOK OP* listop
ApdR	|OP*	|newSTATEOP	|I32 flags|NULLOK char* label|NULLOK OP* o
AdpbM	|CV*	|newSUB		|I32 floor|NULLOK OP* o|NULLOK OP* proto \
				|NULLOK OP* block
ApdRx	|OP*	|newTRYCATCHOP	|I32 flags|NN OP* tryblock|NN OP *catchvar|NN OP* catchblock
ApdRx	|OP*	|newDEFEROP	|I32 flags|NN OP *block
pd	|CV *	|newXS_len_flags|NULLOK const char *name|STRLEN len \
				|NN XSUBADDR_t subaddr\
				|NULLOK const char *const filename \
				|NULLOK const char *const proto \
				|NULLOK SV **const_svp|U32 flags
pX	|CV *	|newXS_deffile	|NN const char *name|NN XSUBADDR_t subaddr
Apx	|CV *	|newXS_flags	|NULLOK const char *name|NN XSUBADDR_t subaddr\
				|NN const char *const filename \
				|NULLOK const char *const proto|U32 flags
ApdU	|CV*	|newXS		|NULLOK const char *name|NN XSUBADDR_t subaddr\
				|NN const char *filename
ApMdbR	|AV*	|newAV
AmdR	|AV*	|newAV_alloc_x  |SSize_t size
AmdR	|AV*	|newAV_alloc_xz |SSize_t size
ApR	|OP*	|newAVREF	|NN OP* o
ApdR	|OP*	|newBINOP	|I32 type|I32 flags|NULLOK OP* first|NULLOK OP* last
ApR	|OP*	|newCVREF	|I32 flags|NULLOK OP* o
ApdR	|OP*	|newGVOP	|I32 type|I32 flags|NN GV* gv
Am	|GV*	|newGVgen	|NN const char* pack
ApR	|GV*	|newGVgen_flags	|NN const char* pack|U32 flags
ApR	|OP*	|newGVREF	|I32 type|NULLOK OP* o
ApR	|OP*	|newHVREF	|NN OP* o
ApMdbR	|HV*	|newHV
ApR	|HV*	|newHVhv	|NULLOK HV *hv
ApRbM	|IO*	|newIO
ApdR	|OP*	|newLISTOP	|I32 type|I32 flags|NULLOK OP* first|NULLOK OP* last
AxpdRT	|PADNAME *|newPADNAMEouter|NN PADNAME *outer
AxpdRT	|PADNAME *|newPADNAMEpvn|NN const char *s|STRLEN len
AxpdRT	|PADNAMELIST *|newPADNAMELIST|size_t max
#ifdef USE_ITHREADS
ApdR	|OP*	|newPADOP	|I32 type|I32 flags|NN SV* sv
#endif
ApdR	|OP*	|newPMOP	|I32 type|I32 flags
ApdR	|OP*	|newPVOP	|I32 type|I32 flags|NULLOK char* pv
ApdR	|SV*	|newRV		|NN SV *const sv
ApdR	|SV*	|newRV_noinc	|NN SV *const tmpRef
ApdR	|SV*	|newSV		|const STRLEN len
ApR	|OP*	|newSVREF	|NN OP* o
ApdR	|OP*	|newSVOP	|I32 type|I32 flags|NN SV* sv
ApdR	|OP*	|newDEFSVOP
pR	|SV*	|newSVavdefelem	|NN AV *av|SSize_t ix|bool extendible
ApdR	|SV*	|newSViv	|const IV i
ApdR	|SV*	|newSVuv	|const UV u
ApdR	|SV*	|newSVnv	|const NV n
ApdR	|SV*	|newSVpv	|NULLOK const char *const s|const STRLEN len
ApdR	|SV*	|newSVpvn	|NULLOK const char *const buffer|const STRLEN len
ApdR	|SV*	|newSVpvn_flags	|NULLOK const char *const s|const STRLEN len|const U32 flags
ApdR	|SV*	|newSVhek	|NULLOK const HEK *const hek
ApdR	|SV*	|newSVpvn_share	|NULLOK const char* s|I32 len|U32 hash
ApdR	|SV*	|newSVpv_share	|NULLOK const char* s|U32 hash
AfpdR	|SV*	|newSVpvf	|NN const char *const pat|...
ApRd	|SV*	|vnewSVpvf	|NN const char *const pat|NULLOK va_list *const args
Apd	|SV*	|newSVrv	|NN SV *const rv|NULLOK const char *const classname
ApMbdR	|SV*	|newSVsv	|NULLOK SV *const old
AmdR	|SV*	|newSVsv_nomg	|NULLOK SV *const old
AdpR	|SV*	|newSVsv_flags	|NULLOK SV *const old|I32 flags
ApdiR	|SV*	|newSV_type	|const svtype type
ApdIR	|SV*    |newSV_type_mortal|const svtype type
ApdR	|OP*	|newUNOP	|I32 type|I32 flags|NULLOK OP* first
ApdR	|OP*	|newUNOP_AUX	|I32 type|I32 flags|NULLOK OP* first \
				|NULLOK UNOP_AUX_item *aux
ApdR	|OP*	|newWHENOP	|NULLOK OP* cond|NN OP* block
ApdR	|OP*	|newWHILEOP	|I32 flags|I32 debuggable|NULLOK LOOP* loop \
				|NULLOK OP* expr|NULLOK OP* block|NULLOK OP* cont \
				|I32 has_my
ApdR	|OP*	|newMETHOP	|I32 type|I32 flags|NN OP* dynamic_meth
ApdR	|OP*	|newMETHOP_named|I32 type|I32 flags|NN SV* const_meth
Apd	|CV*	|rv2cv_op_cv	|NN OP *cvop|U32 flags
Apd	|OP*	|ck_entersub_args_list|NN OP *entersubop
Apd	|OP*	|ck_entersub_args_proto|NN OP *entersubop|NN GV *namegv|NN SV *protosv
Apd	|OP*	|ck_entersub_args_proto_or_list|NN OP *entersubop|NN GV *namegv|NN SV *protosv
po	|OP*	|ck_entersub_args_core|NN OP *entersubop|NN GV *namegv \
				      |NN SV *protosv
Apd	|void	|cv_get_call_checker|NN CV *cv|NN Perl_call_checker *ckfun_p|NN SV **ckobj_p
Apd	|void	|cv_get_call_checker_flags|NN CV *cv|U32 gflags|NN Perl_call_checker *ckfun_p|NN SV **ckobj_p|NN U32 *ckflags_p
Apd	|void	|cv_set_call_checker|NN CV *cv|NN Perl_call_checker ckfun|NN SV *ckobj
Apd	|void	|cv_set_call_checker_flags|NN CV *cv \
					  |NN Perl_call_checker ckfun \
					  |NN SV *ckobj|U32 ckflags
Apd	|void	|wrap_op_checker|Optype opcode|NN Perl_check_t new_checker|NN Perl_check_t *old_checker_p
Axpd	|void	|wrap_keyword_plugin|NN Perl_keyword_plugin_t new_plugin|NN Perl_keyword_plugin_t *old_plugin_p
ApR	|PERL_SI*|new_stackinfo|I32 stitems|I32 cxitems
Ap	|char*	|scan_vstring	|NN const char *s|NN const char *const e \
				|NN SV *sv
Apd	|const char*	|scan_version	|NN const char *s|NN SV *rv|bool qv
Apd	|const char*	|prescan_version	|NN const char *s\
	|bool strict|NULLOK const char** errstr|NULLOK bool *sqv\
	|NULLOK int *ssaw_decimal|NULLOK int *swidth|NULLOK bool *salpha
Apd	|SV*	|new_version	|NN SV *ver
Apd	|SV*	|upg_version	|NN SV *ver|bool qv
Apd	|SV*	|vverify	|NN SV *vs
Apd	|SV*	|vnumify	|NN SV *vs
Apd	|SV*	|vnormal	|NN SV *vs
Apd	|SV*	|vstringify	|NN SV *vs
Apd	|int	|vcmp		|NN SV *lhv|NN SV *rhv
: Used in pp_hot.c and pp_sys.c
p	|PerlIO*|nextargv	|NN GV* gv|bool nomagicopen
AdMTpP	|char*	|ninstr		|NN const char* big|NN const char* bigend \
				|NN const char* little|NN const char* lend
Apd	|void	|op_free	|NULLOK OP* arg
xp	|OP*	|op_unscope	|NULLOK OP* o
#ifdef PERL_CORE
p	|void	|opslab_free	|NN OPSLAB *slab
p	|void	|opslab_free_nopad|NN OPSLAB *slab
p	|void	|opslab_force_free|NN OPSLAB *slab
#endif
: Used in perly.y
p	|void	|package	|NN OP* o
: Used in perly.y
p	|void	|package_version|NN OP* v
: Used in toke.c and perly.y
p	|PADOFFSET|allocmy	|NN const char *const name|const STRLEN len\
				|const U32 flags
#ifdef USE_ITHREADS
Adxp	|PADOFFSET|alloccopstash|NN HV *hv
#endif
: Used in perly.y
pR	|OP*	|oopsAV		|NN OP* o
: Used in perly.y
pR	|OP*	|oopsHV		|NN OP* o

: peephole optimiser
p	|void	|peep		|NULLOK OP* o
p	|void	|rpeep		|NULLOK OP* o
: Defined in doio.c, used only in pp_hot.c
dopx	|PerlIO*|start_glob	|NN SV *tmpglob|NN IO *io

Cp	|void	|reentrant_size
Cp	|void	|reentrant_init
Cp	|void	|reentrant_free
CFTp	|void*	|reentrant_retry|NN const char *f|...

: "Very" special - can't use the O flag for this one:
: (The rename from perl_atexit to Perl_call_atexit was in 864dbfa3ca8032ef)
Ap	|void	|call_atexit	|ATEXIT_t fn|NULLOK void *ptr
ApdO	|I32	|call_argv	|NN const char* sub_name|I32 flags|NN char** argv
ApdO	|I32	|call_method	|NN const char* methname|I32 flags
ApdO	|I32	|call_pv	|NN const char* sub_name|I32 flags
ApdO	|I32	|call_sv	|NN SV* sv|volatile I32 flags
Cp	|void	|despatch_signals
Ap	|OP *	|doref		|NN OP *o|I32 type|bool set_op_ref
ApdO	|SV*	|eval_pv	|NN const char* p|I32 croak_on_error
ApdO	|I32	|eval_sv	|NN SV* sv|I32 flags
ApdO	|SV*	|get_sv		|NN const char *name|I32 flags
ApdO	|AV*	|get_av		|NN const char *name|I32 flags
ApdO	|HV*	|get_hv		|NN const char *name|I32 flags
ApdO	|CV*	|get_cv		|NN const char* name|I32 flags
Apd	|CV*	|get_cvn_flags	|NN const char* name|STRLEN len|I32 flags
ATdo	|const char*|Perl_setlocale|const int category|NULLOK const char* locale
#if defined(HAS_NL_LANGINFO) && defined(PERL_LANGINFO_H)
ATdo	|const char*|Perl_langinfo|const nl_item item
#else
ATdo	|const char*|Perl_langinfo|const int item
#endif
CpO	|int	|init_i18nl10n	|int printwarn
CbpOD	|int	|init_i18nl14n	|int printwarn
p	|char*	|my_strerror	|const int errnum
XpT	|void	|_warn_problematic_locale
Xp	|void	|set_numeric_underlying
Xp	|void	|set_numeric_standard
Xp	|bool	|_is_in_locale_category|const bool compiling|const int category
ApdT	|void	|switch_to_global_locale
ApdT	|bool	|sync_locale
ApxT	|void	|thread_locale_init
ApxT	|void	|thread_locale_term
ApdO	|void	|require_pv	|NN const char* pv
AbpdD	|void	|pack_cat	|NN SV *cat|NN const char *pat|NN const char *patend \
				|NN SV **beglist|NN SV **endlist|NN SV ***next_in_list|U32 flags
Apd	|void	|packlist	|NN SV *cat|NN const char *pat|NN const char *patend|NN SV **beglist|NN SV **endlist
#if defined(PERL_USES_PL_PIDSTATUS) && defined(PERL_IN_UTIL_C)
S	|void	|pidgone	|Pid_t pid|int status
#endif
: Used in perly.y
p	|OP*	|pmruntime	|NN OP *o|NN OP *expr|NULLOK OP *repl \
				|UV flags|I32 floor
#if defined(PERL_IN_OP_C)
S	|OP*	|pmtrans	|NN OP* o|NN OP* expr|NN OP* repl
#endif
p	|void	|invmap_dump	|NN SV* invlist|NN UV * map
Ap	|void	|pop_scope
Ap	|void	|push_scope
#if defined(PERL_IN_PERLY_C) || defined(PERL_IN_OP_C) || defined(PERL_IN_TOKE_C)
pMb	|OP*	|ref		|NULLOK OP* o|I32 type
#endif
#if defined(PERL_IN_OP_C)
S	|OP*	|refkids	|NULLOK OP* o|I32 type
#endif

ATp	|void	|repeatcpy	|NN char* to|NN const char* from|I32 len|IV count
AdTpP	|char*	|rninstr	|NN const char* big|NN const char* bigend \
				|NN const char* little|NN const char* lend
Apd	|Sighandler_t|rsignal	|int i|Sighandler_t t
: Used in pp_sys.c
p	|int	|rsignal_restore|int i|NULLOK Sigsave_t* t
: Used in pp_sys.c
p	|int	|rsignal_save	|int i|Sighandler_t t1|NN Sigsave_t* save
Ap	|Sighandler_t|rsignal_state|int i
#if defined(PERL_IN_PP_CTL_C)
S	|void	|rxres_free	|NN void** rsp
S	|void	|rxres_restore	|NN void **rsp|NN REGEXP *rx
#endif
: Used in pp_hot.c
p	|void	|rxres_save	|NN void **rsp|NN REGEXP *rx
#if !defined(HAS_RENAME)
: Used in pp_sys.c
p	|I32	|same_dirent	|NN const char* a|NN const char* b
#endif
Apda	|char*	|savepv		|NULLOK const char* pv
Apda	|char*	|savepvn	|NULLOK const char* pv|Size_t len
Apda	|char*	|savesharedpv	|NULLOK const char* pv

: NULLOK only to suppress a compiler warning
Apda	|char*	|savesharedpvn	|NULLOK const char *const pv \
				|const STRLEN len
Apda	|char*	|savesharedsvpv	|NN SV *sv
Apda	|char*	|savesvpv	|NN SV* sv
Ap	|void	|savestack_grow
Ap	|void	|savestack_grow_cnt	|I32 need
Am	|void	|save_aelem	|NN AV* av|SSize_t idx|NN SV **sptr
Ap	|void	|save_aelem_flags|NN AV* av|SSize_t idx|NN SV **sptr \
				 |const U32 flags
Ap	|I32	|save_alloc	|I32 size|I32 pad
Apdh	|void	|save_aptr	|NN AV** aptr
Apdh	|AV*	|save_ary	|NN GV* gv
Cp	|void	|save_bool	|NN bool* boolp
Cp	|void	|save_clearsv	|NN SV** svp
Cp	|void	|save_delete	|NN HV *hv|NN char *key|I32 klen
Ap	|void	|save_hdelete	|NN HV *hv|NN SV *keysv
Ap	|void	|save_adelete	|NN AV *av|SSize_t key
Cp	|void	|save_destructor|DESTRUCTORFUNC_NOCONTEXT_t f|NN void* p
Cp	|void	|save_destructor_x|DESTRUCTORFUNC_t f|NULLOK void* p
CpMb	|void	|save_freesv	|NULLOK SV* sv
: Used in SAVEFREOP(), used in op.c, pp_ctl.c
CpMb	|void	|save_freeop	|NULLOK OP* o
CpMb	|void	|save_freepv	|NULLOK char* pv
Ap	|void	|save_generic_svref|NN SV** sptr
Ap	|void	|save_generic_pvref|NN char** str
Ap	|void	|save_shared_pvref|NN char** str
Adp	|void	|save_gp	|NN GV* gv|I32 empty
Apdh	|HV*	|save_hash	|NN GV* gv
Ap	|void	|save_hints
Am	|void	|save_helem	|NN HV *hv|NN SV *key|NN SV **sptr
Ap	|void	|save_helem_flags|NN HV *hv|NN SV *key|NN SV **sptr|const U32 flags
Apdh	|void	|save_hptr	|NN HV** hptr
Cp	|void	|save_I16	|NN I16* intp
Cp	|void	|save_I32	|NN I32* intp
Cp	|void	|save_I8	|NN I8* bytep
Cp	|void	|save_int	|NN int* intp
Apdh	|void	|save_item	|NN SV* item
Cp	|void	|save_iv	|NN IV *ivp
AbpDdh	|void	|save_list	|NN SV** sarg|I32 maxsarg
CbpD	|void	|save_long	|NN long* longp
CpMb	|void	|save_mortalizesv|NN SV* sv
AbpD	|void	|save_nogv	|NN GV* gv
: Used in SAVEFREOP(), used in gv.c, op.c, perl.c, pp_ctl.c, pp_sort.c
ApMb	|void	|save_op
Apdh	|SV*	|save_scalar	|NN GV* gv
Cp	|void	|save_pptr	|NN char** pptr
Ap	|void	|save_vptr	|NN void *ptr
Cp	|void	|save_re_context
Ap	|void	|save_padsv_and_mortalize|PADOFFSET off
Cp	|void	|save_sptr	|NN SV** sptr
Xp	|void	|save_strlen	|NN STRLEN* ptr
Apdh	|SV*	|save_svref	|NN SV** sptr
Axpo	|void	|savetmps
Ap	|void	|save_pushptr	|NULLOK void *const ptr|const int type
Ap	|void	|save_pushi32ptr|const I32 i|NULLOK void *const ptr|const int type
: Used by SAVESWITCHSTACK() in pp.c
Ap	|void	|save_pushptrptr|NULLOK void *const ptr1 \
				|NULLOK void *const ptr2|const int type
#if defined(PERL_IN_SCOPE_C)
S	|void	|save_pushptri32ptr|NULLOK void *const ptr1|const I32 i \
				|NULLOK void *const ptr2|const int type
#endif
Xiop	|I32	|TOPMARK
Xiop	|I32	|POPMARK
: Used in perly.y
p	|OP*	|sawparens	|NULLOK OP* o
Apd	|OP*	|op_contextualize|NN OP* o|I32 context
: Used in perly.y
p	|OP*	|scalar		|NULLOK OP* o
#if defined(PERL_IN_OP_C)
S	|OP*	|scalarkids	|NULLOK OP* o
S	|OP*	|voidnonfinal	|NULLOK OP* o
#endif
: Used in pp_ctl.c
p	|OP*	|scalarvoid	|NN OP* o
Apd	|NV	|scan_bin	|NN const char* start|STRLEN len|NN STRLEN* retlen
Apd	|NV	|scan_hex	|NN const char* start|STRLEN len|NN STRLEN* retlen
Cp	|char*	|scan_num	|NN const char* s|NN YYSTYPE *lvalp
Apd	|NV	|scan_oct	|NN const char* start|STRLEN len|NN STRLEN* retlen
Axpd	|OP*	|op_scope	|NULLOK OP* o
ApdRx	|OP*	|op_wrap_finally|NN OP *block|NN OP *finally
: Only used by perl.c/miniperl.c, but defined in caretx.c
pe	|void	|set_caret_X
Apd	|void	|setdefout	|NN GV* gv
Ap	|HEK*	|share_hek	|NN const char* str|SSize_t len|U32 hash
#ifdef PERL_USE_3ARG_SIGHANDLER
: Used in perl.c
Tp	|Signal_t |sighandler	|int sig|NULLOK Siginfo_t *info|NULLOK void *uap
CTp	|Signal_t |csighandler	|int sig|NULLOK Siginfo_t *info|NULLOK void *uap
#else
Tp	|Signal_t |sighandler	|int sig
CTp	|Signal_t |csighandler	|int sig
#endif
Tp	|Signal_t |sighandler1	|int sig
CTp	|Signal_t |csighandler1	|int sig
Tp	|Signal_t |sighandler3	|int sig|NULLOK Siginfo_t *info|NULLOK void *uap
CTp	|Signal_t |csighandler3	|int sig|NULLOK Siginfo_t *info|NULLOK void *uap
CTp	|Signal_t |perly_sighandler	|int sig|NULLOK Siginfo_t *info|NULLOK void *uap|bool safe
Cp	|SV**	|stack_grow	|NN SV** sp|NN SV** p|SSize_t n
Ap	|I32	|start_subparse	|I32 is_format|U32 flags
Xp	|void	|init_named_cv	|NN CV *cv|NN OP *nameop
: Used in pp_ctl.c
p	|void	|sub_crush_depth|NN CV* cv
CpbMd	|bool	|sv_2bool	|NN SV *const sv
Cpd	|bool	|sv_2bool_flags	|NN SV *sv|I32 flags
Apd	|CV*	|sv_2cv		|NULLOK SV* sv|NN HV **const st|NN GV **const gvp \
				|const I32 lref
Apd	|IO*	|sv_2io		|NN SV *const sv
#if defined(PERL_IN_SV_C)
S	|bool	|glob_2number	|NN GV* const gv
#endif
CpMb	|IV	|sv_2iv		|NN SV *sv
Apd	|IV	|sv_2iv_flags	|NN SV *const sv|const I32 flags
Apd	|SV*	|sv_2mortal	|NULLOK SV *const sv
Apd	|NV	|sv_2nv_flags	|NN SV *const sv|const I32 flags
: Used in pp.c, pp_hot.c, sv.c
pxd	|SV*	|sv_2num	|NN SV *const sv
CpMb	|char*	|sv_2pv		|NN SV *sv|NULLOK STRLEN *lp
Cpd	|char*	|sv_2pv_flags	|NN SV *const sv|NULLOK STRLEN *const lp|const U32 flags
ApdMb	|char*	|sv_2pvutf8	|NN SV *sv|NULLOK STRLEN *const lp
Ap	|char*	|sv_2pvutf8_flags	|NN SV *sv|NULLOK STRLEN *const lp|const U32 flags
ApdMb	|char*	|sv_2pvbyte	|NN SV *sv|NULLOK STRLEN *const lp
Ap	|char*	|sv_2pvbyte_flags	|NN SV *sv|NULLOK STRLEN *const lp|const U32 flags
AbpD	|char*	|sv_pvn_nomg	|NN SV* sv|NULLOK STRLEN* lp
CpMb	|UV	|sv_2uv		|NN SV *sv
Apd	|UV	|sv_2uv_flags	|NN SV *const sv|const I32 flags
CbpdD	|IV	|sv_iv		|NN SV* sv
CbpdD	|UV	|sv_uv		|NN SV* sv
CbpdD	|NV	|sv_nv		|NN SV* sv
CbpdD	|char*	|sv_pvn		|NN SV *sv|NN STRLEN *lp
CbpdD	|char*	|sv_pvutf8n	|NN SV *sv|NN STRLEN *lp
CbpdD	|char*	|sv_pvbyten	|NN SV *sv|NN STRLEN *lp
Cpd	|I32	|sv_true	|NULLOK SV *const sv
#if defined(PERL_IN_SV_C)
Sd	|void	|sv_add_arena	|NN char *const ptr|const U32 size \
				|const U32 flags
#endif
ApdT	|void	|sv_backoff	|NN SV *const sv
Apd	|SV*	|sv_bless	|NN SV *const sv|NN HV *const stash
#if defined(PERL_DEBUG_READONLY_COW)
p	|void	|sv_buf_to_ro	|NN SV *sv
# if defined(PERL_IN_SV_C)
S	|void	|sv_buf_to_rw	|NN SV *sv
# endif
#endif
Afpd	|void	|sv_catpvf	|NN SV *const sv|NN const char *const pat|...
Apd	|void	|sv_vcatpvf	|NN SV *const sv|NN const char *const pat \
				|NULLOK va_list *const args
Apd	|void	|sv_catpv	|NN SV *const dsv|NULLOK const char* sstr
ApMdb	|void	|sv_catpvn	|NN SV *dsv|NN const char *sstr|STRLEN len
ApMdb	|void	|sv_catsv	|NN SV *dsv|NULLOK SV *sstr
Apd	|void	|sv_chop	|NN SV *const sv|NULLOK const char *const ptr
: Used only in perl.c
pd	|I32	|sv_clean_all
: Used only in perl.c
pd	|void	|sv_clean_objs
Apd	|void	|sv_clear	|NN SV *const orig_sv
#if defined(PERL_IN_SV_C)
S	|bool	|curse		|NN SV * const sv|const bool check_refcnt
#endif
AMpd	|I32	|sv_cmp		|NULLOK SV *const sv1|NULLOK SV *const sv2
Apd	|I32	|sv_cmp_flags	|NULLOK SV *const sv1|NULLOK SV *const sv2 \
				|const U32 flags
AMpd	|I32	|sv_cmp_locale	|NULLOK SV *const sv1|NULLOK SV *const sv2
Apd	|I32	|sv_cmp_locale_flags	|NULLOK SV *const sv1 \
				|NULLOK SV *const sv2|const U32 flags
#if defined(USE_LOCALE_COLLATE)
ApbMd	|char*	|sv_collxfrm	|NN SV *const sv|NN STRLEN *const nxp
Apd	|char*	|sv_collxfrm_flags	|NN SV *const sv|NN STRLEN *const nxp|I32 const flags
#endif
Apd	|int	|getcwd_sv	|NN SV* sv
Apd	|void	|sv_dec		|NULLOK SV *const sv
Apd	|void	|sv_dec_nomg	|NULLOK SV *const sv
Apd	|void	|sv_dump	|NULLOK SV* sv
ApdR	|bool	|sv_derived_from|NN SV* sv|NN const char *const name
ApdR	|bool	|sv_derived_from_sv|NN SV* sv|NN SV *namesv|U32 flags
ApdR	|bool	|sv_derived_from_pv|NN SV* sv|NN const char *const name|U32 flags
ApdR	|bool	|sv_derived_from_pvn|NN SV* sv|NN const char *const name \
                                    |const STRLEN len|U32 flags
ApdRx	|bool	|sv_isa_sv	|NN SV* sv|NN SV* namesv
ApdR	|bool	|sv_does	|NN SV* sv|NN const char *const name
ApdR	|bool	|sv_does_sv	|NN SV* sv|NN SV* namesv|U32 flags
ApdR	|bool	|sv_does_pv	|NN SV* sv|NN const char *const name|U32 flags
ApdR	|bool	|sv_does_pvn	|NN SV* sv|NN const char *const name|const STRLEN len \
                                |U32 flags
ApbMd	|I32	|sv_eq		|NULLOK SV* sv1|NULLOK SV* sv2
Apd	|I32	|sv_eq_flags	|NULLOK SV* sv1|NULLOK SV* sv2|const U32 flags
Apd	|void	|sv_free	|NULLOK SV *const sv
poxX	|void	|sv_free2	|NN SV *const sv|const U32 refcnt
: Used only in perl.c
pd	|void	|sv_free_arenas
Apd	|char*	|sv_gets	|NN SV *const sv|NN PerlIO *const fp|I32 append
Cpd	|char*	|sv_grow	|NN SV *const sv|STRLEN newlen
Cpd	|char*	|sv_grow_fresh	|NN SV *const sv|STRLEN newlen
Apd	|void	|sv_inc		|NULLOK SV *const sv
Apd	|void	|sv_inc_nomg	|NULLOK SV *const sv
ApMdb	|void	|sv_insert	|NN SV *const bigstr|const STRLEN offset \
				|const STRLEN len|NN const char *const little \
				|const STRLEN littlelen
Apd	|void	|sv_insert_flags|NN SV *const bigstr|const STRLEN offset|const STRLEN len \
				|NN const char *little|const STRLEN littlelen|const U32 flags
Apd	|int	|sv_isa		|NULLOK SV* sv|NN const char *const name
Apd	|int	|sv_isobject	|NULLOK SV* sv
Apd	|STRLEN	|sv_len		|NULLOK SV *const sv
Apd	|STRLEN	|sv_len_utf8	|NULLOK SV *const sv
Apd	|STRLEN	|sv_len_utf8_nomg|NN SV *const sv
Apd	|void	|sv_magic	|NN SV *const sv|NULLOK SV *const obj|const int how \
				|NULLOK const char *const name|const I32 namlen
Apd	|MAGIC *|sv_magicext	|NN SV *const sv|NULLOK SV *const obj|const int how \
				|NULLOK const MGVTBL *const vtbl|NULLOK const char *const name \
				|const I32 namlen
EiTp	|bool	|sv_only_taint_gmagic|NN SV *sv
: exported for re.pm
EXp	|MAGIC *|sv_magicext_mglob|NN SV *sv
ApdbMR	|SV*	|sv_mortalcopy	|NULLOK SV *const oldsv
ApdR	|SV*	|sv_mortalcopy_flags|NULLOK SV *const oldsv|U32 flags
ApdR	|SV*	|sv_newmortal
Cpd	|SV*	|sv_newref	|NULLOK SV *const sv
Amd	|bool	|sv_numeq	|NULLOK SV* sv1|NULLOK SV* sv2
Apd	|bool	|sv_numeq_flags	|NULLOK SV* sv1|NULLOK SV* sv2|const U32 flags
Ap	|char*	|sv_peek	|NULLOK SV* sv
Apd	|void	|sv_pos_u2b	|NULLOK SV *const sv|NN I32 *const offsetp|NULLOK I32 *const lenp
Apd	|STRLEN	|sv_pos_u2b_flags|NN SV *const sv|STRLEN uoffset \
				|NULLOK STRLEN *const lenp|U32 flags
Apd	|void	|sv_pos_b2u	|NULLOK SV *const sv|NN I32 *const offsetp
Apd	|STRLEN	|sv_pos_b2u_flags|NN SV *const sv|STRLEN const offset \
				 |U32 flags
CpMdb	|char*	|sv_pvn_force	|NN SV* sv|NULLOK STRLEN* lp
Cpd	|char*	|sv_pvutf8n_force|NN SV *const sv|NULLOK STRLEN *const lp
Cpd	|char*	|sv_pvbyten_force|NN SV *const sv|NULLOK STRLEN *const lp
Apd	|char*	|sv_recode_to_utf8	|NN SV* sv|NN SV *encoding
Apd	|bool	|sv_cat_decode	|NN SV* dsv|NN SV *encoding|NN SV *ssv|NN int *offset \
				|NN char* tstr|int tlen
ApdR	|const char*	|sv_reftype	|NN const SV *const sv|const int ob
Apd	|SV*	|sv_ref	|NULLOK SV *dst|NN const SV *const sv|const int ob
Apd	|void	|sv_replace	|NN SV *const sv|NN SV *const nsv
Apd	|void	|sv_report_used
Apd	|void	|sv_reset	|NN const char* s|NULLOK HV *const stash
p	|void	|sv_resetpvn	|NULLOK const char* s|STRLEN len \
				|NULLOK HV *const stash
Afpd	|void	|sv_setpvf	|NN SV *const sv|NN const char *const pat|...
Apd	|void	|sv_vsetpvf	|NN SV *const sv|NN const char *const pat|NULLOK va_list *const args
Apd	|void	|sv_setiv	|NN SV *const sv|const IV num
ApdbD	|void	|sv_setpviv	|NN SV *const sv|const IV num
Apd	|void	|sv_setuv	|NN SV *const sv|const UV num
Apd	|void	|sv_setnv	|NN SV *const sv|const NV num
Apd	|SV*	|sv_setref_iv	|NN SV *const rv|NULLOK const char *const classname|const IV iv
Apd	|SV*	|sv_setref_uv	|NN SV *const rv|NULLOK const char *const classname|const UV uv
Apd	|SV*	|sv_setref_nv	|NN SV *const rv|NULLOK const char *const classname|const NV nv
Apd	|SV*	|sv_setref_pv	|NN SV *const rv|NULLOK const char *const classname \
				|NULLOK void *const pv
Apd	|SV*	|sv_setref_pvn	|NN SV *const rv|NULLOK const char *const classname \
				|NN const char *const pv|const STRLEN n
Apd	|void	|sv_setpv	|NN SV *const sv|NULLOK const char *const ptr
Apd	|void	|sv_setpvn	|NN SV *const sv|NULLOK const char *const ptr|const STRLEN len
Apd	|void	|sv_setpvn_fresh|NN SV *const sv|NULLOK const char *const ptr|const STRLEN len
Apd	|char  *|sv_setpv_bufsize|NN SV *const sv|const STRLEN cur|const STRLEN len
Xp	|void	|sv_sethek	|NN SV *const sv|NULLOK const HEK *const hek
Apd	|void	|sv_setrv_noinc	|NN SV *const sv|NN SV *const ref
Apd	|void	|sv_setrv_inc	|NN SV *const sv|NN SV *const ref
Apd	|void	|sv_setrv_noinc_mg	|NN SV *const sv|NN SV *const ref
Apd	|void	|sv_setrv_inc_mg	|NN SV *const sv|NN SV *const ref
ApMdb	|void	|sv_setsv	|NN SV *dsv|NULLOK SV *ssv
Amd	|bool	|sv_streq	|NULLOK SV* sv1|NULLOK SV* sv2
Apd	|bool	|sv_streq_flags	|NULLOK SV* sv1|NULLOK SV* sv2|const U32 flags
CpMdb	|void	|sv_taint	|NN SV* sv
CpdR	|bool	|sv_tainted	|NN SV *const sv
Apd	|int	|sv_unmagic	|NN SV *const sv|const int type
Apd	|int	|sv_unmagicext	|NN SV *const sv|const int type|NULLOK const MGVTBL *vtbl
ApdMb	|void	|sv_unref	|NN SV* sv
Apd	|void	|sv_unref_flags	|NN SV *const ref|const U32 flags
Cpd	|void	|sv_untaint	|NN SV *const sv
Apd	|void	|sv_upgrade	|NN SV *const sv|svtype new_type
ApdMb	|void	|sv_usepvn	|NN SV* sv|NULLOK char* ptr|STRLEN len
Apd	|void	|sv_usepvn_flags|NN SV *const sv|NULLOK char* ptr|const STRLEN len\
				|const U32 flags
Apd	|void	|sv_vcatpvfn	|NN SV *const sv|NN const char *const pat|const STRLEN patlen \
				|NULLOK va_list *const args|NULLOK SV **const svargs|const Size_t sv_count \
				|NULLOK bool *const maybe_tainted
Apd	|void	|sv_vcatpvfn_flags|NN SV *const sv|NN const char *const pat|const STRLEN patlen \
				|NULLOK va_list *const args|NULLOK SV **const svargs|const Size_t sv_count \
				|NULLOK bool *const maybe_tainted|const U32 flags
Apd	|void	|sv_vsetpvfn	|NN SV *const sv|NN const char *const pat|const STRLEN patlen \
				|NULLOK va_list *const args|NULLOK SV **const svargs \
				|const Size_t sv_count|NULLOK bool *const maybe_tainted
CpR	|NV	|str_to_version	|NN SV *sv
Ap	|void	|regdump	|NN const regexp* r
CiTop	|struct regexp *|ReANY	|NN const REGEXP * const re
Apdh	|I32	|pregexec	|NN REGEXP * const prog|NN char* stringarg \
				|NN char* strend|NN char* strbeg \
				|SSize_t minend |NN SV* screamer|U32 nosave
Ap	|void	|pregfree	|NULLOK REGEXP* r
Cp	|void	|pregfree2	|NN REGEXP *rx
: FIXME - is anything in re using this now?
EXp	|REGEXP*|reg_temp_copy	|NULLOK REGEXP* dsv|NN REGEXP* ssv
Cp	|void	|regfree_internal|NN REGEXP *const rx
#if defined(USE_ITHREADS)
Cp	|void*	|regdupe_internal|NN REGEXP * const r|NN CLONE_PARAMS* param
#endif
EXp	|regexp_engine const *|current_re_engine
Apdh	|REGEXP*|pregcomp	|NN SV * const pattern|const U32 flags
p	|REGEXP*|re_op_compile	|NULLOK SV ** const patternp \
				|int pat_count|NULLOK OP *expr \
				|NN const regexp_engine* eng \
				|NULLOK REGEXP *old_re \
				|NULLOK bool *is_bare_re \
				|const U32 rx_flags|const U32 pm_flags
Ap	|REGEXP*|re_compile	|NN SV * const pattern|U32 orig_rx_flags
Cp	|char*	|re_intuit_start|NN REGEXP * const rx \
				|NULLOK SV* sv \
				|NN const char* const strbeg \
				|NN char* strpos \
				|NN char* strend \
				|const U32 flags \
				|NULLOK re_scream_pos_data *data
Cp	|SV*	|re_intuit_string|NN REGEXP  *const r
Cp	|I32	|regexec_flags	|NN REGEXP *const rx|NN char *stringarg \
				|NN char *strend|NN char *strbeg \
				|SSize_t minend|NN SV *sv \
				|NULLOK void *data|U32 flags
CpR	|regnode*|regnext	|NULLOK regnode* p
EXp	|SV*|reg_named_buff          |NN REGEXP * const rx|NULLOK SV * const key \
                                 |NULLOK SV * const value|const U32 flags
EXp	|SV*|reg_named_buff_iter     |NN REGEXP * const rx|NULLOK const SV * const lastkey \
                                 |const U32 flags
Cp	|SV*|reg_named_buff_fetch    |NN REGEXP * const rx|NN SV * const namesv|const U32 flags
Cp	|bool|reg_named_buff_exists  |NN REGEXP * const rx|NN SV * const key|const U32 flags
Cp	|SV*|reg_named_buff_firstkey |NN REGEXP * const rx|const U32 flags
Cp	|SV*|reg_named_buff_nextkey  |NN REGEXP * const rx|const U32 flags
Cp	|SV*|reg_named_buff_scalar   |NN REGEXP * const rx|const U32 flags
Cp	|SV*|reg_named_buff_all      |NN REGEXP * const rx|const U32 flags

: FIXME - is anything in re using this now?
EXp	|void|reg_numbered_buff_fetch|NN REGEXP * const rx|const I32 paren|NULLOK SV * const sv
: FIXME - is anything in re using this now?
EXp	|void|reg_numbered_buff_store|NN REGEXP * const rx|const I32 paren|NULLOK SV const * const value
: FIXME - is anything in re using this now?
EXp	|I32|reg_numbered_buff_length|NN REGEXP * const rx|NN const SV * const sv|const I32 paren

: FIXME - is anything in re using this now?
EXp	|SV*|reg_qr_package|NN REGEXP * const rx
EXpRT	|I16	|do_uniprop_match|NN const char * const key|const U16 key_len
EXpRT	|const char * const *|get_prop_values|const int table_index
EXpR	|SV *	|get_prop_definition|const int table_index
EXpRT	|const char *|get_deprecated_property_msg|const Size_t warning_offset
#if defined(PERL_IN_REGCOMP_C)
EiRT	|bool	|invlist_is_iterating|NN SV* const invlist
EiR	|SV*	|invlist_contents|NN SV* const invlist		    \
				 |const bool traditional_style
EixRT	|UV	|invlist_lowest|NN SV* const invlist
ERS	|SV*	|make_exactf_invlist	|NN RExC_state_t *pRExC_state \
					|NN regnode *node
ES	|regnode_offset|reg_la_NOTHING	|NN RExC_state_t *pRExC_state \
					|U32 flags|NN const char *type
ES	|regnode_offset|reg_la_OPFAIL	|NN RExC_state_t *pRExC_state \
					|U32 flags|NN const char *type
ES	|regnode_offset|reg	|NN RExC_state_t *pRExC_state \
				|I32 paren|NN I32 *flagp|U32 depth
ES	|regnode_offset|regnode_guts|NN RExC_state_t *pRExC_state          \
				|const STRLEN extra_len
#ifdef DEBUGGING
ES	|regnode_offset|regnode_guts_debug|NN RExC_state_t *pRExC_state     \
				|const U8 op				    \
				|const STRLEN extra_len
#endif
ES	|void	|change_engine_size|NN RExC_state_t *pRExC_state|const Ptrdiff_t size
ES	|regnode_offset|reganode|NN RExC_state_t *pRExC_state|U8 op \
				|U32 arg
ES	|regnode_offset|regpnode|NN RExC_state_t *pRExC_state|U8 op \
				|NN SV * arg
ES	|regnode_offset|reg2Lanode|NN RExC_state_t *pRExC_state		   \
				|const U8 op				   \
				|const U32 arg1				   \
				|const I32 arg2
ES	|regnode_offset|regatom	|NN RExC_state_t *pRExC_state \
				|NN I32 *flagp|U32 depth
ES	|regnode_offset|regbranch	|NN RExC_state_t *pRExC_state \
				|NN I32 *flagp|I32 first|U32 depth
ES	|void	 |set_ANYOF_arg	|NN RExC_state_t* const pRExC_state \
				|NN regnode* const node                    \
				|NULLOK SV* const cp_list                  \
				|NULLOK SV* const runtime_defns		   \
				|NULLOK SV* const only_utf8_locale_list
ES	|void	|output_posix_warnings					    \
				|NN RExC_state_t *pRExC_state		    \
				|NN AV* posix_warnings
EiT	|Size_t	 |find_first_differing_byte_pos|NN const U8 * s1|NN const U8 * s2| const Size_t max
ES	|AV*	 |add_multi_match|NULLOK AV* multi_char_matches		    \
				|NN SV* multi_string			    \
				|const STRLEN cp_count
ES	|regnode_offset|regclass|NN RExC_state_t *pRExC_state                 \
				|NN I32 *flagp|U32 depth|const bool stop_at_1 \
				|bool allow_multi_fold                        \
				|const bool silence_non_portable              \
				|const bool strict                            \
				|bool optimizable			      \
				|NULLOK SV** ret_invlist
ES	|U8|optimize_regclass	|NN RExC_state_t *pRExC_state		    \
				|NULLOK SV* cp_list			    \
				|NULLOK SV* only_utf8_locale_list	    \
				|NULLOK SV* upper_latin1_only_utf8_matches  \
				|const U32 has_runtime_dependency	    \
				|const U32 posixl			    \
				|NN U8 * anyof_flags			    \
				|NN bool * invert			    \
				|NN regnode_offset * ret		    \
				|NN I32 *flagp
ES	|SV *	|parse_uniprop_string|NN const char * const name	    \
				     |Size_t name_len			    \
				     |const bool is_utf8		    \
				     |const bool to_fold		    \
				     |const bool runtime		    \
				     |const bool deferrable		    \
				     |NULLOK AV ** strings		    \
				     |NN bool * user_defined_ptr	    \
				     |NN SV * msg			    \
				     |const STRLEN level
ES	|SV *	|handle_user_defined_property|NN const char * name	    \
					     |const STRLEN name_len	    \
					     |const bool is_utf8	    \
					     |const bool to_fold	    \
					     |const bool runtime	    \
					     |const bool deferrable	    \
					     |NN SV* contents		    \
					     |NN bool *user_defined_ptr	    \
					     |NN SV * msg		    \
					     |const STRLEN level
ERS	|REGEXP*|compile_wildcard|NN const char * subpattern|const STRLEN len\
				 |const bool ignore_case
ES	|I32	|execute_wildcard|NN REGEXP * const prog|NN char* stringarg \
				|NN char* strend|NN char* strbeg \
				|SSize_t minend |NN SV* screamer|U32 nosave
ES	|bool	|handle_names_wildcard					    \
				|NN const char * wname			    \
				|const STRLEN wname_len			    \
				|NN SV ** prop_definition		    \
				|NN AV ** strings
ES	|void|add_above_Latin1_folds|NN RExC_state_t *pRExC_state|const U8 cp \
				|NN SV** invlist
ES	|regnode_offset|handle_named_backref|NN RExC_state_t *pRExC_state   \
				|NN I32 *flagp				    \
				|NN char * backref_parse_start		    \
				|char ch
ESTR	|unsigned int|regex_set_precedence|const U8 my_operator
ES	|regnode_offset|handle_regex_sets|NN RExC_state_t *pRExC_state \
				|NULLOK SV ** return_invlist            \
				|NN I32 *flagp|U32 depth
ES	|void	|set_regex_pv	|NN RExC_state_t *pRExC_state|NN REGEXP *Rx
#  if defined(DEBUGGING) && defined(ENABLE_REGEX_SETS_DEBUGGING)
ES	|void	|dump_regex_sets_structures				    \
				|NN RExC_state_t *pRExC_state		    \
				|NN AV * stack				    \
				|const IV fence|NN AV * fence_stack
#  endif
ES	|void|parse_lparen_question_flags|NN RExC_state_t *pRExC_state
ES	|regnode_offset|reg_node|NN RExC_state_t *pRExC_state|U8 op
ES	|U32	|get_quantifier_value|NN RExC_state_t *pRExC_state	    \
				|NN const char * start|NN const char * end
ES	|regnode_offset|regpiece|NN RExC_state_t *pRExC_state \
				|NN I32 *flagp|U32 depth
ES	|bool	|grok_bslash_N	|NN RExC_state_t *pRExC_state		    \
				|NULLOK regnode_offset* nodep		    \
				|NULLOK UV *code_point_p		    \
				|NULLOK int* cp_count			    \
				|NN I32 *flagp				    \
				|const bool strict			    \
				|const U32 depth
ES	|void	|reginsert	|NN RExC_state_t *pRExC_state \
				|const U8 op				    \
				|const regnode_offset operand		    \
				|const U32 depth
ESR	|bool	|regtail	|NN RExC_state_t * pRExC_state		    \
				|NN const regnode_offset p		    \
				|NN const regnode_offset val		    \
				|const U32 depth
ES	|SV *	|reg_scan_name	|NN RExC_state_t *pRExC_state \
				|U32 flags
ES	|U32	|join_exact	|NN RExC_state_t *pRExC_state \
				|NN regnode *scan|NN UV *min_subtract  \
				|NN bool *unfolded_multi_char          \
				|U32 flags|NULLOK regnode *val|U32 depth
EST	|U8   |compute_EXACTish|NN RExC_state_t *pRExC_state
ES	|void	|nextchar	|NN RExC_state_t *pRExC_state
ES	|void	|skip_to_be_ignored_text|NN RExC_state_t *pRExC_state  \
				|NN char ** p			    \
				|const bool force_to_xmod
EiT	|char *	|reg_skipcomment|NN RExC_state_t *pRExC_state|NN char * p
ES	|void	|scan_commit	|NN const RExC_state_t *pRExC_state \
				|NN struct scan_data_t *data        \
				|NN SSize_t *minlenp		    \
				|int is_inf
ES	|void	|populate_ANYOF_from_invlist|NN regnode *node|NN SV** invlist_ptr
ES	|void	|ssc_anything	|NN regnode_ssc *ssc
ESRT	|int	|ssc_is_anything|NN const regnode_ssc *ssc
ES	|void	|ssc_init	|NN const RExC_state_t *pRExC_state \
				|NN regnode_ssc *ssc
ESRT	|int	|ssc_is_cp_posixl_init|NN const RExC_state_t *pRExC_state \
				|NN const regnode_ssc *ssc
ES	|void	|ssc_and	|NN const RExC_state_t *pRExC_state \
				|NN regnode_ssc *ssc                \
				|NN const regnode_charclass *and_with
ES	|void	|ssc_or		|NN const RExC_state_t *pRExC_state \
				|NN regnode_ssc *ssc \
				|NN const regnode_charclass *or_with
ES	|SV*	|get_ANYOF_cp_list_for_ssc                                 \
				|NN const RExC_state_t *pRExC_state \
				|NN const regnode_charclass* const node
ES	|void	|ssc_intersection|NN regnode_ssc *ssc \
				|NN SV* const invlist|const bool invert_2nd
ES	|void	|ssc_union	|NN regnode_ssc *ssc \
				|NN SV* const invlist|const bool invert_2nd
ES	|void	|ssc_add_range	|NN regnode_ssc *ssc \
				|UV const start|UV const end
ES	|void	|ssc_cp_and	|NN regnode_ssc *ssc \
				|UV const cp
EST	|void	|ssc_clear_locale|NN regnode_ssc *ssc
ETS	|bool	|is_ssc_worth_it|NN const RExC_state_t * pRExC_state \
				|NN const regnode_ssc * ssc
ES	|void	|ssc_finalize	|NN RExC_state_t *pRExC_state \
				|NN regnode_ssc *ssc
ES	|SSize_t|study_chunk	|NN RExC_state_t *pRExC_state \
				|NN regnode **scanp|NN SSize_t *minlenp \
				|NN SSize_t *deltap|NN regnode *last \
				|NULLOK struct scan_data_t *data \
                                |I32 stopparen|U32 recursed_depth \
				|NULLOK regnode_ssc *and_withp \
				|U32 flags|U32 depth|bool was_mutate_ok
ES	|void	|rck_elide_nothing|NN regnode *node
ESR	|SV *	|get_ANYOFM_contents|NN const regnode * n
ESRT	|U32	|add_data	|NN RExC_state_t* const pRExC_state \
				|NN const char* const s|const U32 n
frS	|void	|re_croak	|bool utf8|NN const char* pat|...
ES	|int	|handle_possible_posix					    \
				|NN RExC_state_t *pRExC_state		    \
				|NN const char* const s			    \
				|NULLOK char ** updated_parse_ptr	    \
				|NULLOK AV** posix_warnings		    \
				|const bool check_only
ES	|I32	|make_trie	|NN RExC_state_t *pRExC_state \
				|NN regnode *startbranch|NN regnode *first \
				|NN regnode *last|NN regnode *tail \
				|U32 word_count|U32 flags|U32 depth
ES	|regnode *|construct_ahocorasick_from_trie|NN RExC_state_t *pRExC_state \
                                |NN regnode *source|U32 depth
ETSR	|int	|edit_distance	|NN const UV *src		    \
				|NN const UV *tgt		    \
				|const STRLEN x			    \
				|const STRLEN y			    \
				|const SSize_t maxDistance
#  ifdef DEBUGGING
EFp	|int	|re_indentf	|NN const char *fmt|U32 depth|...
ES	|void        |regdump_intflags|NULLOK const char *lead| const U32 flags
ES	|void	|regdump_extflags|NULLOK const char *lead| const U32 flags
ES	|const regnode*|dumpuntil|NN const regexp *r|NN const regnode *start \
				|NN const regnode *node \
				|NULLOK const regnode *last \
				|NULLOK const regnode *plast \
				|NN SV* sv|I32 indent|U32 depth
ES	|void	|put_code_point	|NN SV* sv|UV c
ES	|bool	|put_charclass_bitmap_innards|NN SV* sv		    \
				|NULLOK char* bitmap		    \
				|NULLOK SV* nonbitmap_invlist	    \
				|NULLOK SV* only_utf8_locale_invlist\
				|NULLOK const regnode * const node  \
				|const U8 flags			    \
				|const bool force_as_is_display
ES	|SV*	|put_charclass_bitmap_innards_common		    \
				|NN SV* invlist			    \
				|NULLOK SV* posixes		    \
				|NULLOK SV* only_utf8		    \
				|NULLOK SV* not_utf8		    \
				|NULLOK SV* only_utf8_locale	    \
				|const bool invert
ES	|void	|put_charclass_bitmap_innards_invlist		    \
				|NN SV *sv			    \
				|NN SV* invlist
ES	|void	|put_range	|NN SV* sv|UV start|const UV end    \
				|const bool allow_literals
ES	|void	|dump_trie	|NN const struct _reg_trie_data *trie\
				|NULLOK HV* widecharmap|NN AV *revcharmap\
				|U32 depth
ES	|void	|dump_trie_interim_list|NN const struct _reg_trie_data *trie\
				|NULLOK HV* widecharmap|NN AV *revcharmap\
				|U32 next_alloc|U32 depth
ES	|void	|dump_trie_interim_table|NN const struct _reg_trie_data *trie\
				|NULLOK HV* widecharmap|NN AV *revcharmap\
				|U32 next_alloc|U32 depth
ESR	|bool	|regtail_study	|NN RExC_state_t *pRExC_state \
				|NN regnode_offset p|NN const regnode_offset val|U32 depth
#  endif
#  ifndef PERL_EXT_RE_BUILD
EiRT	|UV*	|_invlist_array_init	|NN SV* const invlist|const bool will_have_0
EiRT	|UV	|invlist_max	|NN SV* const invlist
EiRT	|IV*	|get_invlist_previous_index_addr|NN SV* invlist
EiT	|void	|invlist_set_previous_index|NN SV* const invlist|const IV index
EiRT	|IV	|invlist_previous_index|NN SV* const invlist
EiT	|void	|invlist_trim	|NN SV* invlist
Ei	|void	|invlist_clear	|NN SV* invlist
ES	|void	|_append_range_to_invlist   |NN SV* const invlist|const UV start|const UV end
ES	|void	|invlist_replace_list_destroys_src|NN SV *dest|NN SV *src
S	|void	|initialize_invlist_guts|NN SV* invlist|const Size_t initial_size
#  endif
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_DOOP_C) || defined(PERL_IN_OP_C)
EiR	|SV*	|add_cp_to_invlist	|NULLOK SV* invlist|const UV cp
Ei	|void	|invlist_extend    |NN SV* const invlist|const UV len
Ei	|void	|invlist_set_len|NN SV* const invlist|const UV len|const bool offset
EiRT	|UV	|invlist_highest|NN SV* const invlist
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_DOOP_C) || defined(PERL_IN_OP_C) || defined(PERL_IN_UTF8_C)
m	|void	|_invlist_intersection	|NN SV* const a|NN SV* const b|NN SV** i
EXp	|void	|_invlist_intersection_maybe_complement_2nd \
		|NULLOK SV* const a|NN SV* const b          \
		|const bool complement_b|NN SV** i
Cm	|void	|_invlist_union	|NULLOK SV* const a|NN SV* const b|NN SV** output
EXp	|void	|_invlist_union_maybe_complement_2nd        \
		|NULLOK SV* const a|NN SV* const b          \
		|const bool complement_b|NN SV** output
m	|void	|_invlist_subtract|NN SV* const a|NN SV* const b|NN SV** result
EXp	|void	|_invlist_invert|NN SV* const invlist
EXpR	|SV*	|_new_invlist	|IV initial_size
EXpR	|SV*	|_add_range_to_invlist	|NULLOK SV* invlist|UV start|UV end
EXpR	|SV*	|_setup_canned_invlist|const STRLEN size|const UV element0|NN UV** other_elements_ptr
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_DQUOTE_C) || defined(PERL_IN_TOKE_C)
EpRX	|bool	|grok_bslash_x	|NN char** s			\
				|NN const char* const send	\
				|NN UV* uv			\
				|NN const char** message	\
				|NULLOK U32 * packed_warn	\
				|const bool strict		\
				|const bool allow_UV_MAX	\
				|const bool utf8
EpRX	|bool	|grok_bslash_c	|const char source		\
				|NN U8 * result			\
				|NN const char** message	\
				|NULLOK U32 * packed_warn
EpRX	|bool	|grok_bslash_o	|NN char** s			\
				|NN const char* const send	\
				|NN UV* uv			\
				|NN const char** message        \
				|NULLOK U32 * packed_warn	\
				|const bool strict              \
				|const bool allow_UV_MAX	\
				|const bool utf8
EpRX	|const char *|form_alien_digit_msg|const U8 which	\
				|const STRLEN valids_len	\
				|NN const char * const first_bad\
				|NN const char * const send	\
				|const bool UTF			\
				|const bool braced
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_DQUOTE_C) || defined(PERL_IN_TOKE_C) || defined(PERL_IN_UTF8_C)
EpRX	|const char *|form_cp_too_large_msg|const U8 which	\
				|NULLOK const char * string	\
				|const Size_t len		\
				|const UV cp
#endif
#if defined(PERL_IN_REGCOMP_C) || defined (PERL_IN_DUMP_C) || defined(PERL_IN_OP_C)
EXp	|void	|_invlist_dump	|NN PerlIO *file|I32 level   \
				|NN const char* const indent \
				|NN SV* const invlist
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_OP_C)
EiRT	|STRLEN*|get_invlist_iter_addr	|NN SV* invlist
EiT	|void	|invlist_iterinit|NN SV* invlist
EiRT	|bool	|invlist_iternext|NN SV* invlist|NN UV* start|NN UV* end
EiT	|void	|invlist_iterfinish|NN SV* invlist
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_PERL_C) || defined(PERL_IN_UTF8_C)
EXpR	|SV*	|_new_invlist_C_array|NN const UV* const list
EXp	|bool	|_invlistEQ	|NN SV* const a|NN SV* const b|const bool complement_b
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_PP_C) || defined(PERL_IN_TOKE_C) || defined(PERL_IN_UNIVERSAL_C)
EiT	|const char *|get_regex_charset_name|const U32 flags|NN STRLEN* const lenp
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_REGEXEC_C)	\
 || defined(PERL_IN_PP_C) || defined(PERL_IN_OP_C)		\
 || defined(PERL_IN_TOKE_C) || defined(PERL_IN_UTF8_C)		\
 || defined(PERL_IN_DOOP_C)
EiRT	|UV*	|invlist_array	|NN SV* const invlist
EiRT	|bool	|is_invlist	|NULLOK SV* const invlist
EiRT	|bool*	|get_invlist_offset_addr|NN SV* invlist
EiRT	|UV	|_invlist_len	|NN SV* const invlist
EiRT	|bool	|_invlist_contains_cp|NN SV* const invlist|const UV cp
EXpRT	|SSize_t|_invlist_search	|NN SV* const invlist|const UV cp
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_REGEXEC_C)
#  ifndef PERL_EXT_RE_BUILD
Ep	|SV*	|get_regclass_nonbitmap_data				   \
				|NULLOK const regexp *prog		   \
				|NN const struct regnode *node		   \
				|bool doinit				   \
				|NULLOK SV **listsvp			   \
				|NULLOK SV **lonly_utf8_locale		   \
				|NULLOK SV **output_invlist
#  else
Ep	|SV*	|get_re_gclass_nonbitmap_data				   \
				|NULLOK const regexp *prog		   \
				|NN const struct regnode *node		   \
				|bool doinit				   \
				|NULLOK SV **listsvp			   \
				|NULLOK SV **lonly_utf8_locale		   \
				|NULLOK SV **output_invlist
#endif
Ep	|void	|regprop	|NULLOK const regexp *prog|NN SV* sv|NN const regnode* o|NULLOK const regmatch_info *reginfo \
				|NULLOK const RExC_state_t *pRExC_state
Efp	|int	|re_printf	|NN const char *fmt|...
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_REGEXEC_C) || defined(PERL_IN_TOKE_C)
ERp	|bool	|is_grapheme	|NN const U8 * strbeg|NN const U8 * s|NN const U8 *strend|const UV cp
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_REGEXEC_C) || defined(PERL_IN_UTF8_C)
EXTp	|UV	|_to_fold_latin1|const U8 c|NN U8 *p|NN STRLEN *lenp|const unsigned int flags
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_SV_C)
EpX	|SV*	|invlist_clone	|NN SV* const invlist|NULLOK SV* newlist
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_IN_TOKE_C)
EXpRT	|bool	|regcurly	|NN const char *s|NN const char *e|NULLOK const char * result[5]
#endif
#if defined(PERL_IN_REGEXEC_C)
ERS	|bool	|isFOO_utf8_lc	|const U8 classnum|NN const U8* character|NN const U8* e
ERTS	|U8 *	|find_next_masked|NN U8 * s				\
				 |NN const U8 * send			\
				 |const U8 byte|const U8 mask
ERTS	|U8 *|find_span_end	|NN U8* s|NN const U8 * send|const U8 span_byte
ERTS	|U8 *|find_span_end_mask|NN U8 * s|NN const U8 * send	\
				|const U8 span_byte|const U8 mask
ERS	|SSize_t|regmatch	|NN regmatch_info *reginfo|NN char *startpos|NN regnode *prog
WERS	|I32	|regrepeat	|NN regexp *prog|NN char **startposp \
				|NN const regnode *p \
				|NN char *loceol		\
				|NN regmatch_info *const reginfo \
				|I32 max
ERS	|bool	|regtry		|NN regmatch_info *reginfo|NN char **startposp
ERS	|bool	|reginclass	|NULLOK regexp * const prog  \
				|NN const regnode * const n  \
				|NN const U8 * const p       \
				|NN const U8 * const p_end   \
				|bool const utf8_target
WES	|CHECKPOINT|regcppush	|NN const regexp *rex|I32 parenfloor\
				|U32 maxopenparen
WES	|void	|regcppop	|NN regexp *rex|NN U32 *maxopenparen_p
WES	|void	|regcp_restore	|NN regexp *rex|I32 ix|NN U32 *maxopenparen_p
ERST	|U8*	|reghop3	|NN U8 *s|SSize_t off|NN const U8 *lim
ERST	|U8*	|reghop4	|NN U8 *s|SSize_t off|NN const U8 *llim \
				|NN const U8 *rlim
ERST	|U8*	|reghopmaybe3	|NN U8 *s|SSize_t off|NN const U8 * const lim
ERS	|char*	|find_byclass	|NN regexp * prog|NN const regnode *c \
				|NN char *s|NN const char *strend \
				|NULLOK regmatch_info *reginfo
ES	|void	|to_utf8_substr	|NN regexp * prog
ES	|bool	|to_byte_substr	|NN regexp * prog
ERST	|I32	|reg_check_named_buff_matched	|NN const regexp *rex \
						|NN const regnode *scan
ESR	|bool	|isGCB		|const GCB_enum before			\
				|const GCB_enum after			\
				|NN const U8 * const strbeg		\
				|NN const U8 * const curpos		\
				|const bool utf8_target
ESR	|GCB_enum|backup_one_GCB|NN const U8 * const strbeg			\
				|NN U8 ** curpos				\
				|const bool utf8_target
ESR	|bool	|isLB		|LB_enum before				\
				|LB_enum after				\
				|NN const U8 * const strbeg		\
				|NN const U8 * const curpos		\
				|NN const U8 * const strend		\
				|const bool utf8_target
ESR	|LB_enum|advance_one_LB |NN U8 ** curpos				\
				|NN const U8 * const strend			\
				|const bool utf8_target
ESR	|LB_enum|backup_one_LB  |NN const U8 * const strbeg			\
				|NN U8 ** curpos				\
				|const bool utf8_target
ESR	|bool	|isSB		|SB_enum before				\
				|SB_enum after				\
				|NN const U8 * const strbeg			\
				|NN const U8 * const curpos			\
				|NN const U8 * const strend			\
				|const bool utf8_target
ESR	|SB_enum|advance_one_SB |NN U8 ** curpos				\
				|NN const U8 * const strend			\
				|const bool utf8_target
ESR	|SB_enum|backup_one_SB  |NN const U8 * const strbeg			\
				|NN U8 ** curpos				\
				|const bool utf8_target
ESR	|bool	|isWB		|WB_enum previous				\
				|WB_enum before				\
				|WB_enum after				\
				|NN const U8 * const strbeg			\
				|NN const U8 * const curpos			\
				|NN const U8 * const strend			\
				|const bool utf8_target
ESR	|WB_enum|advance_one_WB |NN U8 ** curpos				\
				|NN const U8 * const strend			\
				|const bool utf8_target				\
				|const bool skip_Extend_Format
ESR	|WB_enum|backup_one_WB  |NN WB_enum * previous			\
				|NN const U8 * const strbeg			\
				|NN U8 ** curpos				\
				|const bool utf8_target
EiT	|I32	|foldEQ_latin1_s2_folded|NN const char* a|NN const char* b|I32 len
#  ifdef DEBUGGING
ES	|void	|dump_exec_pos	|NN const char *locinput|NN const regnode *scan|NN const char *loc_regeol\
				|NN const char *loc_bostr|NN const char *loc_reg_starttry|const bool do_utf8|const U32 depth
ES	|void	|debug_start_match|NN const REGEXP *prog|const bool do_utf8\
				|NN const char *start|NN const char *end\
				|NN const char *blurb

EFp	|int	|re_exec_indentf|NN const char *fmt|U32 depth|...
#  endif
#endif
#if defined(PERL_IN_REGEXEC_C)
ESR	|bool	|isFOO_lc	|const U8 classnum|const U8 character
#endif

Ap	|void	|taint_env
Ap	|void	|taint_proper	|NULLOK const char* f|NN const char *const s
EXp	|char *	|_byte_dump_string					\
				|NN const U8 * const start		\
				|const STRLEN len			\
				|const bool format

#if defined(PERL_IN_UTF8_C)
iTR	|int	|does_utf8_overflow|NN const U8 * const s		\
				   |NN const U8 * e			\
				   |const bool consider_overlongs
iTR	|int	|is_utf8_overlong|NN const U8 * const s			\
				|const STRLEN len
iTR	|int	|isFF_overlong	|NN const U8 * const s|const STRLEN len
SR	|char *	|unexpected_non_continuation_text			\
		|NN const U8 * const s					\
		|STRLEN print_len					\
		|const STRLEN non_cont_byte_pos				\
		|const STRLEN expect_len
#if 0	/* Not currently used, but may be needed in the future */
S	|void	|warn_on_first_deprecated_use				    \
				|NN const char * const name		    \
				|NN const char * const alternative	    \
				|const bool use_locale			    \
				|NN const char * const file		    \
				|const unsigned line
#endif
S	|UV	|to_case_cp_list|const UV original				\
				|NULLOK const U32 ** const remaining_list	\
				|NULLOK Size_t * remaining_count		\
				|NN SV *invlist					\
				|NN const I32 * const invmap			\
				|NULLOK const U32 * const * const aux_tables	\
				|NULLOK const U8 * const aux_table_lengths	\
				|NN const char * const normal
S	|UV	|_to_utf8_case  |const UV original				\
				|NULLOK const U8 *p				\
				|NN U8* ustrp					\
				|NN STRLEN *lenp				\
				|NN SV *invlist					\
				|NN const I32 * const invmap			\
				|NULLOK const U32 * const * const aux_tables	\
				|NULLOK const U8 * const aux_table_lengths	\
				|NN const char * const normal
S	|UV	|turkic_fc	|NN const U8 * const p |NN const U8 * const e|NN U8* ustrp|NN STRLEN *lenp
S	|UV	|turkic_lc	|NN const U8 * const p0|NN const U8 * const e|NN U8* ustrp|NN STRLEN *lenp
S	|UV	|turkic_uc	|NN const U8 * const p |NN const U8 * const e|NN U8* ustrp|NN STRLEN *lenp
#endif
Cp	|UV	|_to_utf8_lower_flags|NN const U8 *p|NULLOK const U8* e		\
				|NN U8* ustrp|NULLOK STRLEN *lenp|bool flags
Cp	|UV	|_to_utf8_upper_flags	|NN const U8 *p|NULLOK const U8 *e	\
				|NN U8* ustrp|NULLOK STRLEN *lenp|bool flags
Cp	|UV	|_to_utf8_title_flags	|NN const U8 *p|NULLOK const U8* e	\
				|NN U8* ustrp|NULLOK STRLEN *lenp|bool flags
Cp	|UV	|_to_utf8_fold_flags|NN const U8 *p|NULLOK const U8 *e		\
				|NN U8* ustrp|NULLOK STRLEN *lenp|U8 flags
#if defined(PERL_IN_MG_C) || defined(PERL_IN_PP_C)
pT	|bool	|translate_substr_offsets|STRLEN curlen|IV pos1_iv \
					 |bool pos1_is_uv|IV len_iv \
					 |bool len_is_uv|NN STRLEN *posp \
					 |NN STRLEN *lenp
#endif
#if defined(UNLINK_ALL_VERSIONS)
Cp	|I32	|unlnk		|NN const char* f
#endif
AbpdD	|SSize_t|unpack_str	|NN const char *pat|NN const char *patend|NN const char *s \
				|NULLOK const char *strbeg|NN const char *strend|NULLOK char **new_s \
				|I32 ocnt|U32 flags
Apd	|SSize_t|unpackstring	|NN const char *pat|NN const char *patend|NN const char *s \
				|NN const char *strend|U32 flags
Ap	|void	|unsharepvn	|NULLOK const char* sv|I32 len|U32 hash
: Used in gv.c, hv.c
p	|void	|unshare_hek	|NULLOK HEK* hek
: Used in perly.y
p	|void	|utilize	|int aver|I32 floor|NULLOK OP* version|NN OP* idop|NULLOK OP* arg
Cp	|void	|_force_out_malformed_utf8_message			    \
		|NN const U8 *const p|NN const U8 * const e|const U32 flags \
		|const bool die_here
EXp	|U8*	|utf16_to_utf8_base|NN U8* p|NN U8 *d|Size_t bytelen|NN Size_t *newlen	\
				|const bool high|const bool low
EMXp	|U8*	|utf16_to_utf8	|NN U8* p|NN U8 *d|Size_t bytelen|NN Size_t *newlen
EMXp	|U8*	|utf16_to_utf8_reversed|NN U8* p|NN U8 *d|Size_t bytelen|NN Size_t *newlen
EXpx	|U8*	|utf8_to_utf16_base|NN U8* s|NN U8 *d|Size_t bytelen|NN Size_t *newlen	\
				|const bool high|const bool low
AdpR	|STRLEN	|utf8_length	|NN const U8* s|NN const U8 *e
AipdR	|IV	|utf8_distance	|NN const U8 *a|NN const U8 *b
AipdRT	|U8*	|utf8_hop	|NN const U8 *s|SSize_t off
AipdRT	|U8*	|utf8_hop_back|NN const U8 *s|SSize_t off|NN const U8 *start
AipdRT	|U8*	|utf8_hop_forward|NN const U8 *s|SSize_t off|NN const U8 *end
AipdRT	|U8*	|utf8_hop_safe	|NN const U8 *s|SSize_t off|NN const U8 *start|NN const U8 *end
Apxd	|U8*	|utf8_to_bytes	|NN U8 *s|NN STRLEN *lenp
Apd	|int	|bytes_cmp_utf8	|NN const U8 *b|STRLEN blen|NN const U8 *u \
				|STRLEN ulen
AMxdp	|U8*	|bytes_from_utf8|NN const U8 *s|NN STRLEN *lenp|NN bool *is_utf8p
CxTdp	|U8*	|bytes_from_utf8_loc|NN const U8 *s			    \
				    |NN STRLEN *lenp			    \
				    |NN bool *is_utf8p			    \
				    |NULLOK const U8 ** first_unconverted
Apxd	|U8*	|bytes_to_utf8	|NN const U8 *s|NN STRLEN *lenp
ApdDb	|UV	|utf8_to_uvchr	|NN const U8 *s|NULLOK STRLEN *retlen
CbpdD	|UV	|utf8_to_uvuni	|NN const U8 *s|NULLOK STRLEN *retlen
CbpD	|UV	|valid_utf8_to_uvuni	|NN const U8 *s|NULLOK STRLEN *retlen
AMpd	|UV	|utf8_to_uvchr_buf	|NN const U8 *s|NN const U8 *send|NULLOK STRLEN *retlen
Cip	|UV	|utf8_to_uvchr_buf_helper|NN const U8 *s|NN const U8 *send|NULLOK STRLEN *retlen
CpdD	|UV	|utf8_to_uvuni_buf	|NN const U8 *s|NN const U8 *send|NULLOK STRLEN *retlen
p	|bool	|check_utf8_print	|NN const U8 *s|const STRLEN len

AdMTp	|UV	|utf8n_to_uvchr	|NN const U8 *s				    \
				|STRLEN curlen				    \
				|NULLOK STRLEN *retlen			    \
				|const U32 flags
AdMTp	|UV	|utf8n_to_uvchr_error|NN const U8 *s			    \
				|STRLEN curlen				    \
				|NULLOK STRLEN *retlen			    \
				|const U32 flags			    \
				|NULLOK U32 * errors
ATdip	|UV	|utf8n_to_uvchr_msgs|NN const U8 *s			    \
				|STRLEN curlen				    \
				|NULLOK STRLEN *retlen			    \
				|const U32 flags			    \
				|NULLOK U32 * errors			    \
				|NULLOK AV ** msgs
CTp	|UV	|_utf8n_to_uvchr_msgs_helper				    \
				|NN const U8 *s				    \
				|STRLEN curlen				    \
				|NULLOK STRLEN *retlen			    \
				|const U32 flags			    \
				|NULLOK U32 * errors			    \
				|NULLOK AV ** msgs
CipTRd	|UV	|valid_utf8_to_uvchr	|NN const U8 *s|NULLOK STRLEN *retlen
CdbDp	|UV	|utf8n_to_uvuni|NN const U8 *s|STRLEN curlen|NULLOK STRLEN *retlen|U32 flags

Adm	|U8*	|uvchr_to_utf8	|NN U8 *d|UV uv
Cp	|U8*	|uvuni_to_utf8	|NN U8 *d|UV uv
Adm	|U8*	|uvchr_to_utf8_flags	|NN U8 *d|UV uv|UV flags
Adm	|U8*	|uvchr_to_utf8_flags_msgs|NN U8 *d|UV uv|UV flags|NULLOK HV ** msgs
CMpd	|U8*	|uvoffuni_to_utf8_flags	|NN U8 *d|UV uv|UV flags
Cp	|U8*	|uvoffuni_to_utf8_flags_msgs|NN U8 *d|UV input_uv|const UV flags|NULLOK HV** msgs
CdpbD	|U8*	|uvuni_to_utf8_flags	|NN U8 *d|UV uv|UV flags
Apd	|char*	|pv_uni_display	|NN SV *dsv|NN const U8 *spv|STRLEN len|STRLEN pvlim|UV flags
ApdR	|char*	|sv_uni_display	|NN SV *dsv|NN SV *ssv|STRLEN pvlim|UV flags
EXpR	|Size_t	|_inverse_folds	|const UV cp				    \
				|NN U32 * first_folds_to		    \
				|NN const U32 ** remaining_folds_to
: Used by Data::Alias
EXp	|void	|vivify_defelem	|NN SV* sv
: Used in pp.c
pR	|SV*	|vivify_ref	|NN SV* sv|U32 to_what
: Used in pp_sys.c
p	|I32	|wait4pid	|Pid_t pid|NN int* statusp|int flags
: Used in locale.c and perl.c
p	|U32	|parse_unicode_opts|NN const char **popt
Ap	|U32	|seed
XpTo	|double	|drand48_r	|NN perl_drand48_t *random_state
XpTo	|void	|drand48_init_r |NN perl_drand48_t *random_state|U32 seed
: Only used in perl.c
p	|void	|get_hash_seed        |NN unsigned char * const seed_buffer
: Used in doio.c, pp_hot.c, pp_sys.c
p	|void	|report_evil_fh	|NULLOK const GV *gv
: Used in doio.c, pp_hot.c, pp_sys.c
p	|void	|report_wrongway_fh|NULLOK const GV *gv|const char have
: Used in mg.c, pp.c, pp_hot.c, regcomp.c
XEpd	|void	|report_uninit	|NULLOK const SV *uninit_sv
#if defined(PERL_IN_OP_C) || defined(PERL_IN_SV_C)
p	|void	|report_redefined_cv|NN const SV *name \
				    |NN const CV *old_cv \
				    |NULLOK SV * const *new_const_svp
#endif
Apd	|void	|warn_sv	|NN SV *baseex
Afpd	|void	|warn		|NN const char* pat|...
Apd	|void	|vwarn		|NN const char* pat|NULLOK va_list* args
Adfp	|void	|warner		|U32 err|NN const char* pat|...
Adfp	|void	|ck_warner	|U32 err|NN const char* pat|...
Adfp	|void	|ck_warner_d	|U32 err|NN const char* pat|...
Adp	|void	|vwarner	|U32 err|NN const char* pat|NULLOK va_list* args
#ifdef USE_C_BACKTRACE
pd	|Perl_c_backtrace*|get_c_backtrace|int max_depth|int skip
dm	|void	|free_c_backtrace|NN Perl_c_backtrace* bt
Apd	|SV*	|get_c_backtrace_dump|int max_depth|int skip
Apd	|bool	|dump_c_backtrace|NN PerlIO* fp|int max_depth|int skip
#endif
: FIXME
p	|void	|watch		|NN char** addr
Amd	|I32	|whichsig	|NN const char* sig
Apd	|I32    |whichsig_sv    |NN SV* sigsv
Apd	|I32    |whichsig_pv    |NN const char* sig
Apd	|I32    |whichsig_pvn   |NN const char* sig|STRLEN len
: used to check for NULs in pathnames and other names
AiRdp	|bool	|is_safe_syscall|NN const char *pv|STRLEN len|NN const char *what|NN const char *op_name
#ifdef PERL_CORE
iTR	|bool	|should_warn_nl|NN const char *pv
#endif
: Used in pp_ctl.c
p	|void	|write_to_stderr|NN SV* msv
: Used in op.c
p	|int	|yyerror	|NN const char *const s
p	|void	|yyquit
pr	|void	|abort_execution|NN const char * const msg|NN const char * const name
p	|int	|yyerror_pv	|NN const char *const s|U32 flags
p	|int	|yyerror_pvn	|NULLOK const char *const s|STRLEN len|U32 flags
: Used in perly.y, and by Data::Alias
EXp	|int	|yylex
p	|void	|yyunlex
: Used in perl.c, pp_ctl.c
p	|int	|yyparse	|int gramtype
: Only used in scope.c
p	|void	|parser_free	|NN const yy_parser *parser
#ifdef PERL_CORE
p	|void	|parser_free_nexttoke_ops|NN yy_parser *parser \
					 |NN OPSLAB *slab
#endif
#if defined(PERL_IN_TOKE_C)
S	|int	|yywarn		|NN const char *const s|U32 flags
#endif
#if defined(MYMALLOC)
Ap	|void	|dump_mstats	|NN const char* s
Ap	|int	|get_mstats	|NN perl_mstats_t *buf|int buflen|int level
#endif
ATdpa	|Malloc_t|safesysmalloc	|MEM_SIZE nbytes
ATdpa	|Malloc_t|safesyscalloc	|MEM_SIZE elements|MEM_SIZE size
ATdpR	|Malloc_t|safesysrealloc|Malloc_t where|MEM_SIZE nbytes
AdTp	|Free_t	|safesysfree	|Malloc_t where
CrTp	|void	|croak_memory_wrap
Cp	|int	|runops_standard
Cp	|int	|runops_debug
Afpd	|void	|sv_catpvf_mg	|NN SV *const sv|NN const char *const pat|...
Apd	|void	|sv_vcatpvf_mg	|NN SV *const sv|NN const char *const pat \
				|NULLOK va_list *const args
Apd	|void	|sv_catpv_mg	|NN SV *const dsv|NULLOK const char *const sstr
ApdbM	|void	|sv_catpvn_mg	|NN SV *dsv|NN const char *sstr|STRLEN len
ApdbM	|void	|sv_catsv_mg	|NN SV *dsv|NULLOK SV *sstr
Afpd	|void	|sv_setpvf_mg	|NN SV *const sv|NN const char *const pat|...
Apd	|void	|sv_vsetpvf_mg	|NN SV *const sv|NN const char *const pat \
				|NULLOK va_list *const args
Apd	|void	|sv_setiv_mg	|NN SV *const sv|const IV i
ApdbD	|void	|sv_setpviv_mg	|NN SV *const sv|const IV iv
Apd	|void	|sv_setuv_mg	|NN SV *const sv|const UV u
Apd	|void	|sv_setnv_mg	|NN SV *const sv|const NV num
Apd	|void	|sv_setpv_mg	|NN SV *const sv|NULLOK const char *const ptr
Apd	|void	|sv_setpvn_mg	|NN SV *const sv|NN const char *const ptr|const STRLEN len
Apd	|void	|sv_setsv_mg	|NN SV *const dsv|NULLOK SV *const ssv
ApdbM	|void	|sv_usepvn_mg	|NN SV *sv|NULLOK char *ptr|STRLEN len
ApR	|MGVTBL*|get_vtbl	|int vtbl_id
Apd	|char*	|pv_display	|NN SV *dsv|NN const char *pv|STRLEN cur|STRLEN len \
				|STRLEN pvlim
Apd	|char*	|pv_escape	|NULLOK SV *dsv|NN char const * const str\
                                |const STRLEN count|const STRLEN max\
                                |NULLOK STRLEN * const escaped\
                                |const U32 flags
Apd	|char*  |pv_pretty      |NN SV *dsv|NN char const * const str\
                                |const STRLEN count|const STRLEN max\
                                |NULLOK char const * const start_color\
                                |NULLOK char const * const end_color\
                                |const U32 flags
Cfp	|void	|dump_indent	|I32 level|NN PerlIO *file|NN const char* pat|...
Cp	|void	|dump_vindent	|I32 level|NN PerlIO *file|NN const char* pat \
				|NULLOK va_list *args
Cp	|void	|do_gv_dump	|I32 level|NN PerlIO *file|NN const char *name\
				|NULLOK GV *sv
Cp	|void	|do_gvgv_dump	|I32 level|NN PerlIO *file|NN const char *name\
				|NULLOK GV *sv
Cp	|void	|do_hv_dump	|I32 level|NN PerlIO *file|NN const char *name\
				|NULLOK HV *sv
Cp	|void	|do_magic_dump	|I32 level|NN PerlIO *file|NULLOK const MAGIC *mg|I32 nest \
				|I32 maxnest|bool dumpops|STRLEN pvlim
Cp	|void	|do_op_dump	|I32 level|NN PerlIO *file|NULLOK const OP *o
Cp	|void	|do_pmop_dump	|I32 level|NN PerlIO *file|NULLOK const PMOP *pm
Cp	|void	|do_sv_dump	|I32 level|NN PerlIO *file|NULLOK SV *sv|I32 nest \
				|I32 maxnest|bool dumpops|STRLEN pvlim
Ap	|void	|magic_dump	|NULLOK const MAGIC *mg
Cp	|void	|reginitcolors
CpdRMb	|char*	|sv_2pv_nolen	|NN SV* sv
CpdRMb	|char*	|sv_2pvutf8_nolen|NN SV* sv
CpdRMb	|char*	|sv_2pvbyte_nolen|NN SV* sv
CpMdbR	|char*	|sv_pv		|NN SV *sv
CpMdbR	|char*	|sv_pvutf8	|NN SV *sv
CpMdbR	|char*	|sv_pvbyte	|NN SV *sv
ApMdb	|STRLEN	|sv_utf8_upgrade|NN SV *sv
Amd	|STRLEN	|sv_utf8_upgrade_nomg|NN SV *sv
ApdMb	|bool	|sv_utf8_downgrade|NN SV *const sv|const bool fail_ok
Amd	|bool	|sv_utf8_downgrade_nomg|NN SV *const sv|const bool fail_ok
Apd	|bool	|sv_utf8_downgrade_flags|NN SV *const sv|const bool fail_ok|const U32 flags
Apd	|void	|sv_utf8_encode |NN SV *const sv
Apd	|bool	|sv_utf8_decode |NN SV *const sv
ApdMb	|void	|sv_force_normal|NN SV *sv
Apd	|void	|sv_force_normal_flags|NN SV *const sv|const U32 flags
pX	|SSize_t|tmps_grow_p	|SSize_t ix
Apd	|SV*	|sv_rvweaken	|NN SV *const sv
Apd	|SV*	|sv_rvunweaken	|NN SV *const sv
ATpxd	|SV*	|sv_get_backrefs|NN SV *const sv
AiTMdp	|SV *	|SvREFCNT_inc	|NULLOK SV *sv
AiTMdp	|SV *	|SvREFCNT_inc_NN|NN SV *sv
AiTMdp	|void	|SvREFCNT_inc_void|NULLOK SV *sv
AiMdp	|void	|SvREFCNT_dec	|NULLOK SV *sv
AiMdp	|void	|SvREFCNT_dec_NN|NN SV *sv
AiTp	|void	|SvAMAGIC_on	|NN SV *sv
AiTp	|void	|SvAMAGIC_off	|NN SV *sv
Aipd	|bool	|SvTRUE		|NULLOK SV *sv
Aipd	|bool	|SvTRUE_nomg	|NULLOK SV *sv
Aipd	|bool	|SvTRUE_NN	|NN SV *sv
Cip	|bool	|SvTRUE_common	|NN SV *sv|const bool sv_2bool_is_fallback
: This is indirectly referenced by globals.c. This is somewhat annoying.
p	|int	|magic_killbackrefs|NN SV *sv|NN MAGIC *mg
Ap	|OP*	|newANONATTRSUB	|I32 floor|NULLOK OP *proto|NULLOK OP *attrs|NULLOK OP *block
Adm	|CV*	|newATTRSUB	|I32 floor|NULLOK OP *o|NULLOK OP *proto|NULLOK OP *attrs|NULLOK OP *block
pdX	|CV*	|newATTRSUB_x	|I32 floor|NULLOK OP *o|NULLOK OP *proto \
				 |NULLOK OP *attrs|NULLOK OP *block \
				 |bool o_is_gv
Ap	|CV *	|newMYSUB	|I32 floor|NN OP *o|NULLOK OP *proto \
				|NULLOK OP *attrs|NULLOK OP *block
p	|CV*	|newSTUB	|NN GV *gv|bool fake
: Used in perly.y
p	|OP *	|my_attrs	|NN OP *o|NULLOK OP *attrs
#if defined(USE_ITHREADS)
CpR	|PERL_CONTEXT*|cx_dup	|NULLOK PERL_CONTEXT* cx|I32 ix|I32 max|NN CLONE_PARAMS* param
ApR	|PERL_SI*|si_dup	|NULLOK PERL_SI* si|NN CLONE_PARAMS* param
ApR	|ANY*	|ss_dup		|NN PerlInterpreter* proto_perl|NN CLONE_PARAMS* param
ApR	|void*	|any_dup	|NULLOK void* v|NN const PerlInterpreter* proto_perl
ApR	|HE*	|he_dup		|NULLOK const HE* e|bool shared|NN CLONE_PARAMS* param
ApR	|HEK*	|hek_dup	|NULLOK HEK* e|NN CLONE_PARAMS* param
Adp	|void	|re_dup_guts	|NN const REGEXP *sstr|NN REGEXP *dstr \
				|NN CLONE_PARAMS* param
Ap	|PerlIO*|fp_dup		|NULLOK PerlIO *const fp|const char type|NN CLONE_PARAMS *const param
ApR	|DIR*	|dirp_dup	|NULLOK DIR *const dp|NN CLONE_PARAMS *const param
ApR	|GP*	|gp_dup		|NULLOK GP *const gp|NN CLONE_PARAMS *const param
ApR	|MAGIC*	|mg_dup		|NULLOK MAGIC *mg|NN CLONE_PARAMS *const param
#if defined(PERL_IN_SV_C)
S	|SV **	|sv_dup_inc_multiple|NN SV *const *source|NN SV **dest \
				|SSize_t items|NN CLONE_PARAMS *const param
SR	|SV*	|sv_dup_common	|NN const SV *const ssv \
				|NN CLONE_PARAMS *const param
#endif
ApR	|SV*	|sv_dup		|NULLOK const SV *const ssv|NN CLONE_PARAMS *const param
ApR	|SV*	|sv_dup_inc	|NULLOK const SV *const ssv \
				|NN CLONE_PARAMS *const param
Ap	|void	|rvpv_dup	|NN SV *const dsv|NN const SV *const ssv|NN CLONE_PARAMS *const param
Ap	|yy_parser*|parser_dup	|NULLOK const yy_parser *const proto|NN CLONE_PARAMS *const param
#endif
ApR	|PTR_TBL_t*|ptr_table_new
ApR	|void*	|ptr_table_fetch|NN PTR_TBL_t *const tbl|NULLOK const void *const sv
Ap	|void	|ptr_table_store|NN PTR_TBL_t *const tbl|NULLOK const void *const oldsv \
				|NN void *const newsv
Ap	|void	|ptr_table_split|NN PTR_TBL_t *const tbl
Ap	|void	|ptr_table_free|NULLOK PTR_TBL_t *const tbl
#if defined(HAVE_INTERP_INTERN)
Ap	|void	|sys_intern_clear
Ap	|void	|sys_intern_init
#  if defined(USE_ITHREADS)
Ap	|void	|sys_intern_dup	|NN struct interp_intern* src|NN struct interp_intern* dst
#  endif
#endif

: The reason for the 'u' flag is that this passes "aTHX_ x" to its callee: not
: a legal C parameter
Admu	|const XOP *	|Perl_custom_op_xop	|NN const OP *o
AbpRdD	|const char *	|custom_op_name	|NN const OP *o
AbpRdD	|const char *	|custom_op_desc	|NN const OP *o
pRX	|XOPRETANY	|custom_op_get_field	|NN const OP *o|const xop_flags_enum field
Adop	|void	|custom_op_register	|NN Perl_ppaddr_t ppaddr \
			|NN const XOP *xop

Adp	|void	|sv_nosharing	|NULLOK SV *sv
AdpbD	|void	|sv_nolocking	|NULLOK SV *sv
Adp	|bool	|sv_destroyable	|NULLOK SV *sv
AdpbD	|void	|sv_nounlocking	|NULLOK SV *sv
Adp	|int	|nothreadhook
p	|void	|init_constants

#if defined(PERL_IN_DOOP_C)
SR	|Size_t	|do_trans_simple	|NN SV * const sv|NN const OPtrans_map * const tbl
SR	|Size_t	|do_trans_count		|NN SV * const sv|NN const OPtrans_map * const tbl
SR	|Size_t	|do_trans_complex	|NN SV * const sv|NN const OPtrans_map * const tbl
SR	|Size_t	|do_trans_invmap	|NN SV * const sv|NN AV * const map
SR	|Size_t	|do_trans_count_invmap  |NN SV * const sv|NN AV * const map
#endif

#if defined(PERL_IN_GV_C)
S	|void	|gv_init_svtype	|NN GV *gv|const svtype sv_type
S	|void	|gv_magicalize_isa	|NN GV *gv
S	|bool|parse_gv_stash_name|NN HV **stash|NN GV **gv \
                     |NN const char **name|NN STRLEN *len \
                     |NN const char *nambeg|STRLEN full_len \
                     |const U32 is_utf8|const I32 add
S	|bool|find_default_stash|NN HV **stash|NN const char *name \
                     |STRLEN len|const U32 is_utf8|const I32 add \
                     |const svtype sv_type
S	|bool|gv_magicalize|NN GV *gv|NN HV *stash|NN const char *name \
                     |STRLEN len \
                     |const svtype sv_type
S	|void|maybe_multimagic_gv|NN GV *gv|NN const char *name|const svtype sv_type
S	|bool|gv_is_in_main|NN const char *name|STRLEN len \
                      |const U32 is_utf8
S	|void	|require_tie_mod|NN GV *gv|NN const char varname \
				|NN const char * name|STRLEN len \
				|const U32 flags
#endif

#if defined(PERL_IN_HV_C) || defined(PERL_IN_SV_C)
po	|SV*	|hfree_next_entry	|NN HV *hv|NN STRLEN *indexp
#endif

#if defined(PERL_IN_HV_C)
S	|void	|hsplit		|NN HV *hv|STRLEN const oldsize|STRLEN newsize
S	|void	|hv_free_entries|NN HV *hv
S	|SV*	|hv_free_ent_ret|NN HV *hv|NN HE *entry
#if !defined(PURIFY)
SR	|HE*	|new_he
#endif
SaTR	|HEK*	|save_hek_flags	|NN const char *str|I32 len|U32 hash|int flags
ST	|void	|hv_magic_check	|NN HV *hv|NN bool *needs_copy|NN bool *needs_store
S	|void	|unshare_hek_or_pvn|NULLOK const HEK* hek|NULLOK const char* str|I32 len|U32 hash
SR	|HEK*	|share_hek_flags|NN const char *str|STRLEN len|U32 hash|int flags
rS	|void	|hv_notallowed	|int flags|NN const char *key|I32 klen|NN const char *msg
iT	|U32|ptr_hash|PTRV u
S	|struct xpvhv_aux*|hv_auxinit|NN HV *hv
Sx	|SV*	|hv_delete_common|NULLOK HV *hv|NULLOK SV *keysv \
		|NULLOK const char *key|STRLEN klen|int k_flags|I32 d_flags \
		|U32 hash
Sx	|void	|clear_placeholders	|NN HV *hv|U32 items
#endif

#if defined(PERL_IN_MG_C)
S	|void	|save_magic_flags|I32 mgs_ix|NN SV *sv|U32 flags
S	|int	|magic_methpack	|NN SV *sv|NN const MAGIC *mg|NN SV *meth
S	|SV*	|magic_methcall1|NN SV *sv|NN const MAGIC *mg \
				|NN SV *meth|U32 flags \
				|int n|NULLOK SV *val
S	|void	|restore_magic	|NULLOK const void *p
S	|void	|unwind_handler_stack|NULLOK const void *p
S	|void	|fixup_errno_string|NN SV* sv

#endif

#if defined(USE_ITHREADS)
Cip	|AV*	|cop_file_avn	|NN const COP *cop
#endif

#if defined(PERL_IN_OP_C)
SRT	|bool	|is_handle_constructor|NN const OP *o|I32 numargs
SR	|I32	|assignment_type|NULLOK const OP *o
S	|void	|forget_pmop	|NN PMOP *const o
S	|void	|find_and_forget_pmops	|NN OP *o
S	|void	|cop_free	|NN COP *cop
S	|OP*	|modkids	|NULLOK OP *o|I32 type
S	|OP*	|scalarboolean	|NN OP *o
SR	|OP*	|search_const	|NN OP *o
SR	|OP*	|new_logop	|I32 type|I32 flags|NN OP **firstp|NN OP **otherp
S	|void	|simplify_sort	|NN OP *o
SRT	|bool	|scalar_mod_type|NULLOK const OP *o|I32 type
S	|OP *	|my_kid		|NULLOK OP *o|NULLOK OP *attrs|NN OP **imopsp
S	|OP *	|dup_attrlist	|NN OP *o
S	|void	|apply_attrs	|NN HV *stash|NN SV *target|NULLOK OP *attrs
S	|void	|apply_attrs_my	|NN HV *stash|NN OP *target|NULLOK OP *attrs|NN OP **imopsp
S	|void	|bad_type_pv	|I32 n|NN const char *t|NN const OP *o|NN const OP *kid
S	|void	|bad_type_gv	|I32 n|NN GV *gv|NN const OP *kid|NN const char *t
S	|void	|no_bareword_allowed|NN OP *o
SR	|OP*	|no_fh_allowed|NN OP *o
SR	|OP*	|too_few_arguments_pv|NN OP *o|NN const char* name|U32 flags
S	|OP*	|too_many_arguments_pv|NN OP *o|NN const char* name|U32 flags
S	|bool	|looks_like_bool|NN const OP* o
S	|OP*	|newGIVWHENOP	|NULLOK OP* cond|NN OP *block \
				|I32 enter_opcode|I32 leave_opcode \
				|PADOFFSET entertarg
S	|OP*	|ref_array_or_hash|NULLOK OP* cond
S	|bool	|process_special_blocks	|I32 floor \
					|NN const char *const fullname\
					|NN GV *const gv|NN CV *const cv
S	|void	|clear_special_blocks	|NN const char *const fullname\
					|NN GV *const gv|NN CV *const cv
#endif
p	|void   |no_bareword_filehandle|NN const char *fhname
XpR	|void*	|Slab_Alloc	|size_t sz
Xp	|void	|Slab_Free	|NN void *op
#if defined(PERL_DEBUG_READONLY_OPS)
#    if defined(PERL_CORE)
pe	|void	|Slab_to_ro	|NN OPSLAB *slab
pe	|void	|Slab_to_rw	|NN OPSLAB *const slab
#    endif
: Used in OpREFCNT_inc() in sv.c
poex	|OP *	|op_refcnt_inc	|NULLOK OP *o
: FIXME - can be static.
poex	|PADOFFSET	|op_refcnt_dec	|NN OP *o
#endif

#if defined(PERL_IN_PERL_C)
S	|void	|find_beginning	|NN SV* linestr_sv|NN PerlIO *rsfp
S	|void	|forbid_setid	|const char flag|const bool suidscript
S	|void	|incpush	|NN const char *const dir|STRLEN len \
				|U32 flags
S	|SV*	|mayberelocate	|NN const char *const dir|STRLEN len \
				|U32 flags
S	|void	|incpush_use_sep|NN const char *p|STRLEN len|U32 flags
S	|void	|init_interp
S	|void	|init_ids
S	|void	|init_main_stash
S	|void	|init_perllib
S	|void	|init_postdump_symbols|int argc|NN char **argv|NULLOK char **env
S	|void	|init_predump_symbols
rS	|void	|my_exit_jump
S	|void	|nuke_stacks
S	|PerlIO *|open_script	|NN const char *scriptname|bool dosearch \
				|NN bool *suidscript
Sr	|void	|usage
#ifndef SETUID_SCRIPTS_ARE_SECURE_NOW
So	|void	|validate_suid	|NN PerlIO *rsfp
#endif
Sr	|void	|minus_v

S	|void*	|parse_body	|NULLOK char **env|XSINIT_t xsinit
rS	|void	|run_body	|I32 oldscope
#  ifndef PERL_IS_MINIPERL
S	|SV *	|incpush_if_exists|NN AV *const av|NN SV *dir|NN SV *const stem
#  endif
#endif

#if defined(PERL_IN_PP_C)
S	|size_t	|do_chomp	|NN SV *retval|NN SV *sv|bool chomping
S	|OP*	|do_delete_local
SR	|SV*	|refto		|NN SV* sv
#endif
#if defined(PERL_IN_PP_C) || defined(PERL_IN_PP_HOT_C)
: Used in pp_hot.c
pReo	|GV*	|softref2xv	|NN SV *const sv|NN const char *const what \
				|const svtype type|NN SV ***spp
iTR	|bool	|lossless_NV_to_IV|const NV nv|NN IV * ivp
#endif
#if defined(PERL_IN_PP_HOT_C)
IR	|bool	|should_we_output_Debug_r|NN regexp * prog
#endif

#if defined(PERL_IN_PP_PACK_C)
S	|SSize_t|unpack_rec	|NN struct tempsym* symptr|NN const char *s \
				|NN const char *strbeg|NN const char *strend|NULLOK const char **new_s
S	|SV **	|pack_rec	|NN SV *cat|NN struct tempsym* symptr|NN SV **beglist|NN SV **endlist
S	|SV*	|mul128		|NN SV *sv|U8 m
S	|SSize_t|measure_struct	|NN struct tempsym* symptr
S	|bool	|next_symbol	|NN struct tempsym* symptr
SR	|SV*	|is_an_int	|NN const char *s|STRLEN l
S	|int	|div128		|NN SV *pnum|NN bool *done
S	|const char *|group_end	|NN const char *patptr|NN const char *patend \
				|char ender
SR	|const char *|get_num	|NN const char *patptr|NN SSize_t *lenptr
TS	|bool	|need_utf8	|NN const char *pat|NN const char *patend
TS	|char	|first_symbol	|NN const char *pat|NN const char *patend
SR	|char *	|sv_exp_grow	|NN SV *sv|STRLEN needed
STR	|char *	|my_bytes_to_utf8|NN const U8 *start|STRLEN len|NN char *dest \
				|const bool needs_swap
#endif

#if defined(PERL_IN_PP_CTL_C)
SdR	|OP*	|docatch	|Perl_ppaddr_t firstpp
SR	|OP*	|dofindlabel	|NN OP *o|NN const char *label|STRLEN len \
                                |U32 flags|NN OP **opstack|NN OP **oplimit
S	|MAGIC *|doparseform	|NN SV *sv
STR	|bool	|num_overflow	|NV value|I32 fldsize|I32 frcsize
SR	|I32	|dopoptoeval	|I32 startingblock
SR	|I32	|dopoptogivenfor|I32 startingblock
SR	|I32	|dopoptolabel	|NN const char *label|STRLEN len|U32 flags
SR	|I32	|dopoptoloop	|I32 startingblock
SR	|I32	|dopoptosub_at	|NN const PERL_CONTEXT* cxstk|I32 startingblock
SR	|I32	|dopoptowhen	|I32 startingblock
S	|void	|save_lines	|NULLOK AV *array|NN SV *sv
S	|bool	|doeval_compile	|U8 gimme \
				|NULLOK CV* outside|U32 seq|NULLOK HV* hh
SR	|PerlIO *|check_type_and_open|NN SV *name
#ifndef PERL_DISABLE_PMC
SR	|PerlIO *|doopen_pm	|NN SV *name
#endif
iRT	|bool	|path_is_searchable|NN const char *name
SR	|I32	|run_user_filter|int idx|NN SV *buf_sv|int maxlen
SR	|PMOP*	|make_matcher	|NN REGEXP* re
SR	|bool	|matcher_matches_sv|NN PMOP* matcher|NN SV* sv
S	|void	|destroy_matcher|NN PMOP* matcher
S	|OP*	|do_smartmatch	|NULLOK HV* seen_this \
				|NULLOK HV* seen_other|const bool copied
#endif

#if defined(PERL_IN_PP_HOT_C)
S	|void	|do_oddball	|NN SV **oddkey|NN SV **firstkey
i	|HV*	|opmethod_stash	|NN SV* meth
#endif

#if defined(PERL_IN_PP_SORT_C)
I	|I32	|sv_ncmp	|NN SV *const a|NN SV *const b
I	|I32	|sv_ncmp_desc	|NN SV *const a|NN SV *const b
I	|I32	|sv_i_ncmp	|NN SV *const a|NN SV *const b
I	|I32	|sv_i_ncmp_desc	|NN SV *const a|NN SV *const b
I	|I32	|amagic_ncmp	|NN SV *const a|NN SV *const b
I	|I32	|amagic_ncmp_desc	|NN SV *const a|NN SV *const b
I	|I32	|amagic_i_ncmp	|NN SV *const a|NN SV *const b
I	|I32	|amagic_i_ncmp_desc	|NN SV *const a|NN SV *const b
I	|I32	|amagic_cmp	|NN SV *const str1|NN SV *const str2
I	|I32	|amagic_cmp_desc	|NN SV *const str1|NN SV *const str2
I	|I32	|cmp_desc	|NN SV *const str1|NN SV *const str2
#  ifdef USE_LOCALE_COLLATE
I	|I32	|amagic_cmp_locale     |NN SV *const str1|NN SV *const str2
I	|I32	|amagic_cmp_locale_desc|NN SV *const str1|NN SV *const str2
I	|I32	|cmp_locale_desc|NN SV *const str1|NN SV *const str2
#  endif
S	|I32	|sortcv		|NN SV *const a|NN SV *const b
S	|I32	|sortcv_xsub	|NN SV *const a|NN SV *const b
S	|I32	|sortcv_stacked	|NN SV *const a|NN SV *const b
I	|void	|sortsv_flags_impl	|NULLOK SV** array|size_t num_elts|NN SVCOMPARE_t cmp|U32 flags
#endif

#if defined(PERL_IN_PP_SYS_C)
S	|OP*	|doform		|NN CV *cv|NN GV *gv|NULLOK OP *retop
#  if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
SR	|int	|dooneliner	|NN const char *cmd|NN const char *filename
#  endif
S	|SV *	|space_join_names_mortal|NULLOK char *const *array
#endif
Fp	|OP *	|tied_method|NN SV *methname|NN SV **sp \
				|NN SV *const sv|NN const MAGIC *const mg \
				|const U32 flags|U32 argc|...

#if defined(PERL_IN_DUMP_C)
S	|CV*	|deb_curcv	|I32 ix
S	|void	|debprof	|NN const OP *o
S	|UV	|sequence_num	|NULLOK const OP *o
S	|SV*	|pm_description	|NN const PMOP *pm
#endif

#if defined(PERL_IN_SCOPE_C)
S	|SV*	|save_scalar_at	|NN SV **sptr|const U32 flags
#endif

#if defined(PERL_IN_GV_C) || defined(PERL_IN_SV_C) || defined(PERL_IN_PAD_C) || defined(PERL_IN_OP_C)
: Used in gv.c
po	|void	|sv_add_backref	|NN SV *const tsv|NN SV *const sv
#endif

#if defined(PERL_IN_HV_C) || defined(PERL_IN_MG_C) || defined(PERL_IN_SV_C)
: Used in hv.c and mg.c
pox	|void	|sv_kill_backrefs	|NN SV *const sv|NULLOK AV *const av
#endif

#if defined(PERL_IN_SV_C) || defined (PERL_IN_OP_C)
pR	|SV *	|varname	|NULLOK const GV *const gv|const char gvtype \
				|PADOFFSET targ|NULLOK const SV *const keyname \
				|SSize_t aindex|int subscript_type
#endif

pX	|void	|sv_del_backref	|NN SV *const tsv|NN SV *const sv
#if defined(PERL_IN_SV_C)
TiR	|char *	|uiv_2buf	|NN char *const buf|const IV iv|UV uv|const int is_uv|NN char **const peob
i	|void	|sv_unglob	|NN SV *const sv|U32 flags
S	|const char *|sv_display	|NN SV *const sv|NN char *tmpbuf|STRLEN tmpbuf_size
S	|void	|not_a_number	|NN SV *const sv
S	|void	|not_incrementable	|NN SV *const sv
S	|I32	|visit		|NN SVFUNC_t f|const U32 flags|const U32 mask
#  ifdef DEBUGGING
S	|void	|del_sv	|NN SV *p
#  endif
#  if !defined(NV_PRESERVES_UV)
#    ifdef DEBUGGING
S	|int	|sv_2iuv_non_preserve	|NN SV *const sv|I32 numtype
#    else
S	|int	|sv_2iuv_non_preserve	|NN SV *const sv
#    endif
#  endif
SR	|STRLEN	|expect_number	|NN const char **const pattern
ST	|STRLEN	|sv_pos_u2b_forwards|NN const U8 *const start \
		|NN const U8 *const send|NN STRLEN *const uoffset \
		|NN bool *const at_end|NN bool *canonical_position
ST	|STRLEN	|sv_pos_u2b_midway|NN const U8 *const start \
		|NN const U8 *send|STRLEN uoffset|const STRLEN uend
S	|STRLEN	|sv_pos_u2b_cached|NN SV *const sv|NN MAGIC **const mgp \
		|NN const U8 *const start|NN const U8 *const send \
		|STRLEN uoffset|STRLEN uoffset0|STRLEN boffset0
S	|void	|utf8_mg_len_cache_update|NN SV *const sv|NN MAGIC **const mgp \
		|const STRLEN ulen
S	|void	|utf8_mg_pos_cache_update|NN SV *const sv|NN MAGIC **const mgp \
		|const STRLEN byte|const STRLEN utf8|const STRLEN blen
S	|STRLEN	|sv_pos_b2u_midway|NN const U8 *const s|NN const U8 *const target \
		|NN const U8 *end|STRLEN endu
S	|void	|assert_uft8_cache_coherent|NN const char *const func \
		|STRLEN from_cache|STRLEN real|NN SV *const sv
ST	|char *	|F0convert	|NV nv|NN char *const endbuf|NN STRLEN *const len
Cp	|SV *	|more_sv
S	|bool	|sv_2iuv_common	|NN SV *const sv
S	|void	|glob_assign_glob|NN SV *const dsv|NN SV *const ssv \
		|const int dtype
SRT	|PTR_TBL_ENT_t *|ptr_table_find|NN PTR_TBL_t *const tbl|NULLOK const void *const sv
S	|void	|anonymise_cv_maybe	|NN GV *gv|NN CV *cv
#endif

: Used in sv.c and hv.c
Cpo	|void *	|more_bodies	|const svtype sv_type|const size_t body_size \
				|const size_t arena_size
EXpR	|SV*	|get_and_check_backslash_N_name|NN const char* s	\
				|NN const char* e			\
				|const bool is_utf8			\
				|NN const char** error_msg
EXpR	|HV*	|load_charnames	|NN SV * char_name			\
				|NN const char * context		\
				|const STRLEN context_len		\
				|NN const char ** error_msg

: For use ONLY in B::Hooks::Parser, by special dispensation
EXpxR	|char*	|scan_str	|NN char *start|int keep_quoted \
				|int keep_delims|int re_reparse \
				|NULLOK char **delimp
EXpx	|char*	|scan_word	|NN char *s|NN char *dest|STRLEN destlen \
				|int allow_package|NN STRLEN *slp
EXpxR	|char*	|skipspace_flags|NN char *s|U32 flags
#if defined(PERL_IN_TOKE_C)
S	|void	|check_uni
S	|void	|force_next	|I32 type
S	|char*	|force_version	|NN char *s|int guessing
S	|char*	|force_strict_version	|NN char *s
S	|char*	|force_word	|NN char *start|int token|int check_keyword \
				|int allow_pack
S	|SV*	|tokeq		|NN SV *sv
SR	|char*	|scan_const	|NN char *start
SR	|SV*	|get_and_check_backslash_N_name_wrapper|NN const char* s \
				|NN const char* const e
SR	|char*	|scan_formline	|NN char *s
SR	|char*	|scan_heredoc	|NN char *s
S	|char*	|scan_ident	|NN char *s|NN char *dest	\
				|STRLEN destlen|I32 ck_uni
SR	|char*	|scan_inputsymbol|NN char *start
SR	|char*	|scan_pat	|NN char *start|I32 type
SR	|char*	|scan_subst	|NN char *start
SR	|char*	|scan_trans	|NN char *start
S	|void	|update_debugger_info|NULLOK SV *orig_sv \
				|NULLOK const char *const buf|STRLEN len
SR	|char*	|swallow_bom	|NN U8 *s
#ifndef PERL_NO_UTF16_FILTER
S	|I32	|utf16_textfilter|int idx|NN SV *sv|int maxlen
S	|U8*	|add_utf16_textfilter|NN U8 *const s|bool reversed
#endif
S	|void	|checkcomma	|NN const char *s|NN const char *name \
				|NN const char *what
S	|void	|force_ident	|NN const char *s|int kind
S	|void	|force_ident_maybe_lex|char pit
S	|void	|incline	|NN const char *s|NN const char *end
S	|int	|intuit_method	|NN char *s|NULLOK SV *ioname|NULLOK CV *cv
S	|int	|intuit_more	|NN char *s|NN char *e
S	|I32	|lop		|I32 f|U8 x|NN char *s
rS	|void	|missingterm	|NULLOK char *s|STRLEN len
S	|void	|no_op		|NN const char *const what|NULLOK char *s
S	|int	|pending_ident
SR	|I32	|sublex_done
SR	|I32	|sublex_push
SR	|I32	|sublex_start
SR	|char *	|filter_gets	|NN SV *sv|STRLEN append
SR	|HV *	|find_in_my_stash|NN const char *pkgname|STRLEN len
SR	|char *	|tokenize_use	|int is_use|NN char *s
So	|SV*	|new_constant	|NULLOK const char *s|STRLEN len	    \
				|NN const char *key|STRLEN keylen|NN SV *sv \
				|NULLOK SV *pv|NULLOK const char *type	    \
				|STRLEN typelen				    \
				|NULLOK const char ** error_msg
S	|int	|ao		|int toketype
S	|void|parse_ident|NN char **s|NN char **d \
                     |NN char * const e|int allow_package \
				|bool is_utf8|bool check_dollar \
				|bool tick_warn
#  if defined(PERL_CR_FILTER)
S	|I32	|cr_textfilter	|int idx|NULLOK SV *sv|int maxlen
S	|void	|strip_return	|NN SV *sv
#  endif
#  if defined(DEBUGGING)
S	|int	|tokereport	|I32 rv|NN const YYSTYPE* lvalp
Sf	|void	|printbuf	|NN const char *const fmt|NN const char *const s
#  endif
#endif
EdXxp	|bool	|validate_proto	|NN SV *name|NULLOK SV *proto|bool warn \
		|bool curstash

#if defined(PERL_IN_UNIVERSAL_C)
SG	|bool	|isa_lookup	|NULLOK HV *stash|NULLOK SV *namesv|NULLOK const char * name \
                                        |STRLEN len|U32 flags
SG   |bool   |sv_derived_from_svpvn  |NULLOK SV *sv			\
				     |NULLOK SV *namesv			\
				     |NULLOK const char * name		\
				     |const STRLEN len			\
				     |U32 flags
#endif

#if defined(PERL_IN_LOCALE_C)
#  ifdef USE_LOCALE
ST	|const char*|category_name |const int category
S	|const char*|switch_category_locale_to_template|const int switch_category|const int template_category|NULLOK const char * template_locale
S	|void	|restore_switched_locale|const int category|NULLOK const char * const original_locale
#  endif
#  ifdef HAS_NL_LANGINFO
ST	|const char*|my_nl_langinfo|const nl_item item|bool toggle
#  else
ST	|const char*|my_nl_langinfo|const int item|bool toggle
#  endif
iTR	|const char *|save_to_buffer|NULLOK const char * string	\
				    |NULLOK char **buf		\
				    |NN Size_t *buf_size	\
				    |const Size_t offset
#  if defined(USE_LOCALE)
S	|char*	|stdize_locale	|NN char* locs
S	|void	|new_collate	|NULLOK const char* newcoll
S	|void	|new_ctype	|NN const char* newctype
S	|void	|set_numeric_radix|const bool use_locale
S	|void	|new_numeric	|NULLOK const char* newnum
#    ifdef USE_POSIX_2008_LOCALE
ST	|const char*|emulate_setlocale|const int category		\
				    |NULLOK const char* locale		\
				    |unsigned int index			\
				    |const bool is_index_valid
#    endif
#    ifdef WIN32
S	|char*	|win32_setlocale|int category|NULLOK const char* locale
#    endif
#    ifdef DEBUGGING
S	|void	|print_collxfrm_input_and_return		\
			    |NN const char * const s		\
			    |NN const char * const e		\
			    |NULLOK const STRLEN * const xlen	\
			    |const bool is_utf8
S	|void	|print_bytes_for_locale	|NN const char * const s	\
					|NN const char * const e	\
					|const bool is_utf8
STR	|char *	|setlocale_debug_string	|const int category		    \
					|NULLOK const char* const locale    \
					|NULLOK const char* const retval
#    endif
#  endif
#endif

#if        defined(USE_LOCALE)		\
    && (   defined(PERL_IN_LOCALE_C)	\
        || defined(PERL_IN_MG_C)	\
	|| defined (PERL_EXT_POSIX)	\
	|| defined (PERL_EXT_LANGINFO))
Cp	|bool	|_is_cur_LC_category_utf8|int category
#endif


#if defined(PERL_IN_UTIL_C)
S	|SV*	|mess_alloc
S	|SV *	|with_queued_errors|NN SV *ex
S	|bool	|invoke_exception_hook|NULLOK SV *ex|bool warn
#  if defined(PERL_MEM_LOG) && !defined(PERL_MEM_LOG_NOIMPL)
ST	|void	|mem_log_common	|enum mem_log_type mlt|const UV n|const UV typesize \
				|NN const char *type_name|NULLOK const SV *sv \
				|Malloc_t oldalloc|Malloc_t newalloc \
				|NN const char *filename|const int linenumber \
				|NN const char *funcname
#  endif
#endif

#if defined(PERL_MEM_LOG)
pT	|Malloc_t	|mem_log_alloc	|const UV nconst|UV typesize|NN const char *type_name|Malloc_t newalloc|NN const char *filename|const int linenumber|NN const char *funcname
pT	|Malloc_t	|mem_log_realloc	|const UV n|const UV typesize|NN const char *type_name|Malloc_t oldalloc|Malloc_t newalloc|NN const char *filename|const int linenumber|NN const char *funcname
pT	|Malloc_t	|mem_log_free	|Malloc_t oldalloc|NN const char *filename|const int linenumber|NN const char *funcname
#endif

#if defined(PERL_IN_UTF8_C)
SR	|HV *	|new_msg_hv |NN const char * const message		    \
			    |U32 categories				    \
			    |U32 flag
SR	|UV	|check_locale_boundary_crossing				    \
		|NN const U8* const p					    \
		|const UV result					    \
		|NN U8* const ustrp					    \
		|NN STRLEN *lenp
iR	|bool	|is_utf8_common	|NN const U8 *const p			    \
				|NN const U8 *const e			    \
				|NULLOK SV* const invlist
#endif

EXiTp	|void	|append_utf8_from_native_byte|const U8 byte|NN U8** dest

Apd	|void	|sv_set_undef	|NN SV *sv
Apd	|void	|sv_setsv_flags	|NN SV *dsv|NULLOK SV *ssv|const I32 flags
Apd	|void	|sv_catpvn_flags|NN SV *const dsv|NN const char *sstr|const STRLEN len \
				|const I32 flags
Apd	|void	|sv_catpv_flags	|NN SV *dsv|NN const char *sstr \
				|const I32 flags
Apd	|void	|sv_catsv_flags	|NN SV *const dsv|NULLOK SV *const sstr|const I32 flags
Amd	|STRLEN	|sv_utf8_upgrade_flags|NN SV *const sv|const I32 flags
Adp	|STRLEN	|sv_utf8_upgrade_flags_grow|NN SV *const sv|const I32 flags|STRLEN extra
Apd	|char*	|sv_pvn_force_flags|NN SV *const sv|NULLOK STRLEN *const lp|const U32 flags
AdpMb	|void	|sv_copypv	|NN SV *const dsv|NN SV *const ssv
Amd	|void	|sv_copypv_nomg	|NN SV *const dsv|NN SV *const ssv
Apd	|void	|sv_copypv_flags	|NN SV *const dsv|NN SV *const ssv|const I32 flags
Cpo	|char*	|my_atof2	|NN const char *orig|NN NV* value
Cp	|char*	|my_atof3	|NN const char *orig|NN NV* value|const STRLEN len
ApT	|int	|my_socketpair	|int family|int type|int protocol|int fd[2]
ApT	|int	|my_dirfd	|NULLOK DIR* dir
#ifdef PERL_ANY_COW
: Used in regexec.c
pxXE	|SV*	|sv_setsv_cow	|NULLOK SV* dsv|NN SV* ssv
#endif

Aop	|const char *|PerlIO_context_layers|NULLOK const char *mode

#if defined(USE_PERLIO)
Apdh	|int	|PerlIO_close		|NULLOK PerlIO *f
Ap	|int	|PerlIO_fill		|NULLOK PerlIO *f
Apdh	|int	|PerlIO_fileno		|NULLOK PerlIO *f
Apdh	|int	|PerlIO_eof		|NULLOK PerlIO *f
Apdh	|int	|PerlIO_error		|NULLOK PerlIO *f
Apdh	|int	|PerlIO_flush		|NULLOK PerlIO *f
Apdh	|void	|PerlIO_clearerr	|NULLOK PerlIO *f
Apdh	|void	|PerlIO_set_cnt		|NULLOK PerlIO *f|SSize_t cnt
Apdh	|void	|PerlIO_set_ptrcnt	|NULLOK PerlIO *f|NULLOK STDCHAR *ptr \
					|SSize_t cnt
Apdh	|void	|PerlIO_setlinebuf	|NULLOK PerlIO *f
Apdh	|SSize_t|PerlIO_read		|NULLOK PerlIO *f|NN void *vbuf \
					|Size_t count
Apdh	|SSize_t|PerlIO_write		|NULLOK PerlIO *f|NN const void *vbuf \
					|Size_t count
Ap	|SSize_t|PerlIO_unread		|NULLOK PerlIO *f|NN const void *vbuf \
					|Size_t count
Apdh	|Off_t	|PerlIO_tell		|NULLOK PerlIO *f
Apdh	|int	|PerlIO_seek		|NULLOK PerlIO *f|Off_t offset|int whence
Xp	|void	|PerlIO_save_errno	|NULLOK PerlIO *f
Xp	|void	|PerlIO_restore_errno	|NULLOK PerlIO *f

Apdh	|STDCHAR *|PerlIO_get_base	|NULLOK PerlIO *f
Apdh	|STDCHAR *|PerlIO_get_ptr	|NULLOK PerlIO *f
ApRdh	|SSize_t	  |PerlIO_get_bufsiz	|NULLOK PerlIO *f
ApRdh	|SSize_t	  |PerlIO_get_cnt	|NULLOK PerlIO *f

ApRdh	|PerlIO *|PerlIO_stdin
ApRdh	|PerlIO *|PerlIO_stdout
ApRdh	|PerlIO *|PerlIO_stderr
#endif /* USE_PERLIO */

: Only used in dump.c
p	|void	|deb_stack_all
#if defined(PERL_IN_DEB_C)
S	|void	|deb_stack_n	|NN SV** stack_base|I32 stack_min \
				|I32 stack_max|I32 mark_min|I32 mark_max
#endif

: pad API
ApdR	|PADLIST*|pad_new	|int flags
#ifdef DEBUGGING
pTX	|void|set_padlist| NN CV * cv | NULLOK PADLIST * padlist
#endif
#if defined(PERL_IN_PAD_C)
Sd	|PADOFFSET|pad_alloc_name|NN PADNAME *name|U32 flags \
				|NULLOK HV *typestash|NULLOK HV *ourstash
#endif
Apd	|PADOFFSET|pad_add_name_pvn|NN const char *namepv|STRLEN namelen\
				|U32 flags|NULLOK HV *typestash\
				|NULLOK HV *ourstash
Apd	|PADOFFSET|pad_add_name_pv|NN const char *name\
				|const U32 flags|NULLOK HV *typestash\
				|NULLOK HV *ourstash
Apd	|PADOFFSET|pad_add_name_sv|NN SV *name\
				|U32 flags|NULLOK HV *typestash\
				|NULLOK HV *ourstash
Axpd	|PADOFFSET|pad_alloc	|I32 optype|U32 tmptype
Apd	|PADOFFSET|pad_add_anon	|NN CV* func|I32 optype
p	|void	|pad_add_weakref|NN CV* func
#if defined(PERL_IN_PAD_C)
Sd	|void	|pad_check_dup	|NN PADNAME *name|U32 flags \
				|NULLOK const HV *ourstash
#endif
Apd	|PADOFFSET|pad_findmy_pvn|NN const char* namepv|STRLEN namelen|U32 flags
Apd	|PADOFFSET|pad_findmy_pv|NN const char* name|U32 flags
Apd	|PADOFFSET|pad_findmy_sv|NN SV* name|U32 flags
ApdD	|PADOFFSET|find_rundefsvoffset	|
Apd	|SV*	|find_rundefsv	|
#if defined(PERL_IN_PAD_C)
Sd	|PADOFFSET|pad_findlex	|NN const char *namepv|STRLEN namelen|U32 flags \
				|NN const CV* cv|U32 seq|int warn \
				|NULLOK SV** out_capture \
				|NN PADNAME** out_name|NN int *out_flags
#endif
#ifdef DEBUGGING
Cpd	|SV*	|pad_sv		|PADOFFSET po
Cpd	|void	|pad_setsv	|PADOFFSET po|NN SV* sv
#endif
pd	|void	|pad_block_start|int full
Apd	|U32	|intro_my
pd	|OP *	|pad_leavemy
pd	|void	|pad_swipe	|PADOFFSET po|bool refadjust
#if defined(PERL_IN_PAD_C)
Sd	|void	|pad_reset
#endif
Axpd	|void	|pad_tidy	|padtidy_type type
pd	|void	|pad_free	|PADOFFSET po
pd	|void	|do_dump_pad	|I32 level|NN PerlIO *file|NULLOK PADLIST *padlist|int full
#if defined(PERL_IN_PAD_C)
#  if defined(DEBUGGING)
Sd	|void	|cv_dump	|NN const CV *cv|NN const char *title
#  endif
#endif
#if defined(PERL_IN_PAD_C) || defined(PERL_IN_OP_C)
iT	|bool	|PadnameIN_SCOPE|NN const PADNAME * const pn|const U32 seq
#endif
Apd	|CV*	|cv_clone	|NN CV* proto
p	|CV*	|cv_clone_into	|NN CV* proto|NN CV *target
pd	|void	|pad_fixup_inner_anons|NN PADLIST *padlist|NN CV *old_cv|NN CV *new_cv
pdX	|void	|pad_push	|NN PADLIST *padlist|int depth
ApbdDR	|HV*	|pad_compname_type|const PADOFFSET po
AxpdRT	|PADNAME *|padnamelist_fetch|NN PADNAMELIST *pnl|SSize_t key
Xop	|void	|padnamelist_free|NN PADNAMELIST *pnl
Axpd	|PADNAME **|padnamelist_store|NN PADNAMELIST *pnl|SSize_t key \
				     |NULLOK PADNAME *val
Xop	|void	|padname_free	|NN PADNAME *pn
#if defined(USE_ITHREADS)
pdR	|PADNAME *|padname_dup	|NN PADNAME *src|NN CLONE_PARAMS *param
pdR	|PADNAMELIST *|padnamelist_dup|NN PADNAMELIST *srcpad \
				      |NN CLONE_PARAMS *param
pdR	|PADLIST *|padlist_dup	|NN PADLIST *srcpad \
				|NN CLONE_PARAMS *param
#endif
p	|PAD **	|padlist_store	|NN PADLIST *padlist|I32 key \
				|NULLOK PAD *val

ApdR	|CV*	|find_runcv	|NULLOK U32 *db_seqp
pR	|CV*	|find_runcv_where|U8 cond|IV arg \
				 |NULLOK U32 *db_seqp
: Only used in perl.c
p	|void	|free_tied_hv_pool
#if defined(DEBUGGING)
: Used in mg.c
pR	|int	|get_debug_opts	|NN const char **s|bool givehelp
#endif
Ap	|void	|save_set_svflags|NN SV *sv|U32 mask|U32 val
#ifdef DEBUGGING
Apod	|void	|hv_assert	|NN HV *hv
#endif

ApdR	|SV*	|hv_scalar	|NN HV *hv
p	|void	|hv_pushkv	|NN HV *hv|U32 flags
ApdRx	|SV*	|hv_bucket_ratio|NN HV *hv
ApoR	|I32*	|hv_riter_p	|NN HV *hv
ApoR	|HE**	|hv_eiter_p	|NN HV *hv
Apo	|void	|hv_riter_set	|NN HV *hv|I32 riter
Apo	|void	|hv_eiter_set	|NN HV *hv|NULLOK HE *eiter
Ap	|void   |hv_rand_set    |NN HV *hv|U32 new_xhv_rand
Ap	|void	|hv_name_set	|NN HV *hv|NULLOK const char *name|U32 len|U32 flags
pd	|void	|hv_ename_add	|NN HV *hv|NN const char *name|U32 len \
				|U32 flags
pd	|void	|hv_ename_delete|NN HV *hv|NN const char *name|U32 len \
				|U32 flags
: Used in dump.c and hv.c
pox	|AV**	|hv_backreferences_p	|NN HV *hv
#if defined(PERL_IN_DUMP_C) || defined(PERL_IN_HV_C) || defined(PERL_IN_SV_C) || defined(PERL_IN_SCOPE_C)
pox	|void	|hv_kill_backrefs	|NN HV *hv
#endif
Apd	|void	|hv_clear_placeholders	|NN HV *hv
XpoR	|SSize_t*|hv_placeholders_p	|NN HV *hv
ApoR	|I32	|hv_placeholders_get	|NN const HV *hv
Apo	|void	|hv_placeholders_set	|NN HV *hv|I32 ph

: This is indirectly referenced by globals.c. This is somewhat annoying.
p	|SV*	|magic_scalarpack|NN HV *hv|NN MAGIC *mg

#if defined(PERL_IN_SV_C)
S	|SV *	|find_hash_subscript|NULLOK const HV *const hv \
		|NN const SV *const val
S	|SSize_t|find_array_subscript|NULLOK const AV *const av \
		|NN const SV *const val
Sxd	|SV*	|find_uninit_var|NULLOK const OP *const obase \
		|NULLOK const SV *const uninit_sv|bool match \
		|NN const char **desc_p
#endif

Adp	|GV*	|gv_fetchpvn_flags|NN const char* name|STRLEN len|I32 flags|const svtype sv_type
Adp	|GV*	|gv_fetchsv|NN SV *name|I32 flags|const svtype sv_type

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
: Used in sv.c
p	|void	|dump_sv_child	|NN SV *sv
#endif

#ifdef PERL_DONT_CREATE_GVSV
ApbM	|GV*	|gv_SVadd	|NULLOK GV *gv
#endif
#if defined(PERL_IN_UTIL_C)
S	|bool	|ckwarn_common	|U32 w
#endif
CpoP	|bool	|ckwarn		|U32 w
CpoP	|bool	|ckwarn_d	|U32 w
: FIXME - exported for ByteLoader - public or private?
XEopxR	|STRLEN *|new_warnings_bitfield|NULLOK STRLEN *buffer \
				|NN const char *const bits|STRLEN size

AMpTdf	|int	|my_snprintf	|NN char *buffer|const Size_t len|NN const char *format|...
AMpTd	|int	|my_vsnprintf	|NN char *buffer|const Size_t len|NN const char *format|va_list ap
#ifdef USE_QUADMATH
pTd	|bool	|quadmath_format_valid|NN const char* format
pTd	|bool|quadmath_format_needed|NN const char* format
#endif

: Used in mg.c, sv.c
pe	|void	|my_clearenv

#ifdef MULTIPLICITY
Apo	|void*	|my_cxt_init	|NN int *indexp|size_t size
#endif
#if defined(PERL_IN_UTIL_C)
So	|void	|xs_version_bootcheck|U32 items|U32 ax|NN const char *xs_p \
				|STRLEN xs_len
#endif
FXpoT	|I32	|xs_handshake	|const U32 key|NN void * v_my_perl\
				|NN const char * file| ...
Xp	|void	|xs_boot_epilog	|const I32 ax
#ifndef HAS_STRLCAT
ApTd	|Size_t	|my_strlcat	|NULLOK char *dst|NULLOK const char *src|Size_t size
#endif

#ifndef HAS_STRLCPY
ApTd	|Size_t |my_strlcpy     |NULLOK char *dst|NULLOK const char *src|Size_t size
#endif

#ifndef HAS_STRNLEN
AipTd	|Size_t |my_strnlen     |NN const char *str|Size_t maxlen
#endif

#ifndef HAS_MKOSTEMP
pTo	|int	|my_mkostemp	|NN char *templte|int flags
#endif
#ifndef HAS_MKSTEMP
pTo	|int	|my_mkstemp	|NN char *templte
#endif

APpdT	|bool	|isinfnan	|NV nv
pd	|bool	|isinfnansv	|NN SV *sv

#if !defined(HAS_SIGNBIT)
AxdToP	|int	|Perl_signbit	|NV f
#endif

: Used by B
XExop	|void	|emulate_cop_io	|NN const COP *const c|NN SV *const sv
: Used by SvRX and SvRXOK
XExop	|REGEXP *|get_re_arg|NULLOK SV *sv

Coph	|SV*	|mro_get_private_data|NN struct mro_meta *const smeta \
				     |NN const struct mro_alg *const which
Aopdh	|SV*	|mro_set_private_data|NN struct mro_meta *const smeta \
				     |NN const struct mro_alg *const which \
				     |NN SV *const data
Aop	|const struct mro_alg *|mro_get_from_name|NN SV *name
Aopd	|void	|mro_register	|NN const struct mro_alg *mro
Aop	|void	|mro_set_mro	|NN struct mro_meta *const meta \
				|NN SV *const name
: Used in HvMROMETA(), which is public.
Xpo	|struct mro_meta*	|mro_meta_init	|NN HV* stash
#if defined(USE_ITHREADS)
: Only used in sv.c
p	|struct mro_meta*	|mro_meta_dup	|NN struct mro_meta* smeta|NN CLONE_PARAMS* param
#endif
Apd	|AV*	|mro_get_linear_isa|NN HV* stash
#if defined(PERL_IN_MRO_C)
Sd	|AV*	|mro_get_linear_isa_dfs|NN HV* stash|U32 level
S	|void	|mro_clean_isarev|NN HV * const isa   \
				 |NN const char * const name \
				 |const STRLEN len \
				 |NULLOK HV * const exceptions \
				 |U32 hash|U32 flags
S	|void	|mro_gather_and_rename|NN HV * const stashes \
				      |NN HV * const seen_stashes \
				      |NULLOK HV *stash \
				      |NULLOK HV *oldstash \
				      |NN SV *namesv
#endif
: Used in hv.c, mg.c, pp.c, sv.c
pd	|void   |mro_isa_changed_in|NN HV* stash
Apd	|void	|mro_method_changed_in	|NN HV* stash
pde	|void	|mro_package_moved	|NULLOK HV * const stash|NULLOK HV * const oldstash|NN const GV * const gv|U32 flags
: Only used in perl.c
p	|void   |boot_core_mro
CpoT	|void	|sys_init	|NN int* argc|NN char*** argv
CpoT	|void	|sys_init3	|NN int* argc|NN char*** argv|NN char*** env
CpoT	|void	|sys_term
Apxd	|const char *|cop_fetch_label|NN COP *const cop \
		|NULLOK STRLEN *len|NULLOK U32 *flags
: Only used  in op.c and the perl compiler
Apxd	|void|cop_store_label \
		|NN COP *const cop|NN const char *label|STRLEN len|U32 flags

epo	|int	|keyword_plugin_standard|NN char* keyword_ptr|STRLEN keyword_len|NN OP** op_ptr

#if defined(USE_ITHREADS)
#  if defined(PERL_IN_SV_C)
S	|void	|unreferenced_to_tmp_stack|NN AV *const unreferenced
#  endif
ARTop	|CLONE_PARAMS *|clone_params_new|NN PerlInterpreter *const from \
		|NN PerlInterpreter *const to
ATop	|void	|clone_params_del|NN CLONE_PARAMS *param
#endif

: Used in perl.c and toke.c
Fop	|void	|populate_isa	|NN const char *name|STRLEN len|...

: Some static inline functions need predeclaration because they are used
: inside other static inline functions.
#if defined(PERL_CORE) || defined (PERL_EXT)
Ei	|STRLEN	|sv_or_pv_pos_u2b|NN SV *sv|NN const char *pv|STRLEN pos \
				 |NULLOK STRLEN *lenp
#endif

Ap	|void	|clear_defarray	|NN AV* av|bool abandon

Apx	|void	|leave_adjust_stacks|NN SV **from_sp|NN SV **to_sp \
                |U8 gimme|int filter

#ifndef PERL_NO_INLINE_FUNCTIONS
Cixp	|U8	|gimme_V         |
Cixp	|PERL_CONTEXT *	|cx_pushblock|U8 type|U8 gimme|NN SV** sp|I32 saveix
Cixp	|void	|cx_popblock|NN PERL_CONTEXT *cx
Cixp	|void	|cx_topblock|NN PERL_CONTEXT *cx
Cixp	|void	|cx_pushsub      |NN PERL_CONTEXT *cx|NN CV *cv \
				 |NULLOK OP *retop|bool hasargs
Cixp	|void	|cx_popsub_common|NN PERL_CONTEXT *cx
Cixp	|void	|cx_popsub_args  |NN PERL_CONTEXT *cx
Cixp	|void	|cx_popsub       |NN PERL_CONTEXT *cx
Cixp	|void	|cx_pushformat   |NN PERL_CONTEXT *cx|NN CV *cv \
				 |NULLOK OP *retop|NULLOK GV *gv
Cixp	|void	|cx_popformat    |NN PERL_CONTEXT *cx
Cixp	|void	|cx_pusheval     |NN PERL_CONTEXT *cx \
				 |NULLOK OP *retop|NULLOK SV *namesv
Cixp	|void	|cx_pushtry      |NN PERL_CONTEXT *cx|NULLOK OP *retop
Cixp	|void	|cx_popeval      |NN PERL_CONTEXT *cx
Cixp	|void	|cx_pushloop_plain|NN PERL_CONTEXT *cx
Cixp	|void	|cx_pushloop_for |NN PERL_CONTEXT *cx \
				 |NN void *itervarp|NULLOK SV *itersave
Cixp	|void	|cx_poploop      |NN PERL_CONTEXT *cx
Cixp	|void	|cx_pushwhen     |NN PERL_CONTEXT *cx
Cixp	|void	|cx_popwhen      |NN PERL_CONTEXT *cx
Cixp	|void	|cx_pushgiven    |NN PERL_CONTEXT *cx|NULLOK SV *orig_defsv
Cixp	|void	|cx_popgiven     |NN PERL_CONTEXT *cx
#endif

#ifdef USE_DTRACE
XEop	|void   |dtrace_probe_call |NN CV *cv|bool is_call
XEop	|void   |dtrace_probe_load |NN const char *name|bool is_loading
XEop	|void   |dtrace_probe_op   |NN const OP *op
XEop	|void   |dtrace_probe_phase|enum perl_phase phase
#endif

XEop	|STRLEN*|dup_warnings	|NULLOK STRLEN* warnings

#ifndef USE_ITHREADS
Amd	|void	|CopFILEGV_set	|NN COP * c|NN GV * gv
#endif

Amd|const char *const|phase_name|enum perl_phase

: ex: set ts=8 sts=4 sw=4 noet:
