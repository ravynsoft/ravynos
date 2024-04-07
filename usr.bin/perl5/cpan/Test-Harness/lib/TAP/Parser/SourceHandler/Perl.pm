package TAP::Parser::SourceHandler::Perl;

use strict;
use warnings;
use Config;

use constant IS_WIN32 => ( $^O =~ /^(MS)?Win32$/ );
use constant IS_VMS => ( $^O eq 'VMS' );

use TAP::Parser::IteratorFactory           ();
use TAP::Parser::Iterator::Process         ();
use Text::ParseWords qw(shellwords);

use base 'TAP::Parser::SourceHandler::Executable';

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

=head1 NAME

TAP::Parser::SourceHandler::Perl - Stream TAP from a Perl executable

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Source;
  use TAP::Parser::SourceHandler::Perl;

  my $source = TAP::Parser::Source->new->raw( \'script.pl' );
  $source->assemble_meta;

  my $class = 'TAP::Parser::SourceHandler::Perl';
  my $vote  = $class->can_handle( $source );
  my $iter  = $class->make_iterator( $source );

=head1 DESCRIPTION

This is a I<Perl> L<TAP::Parser::SourceHandler> - it has 2 jobs:

1. Figure out if the L<TAP::Parser::Source> it's given is actually a Perl
script (L</can_handle>).

2. Creates an iterator for Perl sources (L</make_iterator>).

Unless you're writing a plugin or subclassing L<TAP::Parser>, you probably
won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<can_handle>

  my $vote = $class->can_handle( $source );

Only votes if $source looks like a file.  Casts the following votes:

  0.9  if it has a shebang ala "#!...perl"
  0.3  if it has any shebang
  0.8  if it's a .t file
  0.9  if it's a .pl file
  0.75 if it's in a 't' directory
  0.25 by default (backwards compat)

=cut

sub can_handle {
    my ( $class, $source ) = @_;
    my $meta = $source->meta;

    return 0 unless $meta->{is_file};
    my $file = $meta->{file};

    if ( my $shebang = $file->{shebang} ) {
        return 0.9 if $shebang =~ /^#!.*\bperl/;

        # We favour Perl as the interpreter for any shebang to preserve
        # previous semantics: we used to execute everything via Perl and
        # relied on it to pass the shebang off to the appropriate
        # interpreter.
        return 0.3;
    }

    return 0.8 if $file->{lc_ext} eq '.t';    # vote higher than Executable
    return 0.9 if $file->{lc_ext} eq '.pl';

    return 0.75 if $file->{dir} =~ /^t\b/;    # vote higher than Executable

    # backwards compat, always vote:
    return 0.25;
}

=head3 C<make_iterator>

  my $iterator = $class->make_iterator( $source );

Constructs & returns a new L<TAP::Parser::Iterator::Process> for the source.
Assumes C<$source-E<gt>raw> contains a reference to the perl script.  C<croak>s
if the file could not be found.

The command to run is built as follows:

  $perl @switches $perl_script @test_args

The perl command to use is determined by L</get_perl>.  The command generated
is guaranteed to preserve:

  PERL5LIB
  PERL5OPT
  Taint Mode, if set in the script's shebang

I<Note:> the command generated will I<not> respect any shebang line defined in
your Perl script.  This is only a problem if you have compiled a custom version
of Perl or if you want to use a specific version of Perl for one test and a
different version for another, for example:

  #!/path/to/a/custom_perl --some --args
  #!/usr/local/perl-5.6/bin/perl -w

Currently you need to write a plugin to get around this.

=cut

sub _autoflush_stdhandles {
    my ($class) = @_;

    $class->_autoflush( \*STDOUT );
    $class->_autoflush( \*STDERR );
}

sub make_iterator {
    my ( $class, $source ) = @_;
    my $meta        = $source->meta;
    my $perl_script = ${ $source->raw };

    $class->_croak("Cannot find ($perl_script)") unless $meta->{is_file};

    # TODO: does this really need to be done here?
    $class->_autoflush_stdhandles;

    my ( $libs, $switches )
      = $class->_mangle_switches(
        $class->_filter_libs( $class->_switches($source) ) );

    $class->_run( $source, $libs, $switches );
}


sub _has_taint_switch {
    my( $class, $switches ) = @_;

    my $has_taint = grep { $_ eq "-T" || $_ eq "-t" } @{$switches};
    return $has_taint ? 1 : 0;
}

sub _mangle_switches {
    my ( $class, $libs, $switches ) = @_;

    # Taint mode ignores environment variables so we must retranslate
    # PERL5LIB as -I switches and place PERL5OPT on the command line
    # in order that it be seen.
    if ( $class->_has_taint_switch($switches) ) {
        my @perl5lib = defined $ENV{PERL5LIB} ? split /$Config{path_sep}/, $ENV{PERL5LIB} : ();
        return (
            $libs,
            [   @{$switches},
                $class->_libs2switches([@$libs, @perl5lib]),
                defined $ENV{PERL5OPT} ? shellwords( $ENV{PERL5OPT} ) : ()
            ],
        );
    }

    return ( $libs, $switches );
}

sub _filter_libs {
    my ( $class, @switches ) = @_;

    my $path_sep = $Config{path_sep};
    my $path_re  = qr{$path_sep};

    # Filter out any -I switches to be handled as libs later.
    #
    # Nasty kludge. It might be nicer if we got the libs separately
    # although at least this way we find any -I switches that were
    # supplied other then as explicit libs.
    #
    # We filter out any names containing colons because they will break
    # PERL5LIB
    my @libs;
    my @filtered_switches;
    for (@switches) {
        if ( !/$path_re/ && m/ ^ ['"]? -I ['"]? (.*?) ['"]? $ /x ) {
            push @libs, $1;
        }
        else {
            push @filtered_switches, $_;
        }
    }

    return \@libs, \@filtered_switches;
}

sub _iterator_hooks {
    my ( $class, $source, $libs, $switches ) = @_;

    my $setup = sub {
        if ( @{$libs} and !$class->_has_taint_switch($switches) ) {
            $ENV{PERL5LIB} = join(
                $Config{path_sep}, grep {defined} @{$libs},
                $ENV{PERL5LIB}
            );
        }
    };

    # VMS environment variables aren't guaranteed to reset at the end of
    # the process, so we need to put PERL5LIB back.
    my $previous = $ENV{PERL5LIB};
    my $teardown = sub {
        if ( defined $previous ) {
            $ENV{PERL5LIB} = $previous;
        }
        else {
            delete $ENV{PERL5LIB};
        }
    };

    return ( $setup, $teardown );
}

sub _run {
    my ( $class, $source, $libs, $switches ) = @_;

    my @command = $class->_get_command_for_switches( $source, $switches )
      or $class->_croak("No command found!");

    my ( $setup, $teardown ) = $class->_iterator_hooks( $source, $libs, $switches );

    return $class->_create_iterator( $source, \@command, $setup, $teardown );
}

sub _create_iterator {
    my ( $class, $source, $command, $setup, $teardown ) = @_;

    return TAP::Parser::Iterator::Process->new(
        {   command  => $command,
            merge    => $source->merge,
            setup    => $setup,
            teardown => $teardown,
        }
    );
}

sub _get_command_for_switches {
    my ( $class, $source, $switches ) = @_;
    my $file    = ${ $source->raw };
    my @args    = @{ $source->test_args || [] };
    my $command = $class->get_perl;

   # XXX don't need to quote if we treat the parts as atoms (except maybe vms)
   #$file = qq["$file"] if ( $file =~ /\s/ ) && ( $file !~ /^".*"$/ );
    my @command = ( $command, @{$switches}, $file, @args );
    return @command;
}

sub _libs2switches {
    my $class = shift;
    return map {"-I$_"} grep {$_} @{ $_[0] };
}

=head3 C<get_taint>

Decode any taint switches from a Perl shebang line.

  # $taint will be 't'
  my $taint = TAP::Parser::SourceHandler::Perl->get_taint( '#!/usr/bin/perl -t' );

  # $untaint will be undefined
  my $untaint = TAP::Parser::SourceHandler::Perl->get_taint( '#!/usr/bin/perl' );

=cut

sub get_taint {
    my ( $class, $shebang ) = @_;
    return
      unless defined $shebang
          && $shebang =~ /^#!.*\bperl.*\s-\w*([Tt]+)/;
    return $1;
}

sub _switches {
    my ( $class, $source ) = @_;
    my $file     = ${ $source->raw };
    my @switches = @{ $source->switches || [] };
    my $shebang  = $source->meta->{file}->{shebang};
    return unless defined $shebang;

    my $taint = $class->get_taint($shebang);
    push @switches, "-$taint" if defined $taint;

    # Quote the argument if we're VMS, since VMS will downcase anything
    # not quoted.
    if (IS_VMS) {
        for (@switches) {
            $_ = qq["$_"];
        }
    }

    return @switches;
}

=head3 C<get_perl>

Gets the version of Perl currently running the test suite.

=cut

sub get_perl {
    my $class = shift;
    return $ENV{HARNESS_PERL} if defined $ENV{HARNESS_PERL};
    return qq["$^X"] if IS_WIN32 && ( $^X =~ /[^\w\.\/\\]/ );
    return $^X;
}

1;

__END__

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

=head2 Example

  package MyPerlSourceHandler;

  use strict;

  use TAP::Parser::SourceHandler::Perl;

  use base 'TAP::Parser::SourceHandler::Perl';

  # use the version of perl from the shebang line in the test file
  sub get_perl {
      my $self = shift;
      if (my $shebang = $self->shebang( $self->{file} )) {
          $shebang =~ /^#!(.*\bperl.*?)(?:(?:\s)|(?:$))/;
	  return $1 if $1;
      }
      return $self->SUPER::get_perl(@_);
  }

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::IteratorFactory>,
L<TAP::Parser::SourceHandler>,
L<TAP::Parser::SourceHandler::Executable>,
L<TAP::Parser::SourceHandler::File>,
L<TAP::Parser::SourceHandler::Handle>,
L<TAP::Parser::SourceHandler::RawTAP>

=cut
