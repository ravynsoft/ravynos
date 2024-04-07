package App::Cpan;

use strict;
use warnings;
use vars qw($VERSION);

use if $] < 5.008 => 'IO::Scalar';

$VERSION = '1.678';

=head1 NAME

App::Cpan - easily interact with CPAN from the command line

=head1 SYNOPSIS

	# with arguments and no switches, installs specified modules
	cpan module_name [ module_name ... ]

	# with switches, installs modules with extra behavior
	cpan [-cfFimtTw] module_name [ module_name ... ]

	# use local::lib
	cpan -I module_name [ module_name ... ]

	# one time mirror override for faster mirrors
	cpan -p ...

	# with just the dot, install from the distribution in the
	# current directory
	cpan .

	# without arguments, starts CPAN.pm shell
	cpan

	# without arguments, but some switches
	cpan [-ahpruvACDLOPX]

=head1 DESCRIPTION

This script provides a command interface (not a shell) to CPAN. At the
moment it uses CPAN.pm to do the work, but it is not a one-shot command
runner for CPAN.pm.

=head2 Options

=over 4

=item -a

Creates a CPAN.pm autobundle with CPAN::Shell->autobundle.

=item -A module [ module ... ]

Shows the primary maintainers for the specified modules.

=item -c module

Runs a `make clean` in the specified module's directories.

=item -C module [ module ... ]

Show the F<Changes> files for the specified modules

=item -D module [ module ... ]

Show the module details. This prints one line for each out-of-date module
(meaning, modules locally installed but have newer versions on CPAN).
Each line has three columns: module name, local version, and CPAN
version.

=item -f

Force the specified action, when it normally would have failed. Use this
to install a module even if its tests fail. When you use this option,
-i is not optional for installing a module when you need to force it:

	% cpan -f -i Module::Foo

=item -F

Turn off CPAN.pm's attempts to lock anything. You should be careful with
this since you might end up with multiple scripts trying to muck in the
same directory. This isn't so much of a concern if you're loading a special
config with C<-j>, and that config sets up its own work directories.

=item -g module [ module ... ]

Downloads to the current directory the latest distribution of the module.

=item -G module [ module ... ]

UNIMPLEMENTED

Download to the current directory the latest distribution of the
modules, unpack each distribution, and create a git repository for each
distribution.

If you want this feature, check out Yanick Champoux's C<Git::CPAN::Patch>
distribution.

=item -h

Print a help message and exit. When you specify C<-h>, it ignores all
of the other options and arguments.

=item -i module [ module ... ]

Install the specified modules. With no other switches, this switch
is implied.

=item -I

Load C<local::lib> (think like C<-I> for loading lib paths). Too bad
C<-l> was already taken.

=item -j Config.pm

Load the file that has the CPAN configuration data. This should have the
same format as the standard F<CPAN/Config.pm> file, which defines
C<$CPAN::Config> as an anonymous hash.

If the file does not exist, C<cpan> dies.

=item -J

Dump the configuration in the same format that CPAN.pm uses. This is useful
for checking the configuration as well as using the dump as a starting point
for a new, custom configuration.

=item -l

List all installed modules with their versions

=item -L author [ author ... ]

List the modules by the specified authors.

=item -m

Make the specified modules.

=item -M mirror1,mirror2,...

A comma-separated list of mirrors to use for just this run. The C<-P>
option can find them for you automatically.

=item -n

Do a dry run, but don't actually install anything. (unimplemented)

=item -O

Show the out-of-date modules.

=item -p

Ping the configured mirrors and print a report

=item -P

Find the best mirrors you could be using and use them for the current
session.

=item -r

Recompiles dynamically loaded modules with CPAN::Shell->recompile.

=item -s

Drop in the CPAN.pm shell. This command does this automatically if you don't
specify any arguments.

=item -t module [ module ... ]

Run a `make test` on the specified modules.

=item -T

Do not test modules. Simply install them.

=item -u

Upgrade all installed modules. Blindly doing this can really break things,
so keep a backup.

=item -v

Print the script version and CPAN.pm version then exit.

=item -V

Print detailed information about the cpan client.

=item -w

UNIMPLEMENTED

Turn on cpan warnings. This checks various things, like directory permissions,
and tells you about problems you might have.

=item -x module [ module ... ]

Find close matches to the named modules that you think you might have
mistyped. This requires the optional installation of Text::Levenshtein or
Text::Levenshtein::Damerau.

=item -X

Dump all the namespaces to standard output.

=back

=head2 Examples

	# print a help message
	cpan -h

	# print the version numbers
	cpan -v

	# create an autobundle
	cpan -a

	# recompile modules
	cpan -r

	# upgrade all installed modules
	cpan -u

	# install modules ( sole -i is optional )
	cpan -i Netscape::Booksmarks Business::ISBN

	# force install modules ( must use -i )
	cpan -fi CGI::Minimal URI

	# install modules but without testing them
	cpan -Ti CGI::Minimal URI

=head2 Environment variables

There are several components in CPAN.pm that use environment variables.
The build tools, L<ExtUtils::MakeMaker> and L<Module::Build> use some,
while others matter to the levels above them. Some of these are specified
by the Perl Toolchain Gang:

Lancaster Consensus: L<https://github.com/Perl-Toolchain-Gang/toolchain-site/blob/master/lancaster-consensus.md>

Oslo Consensus: L<https://github.com/Perl-Toolchain-Gang/toolchain-site/blob/master/oslo-consensus.md>

=over 4

=item NONINTERACTIVE_TESTING

Assume no one is paying attention and skips prompts for distributions
that do that correctly. C<cpan(1)> sets this to C<1> unless it already
has a value (even if that value is false).

=item PERL_MM_USE_DEFAULT

Use the default answer for a prompted questions. C<cpan(1)> sets this
to C<1> unless it already has a value (even if that value is false).

=item CPAN_OPTS

As with C<PERL5OPT>, a string of additional C<cpan(1)> options to
add to those you specify on the command line.

