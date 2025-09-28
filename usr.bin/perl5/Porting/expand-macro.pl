#!perl -w
use strict;

use Pod::Usage;
use Getopt::Std;
use Config;
$Getopt::Std::STANDARD_HELP_VERSION = 1;

my $trysource = "try.c";
my $tryout = "try.i";

getopts('fF:ekvI:X', \my %opt) or pod2usage();

my($expr, @headers) = @ARGV ? splice @ARGV : "-";

pod2usage "-f and -F <tool> are exclusive\n" if $opt{f} and $opt{F};

foreach($trysource, $tryout) {
    unlink $_ if $opt{e};
    die "You already have a $_" if -e $_;
}

if ($expr eq '-') {
    warn "reading from stdin...\n";
    $expr = do { local $/; <> };
}

my($macro, $args) = $expr =~ /^\s*(\w+)((?:\s*\(.*\))?)\s*;?\s*$/s
    or pod2usage "$expr doesn't look like a macro-name or macro-expression to me";

if (!(@ARGV = @headers)) {
    open my $fh, '<', 'MANIFEST' or die "Can't open MANIFEST: $!";
    while (<$fh>) {
	push @ARGV, $1 if m!^([^/]+\.h)\t!;
    }
    push @ARGV, 'config.h' if -f 'config.h';
}

my $header;
while (<>) {
    next unless /^#\s*define\s+$macro\b/;
    my ($def_args) = /^#\s*define\s+$macro\(([^)]*)\)/;
    if (defined $def_args && !$args) {
	my @args = split ',', $def_args;
	print "# macro: $macro args: @args in $_\n" if $opt{v};
	my $argname = "A0";
	$args = '(' . join (', ', map {$argname++} 1..@args) . ')';
    }
    $header = $ARGV;
    last;
}
die "$macro not found\n" unless defined $header;

if ($^O =~ /MSWin(32|64)/) {
    # The Win32 (and Win64) build process expects to be run from
    # bleadperl/Win32
    chdir "Win32"
	or die "Couldn't chdir to win32: $!";
};

open my $out, '>', $trysource or die "Can't open $trysource: $!";

my $sentinel = "$macro expands to";

# These two are included from perl.h, and perl.h sometimes redefines their
# macros. So no need to include them.
my %done_header = ('embed.h' => 1, 'embedvar.h' => 1);

sub do_header {
    my $header = shift;
    return if $done_header{$header}++;
    print $out qq{#include "$header"\n};
}

print $out <<'EOF' if $opt{X};
/* Need to do this like this, as cflags.sh sets it for us come what may.  */
#undef PERL_CORE

EOF

do_header('EXTERN.h');
do_header('perl.h');
do_header($header);
do_header('XSUB.h') if $opt{X};

print $out <<"EOF";
#line 4 "$sentinel"
$macro$args
EOF

close $out or die "Can't close $trysource: $!";

print "doing: $Config{make} $tryout\n" if $opt{v};
my $cmd = "$Config{make} $tryout";
system( $cmd ) == 0
    or die "Couldn't launch [$cmd]: $! / $?";

# if user wants 'indent' formatting ..
my $out_fh;

if ($opt{f} || $opt{F}) {
    # a: indent is a well behaved filter when given 0 arguments, reading from
    #    stdin and writing to stdout
    # b: all our braces should be balanced, indented back to column 0, in the
    #    headers, hence everything before our #line directive can be ignored
    #
    # We can take advantage of this to reduce the work to indent.

    my $indent_command = $opt{f} ? 'indent' : $opt{F};

    if (defined $opt{I}) {
	$indent_command .= " $opt{I}";
    }
    open $out_fh, '|-', $indent_command or die $?;
} else {
    $out_fh = \*STDOUT;
}

{
    open my $fh, '<', $tryout or die "Can't open $tryout: $!";

    while (<$fh>) {
	print $out_fh $_ if /$sentinel/o .. 1;
    }
};

unless ($opt{k}) {
    foreach($trysource, $tryout) {
	die "Can't unlink $_: $!" unless unlink $_;
    }
}

__END__

=head1 NAME

expand-macro.pl - expand C macros using the C preprocessor

=head1 SYNOPSIS

  expand-macro.pl [options]
                  [ < macro-name | macro-expression | - > [headers] ]

  options:
    -f		use 'indent' to format output
    -F	<tool>	use <tool> to format output  (instead of -f)
    -e		erase try.[ic] instead of failing when they're present
                (errdetect)
    -k		keep them after generating (for handy inspection)
    -v		verbose
    -I <indent-opts>	passed into indent
    -X		include "XSUB.h" (and undefine PERL_CORE)

=cut
