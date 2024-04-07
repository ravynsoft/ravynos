/* need to replace pregcomp et al, so enable that */
#ifndef PERL_IN_XSUB_RE
#  define PERL_IN_XSUB_RE
#endif
/* need access to debugger hooks */
#if defined(PERL_EXT_RE_DEBUG) && !defined(DEBUGGING)
#  define DEBUGGING
#  define DEBUGGING_RE_ONLY
#endif

/* We *really* need to overwrite these symbols: */
#define Perl_regexec_flags      my_regexec
#define Perl_regdump            my_regdump
#define Perl_regprop            my_regprop
#define Perl_re_intuit_start    my_re_intuit_start
#define Perl_re_compile         my_re_compile
#define Perl_re_op_compile      my_re_op_compile
#define Perl_regfree_internal   my_regfree
#define Perl_re_intuit_string   my_re_intuit_string
#define Perl_regdupe_internal   my_regdupe
#define Perl_reg_numbered_buff_fetch  my_reg_numbered_buff_fetch
#define Perl_reg_numbered_buff_store  my_reg_numbered_buff_store
#define Perl_reg_numbered_buff_length  my_reg_numbered_buff_length
#define Perl_reg_named_buff      my_reg_named_buff
#define Perl_reg_named_buff_iter my_reg_named_buff_iter
#define Perl_reg_named_buff_fetch    my_reg_named_buff_fetch    
#define Perl_reg_named_buff_exists   my_reg_named_buff_exists  
#define Perl_reg_named_buff_firstkey my_reg_named_buff_firstkey
#define Perl_reg_named_buff_nextkey  my_reg_named_buff_nextkey 
#define Perl_reg_named_buff_scalar   my_reg_named_buff_scalar  
#define Perl_reg_named_buff_all      my_reg_named_buff_all     
#define Perl_reg_qr_package        my_reg_qr_package

#define PERL_NO_GET_CONTEXT

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
