package Config::Perl::V;

use strict;
use warnings;

use Config;
use Exporter;
use vars qw($VERSION @ISA @EXPORT_OK %EXPORT_TAGS);
$VERSION     = "0.36";
@ISA         = qw( Exporter );
@EXPORT_OK   = qw( plv2hash summary myconfig signature );
%EXPORT_TAGS = (
    'all' => [ @EXPORT_OK  ],
    'sig' => [ "signature" ],
    );

#  Characteristics of this binary (from libperl):
#    Compile-time options: DEBUGGING PERL_DONT_CREATE_GVSV PERL_MALLOC_WRAP
#                          USE_64_BIT_INT USE_LARGE_FILES USE_PERLIO

# The list are as the perl binary has stored it in PL_bincompat_options
#  search for it in
#   perl.c line 1643 S_Internals_V ()
#     perl -ne'(/^S_Internals_V/../^}/)&&s/^\s+"( .*)"/$1/ and print' perl.c
#   perl.h line 4566 PL_bincompat_options
#     perl -ne'(/^\w.*PL_bincompat/../^\w}/)&&s/^\s+"( .*)"/$1/ and print' perl.h
my %BTD = map {( $_ => 0 )} qw(

    DEBUGGING
    NO_HASH_SEED
    NO_MATHOMS
    NO_PERL_INTERNAL_RAND_SEED
    NO_PERL_RAND_SEED
    NO_TAINT_SUPPORT
    PERL_BOOL_AS_CHAR
    PERL_COPY_ON_WRITE
    PERL_DISABLE_PMC
    PERL_DONT_CREATE_GVSV
    PERL_EXTERNAL_GLOB
    PERL_HASH_FUNC_DJB2
    PERL_HASH_FUNC_MURMUR3
    PERL_HASH_FUNC_ONE_AT_A_TIME
    PERL_HASH_FUNC_ONE_AT_A_TIME_HARD
    PERL_HASH_FUNC_ONE_AT_A_TIME_OLD
    PERL_HASH_FUNC_SDBM
    PERL_HASH_FUNC_SIPHASH
    PERL_HASH_FUNC_SUPERFAST
    PERL_IS_MINIPERL
    PERL_MALLOC_WRAP
    PERL_MEM_LOG
    PERL_MEM_LOG_ENV
    PERL_MEM_LOG_ENV_FD
    PERL_MEM_LOG_NOIMPL
    PERL_MEM_LOG_STDERR
    PERL_MEM_LOG_TIMESTAMP
    PERL_NEW_COPY_ON_WRITE
    PERL_OP_PARENT
    PERL_PERTURB_KEYS_DETERMINISTIC
    PERL_PERTURB_KEYS_DISABLED
    PERL_PERTURB_KEYS_RANDOM
    PERL_PRESERVE_IVUV
    PERL_RC_STACK
    PERL_RELOCATABLE_INCPUSH
    PERL_USE_DEVEL
    PERL_USE_SAFE_PUTENV
    PERL_USE_UNSHARED_KEYS_IN_LARGE_HASHES
    SILENT_NO_TAINT_SUPPORT
    UNLINK_ALL_VERSIONS
    USE_ATTRIBUTES_FOR_PERLIO
    USE_FAST_STDIO
    USE_HASH_SEED_EXPLICIT
    USE_LOCALE
    USE_LOCALE_CTYPE
    USE_NO_REGISTRY
    USE_PERL_ATOF
    USE_SITECUSTOMIZE
    USE_THREAD_SAFE_LOCALE

    DEBUG_LEAKING_SCALARS
    DEBUG_LEAKING_SCALARS_FORK_DUMP
    DECCRTL_SOCKETS
    FAKE_THREADS
    FCRYPT
    HAS_TIMES
    HAVE_INTERP_INTERN
    MULTIPLICITY
    MYMALLOC
    NO_HASH_SEED
    PERL_DEBUG_READONLY_COW
    PERL_DEBUG_READONLY_OPS
    PERL_GLOBAL_STRUCT
    PERL_GLOBAL_STRUCT_PRIVATE
    PERL_HASH_NO_SBOX32
    PERL_HASH_USE_SBOX32
    PERL_IMPLICIT_CONTEXT
    PERL_IMPLICIT_SYS
    PERLIO_LAYERS
    PERL_MAD
    PERL_MICRO
    PERL_NEED_APPCTX
    PERL_NEED_TIMESBASE
    PERL_OLD_COPY_ON_WRITE
    PERL_POISON
    PERL_SAWAMPERSAND
    PERL_TRACK_MEMPOOL
    PERL_USES_PL_PIDSTATUS
    PL_OP_SLAB_ALLOC
    THREADS_HAVE_PIDS
    USE_64_BIT_ALL
    USE_64_BIT_INT
    USE_IEEE
    USE_ITHREADS
    USE_LARGE_FILES
    USE_LOCALE_COLLATE
    USE_LOCALE_NUMERIC
    USE_LOCALE_TIME
    USE_LONG_DOUBLE
    USE_PERLIO
    USE_QUADMATH
    USE_REENTRANT_API
    USE_SFIO
    USE_SOCKS
    VMS_DO_SOCKETS
    VMS_SHORTEN_LONG_SYMBOLS
    VMS_SYMBOL_CASE_AS_IS
    );

# These are all the keys that are
# 1. Always present in %Config - lib/Config.pm #87 tie %Config
# 2. Reported by 'perl -V' (the rest)
my @config_vars = qw(

    api_subversion
    api_version
    api_versionstring
    archlibexp
    dont_use_nlink
    d_readlink
    d_symlink
    exe_ext
    inc_version_list
    ldlibpthname
    patchlevel
    path_sep
    perl_patchlevel
    privlibexp
    scriptdir
    sitearchexp
    sitelibexp
    subversion
    usevendorprefix
    version

    git_commit_id
    git_describe
    git_branch
    git_uncommitted_changes
    git_commit_id_title
    git_snapshot_date

    package revision version_patchlevel_string

    osname osvers archname
    myuname
    config_args
    hint useposix d_sigaction
    useithreads usemultiplicity
    useperlio d_sfio uselargefiles usesocks
    use64bitint use64bitall uselongdouble
    usemymalloc default_inc_excludes_dot bincompat5005

    cc ccflags
    optimize
    cppflags
    ccversion gccversion gccosandvers
    intsize longsize ptrsize doublesize byteorder
    d_longlong longlongsize d_longdbl longdblsize
    ivtype ivsize nvtype nvsize lseektype lseeksize
    alignbytes prototype

    ld ldflags
    libpth
    libs
    perllibs
    libc so useshrplib libperl
    gnulibc_version

    dlsrc dlext d_dlsymun ccdlflags
    cccdlflags lddlflags
    );

my %empty_build = (
    'osname'  => "",
    'stamp'   => 0,
    'options' => { %BTD },
    'patches' => [],
    );

sub _make_derived {
    my $conf = shift;

    for ( [ 'lseektype'		=> "Off_t"	],
	  [ 'myuname'		=> "uname"	],
	  [ 'perl_patchlevel'	=> "patch"	],
	  ) {
	my ($official, $derived) = @{$_};
	$conf->{'config'}{$derived}  ||= $conf->{'config'}{$official};
	$conf->{'config'}{$official} ||= $conf->{'config'}{$derived};
	$conf->{'derived'}{$derived} = delete $conf->{'config'}{$derived};
	}

    if (exists $conf->{'config'}{'version_patchlevel_string'} &&
       !exists $conf->{'config'}{'api_version'}) {
	my $vps = $conf->{'config'}{'version_patchlevel_string'};
	$vps =~ s{\b revision   \s+ (\S+) }{}x and
	    $conf->{'config'}{'revision'}        ||= $1;

	$vps =~ s{\b version    \s+ (\S+) }{}x and
	    $conf->{'config'}{'api_version'}     ||= $1;
	$vps =~ s{\b subversion \s+ (\S+) }{}x and
	    $conf->{'config'}{'subversion'}      ||= $1;
	$vps =~ s{\b patch      \s+ (\S+) }{}x and
	    $conf->{'config'}{'perl_patchlevel'} ||= $1;
	}

    ($conf->{'config'}{'version_patchlevel_string'} ||= join " ",
	map  { ($_, $conf->{'config'}{$_} ) }
	grep {      $conf->{'config'}{$_}   }
	qw( api_version subversion perl_patchlevel )) =~ s/\bperl_//; 

    $conf->{'config'}{'perl_patchlevel'}  ||= "";	# 0 is not a valid patchlevel

    if ($conf->{'config'}{'perl_patchlevel'} =~ m{^git\w*-([^-]+)}i) {
	$conf->{'config'}{'git_branch'}   ||= $1;
	$conf->{'config'}{'git_describe'} ||= $conf->{'config'}{'perl_patchlevel'};
	}

    $conf->{'config'}{$_} ||= "undef" for grep m{^(?:use|def)} => @config_vars;

    $conf;
    } # _make_derived

sub plv2hash {
    my %config;

    my $pv = join "\n" => @_;

    if ($pv =~ m{^Summary of my\s+(\S+)\s+\(\s*(.*?)\s*\)}m) {
	$config{'package'} = $1;
	my $rev = $2;
	$rev =~ s/^ revision \s+ (\S+) \s*//x and $config{'revision'} = $1;
	$rev and $config{'version_patchlevel_string'} = $rev;
	my ($rel) = $config{'package'} =~ m{perl(\d)};
	my ($vers, $subvers) = $rev =~ m{version\s+(\d+)\s+subversion\s+(\d+)};
	defined $vers && defined $subvers && defined $rel and
	    $config{'version'} = "$rel.$vers.$subvers";
	}

    if ($pv =~ m{^\s+(Snapshot of:)\s+(\S+)}) {
	$config{'git_commit_id_title'} = $1;
	$config{'git_commit_id'}       = $2;
	}

    # these are always last on line and can have multiple quotation styles
    for my $k (qw( ccflags ldflags lddlflags )) {
	$pv =~ s{, \s* $k \s*=\s* (.*) \s*$}{}mx or next;
	my $v = $1;
	$v =~ s/\s*,\s*$//;
	$v =~ s/^(['"])(.*)\1$/$2/;
	$config{$k} = $v;
	}

    my %kv;
    if ($pv =~ m{\S,? (?:osvers|archname)=}) { # attr is not the first on the line
	# up to and including 5.24, a line could have multiple kv pairs
	%kv = ($pv =~ m{\b
	    (\w+)		# key
	    \s*=		# assign
	    ( '\s*[^']*?\s*'	# quoted value
	    | \S+[^=]*?\s*\n	# unquoted running till end of line
	    | \S+		# unquoted value
	    | \s*\n		# empty
	    )
	    (?:,?\s+|\s*\n)?	# optional separator (5.8.x reports did
	    }gx);		# not have a ',' between every kv pair)
	}
    else {
	# as of 5.25, each kv pair is listed on its own line
	%kv = ($pv =~ m{^
	    \s+
	    (\w+)		# key
	    \s*=\s*		# assign
	    (.*?)		# value
	    \s*,?\s*$
	    }gmx);
	}

    while (my ($k, $v) = each %kv) {
	$k =~ s{\s+$}		{};
	$v =~ s{\s*\n\z}	{};
	$v =~ s{,$}		{};
	$v =~ m{^'(.*)'$} and $v = $1;
	$v =~ s{\s+$}	{};
	$config{$k} = $v;
	}

    my $build = { %empty_build };

    $pv =~ m{^\s+Compiled at\s+(.*)}m
	and $build->{'stamp'}   = $1;
    $pv =~ m{^\s+Locally applied patches:(?:\s+|\n)(.*?)(?:[\s\n]+Buil[td] under)}ms
	and $build->{'patches'} = [ split m{\n+\s*}, $1 ];
    $pv =~ m{^\s+Compile-time options:(?:\s+|\n)(.*?)(?:[\s\n]+(?:Locally applied|Buil[td] under))}ms
	and map { $build->{'options'}{$_} = 1 } split m{\s+|\n} => $1;

    $build->{'osname'} = $config{'osname'};
    $pv =~ m{^\s+Built under\s+(.*)}m
	and $build->{'osname'}  = $1;
    $config{'osname'} ||= $build->{'osname'};

    return _make_derived ({
	'build'		=> $build,
	'environment'	=> {},
	'config'	=> \%config,
	'derived'	=> {},
	'inc'		=> [],
	});
    } # plv2hash

sub summary {
    my $conf = shift || myconfig ();
    ref $conf eq "HASH"
    && exists $conf->{'config'}
    && exists $conf->{'build'}
    && ref $conf->{'config'} eq "HASH"
    && ref $conf->{'build'}  eq "HASH" or return;

    my %info = map {
	exists $conf->{'config'}{$_} ? ( $_ => $conf->{'config'}{$_} ) : () }
	qw( archname osname osvers revision patchlevel subversion version
	    cc ccversion gccversion config_args inc_version_list
	    d_longdbl d_longlong use64bitall use64bitint useithreads
	    uselongdouble usemultiplicity usemymalloc useperlio useshrplib 
	    doublesize intsize ivsize nvsize longdblsize longlongsize lseeksize
	    default_inc_excludes_dot
	    );
    $info{$_}++ for grep { $conf->{'build'}{'options'}{$_} } keys %{$conf->{'build'}{'options'}};

    return \%info;
    } # summary

sub signature {
    my $no_md5 = "0" x 32;
    my $conf = summary (shift) or return $no_md5;

    eval { require Digest::MD5 };
    $@ and return $no_md5;

    $conf->{'cc'} =~ s{.*\bccache\s+}{};
    $conf->{'cc'} =~ s{.*[/\\]}{};

    delete $conf->{'config_args'};
    return Digest::MD5::md5_hex (join "\xFF" => map {
	"$_=".(defined $conf->{$_} ? $conf->{$_} : "\xFE");
	} sort keys %{$conf});
    } # signature

sub myconfig {
    my $args = shift;
    my %args = ref $args eq "HASH"  ? %{$args} :
               ref $args eq "ARRAY" ? @{$args} : ();

    my $build = { %empty_build };

    # 5.14.0 and later provide all the information without shelling out
    my $stamp = eval { Config::compile_date () };
    if (defined $stamp) {
	$stamp =~ s/^Compiled at //;
	$build->{'osname'}      = $^O;
	$build->{'stamp'}       = $stamp;
	$build->{'patches'}     =     [ Config::local_patches () ];
	$build->{'options'}{$_} = 1 for Config::bincompat_options (),
					Config::non_bincompat_options ();
	}
    else {
	#y $pv = qx[$^X -e"sub Config::myconfig{};" -V];
	my $cnf = plv2hash (qx[$^X -V]);

	$build->{$_} = $cnf->{'build'}{$_} for qw( osname stamp patches options );
	}

    my @KEYS = keys %ENV;
    my %env  =
	map {( $_ => $ENV{$_} )}  grep m{^PERL}        => @KEYS;
    if ($args{'env'}) {
	$env{$_}  =  $ENV{$_} for grep m{$args{'env'}} => @KEYS;
	}

    my %config = map { $_ => $Config{$_} } @config_vars;

    return _make_derived ({
	'build'		=> $build,
	'environment'	=> \%env,
	'config'	=> \%config,
	'derived'	=> {},
	'inc'		=> \@INC,
	});
    } # myconfig

1;

__END__

=head1 NAME

Config::Perl::V - Structured data retrieval of perl -V output

=head1 SYNOPSIS

 use Config::Perl::V;

 my $local_config = Config::Perl::V::myconfig ();
 print $local_config->{config}{osname};

=head1 DESCRIPTION

=head2 $conf = myconfig ()

This function will collect the data described in L</"The hash structure"> below,
and return that as a hash reference. It optionally accepts an option to
include more entries from %ENV. See L</environment> below.

Note that this will not work on uninstalled perls when called with
C<-I/path/to/uninstalled/perl/lib>, but it works when that path is in
C<$PERL5LIB> or in C<$PERL5OPT>, as paths passed using C<-I> are not
known when the C<-V> information is collected.

=head2 $conf = plv2hash ($text [, ...])

Convert a sole 'perl -V' text block, or list of lines, to a complete
myconfig hash.  All unknown entries are defaulted.

=head2 $info = summary ([$conf])

Return an arbitrary selection of the information. If no C<$conf> is
given, C<myconfig ()> is used instead.

=head2 $md5 = signature ([$conf])

Return the MD5 of the info returned by C<summary ()> without the
C<config_args> entry.

If C<Digest::MD5> is not available, it return a string with only C<0>'s.

=head2 The hash structure

The returned hash consists of 4 parts:

=over 4

=item build

This information is extracted from the second block that is emitted by
C<perl -V>, and usually looks something like

 Characteristics of this binary (from libperl):
   Compile-time options: DEBUGGING USE_64_BIT_INT USE_LARGE_FILES
   Locally applied patches:
	 defined-or
	 MAINT24637
   Built under linux
   Compiled at Jun 13 2005 10:44:20
   @INC:
     /usr/lib/perl5/5.8.7/i686-linux-64int
     /usr/lib/perl5/5.8.7
     /usr/lib/perl5/site_perl/5.8.7/i686-linux-64int
     /usr/lib/perl5/site_perl/5.8.7
     /usr/lib/perl5/site_perl
     .

or

 Characteristics of this binary (from libperl):
   Compile-time options: DEBUGGING MULTIPLICITY
			 PERL_DONT_CREATE_GVSV PERL_IMPLICIT_CONTEXT
			 PERL_MALLOC_WRAP PERL_TRACK_MEMPOOL
			 PERL_USE_SAFE_PUTENV USE_ITHREADS
			 USE_LARGE_FILES USE_PERLIO
			 USE_REENTRANT_API
   Built under linux
   Compiled at Jan 28 2009 15:26:59

This information is not available anywhere else, including C<%Config>,
but it is the information that is only known to the perl binary.

The extracted information is stored in 5 entries in the C<build> hash:

=over 4

=item osname

This is most likely the same as C<$Config{osname}>, and was the name
known when perl was built. It might be different if perl was cross-compiled.

The default for this field, if it cannot be extracted, is to copy
C<$Config{osname}>. The two may be differing in casing (OpenBSD vs openbsd).

=item stamp

This is the time string for which the perl binary was compiled. The default
value is 0.

=item options

This is a hash with all the known defines as keys. The value is either 0,
which means unknown or unset, or 1, which means defined.

=item derived

As some variables are reported by a different name in the output of C<perl -V>
than their actual name in C<%Config>, I decided to leave the C<config> entry
as close to reality as possible, and put in the entries that might have been
guessed by the printed output in a separate block.

=item patches

This is a list of optionally locally applied patches. Default is an empty list.

=back

=item environment

By default this hash is only filled with the environment variables
out of %ENV that start with C<PERL>, but you can pass the C<env> option
to myconfig to get more

 my $conf = Config::Perl::V::myconfig ({ env => qr/^ORACLE/ });
 my $conf = Config::Perl::V::myconfig ([ env => qr/^ORACLE/ ]);

=item config

This hash is filled with the variables that C<perl -V> fills its report
with, and it has the same variables that C<Config::myconfig> returns
from C<%Config>.

=item inc

This is the list of default @INC.

=back

=head1 REASONING

This module was written to be able to return the configuration for the
currently used perl as deeply as needed for the CPANTESTERS framework.
Up until now they used the output of myconfig as a single text blob,
and so it was missing the vital binary characteristics of the running
perl and the optional applied patches.

=head1 BUGS

Please feedback what is wrong

=head1 TODO

 * Implement retrieval functions/methods
 * Documentation
 * Error checking
 * Tests

=head1 AUTHOR

H.Merijn Brand <h.m.brand@xs4all.nl>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009-2023 H.Merijn Brand

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut
