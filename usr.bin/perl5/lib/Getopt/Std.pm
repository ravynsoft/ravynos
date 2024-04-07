package Getopt::Std;

use strict;
use warnings;

require Exporter;

=head1 NAME

Getopt::Std - Process single-character switches with switch clustering

=head1 SYNOPSIS

    use Getopt::Std;

    getopts('oif:');  # -o & -i are boolean flags, -f takes an argument
		      # Sets $opt_* as a side effect.
    getopts('oif:', \%opts);  # options as above. Values in %opts
    getopt('oDI');    # -o, -D & -I take arg.
                      # Sets $opt_* as a side effect.
    getopt('oDI', \%opts);    # -o, -D & -I take arg.  Values in %opts

=head1 DESCRIPTION

The C<getopts()> function processes single-character switches with switch
clustering.  Pass one argument which is a string containing all switches to be
recognized.  For each switch found, if an argument is expected and provided,
C<getopts()> sets C<$opt_x> (where C<x> is the switch name) to the value of
the argument.  If an argument is expected but none is provided, C<$opt_x> is
set to an undefined value.  If a switch does not take an argument, C<$opt_x>
is set to C<1>.

Switches which take an argument don't care whether there is a space between
the switch and the argument.  If unspecified switches are found on the
command-line, the user will be warned that an unknown option was given.

The C<getopts()> function returns true unless an invalid option was found.

The C<getopt()> function is similar, but its argument is a string containing
all switches that take an argument.  If no argument is provided for a switch,
say, C<y>, the corresponding C<$opt_y> will be set to an undefined value.
Unspecified switches are silently accepted.  Use of C<getopt()> is not
recommended.

Note that, if your code is running under the recommended C<use strict
vars> pragma, you will need to declare these package variables
with C<our>:

    our($opt_x, $opt_y);

For those of you who don't like additional global variables being created,
C<getopt()> and C<getopts()> will also accept a hash reference as an optional
second argument.  Hash keys will be C<x> (where C<x> is the switch name) with
key values the value of the argument or C<1> if no argument is specified.

To allow programs to process arguments that look like switches, but aren't,
both functions will stop processing switches when they see the argument
C<-->.  The C<--> will be removed from @ARGV.

=head1 C<--help> and C<--version>

If C<-> is not a recognized switch letter, getopts() supports arguments
C<--help> and C<--version>.  If C<main::HELP_MESSAGE()> and/or
C<main::VERSION_MESSAGE()> are defined, they are called; the arguments are
the output file handle, the name of option-processing package, its version,
and the switches string.  If the subroutines are not defined, an attempt is
made to generate intelligent messages; for best results, define $main::VERSION.

If embedded documentation (in pod format, see L<perlpod>) is detected
in the script, C<--help> will also show how to access the documentation.

Note that due to excessive paranoia, if $Getopt::Std::STANDARD_HELP_VERSION
isn't true (the default is false), then the messages are printed on STDERR,
and the processing continues after the messages are printed.  This being
the opposite of the standard-conforming behaviour, it is strongly recommended
to set $Getopt::Std::STANDARD_HELP_VERSION to true.

One can change the output file handle of the messages by setting
$Getopt::Std::OUTPUT_HELP_VERSION.  One can print the messages of C<--help>
(without the C<Usage:> line) and C<--version> by calling functions help_mess()
and version_mess() with the switches string as an argument.

=cut

our @ISA = qw(Exporter);
our @EXPORT = qw(getopt getopts);
our $VERSION = '1.13';
# uncomment the next line to disable 1.03-backward compatibility paranoia
# $STANDARD_HELP_VERSION = 1;

# Process single-character switches with switch clustering.  Pass one argument
# which is a string containing all switches that take an argument.  For each
# switch found, sets $opt_x (where x is the switch name) to the value of the
# argument, or 1 if no argument.  Switches which take an argument don't care
# whether there is a space between the switch and the argument.

# Usage:
#	getopt('oDI');  # -o, -D & -I take arg.  Sets opt_* as a side effect.

sub getopt (;$$) {
    my ($argumentative, $hash) = @_;
    $argumentative = '' if !defined $argumentative;
    my ($first,$rest);
    local $_;
    local @EXPORT;

    while (@ARGV && ($_ = $ARGV[0]) =~ /^-(.)(.*)/) {
	($first,$rest) = ($1,$2);
	if (/^--$/) {	# early exit if --
	    shift @ARGV;
	    last;
	}
	if (index($argumentative,$first) >= 0) {
	    if ($rest ne '') {
		shift(@ARGV);
	    }
	    else {
		shift(@ARGV);
		$rest = shift(@ARGV);
	    }
	    if (ref $hash) {
	        $$hash{$first} = $rest;
	    }
	    else {
            no strict 'refs';
	        ${"opt_$first"} = $rest;
	        push( @EXPORT, "\$opt_$first" );
	    }
	}
	else {
	    if (ref $hash) {
	        $$hash{$first} = 1;
	    }
	    else {
            no strict 'refs';
	        ${"opt_$first"} = 1;
	        push( @EXPORT, "\$opt_$first" );
	    }
	    if ($rest ne '') {
		$ARGV[0] = "-$rest";
	    }
	    else {
		shift(@ARGV);
	    }
	}
    }
    unless (ref $hash) { 
	local $Exporter::ExportLevel = 1;
	import Getopt::Std;
    }
}

our ($OUTPUT_HELP_VERSION, $STANDARD_HELP_VERSION);
sub output_h () {
  return $OUTPUT_HELP_VERSION if defined $OUTPUT_HELP_VERSION;
  return \*STDOUT if $STANDARD_HELP_VERSION;
  return \*STDERR;
}

sub try_exit () {
    exit 0 if $STANDARD_HELP_VERSION;
    my $p = __PACKAGE__;
    print {output_h()} <<EOM;
  [Now continuing due to backward compatibility and excessive paranoia.
   See 'perldoc $p' about \$$p\::STANDARD_HELP_VERSION.]
EOM
}

sub version_mess ($;$) {
    my $args = shift;
    my $h = output_h;
    if (@_ and defined &main::VERSION_MESSAGE) {
	main::VERSION_MESSAGE($h, __PACKAGE__, $VERSION, $args);
    } else {
	my $v = $main::VERSION;
	$v = '[unknown]' unless defined $v;
	my $myv = $VERSION;
	$myv .= ' [paranoid]' unless $STANDARD_HELP_VERSION;
	my $perlv = $];
	$perlv = sprintf "%vd", $^V if $] >= 5.006;
	print $h <<EOH;
$0 version $v calling Getopt::Std::getopts (version $myv),
running under Perl version $perlv.
EOH
    }
}

sub help_mess ($;$) {
    my $args = shift;
    my $h = output_h;
    if (@_ and defined &main::HELP_MESSAGE) {
	main::HELP_MESSAGE($h, __PACKAGE__, $VERSION, $args);
    } else {
	my (@witharg) = ($args =~ /(\S)\s*:/g);
	my (@rest) = ($args =~ /([^\s:])(?!\s*:)/g);
	my ($help, $arg) = ('', '');
	if (@witharg) {
	    $help .= "\n\tWith arguments: -" . join " -", @witharg;
	    $arg = "\nSpace is not required between options and their arguments.";
	}
	if (@rest) {
	    $help .= "\n\tBoolean (without arguments): -" . join " -", @rest;
	}
	my ($scr) = ($0 =~ m,([^/\\]+)$,);
	print $h <<EOH if @_;			# Let the script override this

Usage: $scr [-OPTIONS [-MORE_OPTIONS]] [--] [PROGRAM_ARG1 ...]
EOH
	print $h <<EOH;

The following single-character options are accepted:$help

Options may be merged together.  -- stops processing of options.$arg
EOH
	my $has_pod;
	if ( defined $0 and $0 ne '-e' and -f $0 and -r $0
	     and open my $script, '<', $0 ) {
	    while (<$script>) {
		$has_pod = 1, last if /^=(pod|head1)/;
	    }
	}
	print $h <<EOH if $has_pod;

For more details run
	perldoc -F $0
EOH
    }
}

# Usage:
#   getopts('a:bc');	# -a takes arg. -b & -c not. Sets opt_* as a
#			#  side effect.

sub getopts ($;$) {
    my ($argumentative, $hash) = @_;
    my (@args,$first,$rest,$exit);
    my $errs = 0;
    local $_;
    local @EXPORT;

    @args = split( / */, $argumentative );
    while(@ARGV && ($_ = $ARGV[0]) =~ /^-(.)(.*)/s) {
	($first,$rest) = ($1,$2);
	if (/^--$/) {	# early exit if --
	    shift @ARGV;
	    last;
	}
	my $pos = index($argumentative,$first);
	if ($pos >= 0) {
	    if (defined($args[$pos+1]) and ($args[$pos+1] eq ':')) {
		shift(@ARGV);
		if ($rest eq '') {
		    ++$errs unless @ARGV;
		    $rest = shift(@ARGV);
		}
		if (ref $hash) {
		    $$hash{$first} = $rest;
		}
		else {
            no strict 'refs';
		    ${"opt_$first"} = $rest;
		    push( @EXPORT, "\$opt_$first" );
		}
	    }
	    else {
		if (ref $hash) {
		    $$hash{$first} = 1;
		}
		else {
            no strict 'refs';
		    ${"opt_$first"} = 1;
		    push( @EXPORT, "\$opt_$first" );
		}
		if ($rest eq '') {
		    shift(@ARGV);
		}
		else {
		    $ARGV[0] = "-$rest";
		}
	    }
	}
	else {
	    if ($first eq '-' and $rest eq 'help') {
		version_mess($argumentative, 'main');
		help_mess($argumentative, 'main');
		try_exit();
		shift(@ARGV);
		next;
	    } elsif ($first eq '-' and $rest eq 'version') {
		version_mess($argumentative, 'main');
		try_exit();
		shift(@ARGV);
		next;
	    }
	    warn "Unknown option: $first\n";
	    ++$errs;
	    if ($rest ne '') {
		$ARGV[0] = "-$rest";
	    }
	    else {
		shift(@ARGV);
	    }
	}
    }
    unless (ref $hash) { 
	local $Exporter::ExportLevel = 1;
	import Getopt::Std;
    }
    $errs == 0;
}

1;
