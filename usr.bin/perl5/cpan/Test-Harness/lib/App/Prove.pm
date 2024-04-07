package App::Prove;

use strict;
use warnings;

use TAP::Harness::Env;
use Text::ParseWords qw(shellwords);
use File::Spec;
use Getopt::Long;
use App::Prove::State;
use Carp;

use base 'TAP::Object';

=head1 NAME

App::Prove - Implements the C<prove> command.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 DESCRIPTION

L<Test::Harness> provides a command, C<prove>, which runs a TAP based
test suite and prints a report. The C<prove> command is a minimal
wrapper around an instance of this module.

=head1 SYNOPSIS

    use App::Prove;

    my $app = App::Prove->new;
    $app->process_args(@ARGV);
    $app->run;

=cut

use constant IS_WIN32 => ( $^O =~ /^(MS)?Win32$/ );
use constant IS_VMS => $^O eq 'VMS';
use constant IS_UNIXY => !( IS_VMS || IS_WIN32 );

use constant STATE_FILE => IS_UNIXY ? '.prove'   : '_prove';
use constant RC_FILE    => IS_UNIXY ? '.proverc' : '_proverc';

use constant PLUGINS => 'App::Prove::Plugin';

my @ATTR;

BEGIN {
    @ATTR = qw(
      archive argv blib show_count color directives exec failures comments
      formatter harness includes modules plugins jobs lib merge parse quiet
      really_quiet recurse backwards shuffle taint_fail taint_warn timer
      verbose warnings_fail warnings_warn show_help show_man show_version
      state_class test_args state dry extensions ignore_exit rules state_manager
      normalize sources tapversion trap
      statefile
    );
    __PACKAGE__->mk_methods(@ATTR);
}

=head1 METHODS

=head2 Class Methods

=head3 C<new>

Create a new C<App::Prove>. Optionally a hash ref of attribute
initializers may be passed.

=cut

# new() implementation supplied by TAP::Object

sub _initialize {
    my $self = shift;
    my $args = shift || {};

    my @is_array = qw(
      argv rc_opts includes modules state plugins rules sources
    );

    # setup defaults:
    for my $key (@is_array) {
        $self->{$key} = [];
    }

    for my $attr (@ATTR) {
        if ( exists $args->{$attr} ) {

            # TODO: Some validation here
            $self->{$attr} = $args->{$attr};
        }
    }

    $self->state_class('App::Prove::State');
    return $self;
}

=head3 C<state_class>

Getter/setter for the name of the class used for maintaining state.  This
class should either subclass from C<App::Prove::State> or provide an identical
interface.

=head3 C<state_manager>

Getter/setter for the instance of the C<state_class>.

=cut

=head3 C<add_rc_file>

    $prove->add_rc_file('myproj/.proverc');

Called before C<process_args> to prepend the contents of an rc file to
the options.

=cut

sub add_rc_file {
    my ( $self, $rc_file ) = @_;

    local *RC;
    open RC, "<$rc_file" or croak "Can't read $rc_file ($!)";
    while ( defined( my $line = <RC> ) ) {
        push @{ $self->{rc_opts} },
          grep { defined and not /^#/ }
          $line =~ m{ ' ([^']*) ' | " ([^"]*) " | (\#.*) | (\S+) }xg;
    }
    close RC;
}

=head3 C<process_args>

    $prove->process_args(@args);

Processes the command-line arguments. Attributes will be set
appropriately. Any filenames may be found in the C<argv> attribute.

Dies on invalid arguments.

=cut