=item CPANSCRIPT_LOGLEVEL

The log level to use, with either the embedded, minimal logger or
L<Log::Log4perl> if it is installed. Possible values are the same as
the C<Log::Log4perl> levels: C<TRACE>, C<DEBUG>, C<INFO>, C<WARN>,
C<ERROR>, and C<FATAL>. The default is C<INFO>.

=item GIT_COMMAND

The path to the C<git> binary to use for the Git features. The default
is C</usr/local/bin/git>.

=back

=head2 Methods

=over 4

=cut

use autouse Carp => qw(carp croak cluck);
use CPAN 1.80 (); # needs no test
use Config;
use autouse Cwd => qw(cwd);
use autouse 'Data::Dumper' => qw(Dumper);
use File::Spec::Functions qw(catfile file_name_is_absolute rel2abs);
use File::Basename;
use Getopt::Std;

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Internal constants
use constant TRUE  => 1;
use constant FALSE => 0;


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# The return values
use constant HEY_IT_WORKED              =>   0;
use constant I_DONT_KNOW_WHAT_HAPPENED  =>   1; # 0b0000_0001
use constant ITS_NOT_MY_FAULT           =>   2;
use constant THE_PROGRAMMERS_AN_IDIOT   =>   4;
use constant A_MODULE_FAILED_TO_INSTALL =>   8;


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# set up the order of options that we layer over CPAN::Shell
BEGIN { # most of this should be in methods
use vars qw( @META_OPTIONS $Default %CPAN_METHODS @CPAN_OPTIONS  @option_order
	%Method_table %Method_table_index );

@META_OPTIONS = qw( h v V I g G M: C A D O l L a r p P j: J w x X );

$Default = 'default';

%CPAN_METHODS = ( # map switches to method names in CPAN::Shell
	$Default => 'install',
	'c'      => 'clean',
	'f'      => 'force',
	'i'      => 'install',
	'm'      => 'make',
	't'      => 'test',
	'u'      => 'upgrade',
	'T'      => 'notest',
	's'      => 'shell',
	);
@CPAN_OPTIONS = grep { $_ ne $Default } sort keys %CPAN_METHODS;

@option_order = ( @META_OPTIONS, @CPAN_OPTIONS );


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# map switches to the subroutines in this script, along with other information.
# use this stuff instead of hard-coded indices and values
sub NO_ARGS   () { 0 }
sub ARGS      () { 1 }
sub GOOD_EXIT () { 0 }

%Method_table = (
# key => [ sub ref, takes args?, exit value, description ]

	# options that do their thing first, then exit
	h =>  [ \&_print_help,          NO_ARGS, GOOD_EXIT, 'Printing help'                ],
	v =>  [ \&_print_version,       NO_ARGS, GOOD_EXIT, 'Printing version'             ],
	V =>  [ \&_print_details,       NO_ARGS, GOOD_EXIT, 'Printing detailed version'    ],
	X =>  [ \&_list_all_namespaces, NO_ARGS, GOOD_EXIT, 'Listing all namespaces'       ],

	# options that affect other options
	j =>  [ \&_load_config,          ARGS, GOOD_EXIT, 'Use specified config file'    ],
	J =>  [ \&_dump_config,       NO_ARGS, GOOD_EXIT, 'Dump configuration to stdout' ],
	F =>  [ \&_lock_lobotomy,     NO_ARGS, GOOD_EXIT, 'Turn off CPAN.pm lock files'  ],
	I =>  [ \&_load_local_lib,    NO_ARGS, GOOD_EXIT, 'Loading local::lib'           ],
	M =>  [ \&_use_these_mirrors,    ARGS, GOOD_EXIT, 'Setting per session mirrors'  ],
	P =>  [ \&_find_good_mirrors, NO_ARGS, GOOD_EXIT, 'Finding good mirrors'         ],
	w =>  [ \&_turn_on_warnings,  NO_ARGS, GOOD_EXIT, 'Turning on warnings'          ],

	# options that do their one thing
	g =>  [ \&_download,             ARGS, GOOD_EXIT, 'Download the latest distro'        ],
	G =>  [ \&_gitify,               ARGS, GOOD_EXIT, 'Down and gitify the latest distro' ],

	C =>  [ \&_show_Changes,         ARGS, GOOD_EXIT, 'Showing Changes file'         ],
	A =>  [ \&_show_Author,          ARGS, GOOD_EXIT, 'Showing Author'               ],
	D =>  [ \&_show_Details,         ARGS, GOOD_EXIT, 'Showing Details'              ],
	O =>  [ \&_show_out_of_date,  NO_ARGS, GOOD_EXIT, 'Showing Out of date'          ],
	l =>  [ \&_list_all_mods,     NO_ARGS, GOOD_EXIT, 'Listing all modules'          ],

	L =>  [ \&_show_author_mods,     ARGS, GOOD_EXIT, 'Showing author mods'          ],
	a =>  [ \&_create_autobundle, NO_ARGS, GOOD_EXIT, 'Creating autobundle'          ],
	p =>  [ \&_ping_mirrors,      NO_ARGS, GOOD_EXIT, 'Pinging mirrors'              ],

	r =>  [ \&_recompile,         NO_ARGS, GOOD_EXIT, 'Recompiling'                  ],
	u =>  [ \&_upgrade,           NO_ARGS, GOOD_EXIT, 'Running `make test`'          ],
	's' => [ \&_shell,            NO_ARGS, GOOD_EXIT, 'Drop into the CPAN.pm shell'  ],

	'x' => [ \&_guess_namespace,     ARGS, GOOD_EXIT, 'Guessing namespaces'          ],
	c =>  [ \&_default,              ARGS, GOOD_EXIT, 'Running `make clean`'         ],
	f =>  [ \&_default,              ARGS, GOOD_EXIT, 'Installing with force'        ],
	i =>  [ \&_default,              ARGS, GOOD_EXIT, 'Running `make install`'       ],
	'm' => [ \&_default,             ARGS, GOOD_EXIT, 'Running `make`'               ],
	t =>  [ \&_default,              ARGS, GOOD_EXIT, 'Running `make test`'          ],
	T =>  [ \&_default,              ARGS, GOOD_EXIT, 'Installing with notest'       ],
	);

%Method_table_index = (
	code        => 0,
	takes_args  => 1,
	exit_value  => 2,
	description => 3,
	);
}


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# finally, do some argument processing

sub _stupid_interface_hack_for_non_rtfmers
	{
	no warnings 'uninitialized';
	shift @ARGV if( $ARGV[0] eq 'install' and @ARGV > 1 )
	}

sub _process_options
	{
	my %options;

	push @ARGV, grep $_, split /\s+/, $ENV{CPAN_OPTS} || '';

	# if no arguments, just drop into the shell
	if( 0 == @ARGV ) { CPAN::shell(); exit 0 }
	elsif (Getopt::Std::getopts(
		  join( '', @option_order ), \%options ))
		{
		 \%options;
		}
	else { exit 1 }
}

sub _process_setup_options
	{
	my( $class, $options ) = @_;

	if( $options->{j} )
		{
		$Method_table{j}[ $Method_table_index{code} ]->( $options->{j} );
		delete $options->{j};
		}
	elsif ( ! $options->{h} ) { # h "ignores all of the other options and arguments"
		# this is what CPAN.pm would do otherwise
		local $CPAN::Be_Silent = 1;
		CPAN::HandleConfig->load(
			# be_silent  => 1, deprecated
			write_file => 0,
			);
		}

	$class->_turn_off_testing if $options->{T};

	foreach my $o ( qw(F I w P M) )
		{
		next unless exists $options->{$o};
		$Method_table{$o}[ $Method_table_index{code} ]->( $options->{$o} );
		delete $options->{$o};
		}

	if( $options->{o} )
		{
		my @pairs = map { [ split /=/, $_, 2 ] } split /,/, $options->{o};
		foreach my $pair ( @pairs )
			{
			my( $setting, $value ) = @$pair;
			$CPAN::Config->{$setting} = $value;
		#	$logger->debug( "Setting [$setting] to [$value]" );
			}
		delete $options->{o};
		}

	my $option_count = grep { $options->{$_} } @option_order;
	no warnings 'uninitialized';

	# don't count options that imply installation
	foreach my $opt ( qw(f T) ) { # don't count force or notest
		$option_count -= $options->{$opt};
		}

	# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
	# if there are no options, set -i (this line fixes RT ticket 16915)
	$options->{i}++ unless $option_count;
	}

sub _setup_environment {
# should we override or set defaults? If this were a true interactive
# session, we'd be in the CPAN shell.

# https://github.com/Perl-Toolchain-Gang/toolchain-site/blob/master/lancaster-consensus.md
	$ENV{NONINTERACTIVE_TESTING} = 1 unless defined $ENV{NONINTERACTIVE_TESTING};
	$ENV{PERL_MM_USE_DEFAULT}    = 1 unless defined $ENV{PERL_MM_USE_DEFAULT};
	}

=item run( ARGS )

Just do it.

The C<run> method returns 0 on success and a positive number on
failure. See the section on EXIT CODES for details on the values.

=cut

my $logger;

sub run
	{
	my( $class, @args ) = @_;
	local @ARGV = @args;
	my $return_value = HEY_IT_WORKED; # assume that things will work

	$logger = $class->_init_logger;
	$logger->debug( "Using logger from @{[ref $logger]}" );

	$class->_hook_into_CPANpm_report;
	$logger->debug( "Hooked into output" );

	$class->_stupid_interface_hack_for_non_rtfmers;
	$logger->debug( "Patched cargo culting" );

	my $options = $class->_process_options;
	$logger->debug( "Options are @{[Dumper($options)]}" );

	$class->_process_setup_options( $options );

	$class->_setup_environment( $options );

	OPTION: foreach my $option ( @option_order )
		{
		next unless $options->{$option};

		my( $sub, $takes_args, $description ) =
			map { $Method_table{$option}[ $Method_table_index{$_} ] }
			qw( code takes_args description );

		unless( ref $sub eq ref sub {} )
			{
			$return_value = THE_PROGRAMMERS_AN_IDIOT;
			last OPTION;
			}

		$logger->info( "[$option] $description -- ignoring other arguments" )
			if( @ARGV && ! $takes_args );

		$return_value = $sub->( \ @ARGV, $options );

		last;
		}

	return $return_value;
	}

my $LEVEL;
{
package
  Local::Null::Logger; # hide from PAUSE

my @LOGLEVELS = qw(TRACE DEBUG INFO WARN ERROR FATAL);
$LEVEL        = uc($ENV{CPANSCRIPT_LOGLEVEL} || 'INFO');
my %LL        = map { $LOGLEVELS[$_] => $_ } 0..$#LOGLEVELS;
unless (defined $LL{$LEVEL}){
	warn "Unsupported loglevel '$LEVEL', setting to INFO";
	$LEVEL = 'INFO';
}
sub new { bless \ my $x, $_[0] }
sub AUTOLOAD {
	my $autoload = our $AUTOLOAD;
	$autoload =~ s/.*://;
	return if $LL{uc $autoload} < $LL{$LEVEL};
	$CPAN::Frontend->mywarn(">($autoload): $_\n")
		for split /[\r\n]+/, $_[1];
}
sub DESTROY { 1 }
}

# load a module without searching the default entry for the current
# directory
sub _safe_load_module {
	my $name = shift;

	local @INC = @INC;
	pop @INC if $INC[-1] eq '.';

	eval "require $name; 1";
}

sub _init_logger
	{
	my $log4perl_loaded = _safe_load_module("Log::Log4perl");

	unless( $log4perl_loaded )
		{
		print STDOUT "Loading internal logger. Log::Log4perl recommended for better logging\n";
		$logger = Local::Null::Logger->new;
		return $logger;
		}

	Log::Log4perl::init( \ <<"HERE" );
log4perl.rootLogger=$LEVEL, A1
log4perl.appender.A1=Log::Log4perl::Appender::Screen
log4perl.appender.A1.layout=PatternLayout
log4perl.appender.A1.layout.ConversionPattern=%m%n
HERE

	$logger = Log::Log4perl->get_logger( 'App::Cpan' );
	}

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
 # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

sub _default
	{
	my( $args, $options ) = @_;

	my $switch = '';

	# choose the option that we're going to use
	# we'll deal with 'f' (force) later, so skip it
	foreach my $option ( @CPAN_OPTIONS )
		{
		next if ( $option eq 'f' or $option eq 'T' );
		next unless $options->{$option};
		$switch = $option;
		last;
		}

	# 1. with no switches, but arguments, use the default switch (install)
	# 2. with no switches and no args, start the shell
	# 3. With a switch but no args, die! These switches need arguments.
	   if( not $switch and     @$args ) { $switch = $Default;  }
	elsif( not $switch and not @$args ) { return CPAN::shell() }
	elsif(     $switch and not @$args )
		{ die "Nothing to $CPAN_METHODS{$switch}!\n"; }

	# Get and check the method from CPAN::Shell
	my $method = $CPAN_METHODS{$switch};
	die "CPAN.pm cannot $method!\n" unless CPAN::Shell->can( $method );

	# call the CPAN::Shell method, with force or notest if specified
	my $action = do {
		   if( $options->{f} ) { sub { CPAN::Shell->force( $method, @_ )  } }
		elsif( $options->{T} ) { sub { CPAN::Shell->notest( $method, @_ ) } }
		else                   { sub { CPAN::Shell->$method( @_ )         } }
		};

	# How do I handle exit codes for multiple arguments?
	my @errors = ();

	$options->{x} or _disable_guessers();

	foreach my $arg ( @$args )
		{
		# check the argument and perhaps capture typos
		my $module = _expand_module( $arg ) or do {
			$logger->error( "Skipping $arg because I couldn't find a matching namespace." );
			next;
			};

		_clear_cpanpm_output();
		$action->( $arg );

		my $error = _cpanpm_output_indicates_failure();
		push @errors, $error if $error;
		}

	return do {
		if( @errors ) { $errors[0] }
		else { HEY_IT_WORKED }
		};

	}

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

=for comment

CPAN.pm sends all the good stuff either to STDOUT, or to a temp
file if $CPAN::Be_Silent is set. I have to intercept that output
so I can find out what happened.

=cut

BEGIN {
my $scalar = '';

sub _hook_into_CPANpm_report
	{
	no warnings 'redefine';

	*CPAN::Shell::myprint = sub {
		my($self,$what) = @_;
		$scalar .= $what if defined $what;
		$self->print_ornamented($what,
			$CPAN::Config->{colorize_print}||'bold blue on_white',
			);
		};

	*CPAN::Shell::mywarn = sub {
		my($self,$what) = @_;
		$scalar .= $what if defined $what;
		$self->print_ornamented($what,
			$CPAN::Config->{colorize_warn}||'bold red on_white'
			);
		};

	}

sub _clear_cpanpm_output { $scalar = '' }

sub _get_cpanpm_output   { $scalar }

# These are lines I don't care about in CPAN.pm output. If I can
# filter out the informational noise, I have a better chance to
# catch the error signal
my @skip_lines = (
	qr/^\QWarning \(usually harmless\)/,
	qr/\bwill not store persistent state\b/,
	qr(//hint//),
	qr/^\s+reports\s+/,
	qr/^Try the command/,
	qr/^\s+$/,
	qr/^to find objects/,
	qr/^\s*Database was generated on/,
	qr/^Going to read/,
	qr|^\s+i\s+/|,    # the i /Foo::Whatever/ line when it doesn't know
	);

sub _get_cpanpm_last_line
	{
	my $fh;

	if( $] < 5.008 ) {
		$fh = IO::Scalar->new( \ $scalar );
		}
	else {
		eval q{ open $fh, '<', \\ $scalar; };
		}

	my @lines = <$fh>;

	# This is a bit ugly. Once we examine a line, we have to
	# examine the line before it and go through all of the same
	# regexes. I could do something fancy, but this works.
	REGEXES: {
	foreach my $regex ( @skip_lines )
		{
		if( $lines[-1] =~ m/$regex/ )
			{
			pop @lines;
			redo REGEXES; # we have to go through all of them for every line!
			}
		}
	}

	$logger->debug( "Last interesting line of CPAN.pm output is:\n\t$lines[-1]" );

	$lines[-1];
	}
}

BEGIN {
my $epic_fail_words = join '|',
	qw( Error stop(?:ping)? problems force not unsupported
		fail(?:ed)? Cannot\s+install );

sub _cpanpm_output_indicates_failure
	{
	my $last_line = _get_cpanpm_last_line();

	my $result = $last_line =~ /\b(?:$epic_fail_words)\b/i;
	return A_MODULE_FAILED_TO_INSTALL if $last_line =~ /\b(?:Cannot\s+install)\b/i;

	$result || ();
	}
}

sub _cpanpm_output_indicates_success
	{
	my $last_line = _get_cpanpm_last_line();

	my $result = $last_line =~ /\b(?:\s+-- OK|PASS)\b/;
	$result || ();
	}

sub _cpanpm_output_is_vague
	{
	return FALSE if
		_cpanpm_output_indicates_failure() ||
		_cpanpm_output_indicates_success();

	return TRUE;
	}

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
sub _turn_on_warnings {
	carp "Warnings are implemented yet";
	return HEY_IT_WORKED;
	}

sub _turn_off_testing {
	$logger->debug( 'Trusting test report history' );
	$CPAN::Config->{trust_test_report_history} = 1;
	return HEY_IT_WORKED;
	}

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
sub _print_help
	{
	$logger->info( "Use perldoc to read the documentation" );
	my $HAVE_PERLDOC = eval { require Pod::Perldoc; 1; };
	if ($HAVE_PERLDOC) {
		system qq{"$^X" -e "require Pod::Perldoc; Pod::Perldoc->run()" $0};
		exit;
	} else {
		warn "Please install Pod::Perldoc, maybe try 'cpan -i Pod::Perldoc'\n";
		return HEY_IT_WORKED;
	}
	}

sub _print_version # -v
	{
	$logger->info(
		"$0 script version $VERSION, CPAN.pm version " . CPAN->VERSION );

	return HEY_IT_WORKED;
	}

sub _print_details # -V
	{
	_print_version();

	_check_install_dirs();

	$logger->info( '-' x 50 . "\nChecking configured mirrors..." );
	foreach my $mirror ( @{ $CPAN::Config->{urllist} } ) {
		_print_ping_report( $mirror );
		}

	$logger->info( '-' x 50 . "\nChecking for faster mirrors..." );

	{
	require CPAN::Mirrors;

	if ( $CPAN::Config->{connect_to_internet_ok} ) {
		$CPAN::Frontend->myprint(qq{Trying to fetch a mirror list from the Internet\n});
		eval { CPAN::FTP->localize('MIRRORED.BY',File::Spec->catfile($CPAN::Config->{keep_source_where},'MIRRORED.BY'),3,1) }
			or $CPAN::Frontend->mywarn(<<'HERE');
We failed to get a copy of the mirror list from the Internet.
You will need to provide CPAN mirror URLs yourself.
HERE
		$CPAN::Frontend->myprint("\n");
		}

	my $mirrors   = CPAN::Mirrors->new( _mirror_file() );
	my @continents = $mirrors->find_best_continents;

	my @mirrors   = $mirrors->get_mirrors_by_continents( $continents[0] );
	my @timings   = $mirrors->get_mirrors_timings( \@mirrors );

	foreach my $timing ( @timings ) {
		$logger->info( sprintf "%s (%0.2f ms)",
			$timing->hostname, $timing->rtt );
		}
	}

	return HEY_IT_WORKED;
	}

sub _check_install_dirs
	{
	my $makepl_arg   = $CPAN::Config->{makepl_arg};
	my $mbuildpl_arg = $CPAN::Config->{mbuildpl_arg};

	my @custom_dirs;
	# PERL_MM_OPT
	push @custom_dirs,
		$makepl_arg   =~ m/INSTALL_BASE\s*=\s*(\S+)/g,
		$mbuildpl_arg =~ m/--install_base\s*=\s*(\S+)/g;

	if( @custom_dirs ) {
		foreach my $dir ( @custom_dirs ) {
			_print_inc_dir_report( $dir );
			}
		}

	# XXX: also need to check makepl_args, etc

	my @checks = (
		[ 'core',         [ grep $_, @Config{qw(installprivlib installarchlib)}      ] ],
		[ 'vendor',       [ grep $_, @Config{qw(installvendorlib installvendorarch)} ] ],
		[ 'site',         [ grep $_, @Config{qw(installsitelib installsitearch)}     ] ],
		[ 'PERL5LIB',     _split_paths( $ENV{PERL5LIB} ) ],
		[ 'PERLLIB',      _split_paths( $ENV{PERLLIB} )  ],
		);

	$logger->info( '-' x 50 . "\nChecking install dirs..." );
	foreach my $tuple ( @checks ) {
		my( $label ) = $tuple->[0];

		$logger->info( "Checking $label" );
		$logger->info( "\tno directories for $label" ) unless @{ $tuple->[1] };
		foreach my $dir ( @{ $tuple->[1] } ) {
			_print_inc_dir_report( $dir );
			}
		}

	}

sub _split_paths
	{
	[ map { _expand_filename( $_ ) } split /$Config{path_sep}/, $_[0] || '' ];
	}


=pod

Stolen from File::Path::Expand

=cut

sub _expand_filename
	{
	my( $path ) = @_;
	no warnings 'uninitialized';
	$logger->debug( "Expanding path $path\n" );
	$path =~ s{\A~([^/]+)?}{
		_home_of( $1 || $> ) || "~$1"
		}e;
	return $path;
	}

sub _home_of
	{
	require User::pwent;
	my( $user ) = @_;
	my $ent = User::pwent::getpw($user) or return;
	return $ent->dir;
	}

sub _get_default_inc
	{
	require Config;

	[ @Config::Config{ _vars() }, '.' ];
	}

sub _vars {
	qw(
	installarchlib
	installprivlib
	installsitearch
	installsitelib
	);
	}

sub _ping_mirrors {
	my $urls   = $CPAN::Config->{urllist};
	require URI;

	foreach my $url ( @$urls ) {
		my( $obj ) = URI->new( $url );
		next unless _is_pingable_scheme( $obj );
		my $host = $obj->host;
		_print_ping_report( $obj );
		}

	}

sub _is_pingable_scheme {
	my( $uri ) = @_;

	$uri->scheme eq 'file'
	}

sub _mirror_file {
	my $file = do {
		my $file = 'MIRRORED.BY';
		my $local_path = File::Spec->catfile(
			$CPAN::Config->{keep_source_where}, $file );

		if( -e $local_path ) { $local_path }
		else {
			require CPAN::FTP;
			CPAN::FTP->localize( $file, $local_path, 3, 1 );
			$local_path;
			}
		};
	}

sub _find_good_mirrors {
	require CPAN::Mirrors;

	my $mirrors = CPAN::Mirrors->new( _mirror_file() );

	my @mirrors = $mirrors->best_mirrors(
		how_many   => 5,
		verbose    => 1,
		);

	foreach my $mirror ( @mirrors ) {
		next unless eval { $mirror->can( 'http' ) };
		_print_ping_report( $mirror->http );
		}

	$CPAN::Config->{urllist} = [
		map { $_->http } @mirrors
		];
	}

sub _print_inc_dir_report
	{
	my( $dir ) = shift;

	my $writeable = -w $dir ? '+' : '!!! (not writeable)';
	$logger->info( "\t$writeable $dir" );
	return -w $dir;
	}

sub _print_ping_report
	{
	my( $mirror ) = @_;

	my $rtt = eval { _get_ping_report( $mirror ) };
	my $result = $rtt ? sprintf "+ (%4d ms)", $rtt * 1000 : '!';

	$logger->info(
		sprintf "\t%s %s", $result, $mirror
		);
	}

sub _get_ping_report
	{
	require URI;
	my( $mirror ) = @_;
	my( $url ) = ref $mirror ? $mirror : URI->new( $mirror ); #XXX
	require Net::Ping;

	my $ping = Net::Ping->new( 'tcp', 1 );

	if( $url->scheme eq 'file' ) {
		return -e $url->file;
		}

	my( $port ) = $url->port;

	return unless $port;

	if ( $ping->can('port_number') ) {
		$ping->port_number($port);
		}
	else {
		$ping->{'port_num'} = $port;
		}

	$ping->hires(1) if $ping->can( 'hires' );
	my( $alive, $rtt ) = eval{ $ping->ping( $url->host ) };
	$alive ? $rtt : undef;
	}

sub _load_local_lib # -I
	{
	$logger->debug( "Loading local::lib" );

	my $rc = _safe_load_module("local::lib");
	unless( $rc ) {
		$logger->logdie( "Could not load local::lib" );
		}

	local::lib->import;

	return HEY_IT_WORKED;
	}

sub _use_these_mirrors # -M
	{
	$logger->debug( "Setting per session mirrors" );
	unless( $_[0] ) {
		$logger->logdie( "The -M switch requires a comma-separated list of mirrors" );
		}

	$CPAN::Config->{urllist} = [ split /,/, $_[0] ];

	$logger->debug( "Mirrors are @{$CPAN::Config->{urllist}}" );

	}

sub _create_autobundle
	{
	$logger->info(
		"Creating autobundle in $CPAN::Config->{cpan_home}/Bundle" );

	CPAN::Shell->autobundle;

	return HEY_IT_WORKED;
	}

sub _recompile
	{
	$logger->info( "Recompiling dynamically-loaded extensions" );

	CPAN::Shell->recompile;

	return HEY_IT_WORKED;
	}

sub _upgrade
	{
	$logger->info( "Upgrading all modules" );

	CPAN::Shell->upgrade();

	return HEY_IT_WORKED;
	}

sub _shell
	{
	$logger->info( "Dropping into shell" );

	CPAN::shell();

	return HEY_IT_WORKED;
	}

sub _load_config # -j
	{
	my $argument = shift;

	my $file = file_name_is_absolute( $argument ) ? $argument : rel2abs( $argument );
	croak( "cpan config file [$file] for -j does not exist!\n" ) unless -e $file;

	# should I clear out any existing config here?
	$CPAN::Config = {};
	delete $INC{'CPAN/Config.pm'};

	my $rc = eval "require '$file'";

	# CPAN::HandleConfig::require_myconfig_or_config looks for this
	$INC{'CPAN/MyConfig.pm'} = 'fake out!';

	# CPAN::HandleConfig::load looks for this
	$CPAN::Config_loaded = 'fake out';

	croak( "Could not load [$file]: $@\n") unless $rc;

	return HEY_IT_WORKED;
	}

sub _dump_config # -J
	{
	my $args = shift;
	require Data::Dumper;

	my $fh = $args->[0] || \*STDOUT;

	local $Data::Dumper::Sortkeys = 1;
	my $dd = Data::Dumper->new(
		[$CPAN::Config],
		['$CPAN::Config']
		);

	print $fh $dd->Dump, "\n1;\n__END__\n";

	return HEY_IT_WORKED;
	}

sub _lock_lobotomy # -F
	{
	no warnings 'redefine';

	*CPAN::_flock    = sub { 1 };
	*CPAN::checklock = sub { 1 };

	return HEY_IT_WORKED;
	}

sub _download
	{
	my $args = shift;

	local $CPAN::DEBUG = 1;

	my %paths;

	foreach my $arg ( @$args ) {
		$logger->info( "Checking $arg" );

		my $module = _expand_module( $arg ) or next;
		my $path = $module->cpan_file;

		$logger->debug( "Inst file would be $path\n" );

		$paths{$module} = _get_file( _make_path( $path ) );

		$logger->info( "Downloaded [$arg] to [$paths{$arg}]" );
		}

	return \%paths;
	}

sub _make_path { join "/", qw(authors id), $_[0] }

sub _get_file
	{
	my $path = shift;

	my $loaded = _safe_load_module("LWP::Simple");
	croak "You need LWP::Simple to use features that fetch files from CPAN\n"
		unless $loaded;

	my $file = substr $path, rindex( $path, '/' ) + 1;
	my $store_path = catfile( cwd(), $file );
	$logger->debug( "Store path is $store_path" );

	foreach my $site ( @{ $CPAN::Config->{urllist} } )
		{
		my $fetch_path = join "/", $site, $path;
		$logger->debug( "Trying $fetch_path" );
		my $status_code = LWP::Simple::getstore( $fetch_path, $store_path );
		last if( 200 <= $status_code and $status_code <= 300 );
		$logger->warn( "Could not get [$fetch_path]: Status code $status_code" );
		}

	return $store_path;
	}

sub _gitify
	{
	my $args = shift;

	my $loaded = _safe_load_module("Archive::Extract");
	croak "You need Archive::Extract to use features that gitify distributions\n"
		unless $loaded;

	my $starting_dir = cwd();

	foreach my $arg ( @$args )
		{
		$logger->info( "Checking $arg" );
		my $store_paths = _download( [ $arg ] );
		$logger->debug( "gitify Store path is $store_paths->{$arg}" );
		my $dirname = dirname( $store_paths->{$arg} );

		my $ae = Archive::Extract->new( archive => $store_paths->{$arg} );
		$ae->extract( to => $dirname );

		chdir $ae->extract_path;

		my $git = $ENV{GIT_COMMAND} || '/usr/local/bin/git';
		croak "Could not find $git"    unless -e $git;
		croak "$git is not executable" unless -x $git;

		# can we do this in Pure Perl?
		system( $git, 'init'    );
		system( $git, qw( add . ) );
		system( $git, qw( commit -a -m ), 'initial import' );
		}

	chdir $starting_dir;

	return HEY_IT_WORKED;
	}

sub _show_Changes
	{
	my $args = shift;

	foreach my $arg ( @$args )
		{
		$logger->info( "Checking $arg\n" );

		my $module = _expand_module( $arg ) or next;

		my $out = _get_cpanpm_output();

		next unless eval { $module->inst_file };
		#next if $module->uptodate;

		( my $id = $module->id() ) =~ s/::/\-/;

		my $url = "http://search.cpan.org/~" . lc( $module->userid ) . "/" .
			$id . "-" . $module->cpan_version() . "/";

		#print "URL: $url\n";
		_get_changes_file($url);
		}

	return HEY_IT_WORKED;
	}

sub _get_changes_file
	{
	croak "Reading Changes files requires LWP::Simple and URI\n"
		unless _safe_load_module("LWP::Simple") && _safe_load_module("URI");

	my $url = shift;

	my $content = LWP::Simple::get( $url );
	$logger->info( "Got $url ..." ) if defined $content;
	#print $content;

	my( $change_link ) = $content =~ m|<a href="(.*?)">Changes</a>|gi;

	my $changes_url = URI->new_abs( $change_link, $url );
	$logger->debug( "Change link is: $changes_url" );

	my $changes =  LWP::Simple::get( $changes_url );

	print $changes;

	return HEY_IT_WORKED;
	}

sub _show_Author
	{
	my $args = shift;

	foreach my $arg ( @$args )
		{
		my $module = _expand_module( $arg ) or next;

		unless( $module )
			{
			$logger->info( "Didn't find a $arg module, so no author!" );
			next;
			}

		my $author = CPAN::Shell->expand( "Author", $module->userid );

		next unless $module->userid;

		printf "%-25s %-8s %-25s %s\n",
			$arg, $module->userid, $author->email, $author->name;
		}

	return HEY_IT_WORKED;
	}

sub _show_Details
	{
	my $args = shift;

	foreach my $arg ( @$args )
		{
		my $module = _expand_module( $arg ) or next;
		my $author = CPAN::Shell->expand( "Author", $module->userid );

		next unless $module->userid;

		print "$arg\n", "-" x 73, "\n\t";
		print join "\n\t",
			$module->description ? $module->description : "(no description)",
			$module->cpan_file ? $module->cpan_file : "(no cpanfile)",
			$module->inst_file ? $module->inst_file :"(no installation file)" ,
			'Installed: ' . ($module->inst_version ? $module->inst_version : "not installed"),
			'CPAN:      ' . $module->cpan_version . '  ' .
				($module->uptodate ? "" : "Not ") . "up to date",
			$author->fullname . " (" . $module->userid . ")",
			$author->email;
		print "\n\n";

		}

	return HEY_IT_WORKED;
	}

BEGIN {
my $modules;
sub _get_all_namespaces
	{
	return $modules if $modules;
	$modules = [ map { $_->id } CPAN::Shell->expand( "Module", "/./" ) ];
	}
}

sub _show_out_of_date
	{
	my $modules = _get_all_namespaces();

	printf "%-40s  %6s  %6s\n", "Module Name", "Local", "CPAN";
	print "-" x 73, "\n";

	foreach my $module ( @$modules )
		{
		next unless $module = _expand_module($module);
		next unless $module->inst_file;
		next if $module->uptodate;
		printf "%-40s  %.4f  %.4f\n",
			$module->id,
			$module->inst_version ? $module->inst_version : '',
			$module->cpan_version;
		}

	return HEY_IT_WORKED;
	}

sub _show_author_mods
	{
	my $args = shift;

	my %hash = map { lc $_, 1 } @$args;

	my $modules = _get_all_namespaces();

	foreach my $module ( @$modules ) {
		next unless exists $hash{ lc $module->userid };
		print $module->id, "\n";
		}

	return HEY_IT_WORKED;
	}

sub _list_all_mods # -l
	{
	require File::Find;

	my $args = shift;


	my $fh = \*STDOUT;

	INC: foreach my $inc ( @INC )
		{
		my( $wanted, $reporter ) = _generator();
		File::Find::find( { wanted => $wanted }, $inc );

		my $count = 0;
		FILE: foreach my $file ( @{ $reporter->() } )
			{
			my $version = _parse_version_safely( $file );

			my $module_name = _path_to_module( $inc, $file );
			next FILE unless defined $module_name;

			print $fh "$module_name\t$version\n";

			#last if $count++ > 5;
			}
		}

	return HEY_IT_WORKED;
	}

sub _generator
	{
	my @files = ();

	sub { push @files,
		File::Spec->canonpath( $File::Find::name )
		if m/\A\w+\.pm\z/ },
	sub { \@files },
	}

sub _parse_version_safely # stolen from PAUSE's mldistwatch, but refactored
	{
	my( $file ) = @_;

	local $/ = "\n";
	local $_; # don't mess with the $_ in the map calling this

	return unless open FILE, "<$file";

	my $in_pod = 0;
	my $version;
	while( <FILE> )
		{
		chomp;
		$in_pod = /^=(?!cut)/ ? 1 : /^=cut/ ? 0 : $in_pod;
		next if $in_pod || /^\s*#/;

		next unless /([\$*])(([\w\:\']*)\bVERSION)\b.*\=/;
		my( $sigil, $var ) = ( $1, $2 );

		$version = _eval_version( $_, $sigil, $var );
		last;
		}
	close FILE;

	return 'undef' unless defined $version;

	return $version;
	}

sub _eval_version
	{
	my( $line, $sigil, $var ) = @_;

        # split package line to hide from PAUSE
	my $eval = qq{
		package
		  ExtUtils::MakeMaker::_version;

		local $sigil$var;
		\$$var=undef; do {
			$line
			}; \$$var
		};

	my $version = do {
		local $^W = 0;
		no strict;
		eval( $eval );
		};

	return $version;
	}

sub _path_to_module
	{
	my( $inc, $path ) = @_;
	return if length $path < length $inc;

	my $module_path = substr( $path, length $inc );
	$module_path =~ s/\.pm\z//;

	# XXX: this is cheating and doesn't handle everything right
	my @dirs = grep { ! /\W/ } File::Spec->splitdir( $module_path );
	shift @dirs;

	my $module_name = join "::", @dirs;

	return $module_name;
	}


sub _expand_module
	{
	my( $module ) = @_;

	my $expanded = CPAN::Shell->expandany( $module );
	return $expanded if $expanded;
	$expanded = CPAN::Shell->expand( "Module", $module );
	unless( defined $expanded ) {
		$logger->error( "Could not expand [$module]. Check the module name." );
		my $threshold = (
			grep { int }
			sort { length $a <=> length $b }
				length($module)/4, 4
			)[0];

		my $guesses = _guess_at_module_name( $module, $threshold );
		if( defined $guesses and @$guesses ) {
			$logger->info( "Perhaps you meant one of these:" );
			foreach my $guess ( @$guesses ) {
				$logger->info( "\t$guess" );
				}
			}
		return;
		}

	return $expanded;
	}

my $guessers = [
	[ qw( Text::Levenshtein::XS distance 7 1 ) ],
	[ qw( Text::Levenshtein::Damerau::XS     xs_edistance 7 1 ) ],

	[ qw( Text::Levenshtein     distance 7 1 ) ],
	[ qw( Text::Levenshtein::Damerau::PP     pp_edistance 7 1 ) ],

	];

sub _disable_guessers
	{
	$_->[-1] = 0 for @$guessers;
	}

# for -x
sub _guess_namespace
	{
	my $args = shift;

	foreach my $arg ( @$args )
		{
		$logger->debug( "Checking $arg" );
		my $guesses = _guess_at_module_name( $arg );

		foreach my $guess ( @$guesses ) {
			print $guess, "\n";
			}
		}

	return HEY_IT_WORKED;
	}

sub _list_all_namespaces {
	my $modules = _get_all_namespaces();

	foreach my $module ( @$modules ) {
		print $module, "\n";
		}
	}

BEGIN {
my $distance;
my $_threshold;
my $can_guess;
my $shown_help = 0;
sub _guess_at_module_name
	{
	my( $target, $threshold ) = @_;

	unless( defined $distance ) {
		foreach my $try ( @$guessers ) {
			$can_guess = eval "require $try->[0]; 1" or next;

			$try->[-1] or next; # disabled
			no strict 'refs';
			$distance = \&{ join "::", @$try[0,1] };
			$threshold ||= $try->[2];
			}
		}
	$_threshold ||= $threshold;

	unless( $distance ) {
		unless( $shown_help ) {
			my $modules = join ", ", map { $_->[0] } @$guessers;
			substr $modules, rindex( $modules, ',' ), 1, ', and';

			# Should this be colorized?
			if( $can_guess ) {
				$logger->info( "I can suggest names if you provide the -x option on invocation." );
				}
			else {
				$logger->info( "I can suggest names if you install one of $modules" );
				$logger->info( "and you provide the -x option on invocation." );
				}
			$shown_help++;
			}
		return;
		}

	my $modules = _get_all_namespaces();
	$logger->info( "Checking " . @$modules . " namespaces for close match suggestions" );

	my %guesses;
	foreach my $guess ( @$modules ) {
		my $distance = $distance->( $target, $guess );
		next if $distance > $_threshold;
		$guesses{$guess} = $distance;
		}

	my @guesses = sort { $guesses{$a} <=> $guesses{$b} } keys %guesses;
	return [ grep { defined } @guesses[0..9] ];
	}
}

1;

=back

=head1 EXIT VALUES

The script exits with zero if it thinks that everything worked, or a
positive number if it thinks that something failed. Note, however, that
in some cases it has to divine a failure by the output of things it does
not control. For now, the exit codes are vague:

	1	An unknown error

	2	The was an external problem

	4	There was an internal problem with the script

	8	A module failed to install

=head1 TO DO

* There is initial support for Log4perl if it is available, but I
haven't gone through everything to make the NullLogger work out
correctly if Log4perl is not installed.

* When I capture CPAN.pm output, I need to check for errors and
report them to the user.

* Warnings switch

* Check then exit

=head1 BUGS

* none noted

=head1 SEE ALSO

L<CPAN>, L<App::cpanminus>

=head1 SOURCE AVAILABILITY

This code is in Github in the CPAN.pm repository:

	https://github.com/andk/cpanpm

The source used to be tracked separately in another GitHub repo,
but the canonical source is now in the above repo.

=head1 CREDITS

Japheth Cleaver added the bits to allow a forced install (C<-f>).

Jim Brandt suggested and provided the initial implementation for the
up-to-date and Changes features.

Adam Kennedy pointed out that C<exit()> causes problems on Windows
where this script ends up with a .bat extension

David Golden helps integrate this into the C<CPAN.pm> repos.

Jim Keenan fixed up various issues with _download

=head1 AUTHOR

brian d foy, C<< <bdfoy@cpan.org> >>

=head1 COPYRIGHT

Copyright (c) 2001-2021, brian d foy, All Rights Reserved.

You may redistribute this under the same terms as Perl itself.

=cut

# Local Variables:
# mode: cperl
# indent-tabs-mode: t
# cperl-indent-level: 8
# cperl-continued-statement-offset: 8
# End:
