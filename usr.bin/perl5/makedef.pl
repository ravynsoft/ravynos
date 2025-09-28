#./perl -w
#
# Create the export list for perl.
#
# Needed by WIN32 and OS/2 for creating perl.dll,
# and by AIX for creating libperl.a when -Duseshrplib is in effect,
# and by VMS for creating perlshr.exe.
#
# Reads from information stored in
#
#    %Config::Config (ie config.sh)
#    config.h
#    embed.fnc
#    globvar.sym
#    intrpvar.h
#    miniperl.map (on OS/2)
#    perl5.def    (on OS/2; this is the old version of the file being made)
#    perlio.sym
#    perlvars.h
#    regen/opcodes
#
# plus long lists of function names hard-coded directly in this script.
#
# Writes the result to STDOUT.
#
# Normally this script is invoked from a makefile (e.g. win32/Makefile),
# which redirects STDOUT to a suitable file, such as:
#
#    perl5.def   OS/2
#    perldll.def Windows
#    perl.exp    AIX
#    makedef.lis VMS

use strict;
use Config;
use warnings;

my $fold;
my %ARGS;
my %define;

BEGIN {
    %ARGS = (CCTYPE => 'MSVC', TARG_DIR => '');

    sub process_cc_flags {
	foreach (map {split /\s+/, $_} @_) {
	    $define{$1} = $2 // 1 if /^-D(\w+)(?:=(.+))?/;
	}
    }

    while (@ARGV) {
	my $flag = shift;
	if ($flag =~ /^(?:CC_FLAGS=)?(-D\w.*)/) {
	    process_cc_flags($1);
	} elsif ($flag =~ /^(CCTYPE|FILETYPE|PLATFORM|TARG_DIR)=(.+)$/) {
	    $ARGS{$1} = $2;
	} elsif ($flag eq '--sort-fold') {
	    ++$fold;
	}
    }
    my @PLATFORM = qw(aix win32 os2 vms test);
    my %PLATFORM;
    @PLATFORM{@PLATFORM} = ();

    die "PLATFORM undefined, must be one of: @PLATFORM\n"
	unless defined $ARGS{PLATFORM};
    die "PLATFORM must be one of: @PLATFORM\n"
	unless exists $PLATFORM{$ARGS{PLATFORM}};
}

use constant PLATFORM => $ARGS{PLATFORM};

# This makes us able to use, e.g., $define{WIN32}, so you don't have to
# remember what things came from %ARGS.
$define{uc $ARGS{'PLATFORM'}} = 1;

require "./$ARGS{TARG_DIR}regen/embed_lib.pl";

# Is the following guard strictly necessary? Added during refactoring
# to keep the same behaviour when merging other code into here.
process_cc_flags(@Config{qw(ccflags optimize)})
    if PLATFORM ne 'win32';

# Add the compile-time options that miniperl was built with to %define.
# On Win32 these are not the same options as perl itself will be built
# with since miniperl is built with a canned config (one of the win32/
# config_H.*) and none of the BUILDOPT's that are set in the makefiles,
# but they do include some #define's that are hard-coded in various
# source files and header files and don't include any BUILDOPT's that
# the user might have chosen to disable because the canned configs are
# minimal configs that don't include any of those options.

my @options = sort(Config::bincompat_options(), Config::non_bincompat_options());
print STDERR "Options: (@options)\n" unless PLATFORM eq 'test';
$define{$_} = 1 foreach @options;

my %exportperlmalloc =
    (
       Perl_malloc		=>	"malloc",
       Perl_mfree		=>	"free",
       Perl_realloc		=>	"realloc",
       Perl_calloc		=>	"calloc",
    );

my $exportperlmalloc = PLATFORM eq 'os2';

my $config_h = 'config.h';
open(CFG, '<', $config_h) || die "Cannot open $config_h: $!\n";
while (<CFG>) {
    $define{$1} = 1 if /^\s*\#\s*define\s+(MYMALLOC|MULTIPLICITY
                                           |KILL_BY_SIGPRC
                                           |(?:PERL|USE|HAS)_\w+)\b/x;
}
close(CFG);

#==========================================================================
# perl.h logic duplication begins


if ($define{USE_ITHREADS}) {
    if (!$define{MULTIPLICITY}) {
        $define{MULTIPLICITY} = 1;
    }
}

$define{MULTIPLICITY} ||=
    $define{USE_ITHREADS} ||
    $define{PERL_IMPLICIT_CONTEXT} ;

if ($define{USE_ITHREADS} && ! $define{WIN32}) {
    $define{USE_REENTRANT_API} = 1;
}

if (! $define{NO_LOCALE}) {
    if ( ! $define{NO_POSIX_2008_LOCALE}
        && $define{HAS_NEWLOCALE}
        && $define{HAS_USELOCALE}
        && $define{HAS_DUPLOCALE}
        && $define{HAS_FREELOCALE})
    {
        $define{HAS_POSIX_2008_LOCALE} = 1;
        $define{USE_LOCALE} = 1;
    }
    elsif ($define{HAS_SETLOCALE}) {
        $define{USE_LOCALE} = 1;
    }
}