sub process_args {
    my $self = shift;

    my @rc = RC_FILE;
    unshift @rc, glob '~/' . RC_FILE if IS_UNIXY;

    # Preprocess meta-args.
    my @args;
    while ( defined( my $arg = shift ) ) {
        if ( $arg eq '--norc' ) {
            @rc = ();
        }
        elsif ( $arg eq '--rc' ) {
            defined( my $rc = shift )
              or croak "Missing argument to --rc";
            push @rc, $rc;
        }
        elsif ( $arg =~ m{^--rc=(.+)$} ) {
            push @rc, $1;
        }
        else {
            push @args, $arg;
        }
    }

    # Everything after the arisdottle '::' gets passed as args to
    # test programs.
    if ( defined( my $stop_at = _first_pos( '::', @args ) ) ) {
        my @test_args = splice @args, $stop_at;
        shift @test_args;
        $self->{test_args} = \@test_args;
    }

    # Grab options from RC files
    $self->add_rc_file($_) for grep -f, @rc;
    unshift @args, @{ $self->{rc_opts} };

    if ( my @bad = map {"-$_"} grep {/^-(man|help)$/} @args ) {
        die "Long options should be written with two dashes: ",
          join( ', ', @bad ), "\n";
    }

    # And finally...

    {
        local @ARGV = @args;
        Getopt::Long::Configure(qw(no_ignore_case bundling pass_through));

        # Don't add coderefs to GetOptions
        GetOptions(
            'v|verbose'  => \$self->{verbose},
            'f|failures' => \$self->{failures},
            'o|comments' => \$self->{comments},
            'l|lib'      => \$self->{lib},
            'b|blib'     => \$self->{blib},
            's|shuffle'  => \$self->{shuffle},
            'color!'     => \$self->{color},
            'colour!'    => \$self->{color},
            'count!'     => \$self->{show_count},
            'c'          => \$self->{color},
            'D|dry'      => \$self->{dry},
            'ext=s@'     => sub {
                my ( $opt, $val ) = @_;

                # Workaround for Getopt::Long 2.25 handling of
                # multivalue options
                push @{ $self->{extensions} ||= [] }, $val;
            },
            'harness=s'    => \$self->{harness},
            'ignore-exit'  => \$self->{ignore_exit},
            'source=s@'    => $self->{sources},
            'formatter=s'  => \$self->{formatter},
            'r|recurse'    => \$self->{recurse},
            'reverse'      => \$self->{backwards},
            'p|parse'      => \$self->{parse},
            'q|quiet'      => \$self->{quiet},
            'Q|QUIET'      => \$self->{really_quiet},
            'e|exec=s'     => \$self->{exec},
            'm|merge'      => \$self->{merge},
            'I=s@'         => $self->{includes},
            'M=s@'         => $self->{modules},
            'P=s@'         => $self->{plugins},
            'state=s@'     => $self->{state},
            'statefile=s'  => \$self->{statefile},
            'directives'   => \$self->{directives},
            'h|help|?'     => \$self->{show_help},
            'H|man'        => \$self->{show_man},
            'V|version'    => \$self->{show_version},
            'a|archive=s'  => \$self->{archive},
            'j|jobs=i'     => \$self->{jobs},
            'timer'        => \$self->{timer},
            'T'            => \$self->{taint_fail},
            't'            => \$self->{taint_warn},
            'W'            => \$self->{warnings_fail},
            'w'            => \$self->{warnings_warn},
            'normalize'    => \$self->{normalize},
            'rules=s@'     => $self->{rules},
            'tapversion=s' => \$self->{tapversion},
            'trap'         => \$self->{trap},
        ) or croak('Unable to continue');

        # Stash the remainder of argv for later
        $self->{argv} = [@ARGV];
    }

    return;
}

sub _first_pos {
    my $want = shift;
    for ( 0 .. $#_ ) {
        return $_ if $_[$_] eq $want;
    }
    return;
}

sub _help {
    my ( $self, $verbosity ) = @_;

    eval('use Pod::Usage 1.12 ()');
    if ( my $err = $@ ) {
        die 'Please install Pod::Usage for the --help option '
          . '(or try `perldoc prove`.)'
          . "\n ($@)";
    }

    Pod::Usage::pod2usage( { -verbose => $verbosity } );

    return;
}

sub _color_default {
    my $self = shift;

    return -t STDOUT && !$ENV{HARNESS_NOTTY};
}

sub _get_args {
    my $self = shift;

    my %args;

    $args{trap} = 1 if $self->trap;

    if ( defined $self->color ? $self->color : $self->_color_default ) {
        $args{color} = 1;
    }
    if ( !defined $self->show_count ) {
        $args{show_count} = 1;
    }
    else {
        $args{show_count} = $self->show_count;
    }

    if ( $self->archive ) {
        $self->require_harness( archive => 'TAP::Harness::Archive' );
        $args{archive} = $self->archive;
    }

    if ( my $jobs = $self->jobs ) {
        $args{jobs} = $jobs;
    }

    if ( my $harness_opt = $self->harness ) {
        $self->require_harness( harness => $harness_opt );
    }

    if ( my $formatter = $self->formatter ) {
        $args{formatter_class} = $formatter;
    }

    for my $handler ( @{ $self->sources } ) {
        my ( $name, $config ) = $self->_parse_source($handler);
        $args{sources}->{$name} = $config;
    }

    if ( $self->ignore_exit ) {
        $args{ignore_exit} = 1;
    }

    if ( $self->taint_fail && $self->taint_warn ) {
        die '-t and -T are mutually exclusive';
    }

    if ( $self->warnings_fail && $self->warnings_warn ) {
        die '-w and -W are mutually exclusive';
    }

    for my $a (qw( lib switches )) {
        my $method = "_get_$a";
        my $val    = $self->$method();
        $args{$a} = $val if defined $val;
    }

    # Handle verbose, quiet, really_quiet flags
    my %verb_map = ( verbose => 1, quiet => -1, really_quiet => -2, );

    my @verb_adj = map { $self->$_() ? $verb_map{$_} : () }
      keys %verb_map;

    die "Only one of verbose, quiet or really_quiet should be specified\n"
      if @verb_adj > 1;

    $args{verbosity} = shift @verb_adj if @verb_adj;

    for my $a (qw( merge failures comments timer directives normalize )) {
        $args{$a} = 1 if $self->$a();
    }

    $args{errors} = 1 if $self->parse;

    # defined but zero-length exec runs test files as binaries
    $args{exec} = [ split( /\s+/, $self->exec ) ]
      if ( defined( $self->exec ) );

    $args{version} = $self->tapversion if defined( $self->tapversion );

    if ( defined( my $test_args = $self->test_args ) ) {
        $args{test_args} = $test_args;
    }

    if ( @{ $self->rules } ) {
        my @rules;
        for ( @{ $self->rules } ) {
            if (/^par=(.*)/) {
                push @rules, $1;
            }
            elsif (/^seq=(.*)/) {
                push @rules, { seq => $1 };
            }
        }
        $args{rules} = { par => [@rules] };
    }
    $args{harness_class} = $self->{harness_class} if $self->{harness_class};

    return \%args;
}

sub _find_module {
    my ( $self, $class, @search ) = @_;

    croak "Bad module name $class"
      unless $class =~ /^ \w+ (?: :: \w+ ) *$/x;

    for my $pfx (@search) {
        my $name = join( '::', $pfx, $class );
        eval "require $name";
        return $name unless $@;
    }

    eval "require $class";
    return $class unless $@;
    return;
}

sub _load_extension {
    my ( $self, $name, @search ) = @_;

    my @args = ();
    if ( $name =~ /^(.*?)=(.*)/ ) {
        $name = $1;
        @args = split( /,/, $2 );
    }

    if ( my $class = $self->_find_module( $name, @search ) ) {
        $class->import(@args);
        if ( $class->can('load') ) {
            $class->load( { app_prove => $self, args => [@args] } );
        }
    }
    else {
        croak "Can't load module $name";
    }
}

sub _load_extensions {
    my ( $self, $ext, @search ) = @_;
    $self->_load_extension( $_, @search ) for @$ext;
}

sub _parse_source {
    my ( $self, $handler ) = @_;

    # Load any options.
    ( my $opt_name = lc $handler ) =~ s/::/-/g;
    local @ARGV = @{ $self->{argv} };
    my %config;
    Getopt::Long::GetOptions(
        "$opt_name-option=s%" => sub {
            my ( $name, $k, $v ) = @_;
            if ( $v =~ /(?<!\\)=/ ) {

                # It's a hash option.
                croak "Option $name must be consistently used as a hash"
                  if exists $config{$k} && ref $config{$k} ne 'HASH';
                $config{$k} ||= {};
                my ( $hk, $hv ) = split /(?<!\\)=/, $v, 2;
                $config{$k}{$hk} = $hv;
            }
            else {
                $v =~ s/\\=/=/g;
                if ( exists $config{$k} ) {
                    $config{$k} = [ $config{$k} ]
                      unless ref $config{$k} eq 'ARRAY';
                    push @{ $config{$k} } => $v;
                }
                else {
                    $config{$k} = $v;
                }
            }
        }
    );
    $self->{argv} = \@ARGV;
    return ( $handler, \%config );
}

=head3 C<run>

Perform whatever actions the command line args specified. The C<prove>
command line tool consists of the following code:

    use App::Prove;

    my $app = App::Prove->new;
    $app->process_args(@ARGV);
    exit( $app->run ? 0 : 1 );  # if you need the exit code

=cut

sub run {
    my $self = shift;

    unless ( $self->state_manager ) {
        $self->state_manager(
            $self->state_class->new( { store => $self->statefile || STATE_FILE } ) );
    }

    if ( $self->show_help ) {
        $self->_help(1);
    }
    elsif ( $self->show_man ) {
        $self->_help(2);
    }
    elsif ( $self->show_version ) {
        $self->print_version;
    }
    elsif ( $self->dry ) {
        print "$_\n" for $self->_get_tests;
    }
    else {

        $self->_load_extensions( $self->modules );
        $self->_load_extensions( $self->plugins, PLUGINS );

        local $ENV{TEST_VERBOSE} = 1 if $self->verbose;

        return $self->_runtests( $self->_get_args, $self->_get_tests );
    }

    return 1;
}

sub _get_tests {
    my $self = shift;

    my $state = $self->state_manager;
    my $ext   = $self->extensions;
    $state->extensions($ext) if defined $ext;
    if ( defined( my $state_switch = $self->state ) ) {
        $state->apply_switch(@$state_switch);
    }

    my @tests = $state->get_tests( $self->recurse, @{ $self->argv } );

    $self->_shuffle(@tests) if $self->shuffle;
    @tests = reverse @tests if $self->backwards;

    return @tests;
}

sub _runtests {
    my ( $self, $args, @tests ) = @_;
    my $harness = TAP::Harness::Env->create($args);

    my $state = $self->state_manager;

    $harness->callback(
        after_test => sub {
            $state->observe_test(@_);
        }
    );

    $harness->callback(
        after_runtests => sub {
            $state->commit(@_);
        }
    );

    my $aggregator = $harness->runtests(@tests);

    return !$aggregator->has_errors;
}

sub _get_switches {
    my $self = shift;
    my @switches;

    # notes that -T or -t must be at the front of the switches!
    if ( $self->taint_fail ) {
        push @switches, '-T';
    }
    elsif ( $self->taint_warn ) {
        push @switches, '-t';
    }
    if ( $self->warnings_fail ) {
        push @switches, '-W';
    }
    elsif ( $self->warnings_warn ) {
        push @switches, '-w';
    }

    return @switches ? \@switches : ();
}

sub _get_lib {
    my $self = shift;
    my @libs;
    if ( $self->lib ) {
        push @libs, 'lib';
    }
    if ( $self->blib ) {
        push @libs, 'blib/lib', 'blib/arch';
    }
    if ( @{ $self->includes } ) {
        push @libs, @{ $self->includes };
    }

    #24926
    @libs = map { File::Spec->rel2abs($_) } @libs;

    # Huh?
    return @libs ? \@libs : ();
}

sub _shuffle {
    my $self = shift;

    # Fisher-Yates shuffle
    my $i = @_;
    while ($i) {
        my $j = rand $i--;
        @_[ $i, $j ] = @_[ $j, $i ];
    }
    return;
}

=head3 C<require_harness>

Load a harness replacement class.

  $prove->require_harness($for => $class_name);

=cut

sub require_harness {
    my ( $self, $for, $class ) = @_;

    my ($class_name) = $class =~ /^(\w+(?:::\w+)*)/;

    # Emulate Perl's -MModule=arg1,arg2 behaviour
    $class =~ s!^(\w+(?:::\w+)*)=(.*)$!$1 split(/,/,q{$2})!;

    eval("use $class;");
    die "$class_name is required to use the --$for feature: $@" if $@;

    $self->{harness_class} = $class_name;

    return;
}

=head3 C<print_version>

Display the version numbers of the loaded L<TAP::Harness> and the
current Perl.

=cut

sub print_version {
    my $self = shift;
    require TAP::Harness;
    printf(
        "TAP::Harness v%s and Perl v%vd\n",
        $TAP::Harness::VERSION, $^V
    );

    return;
}

1;

# vim:ts=4:sw=4:et:sta

__END__

=head2 Attributes

After command line parsing the following attributes reflect the values
of the corresponding command line switches. They may be altered before
calling C<run>.

=over

=item C<archive>

=item C<argv>

=item C<backwards>

=item C<blib>

=item C<color>

=item C<directives>

=item C<dry>

=item C<exec>

=item C<extensions>

=item C<failures>

=item C<comments>

=item C<formatter>

=item C<harness>

=item C<ignore_exit>

=item C<includes>

=item C<jobs>

=item C<lib>

=item C<merge>

=item C<modules>

=item C<parse>

=item C<plugins>

=item C<quiet>

=item C<really_quiet>

=item C<recurse>

=item C<rules>

=item C<show_count>

=item C<show_help>

=item C<show_man>

=item C<show_version>

=item C<shuffle>

=item C<state>

=item C<state_class>

=item C<taint_fail>

=item C<taint_warn>

=item C<test_args>

=item C<timer>

=item C<verbose>

=item C<warnings_fail>

=item C<warnings_warn>

=item C<tapversion>

=item C<trap>

=back

=head1 PLUGINS

C<App::Prove> provides support for 3rd-party plugins.  These are currently
loaded at run-time, I<after> arguments have been parsed (so you can not
change the way arguments are processed, sorry), typically with the
C<< -PI<plugin> >> switch, eg:

  prove -PMyPlugin

This will search for a module named C<App::Prove::Plugin::MyPlugin>, or failing
that, C<MyPlugin>.  If the plugin can't be found, C<prove> will complain & exit.

You can pass an argument to your plugin by appending an C<=> after the plugin
name, eg C<-PMyPlugin=foo>.  You can pass multiple arguments using commas:

  prove -PMyPlugin=foo,bar,baz

These are passed in to your plugin's C<load()> class method (if it has one),
along with a reference to the C<App::Prove> object that is invoking your plugin:

  sub load {
      my ($class, $p) = @_;

      my @args = @{ $p->{args} };
      # @args will contain ( 'foo', 'bar', 'baz' )
      $p->{app_prove}->do_something;
      ...
  }

Note that the user's arguments are also passed to your plugin's C<import()>
function as a list, eg:

  sub import {
      my ($class, @args) = @_;
      # @args will contain ( 'foo', 'bar', 'baz' )
      ...
  }

This is for backwards compatibility, and may be deprecated in the future.

=head2 Sample Plugin

Here's a sample plugin, for your reference:

  package App::Prove::Plugin::Foo;

  # Sample plugin, try running with:
  # prove -PFoo=bar -r -j3
  # prove -PFoo -Q
  # prove -PFoo=bar,My::Formatter

  use strict;
  use warnings;

  sub load {
      my ($class, $p) = @_;
      my @args = @{ $p->{args} };
      my $app  = $p->{app_prove};

      print "loading plugin: $class, args: ", join(', ', @args ), "\n";

      # turn on verbosity
      $app->verbose( 1 );

      # set the formatter?
      $app->formatter( $args[1] ) if @args > 1;

      # print some of App::Prove's state:
      for my $attr (qw( jobs quiet really_quiet recurse verbose )) {
          my $val = $app->$attr;
          $val    = 'undef' unless defined( $val );
          print "$attr: $val\n";
      }

      return 1;
  }

  1;

=head1 SEE ALSO

L<prove>, L<TAP::Harness>

=cut
