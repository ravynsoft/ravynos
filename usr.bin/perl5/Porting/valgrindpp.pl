#!/usr/bin/perl
use IO::File ();
use File::Find qw(find);
use Text::Wrap qw(wrap);
use Getopt::Long qw(GetOptions);
use Pod::Usage qw(pod2usage);
use Cwd qw(cwd);
use File::Spec;
use strict;

my %opt = (
  frames  => 3,
  lines   => 0,
  tests   => 0,
  top     => 0,
  verbose => 0,
);

GetOptions(\%opt, qw(
            dir=s
            frames=i
            hide=s@
            lines!
            output-file=s
            tests!
            top=i
            verbose+
          )) or pod2usage(2);

# Setup the directory to process
if (exists $opt{dir}) {
  $opt{dir} = File::Spec->canonpath($opt{dir});
}
else {
  # Check if we're in 't'
  $opt{dir} = cwd =~ /\/t$/ ? '..' : '.';

  # Check if we're in the right directory
  -d "$opt{dir}/$_" or die "$0: must be run from the perl source directory"
                         . " when --dir is not given\n"
      for qw(t lib ext);
}

# Assemble regex for functions whose leaks should be hidden
# (no, a hash won't be significantly faster)
my $hidden = do { local $"='|'; $opt{hide} ? qr/^(?:@{$opt{hide}})$/o : '' };

# Setup our output file handle
# (do it early, as it may fail)
my $fh = \*STDOUT;
if (exists $opt{'output-file'}) {
  $fh = new IO::File ">$opt{'output-file'}"
        or die "$0: cannot open $opt{'output-file'} ($!)\n";
}

# These hashes will receive the error and leak summary data:
#
# %error = (
#   error_name => {
#                   stack_frame => {
#                                    test_script => occurrences
#                                  }
#                 }
# );
#
# %leak = (
#   leak_type => {
#                  stack_frames => {
#                                    test_script => occurrences
#                                  }
#                } # stack frames are separated by '<'s
# );
my(%error, %leak);

# Collect summary data
find({wanted => \&filter, no_chdir => 1}, $opt{dir});

# Format the output nicely
$Text::Wrap::columns = 80;
$Text::Wrap::unexpand = 0;

# Write summary
summary($fh, \%error, \%leak);

exit 0;

sub summary {
  my($fh, $error, $leak) = @_;
  my(%ne, %nl, %top);

  # Prepare the data

  for my $e (keys %$error) {
    for my $f (keys %{$error->{$e}}) {
      my($func, $file, $line) = split /:/, $f;
      my $nf = $opt{lines} ? "$func ($file:$line)" : "$func ($file)";
      $ne{$e}{$nf}{count}++;
      while (my($k,$v) = each %{$error->{$e}{$f}}) {
        $ne{$e}{$nf}{tests}{$k} += $v;
        $top{$k}{error}++;
      }
    }
  }

  for my $l (keys %$leak) {
    for my $s (keys %{$leak->{$l}}) {
      my $ns = join '<', map {
                 my($func, $file, $line) = split /:/;
                 /:/ ? $opt{lines}
                       ? "$func ($file:$line)" : "$func ($file)"
                     : $_
               } split /</, $s;
      $nl{$l}{$ns}{count}++;
      while (my($k,$v) = each %{$leak->{$l}{$s}}) {
        $nl{$l}{$ns}{tests}{$k} += $v;
        $top{$k}{leak}++;
      }
    }
  }

  # Print the Top N

  if ($opt{top}) {
    for my $what (qw(error leak)) {
      my @t = sort { $top{$b}{$what} <=> $top{$a}{$what} or $a cmp $b }
              grep $top{$_}{$what}, keys %top;
      @t > $opt{top} and splice @t, $opt{top};
      my $n = @t;
      my $s = $n > 1 ? 's' : '';
      my $prev = 0;
      print $fh "Top $n test scripts for ${what}s:\n\n";
      for my $i (1 .. $n) {
        $n = $top{$t[$i-1]}{$what};
        $s = $n > 1 ? 's' : '';
        printf $fh "    %3s %-40s %3d $what$s\n",
                   $n != $prev ? "$i." : '', $t[$i-1], $n;
        $prev = $n;
      }
      print $fh "\n";
    }
  }

  # Print the real summary

  print $fh "MEMORY ACCESS ERRORS\n\n";

  for my $e (sort keys %ne) {
    print $fh qq("$e"\n);
    for my $frame (sort keys %{$ne{$e}}) {
      my $data = $ne{$e}{$frame};
      my $count = $data->{count} > 1 ? " [$data->{count} paths]" : '';
      print $fh ' 'x4, "$frame$count\n",
                format_tests($data->{tests}), "\n";
    }
    print $fh "\n";
  }

  print $fh "\nMEMORY LEAKS\n\n";
 
  for my $l (sort keys %nl) {
    print $fh qq("$l"\n);
    for my $frames (sort keys %{$nl{$l}}) {
      my $data = $nl{$l}{$frames};
      my @stack = split /</, $frames;
      $data->{count} > 1 and $stack[-1] .= " [$data->{count} paths]";
      print $fh join('', map { ' 'x4 . "$_:$stack[$_]\n" } 0 .. $#stack ),
                format_tests($data->{tests}), "\n\n";
    }
  }
}

sub format_tests {
  my $tests = shift;
  my $indent = ' 'x8;

  if ($opt{tests}) {
    return wrap($indent, $indent, join ', ', sort keys %$tests);
  }
  else {
    my $count = keys %$tests;
    my $s = $count > 1 ? 's' : '';
    return $indent . "triggered by $count test$s";
  }
}

sub filter {
  debug(2, "$File::Find::name\n");

  # Only process '*.t.valgrind' files
  /(.*)\.t\.valgrind$/ or return;

  # Strip all unnecessary stuff from the test name
  my $test = $1;
  $test =~ s/^(?:(?:\Q$opt{dir}\E|[.t])\/)+//;

  debug(1, "processing $test ($_)\n");

  # Get all the valgrind output lines
  my @l = do {
    my $fh = new IO::File $_ or die "$0: cannot open $_ ($!)\n";
    # Process outputs can interrupt each other, so sort by pid first
    my %pid; local $_;
    while (<$fh>) {
      chomp;
      s/^==(\d+)==\s?// and push @{$pid{$1}}, $_;
    }
    map @$_, values %pid;
  };

  # Setup some useful regexes
  my $hexaddr  = '0x[[:xdigit:]]+';
  my $topframe = qr/^\s+at $hexaddr:\s+/;
  my $address  = qr/^\s+Address $hexaddr is \d+ bytes (?:before|inside|after) a block of size \d+/;
  my $leak     = qr/^\s*\d+ bytes in \d+ blocks are (still reachable|(?:definite|possib)ly lost)/;

  for my $i (0 .. $#l) {
    $l[$i]   =~ $topframe or next; # Match on any topmost frame...
    $l[$i-1] =~ $address and next; # ...but not if it's only address details
    my $line = $l[$i-1]; # The error / leak description line
    my $j    = $i;

    if ($line =~ $leak) {
      debug(2, "LEAK: $line\n");

      my $type   = $1;     # Type of leak (still reachable, ...)
      my $inperl = 0;      # Are we inside the perl source? (And how deep?)
      my @stack;           # Call stack

      while ($l[$j++] =~ /^\s+(?:at|by) $hexaddr:\s+(\w+)\s+\((?:([^:]+):(\d+)|[^)]+)\)/o) {
        my($func, $file, $lineno) = ($1, $2, $3);

        # If the stack frame is inside perl => increment $inperl
        # If we've already been inside perl, but are no longer => leave
        defined $file && ++$inperl or $inperl && last;

        # A function that should be hidden? => clear stack and leave
        $hidden && $func =~ $hidden and @stack = (), last;

        # Add stack frame if it's within our threshold
        if ($inperl <= $opt{frames}) {
          push @stack, $inperl ? "$func:$file:$lineno" : $func;
        }
      }

      # If there's something on the stack and we've seen perl code,
      # add this memory leak to the summary data
      @stack and $inperl and $leak{$type}{join '<', @stack}{$test}++;
    } else {
      debug(1, "ERROR: $line\n");

      # Simply find the topmost frame in the call stack within
      # the perl source code
      while ($l[$j++] =~ /^\s+(?:at|by) $hexaddr:\s+(?:(\w+)\s+\(([^:]+):(\d+)\))?/o) {
        if (defined $1) {
          $error{$line}{"$1:$2:$3"}{$test}++;
          last;
        }
      }
    }
  }
}

sub debug {
  my $level = shift;
  $opt{verbose} >= $level and print STDERR @_;
}

__END__

=head1 NAME

valgrindpp.pl - A post processor for make test.valgrind

=head1 SYNOPSIS

valgrindpp.pl [B<--dir>=I<dir>] [B<--frames>=I<number>]
[B<--hide>=I<identifier>] [B<--lines>]
[B<--output-file>=I<file>] [B<--tests>] 
[B<--top>=I<number>] [B<--verbose>]

=head1 DESCRIPTION

B<valgrindpp.pl> is a post processor for I<.valgrind> files
created during I<make test.valgrind>. It collects all these
files, extracts most of the information and produces a
significantly shorter summary of all detected memory access
errors and memory leaks.

=head1 OPTIONS

=over 4

=item B<--dir>=I<dir>

Recursively process I<.valgrind> files in I<dir>. If this
options is not given, B<valgrindpp.pl> must be run from
either the perl source or the I<t> directory and will process
all I<.valgrind> files within the distribution.

=item B<--frames>=I<number>

Number of stack frames within the perl source code to 
consider when distinguishing between memory leak sources.
Increasing this value will give you a longer backtrace,
while decreasing the number will show you fewer sources
for memory leaks. The default is 3 frames.

=item B<--hide>=I<identifier>

Hide all memory leaks that have I<identifier> in their backtrace.
Useful if you want to hide leaks from functions that are known to
have lots of memory leaks. I<identifier> can also be a regular
expression, in which case all leaks with symbols matching the
expression are hidden. Can be given multiple times.

=item B<--lines>

Show line numbers for stack frames. This is useful for further
increasing the error/leak resolution, but makes it harder to
compare different reports using I<diff>.

=item B<--output-file>=I<file>

Redirect the output into I<file>. If this option is not
given, the output goes to I<stdout>.

=item B<--tests>

List all tests that trigger memory access errors or memory
leaks explicitly instead of only printing a count.

=item B<--top>=I<number>

List the top I<number> test scripts for memory access errors
and memory leaks. Set to C<0> for no top-I<n> statistics.

=item B<--verbose>

Increase verbosity level. Can be given multiple times.

=back

=head1 COPYRIGHT

Copyright 2003 by Marcus Holland-Moritz <mhx@cpan.org>.

This program is free software; you may redistribute it
and/or modify it under the same terms as Perl itself.

=cut