# https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering
my $cctype = $ARGS{CCTYPE} =~ s/MSVC//r;
if ($define{USE_ITHREADS} && ! $define{NO_LOCALE_THREADS}) {
    $define{USE_LOCALE_THREADS} = 1;
}

if (   $define{HAS_POSIX_2008_LOCALE}
    && (  ! $define{HAS_SETLOCALE} || (     $define{USE_LOCALE_THREADS}
                                       && ! $define{NO_POSIX_2008_LOCALE})))
{
    $define{USE_POSIX_2008_LOCALE} = 1;
}

if ($define{USE_LOCALE_THREADS} && ! $define{NO_THREAD_SAFE_LOCALE})
{
    if (    $define{USE_POSIX_2008_LOCALE}
        || ($define{WIN32} && (   $cctype !~ /\D/
                               && $cctype >= 80)))
    {
        $define{USE_THREAD_SAFE_LOCALE} = 1;
    }
}

if ($define{USE_POSIX_2008_LOCALE} && $define{HAS_QUERYLOCALE})
{
    $define{USE_QUERYLOCALE} = 1;

    # Don't need glibc only code from perl.h
}

if ($define{USE_POSIX_2008_LOCALE} && ! $define{USE_QUERYLOCALE})
{
    $define{USE_PL_CURLOCALES} = 1;
    $define{USE_PL_CUR_LC_ALL} = 1;
}

if ($define{WIN32} && $define{USE_THREAD_SAFE_LOCALE})
{
    $define{USE_PL_CUR_LC_ALL} = 1;

    if ($cctype < 140) {
        $define{TS_W32_BROKEN_LOCALECONV} = 1;
    }
}

if ($define{MULTIPLICITY} && (   $define{USE_POSIX_2008_LOCALE}
                              || (   $define{WIN32}
                                  && $define{USE_THREAD_SAFE_LOCALE})))
{
    $define{USE_PERL_SWITCH_LOCALE_CONTEXT} = 1;
}

# perl.h logic duplication ends
#==========================================================================

print STDERR "Defines: (" . join(' ', sort keys %define) . ")\n"
     unless PLATFORM eq 'test';

my $sym_ord = 0;
my %ordinal;

if (PLATFORM eq 'os2') {
    if (open my $fh, '<', 'perl5.def') {
      while (<$fh>) {
	last if /^\s*EXPORTS\b/;
      }
      while (<$fh>) {
	$ordinal{$1} = $2 if /^\s*"(\w+)"\s*(?:=\s*"\w+"\s*)?\@(\d+)\s*$/;
	# This allows skipping ordinals which were used in older versions
	$sym_ord = $1 if /^\s*;\s*LAST_ORDINAL\s*=\s*(\d+)\s*$/;
      }
      $sym_ord < $_ and $sym_ord = $_ for values %ordinal; # Take the max
    }
}

my %skip;
# All platforms export boot_DynaLoader unconditionally.
my %export = ( boot_DynaLoader => 1 );

# d_thread_local not perl_thread_local - see hints/darwin.sh
++$export{PL_current_context}
    if defined $Config{d_thread_local} && $define{USE_ITHREADS};

sub try_symbols {
    foreach my $symbol (@_) {
	++$export{$symbol} unless exists $skip{$symbol};
    }
}

sub readvar {
    # $hash is the hash that we're adding to. For one of our callers, it will
    # actually be the skip hash but that doesn't affect the intent of what
    # we're doing, as in that case we skip adding something to the skip hash
    # for the second time.

    my $file = $ARGS{TARG_DIR} . shift;
    my $hash = shift;
    my $proc = shift;
    open my $vars, '<', $file or die "Cannot open $file: $!\n";

    while (<$vars>) {
	# All symbols have a Perl_ prefix because that's what embed.h sticks
	# in front of them.  The A?I?S?C? is strictly speaking wrong.
	next unless /\bPERLVAR(A?I?S?C?)\(([IGT]),\s*(\w+)/;

	my $var = "PL_$3";
	my $symbol = $proc ? &$proc($1,$2,$3) : $var;
	++$hash->{$symbol} unless exists $skip{$var};
    }
}

if (PLATFORM ne 'os2') {
    ++$skip{$_} foreach qw(
		     PL_opsave
		     Perl_dump_fds
		     Perl_my_bcopy
		     Perl_my_bzero
		     Perl_my_chsize
		     Perl_my_htonl
		     Perl_my_memcmp
		     Perl_my_memset
		     Perl_my_ntohl
		     Perl_my_swap
			 );
    if (PLATFORM eq 'vms') {
	++$skip{PL_statusvalue_posix};
        # This is a wrapper if we have symlink, not a replacement
        # if we don't.
        ++$skip{Perl_my_symlink} unless $Config{d_symlink};
    } else {
	++$skip{PL_statusvalue_vms};
	++$skip{PL_perllib_sep};
	if (PLATFORM ne 'aix') {
	    ++$skip{$_} foreach qw(
				PL_DBcv
				PL_generation
				PL_lastgotoprobe
				PL_modcount
				main
				 );
	}
    }
}

if (PLATFORM ne 'vms') {
    # VMS does its own thing for these symbols.
    ++$skip{$_} foreach qw(
			PL_sig_handlers_initted
			PL_sig_ignoring
			PL_sig_defaulting
			 );
    if (PLATFORM ne 'win32') {
	++$skip{$_} foreach qw(
			    Perl_do_spawn
			    Perl_do_spawn_nowait
			    Perl_do_aspawn
			     );
    }
}

if (PLATFORM ne 'win32') {
    ++$skip{$_} foreach qw(
		    Perl_get_context
		    Perl_get_win32_message_utf8ness
		    Perl_Win_utf8_string_to_wstring
		    Perl_Win_wstring_to_utf8_string
			 );
}

unless ($define{UNLINK_ALL_VERSIONS}) {
    ++$skip{Perl_unlnk};
}

unless ($define{'DEBUGGING'}) {
    ++$skip{$_} foreach qw(
		    Perl_debop
		    Perl_debprofdump
		    Perl_debstack
		    Perl_debstackptrs
		    Perl_pad_sv
		    Perl_pad_setsv
		    Perl_set_padlist
		    Perl_hv_assert
		    PL_watchaddr
		    PL_watchok
			 );
}

if ($define{'PERL_IMPLICIT_SYS'}) {
    ++$skip{$_} foreach qw(
		    Perl_my_popen
		    Perl_my_pclose
			 );
    ++$export{$_} foreach qw(perl_get_host_info perl_alloc_override);
    ++$export{perl_clone_host} if $define{USE_ITHREADS};
}
else {
    ++$skip{$_} foreach qw(
		    PL_Mem
		    PL_MemShared
		    PL_MemParse
		    PL_Env
		    PL_StdIO
		    PL_LIO
		    PL_Dir
		    PL_Sock
		    PL_Proc
		    perl_alloc_using
		    perl_clone_using
			 );
}

if (!$define{'PERL_COPY_ON_WRITE'} || $define{'PERL_NO_COW'}) {
    ++$skip{Perl_sv_setsv_cow};
}

unless ($define{PERL_SAWAMPERSAND}) {
    ++$skip{PL_sawampersand};
}

unless ($define{'USE_REENTRANT_API'}) {
    ++$skip{PL_reentrant_buffer};
}

if ($define{'MYMALLOC'}) {
    try_symbols(qw(
		    Perl_dump_mstats
		    Perl_get_mstats
		    Perl_strdup
		    Perl_putenv
		    MallocCfg_ptr
		    MallocCfgP_ptr
		    ));
    unless ($define{USE_ITHREADS}) {
	++$skip{PL_malloc_mutex}
    }
}
else {
    ++$skip{$_} foreach qw(
		    PL_malloc_mutex
		    Perl_dump_mstats
		    Perl_get_mstats
		    MallocCfg_ptr
		    MallocCfgP_ptr
			 );
}

unless ($define{'USE_ITHREADS'}) {
    ++$skip{PL_thr_key};
    ++$skip{PL_user_prop_mutex};
    ++$skip{PL_user_def_props_aTHX};
}

unless ($define{'USE_ITHREADS'}) {
    ++$skip{$_} foreach qw(
                    PL_keyword_plugin_mutex
		    PL_check_mutex
                    PL_cur_locale_obj
		    PL_op_mutex
		    PL_regex_pad
		    PL_regex_padav
		    PL_dollarzero_mutex
		    PL_env_mutex
		    PL_hints_mutex
		    PL_locale_mutex
		    PL_locale_mutex_depth
		    PL_my_ctx_mutex
		    PL_perlio_mutex
		    PL_stashpad
		    PL_stashpadix
		    PL_stashpadmax
                    PL_veto_switch_non_tTHX_context
		    Perl_alloccopstash
		    Perl_allocfilegv
		    Perl_clone_params_del
		    Perl_clone_params_new
		    Perl_parser_dup
		    Perl_dirp_dup
		    Perl_cx_dup
		    Perl_si_dup
		    Perl_any_dup
		    Perl_ss_dup
		    Perl_fp_dup
		    Perl_gp_dup
		    Perl_he_dup
		    Perl_mg_dup
		    Perl_re_dup_guts
		    Perl_sv_dup
		    Perl_sv_dup_inc
		    Perl_rvpv_dup
		    Perl_hek_dup
		    Perl_sys_intern_dup
		    perl_clone
		    perl_clone_using
		    Perl_stashpv_hvname_match
		    Perl_regdupe_internal
		    Perl_newPADOP
			 );
}

unless ($define{USE_POSIX_2008_LOCALE})
{
    ++$skip{$_} foreach qw(
        PL_C_locale_obj
        PL_scratch_locale_obj
        PL_underlying_numeric_obj
    );
}
unless ($define{USE_PL_CURLOCALES})
{
    ++$skip{$_} foreach qw(
        PL_curlocales
    );
}

unless ($define{USE_PL_CUR_LC_ALL})
{
    ++$skip{$_} foreach qw(
        PL_cur_LC_ALL
    );
}

unless ($define{USE_PERL_SWITCH_LOCALE_CONTEXT})
{
    ++$skip{$_} foreach qw(
        Perl_switch_locale_context
    );
}

unless ($define{'MULTIPLICITY'}) {
    ++$skip{$_} foreach qw(
		    PL_my_cxt_index
		    PL_my_cxt_list
		    PL_my_cxt_size
		    PL_my_cxt_keys
		    PL_my_cxt_keys_size
		    Perl_croak_nocontext
		    Perl_die_nocontext
		    Perl_deb_nocontext
		    Perl_form_nocontext
		    Perl_load_module_nocontext
		    Perl_mess_nocontext
		    Perl_warn_nocontext
		    Perl_warner_nocontext
		    Perl_newSVpvf_nocontext
		    Perl_sv_catpvf_nocontext
		    Perl_sv_setpvf_nocontext
		    Perl_sv_catpvf_mg_nocontext
		    Perl_sv_setpvf_mg_nocontext
		    Perl_my_cxt_init
		    Perl_my_cxt_index
			 );
}

unless ($define{'USE_DTRACE'}) {
    ++$skip{$_} foreach qw(
                    Perl_dtrace_probe_call
                    Perl_dtrace_probe_load
                    Perl_dtrace_probe_op
                    Perl_dtrace_probe_phase
                );
}

unless ($define{'DEBUG_LEAKING_SCALARS'}) {
    ++$skip{PL_sv_serial};
}

unless ($define{'DEBUG_LEAKING_SCALARS_FORK_DUMP'}) {
    ++$skip{PL_dumper_fd};
}

unless ($define{'PERL_DONT_CREATE_GVSV'}) {
    ++$skip{Perl_gv_SVadd};
}

unless ($define{'PERL_USES_PL_PIDSTATUS'}) {
    ++$skip{PL_pidstatus};
}

unless ($define{'PERL_TRACK_MEMPOOL'}) {
    ++$skip{PL_memory_debug_header};
}

unless ($define{'PERL_MEM_LOG'}) {
    ++$skip{$_} foreach qw(
                    PL_mem_log
                    Perl_mem_log_alloc
                    Perl_mem_log_realloc
                    Perl_mem_log_free
                    Perl_mem_log_new_sv
                    Perl_mem_log_del_sv
                );
}

unless ($define{'MULTIPLICITY'}) {
    ++$skip{$_} foreach qw(
		    PL_interp_size
		    PL_interp_size_5_18_0
                    PL_sv_yes
                    PL_sv_undef
                    PL_sv_no
                    PL_sv_zero
			 );
}

unless ($define{HAS_MMAP}) {
    ++$skip{PL_mmap_page_size};
}

if ($define{HAS_SIGACTION}) {
    ++$skip{PL_sig_trapped};

    if (PLATFORM eq 'vms') {
        # FAKE_PERSISTENT_SIGNAL_HANDLERS defined as !defined(HAS_SIGACTION)
        ++$skip{PL_sig_ignoring};
        ++$skip{PL_sig_handlers_initted} unless $define{KILL_BY_SIGPRC};
    }
}

if (PLATFORM eq 'vms' && !$define{KILL_BY_SIGPRC}) {
    # FAKE_DEFAULT_SIGNAL_HANDLERS defined as KILL_BY_SIGPRC
    ++$skip{Perl_csighandler_init};
    ++$skip{Perl_my_kill};
    ++$skip{Perl_sig_to_vmscondition};
    ++$skip{PL_sig_defaulting};
    ++$skip{PL_sig_handlers_initted} unless !$define{HAS_SIGACTION};
}

if ($define{'HAS_STRNLEN'})
{
    ++$skip{Perl_my_strnlen};
}

unless ($define{USE_LOCALE_COLLATE}) {
    ++$skip{$_} foreach qw(
		    PL_collation_ix
		    PL_collation_name
		    PL_collation_standard
		    PL_collxfrm_base
		    PL_collxfrm_mult
		    Perl_sv_collxfrm
		    Perl_sv_collxfrm_flags
                    PL_strxfrm_NUL_replacement
                    PL_strxfrm_is_behaved
                    PL_strxfrm_max_cp
		    PL_in_utf8_COLLATE_locale
			 );
}

unless ($define{USE_LOCALE_NUMERIC}) {
    ++$skip{$_} foreach qw(
                    PL_underlying_numeric_obj
			 );
}

unless ($define{USE_LOCALE_CTYPE}) {
    ++$skip{$_} foreach qw(
		    PL_ctype_name
                    PL_in_utf8_CTYPE_locale
                    PL_in_utf8_turkic_locale
			 );
}

unless ($define{'USE_C_BACKTRACE'}) {
    ++$skip{Perl_get_c_backtrace_dump};
    ++$skip{Perl_dump_c_backtrace};
}

unless ($define{HAVE_INTERP_INTERN}) {
    ++$skip{$_} foreach qw(
		    Perl_sys_intern_clear
		    Perl_sys_intern_dup
		    Perl_sys_intern_init
		    PL_sys_intern
			 );
}

if ($define{HAS_SIGNBIT}) {
    ++$skip{Perl_signbit};
}

++$skip{PL_op_exec_cnt}
    unless $define{PERL_TRACE_OPS};

++$skip{PL_hash_chars}
    unless $define{PERL_USE_SINGLE_CHAR_HASH_CACHE};

# functions from *.sym files

my @syms = qw(globvar.sym);

# Symbols that are the public face of the PerlIO layers implementation
# These are in _addition to_ the public face of the abstraction
# and need to be exported to allow XS modules to implement layers
my @layer_syms = qw(
		    PerlIOBase_binmode
		    PerlIOBase_clearerr
		    PerlIOBase_close
		    PerlIOBase_dup
		    PerlIOBase_eof
		    PerlIOBase_error
		    PerlIOBase_fileno
		    PerlIOBase_open
		    PerlIOBase_noop_fail
		    PerlIOBase_noop_ok
		    PerlIOBase_popped
		    PerlIOBase_pushed
		    PerlIOBase_read
		    PerlIOBase_setlinebuf
		    PerlIOBase_unread
		    PerlIOBuf_bufsiz
		    PerlIOBuf_close
		    PerlIOBuf_dup
		    PerlIOBuf_fill
		    PerlIOBuf_flush
		    PerlIOBuf_get_base
		    PerlIOBuf_get_cnt
		    PerlIOBuf_get_ptr
		    PerlIOBuf_open
		    PerlIOBuf_popped
		    PerlIOBuf_pushed
		    PerlIOBuf_read
		    PerlIOBuf_seek
		    PerlIOBuf_set_ptrcnt
		    PerlIOBuf_tell
		    PerlIOBuf_unread
		    PerlIOBuf_write
		    PerlIO_allocate
		    PerlIO_apply_layera
		    PerlIO_apply_layers
		    PerlIO_arg_fetch
		    PerlIO_debug
		    PerlIO_define_layer
		    PerlIO_find_layer
		    PerlIO_isutf8
		    PerlIO_layer_fetch
		    PerlIO_list_alloc
		    PerlIO_list_free
		    PerlIO_modestr
		    PerlIO_parse_layers
		    PerlIO_pending
		    PerlIO_perlio
		    PerlIO_pop
		    PerlIO_push
		    PerlIO_sv_dup
		    Perl_PerlIO_clearerr
		    Perl_PerlIO_close
		    Perl_PerlIO_context_layers
		    Perl_PerlIO_eof
		    Perl_PerlIO_error
		    Perl_PerlIO_fileno
		    Perl_PerlIO_fill
		    Perl_PerlIO_flush
		    Perl_PerlIO_get_base
		    Perl_PerlIO_get_bufsiz
		    Perl_PerlIO_get_cnt
		    Perl_PerlIO_get_ptr
		    Perl_PerlIO_read
		    Perl_PerlIO_restore_errno
		    Perl_PerlIO_save_errno
		    Perl_PerlIO_seek
		    Perl_PerlIO_set_cnt
		    Perl_PerlIO_set_ptrcnt
		    Perl_PerlIO_setlinebuf
		    Perl_PerlIO_stderr
		    Perl_PerlIO_stdin
		    Perl_PerlIO_stdout
		    Perl_PerlIO_tell
		    Perl_PerlIO_unread
		    Perl_PerlIO_write
);

# Export the symbols that make up the PerlIO abstraction, regardless
# of its implementation - read from a file
push @syms, 'perlio.sym';

# PerlIO with layers - export implementation
try_symbols(@layer_syms, 'perlsio_binmode');


unless ($define{'USE_QUADMATH'}) {
  ++$skip{Perl_quadmath_format_needed};
  ++$skip{Perl_quadmath_format_single};
}

unless ($Config{d_mbrlen}) {
    ++$skip{PL_mbrlen_ps};
}

unless ($Config{d_mbrtowc}) {
    ++$skip{PL_mbrtowc_ps};
}

unless ($Config{d_wcrtomb}) {
    ++$skip{PL_wcrtomb_ps};
}

###############################################################################

# At this point all skip lists should be completed, as we are about to test
# many symbols against them.

{
    my %seen;
    my ($embed_array) = setup_embed($ARGS{TARG_DIR});
    my $excludedre = $define{'NO_MATHOMS'} ? qr/[emiIsb]/ : qr/[emiIs]/;

    foreach (@$embed_array) {
        my $embed= $_->{embed}
            or next;
	my ($flags, $retval, $func, $args) = @{$embed}{qw(flags return_type name args)};
	next unless $func;
	if (($flags =~ /[AXC]/ && $flags !~ $excludedre)
            || (!$define{'NO_MATHOMS'} && $flags =~ /b/))
        {
	    # public API, so export

	    # If a function is defined twice, for example before and after
	    # an #else, only export its name once. Important to do this test
	    # within the block, as the *first* definition may have flags which
	    # mean "don't export"
	    next if $seen{$func}++;
	    # Should we also skip adding the Perl_ prefix if $flags =~ /o/ ?
	    $func = "Perl_$func" if ($flags =~ /[psX]/ && $func !~ /^Perl_/);
	    ++$export{$func} unless exists $skip{$func};
	}
    }
}

foreach (@syms) {
    my $syms = $ARGS{TARG_DIR} . $_;
    open my $global, '<', $syms or die "failed to open $syms: $!\n";
    while (<$global>) {
	next unless /^([A-Za-z].*)/;
	my $symbol = "$1";
	++$export{$symbol} unless exists $skip{$symbol};
    }
}

# variables

readvar('perlvars.h', \%export);
unless ($define{MULTIPLICITY}) {
    readvar('intrpvar.h', \%export);
}

# Oddities from PerlIO
# All have alternate implementations in perlio.c, so always exist.
# Should they be considered to be part of the API?
try_symbols(qw(
		    PerlIO_binmode
		    PerlIO_getpos
		    PerlIO_init
		    PerlIO_setpos
		    PerlIO_tmpfile
	     ));

if (PLATFORM eq 'win32') {
    try_symbols(qw(
		    win32_free_childdir
		    win32_free_childenv
		    win32_get_childdir
		    win32_get_childenv
		    win32_spawnvp
		    Perl_init_os_extras
		    Perl_win32_init
		    Perl_win32_term
		    RunPerl
		    win32_async_check
		    win32_errno
		    win32_environ
		    win32_abort
		    win32_fstat
		    win32_stat
		    win32_pipe
		    win32_popen
		    win32_pclose
		    win32_rename
		    win32_setmode
		    win32_chsize
		    win32_lseek
		    win32_tell
		    win32_dup
		    win32_dup2
		    win32_open
		    win32_close
		    win32_eof
		    win32_isatty
		    win32_read
		    win32_write
		    win32_mkdir
		    win32_rmdir
		    win32_chdir
		    win32_flock
		    win32_execv
		    win32_execvp
		    win32_htons
		    win32_ntohs
		    win32_htonl
		    win32_ntohl
		    win32_inet_addr
		    win32_inet_ntoa
		    win32_socket
		    win32_bind
		    win32_listen
		    win32_accept
		    win32_connect
		    win32_send
		    win32_sendto
		    win32_recv
		    win32_recvfrom
		    win32_shutdown
		    win32_closesocket
		    win32_ioctlsocket
		    win32_setsockopt
		    win32_getsockopt
		    win32_getpeername
		    win32_getsockname
		    win32_gethostname
		    win32_gethostbyname
		    win32_gethostbyaddr
		    win32_getprotobyname
		    win32_getprotobynumber
		    win32_getservbyname
		    win32_getservbyport
		    win32_select
		    win32_endhostent
		    win32_endnetent
		    win32_endprotoent
		    win32_endservent
		    win32_getnetent
		    win32_getnetbyname
		    win32_getnetbyaddr
		    win32_getprotoent
		    win32_getservent
		    win32_sethostent
		    win32_setnetent
		    win32_setprotoent
		    win32_setservent
		    win32_getenv
		    win32_putenv
		    win32_perror
		    win32_malloc
		    win32_calloc
		    win32_realloc
		    win32_free
		    win32_sleep
		    win32_pause
		    win32_times
		    win32_access
		    win32_alarm
		    win32_chmod
		    win32_open_osfhandle
		    win32_get_osfhandle
		    win32_ioctl
		    win32_link
		    win32_unlink
		    win32_utime
		    win32_gettimeofday
		    win32_uname
		    win32_wait
		    win32_waitpid
		    win32_kill
		    win32_str_os_error
		    win32_opendir
		    win32_readdir
		    win32_telldir
		    win32_seekdir
		    win32_rewinddir
		    win32_closedir
		    win32_longpath
		    win32_ansipath
		    win32_os_id
		    win32_getpid
		    win32_crypt
		    win32_dynaload
		    win32_clearenv
		    win32_stdin
		    win32_stdout
		    win32_stderr
		    win32_ferror
		    win32_feof
		    win32_strerror
		    win32_fprintf
		    win32_printf
		    win32_vfprintf
		    win32_vprintf
		    win32_fread
		    win32_fwrite
		    win32_fopen
		    win32_fdopen
		    win32_freopen
		    win32_fclose
		    win32_fputs
		    win32_fputc
		    win32_ungetc
		    win32_getc
		    win32_fileno
		    win32_clearerr
		    win32_fflush
		    win32_ftell
		    win32_fseek
		    win32_fgetpos
		    win32_fsetpos
		    win32_rewind
		    win32_tmpfile
		    win32_setbuf
		    win32_setvbuf
		    win32_flushall
		    win32_fcloseall
		    win32_fgets
		    win32_gets
		    win32_fgetc
		    win32_putc
		    win32_puts
		    win32_getchar
		    win32_putchar
                    win32_symlink
                    win32_lstat
                    win32_readlink
		 ));
}
elsif (PLATFORM eq 'vms') {
    try_symbols(qw(
		      Perl_cando
		      Perl_cando_by_name
		      Perl_closedir
		      Perl_csighandler_init
		      Perl_do_rmdir
		      Perl_fileify_dirspec
		      Perl_fileify_dirspec_ts
		      Perl_fileify_dirspec_utf8
		      Perl_fileify_dirspec_utf8_ts
		      Perl_flex_fstat
		      Perl_flex_lstat
		      Perl_flex_stat
		      Perl_kill_file
		      Perl_my_chdir
		      Perl_my_chmod
		      Perl_my_crypt
		      Perl_my_endpwent
		      Perl_my_fclose
		      Perl_my_fdopen
		      Perl_my_fgetname
		      Perl_my_flush
		      Perl_my_fwrite
		      Perl_my_gconvert
		      Perl_my_getenv
		      Perl_my_getenv_len
		      Perl_my_getpwnam
		      Perl_my_getpwuid
		      Perl_my_gmtime
		      Perl_my_kill
		      Perl_my_killpg
		      Perl_my_localtime
		      Perl_my_mkdir
		      Perl_my_sigaction
		      Perl_my_symlink
		      Perl_my_time
		      Perl_my_tmpfile
		      Perl_my_trnlnm
		      Perl_my_utime
		      Perl_my_waitpid
		      Perl_opendir
		      Perl_pathify_dirspec
		      Perl_pathify_dirspec_ts
		      Perl_pathify_dirspec_utf8
		      Perl_pathify_dirspec_utf8_ts
		      Perl_readdir
		      Perl_readdir_r
		      Perl_rename
		      Perl_rmscopy
		      Perl_rmsexpand
		      Perl_rmsexpand_ts
		      Perl_rmsexpand_utf8
		      Perl_rmsexpand_utf8_ts
		      Perl_seekdir
		      Perl_sig_to_vmscondition
		      Perl_telldir
		      Perl_tounixpath
		      Perl_tounixpath_ts
		      Perl_tounixpath_utf8
		      Perl_tounixpath_utf8_ts
		      Perl_tounixspec
		      Perl_tounixspec_ts
		      Perl_tounixspec_utf8
		      Perl_tounixspec_utf8_ts
		      Perl_tovmspath
		      Perl_tovmspath_ts
		      Perl_tovmspath_utf8
		      Perl_tovmspath_utf8_ts
		      Perl_tovmsspec
		      Perl_tovmsspec_ts
		      Perl_tovmsspec_utf8
		      Perl_tovmsspec_utf8_ts
		      Perl_trim_unixpath
		      Perl_vms_case_tolerant
		      Perl_vms_do_aexec
		      Perl_vms_do_exec
		      Perl_vms_image_init
		      Perl_vms_realpath
		      Perl_vmssetenv
		      Perl_vmssetuserlnm
		      Perl_vmstrnenv
		      PerlIO_openn
		 ));
}
elsif (PLATFORM eq 'os2') {
    try_symbols(qw(
		      ctermid
		      get_sysinfo
		      Perl_OS2_init
		      Perl_OS2_init3
		      Perl_OS2_term
		      OS2_Perl_data
		      dlopen
		      dlsym
		      dlerror
		      dlclose
		      dup2
		      dup
		      my_tmpfile
		      my_tmpnam
		      my_flock
		      my_rmdir
		      my_mkdir
		      my_getpwuid
		      my_getpwnam
		      my_getpwent
		      my_setpwent
		      my_endpwent
		      fork_with_resources
		      croak_with_os2error
		      setgrent
		      endgrent
		      getgrent
		      malloc_mutex
		      threads_mutex
		      nthreads
		      nthreads_cond
		      os2_cond_wait
		      os2_stat
		      os2_execname
		      async_mssleep
		      msCounter
		      InfoTable
		      pthread_join
		      pthread_create
		      pthread_detach
		      XS_Cwd_change_drive
		      XS_Cwd_current_drive
		      XS_Cwd_extLibpath
		      XS_Cwd_extLibpath_set
		      XS_Cwd_sys_abspath
		      XS_Cwd_sys_chdir
		      XS_Cwd_sys_cwd
		      XS_Cwd_sys_is_absolute
		      XS_Cwd_sys_is_relative
		      XS_Cwd_sys_is_rooted
		      XS_DynaLoader_mod2fname
		      XS_File__Copy_syscopy
		      Perl_Register_MQ
		      Perl_Deregister_MQ
		      Perl_Serve_Messages
		      Perl_Process_Messages
		      init_PMWIN_entries
		      PMWIN_entries
		      Perl_hab_GET
		      loadByOrdinal
		      pExtFCN
		      os2error
		      ResetWinError
		      CroakWinError
		      PL_do_undump
		 ));
}

# When added this code was only run for Win32 (and WinCE at the time)
# Currently only Win32 links static extensions into the shared library.
# For *nix (and presumably OS/2) with a shared libperl, Makefile.SH compiles
# static extensions with -fPIC, but links them to perl, not libperl.so
# The VMS build scripts don't yet implement static extensions at all.

if (PLATFORM eq 'win32') {
    # records of type boot_module for statically linked modules (except Dynaloader)
    my $static_ext = $Config{static_ext} // "";
    $static_ext =~ s/\//__/g;
    $static_ext =~ s/\bDynaLoader\b//;
    try_symbols(map {"boot_$_"} grep {/\S/} split /\s+/, $static_ext);
    try_symbols("init_Win32CORE") if $static_ext =~ /\bWin32CORE\b/;
}

if (PLATFORM eq 'os2') {
    my (%mapped, @missing);
    open MAP, '<', 'miniperl.map' or die 'Cannot read miniperl.map';
    /^\s*[\da-f:]+\s+(\w+)/i and $mapped{$1}++ foreach <MAP>;
    close MAP or die 'Cannot close miniperl.map';

    @missing = grep { !exists $mapped{$_} }
		    keys %export;
    @missing = grep { !exists $exportperlmalloc{$_} } @missing;
    delete $export{$_} foreach @missing;
}

###############################################################################

# Now all symbols should be defined because next we are going to output them.

# Start with platform specific headers:

if (PLATFORM eq 'win32') {
    my $dll = $define{PERL_DLL} ? $define{PERL_DLL} =~ s/\.dll$//ir
	: "perl$Config{api_revision}$Config{api_version}";
    print "LIBRARY $dll\n";
    # The DESCRIPTION module definition file statement is not supported
    # by VC7 onwards.
    if ($ARGS{CCTYPE} eq 'GCC') {
	print "DESCRIPTION 'Perl interpreter'\n";
    }
    print "EXPORTS\n";
}
elsif (PLATFORM eq 'os2') {
    (my $v = $]) =~ s/(\d\.\d\d\d)(\d\d)$/$1_$2/;
    $v .= '-thread' if $Config{archname} =~ /-thread/;
    (my $dll = $define{PERL_DLL}) =~ s/\.dll$//i;
    $v .= "\@$Config{perl_patchlevel}" if $Config{perl_patchlevel};
    my $d = "DESCRIPTION '\@#perl5-porters\@perl.org:$v#\@ Perl interpreter, configured as $Config{config_args}'";
    $d = substr($d, 0, 249) . "...'" if length $d > 253;
    print <<"---EOP---";
LIBRARY '$dll' INITINSTANCE TERMINSTANCE
$d
STACKSIZE 32768
CODE LOADONCALL
DATA LOADONCALL NONSHARED MULTIPLE
EXPORTS
---EOP---
}
elsif (PLATFORM eq 'aix') {
    my $OSVER = `uname -v`;
    chop $OSVER;
    my $OSREL = `uname -r`;
    chop $OSREL;
    if ($OSVER > 4 || ($OSVER == 4 && $OSREL >= 3)) {
	print "#! ..\n";
    } else {
	print "#!\n";
    }
}

# Then the symbols

my @symbols = $fold ? sort {lc $a cmp lc $b} keys %export : sort keys %export;
foreach my $symbol (@symbols) {
    if (PLATFORM eq 'win32') {
	# Remembering the origin file of each symbol is an alternative to PL_ matching
	if (substr($symbol, 0, 3) eq 'PL_') {
	    print "\t$symbol DATA\n";
	}
	else {
	    print "\t$symbol\n";
	}
    }
    elsif (PLATFORM eq 'os2') {
	printf qq(    %-31s \@%s\n),
	  qq("$symbol"), $ordinal{$symbol} || ++$sym_ord;
	printf qq(    %-31s \@%s\n),
	  qq("$exportperlmalloc{$symbol}" = "$symbol"),
	  $ordinal{$exportperlmalloc{$symbol}} || ++$sym_ord
	  if $exportperlmalloc and exists $exportperlmalloc{$symbol};
    } else {
	print "$symbol\n";
    }
}

# Then platform specific footers.

if (PLATFORM eq 'os2') {
    print <<EOP;
    dll_perlmain=main
    fill_extLibpath
    dir_subst
    Perl_OS2_handler_install

; LAST_ORDINAL=$sym_ord
EOP
}

1;
