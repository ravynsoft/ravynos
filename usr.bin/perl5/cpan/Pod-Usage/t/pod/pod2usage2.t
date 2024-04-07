#!/usr/bin/perl -w

use strict;
use warnings;
use Test::More;
use File::Basename qw(fileparse);
use File::Spec;

BEGIN {
  if ($^O eq 'MSWin32' || $^O eq 'VMS') {
    plan skip_all => "Not portable on Win32 or VMS\n";
  }
  else {
    plan tests => 42;
  }
  use_ok ("Pod::Usage");
}

sub getoutput
{
  my ($code) = @_;
  my $pid = open(my $in, "-|");
  die "Cannot fork: $!" unless defined $pid;
  if ($pid) {
    # parent
    my @out = <$in>;
    close($in);

    my $exit = $?>>8;
    s/^/#/ for @out;

    local $" = "";

    print "#EXIT=$exit OUTPUT=+++#@out#+++\n";
    waitpid( $pid, 1 );

    return ($exit, join("", @out) );
  }
  # child
  open (STDERR, ">&STDOUT");

  Test::More->builder->no_ending(1);
  local $SIG{ALRM} = sub { die "Alarm reached" };
  alarm(600);

  # this could hang
  $code->();
  print "--NORMAL-RETURN--\n";
  exit 0;
}

sub compare
{
  my ($left,$right) = @_;
  $left  =~ s/^#\s+/#/gm;
  $right =~ s/^#\s+/#/gm;
  $left  =~ s/\s+/ /gm;
  $right =~ s/\s+/ /gm;
  $left eq $right;
}

SKIP: {
if('Pod::Usage'->isa('Pod::Text') && $Pod::Text::VERSION < 2.18) {
  skip("Formatting with Pod::Text $Pod::Text::VERSION not reliable", 33);
}

my ($exit, $text) = getoutput( sub { pod2usage() } );
is ($exit, 2,                 "Exit status pod2usage ()");
ok (compare ($text, <<'EOT'), "Output test pod2usage ()");
#Usage:
#    frobnicate [ -r | --recursive ] [ -f | --force ] file ...
#
EOT

($exit, $text) = getoutput( sub { pod2usage(
  -message => 'You naughty person, what did you say?',
  -verbose => 1 ) });
is ($exit, 1,                 "Exit status pod2usage (-message => '...', -verbose => 1)");
ok (compare ($text, <<'EOT'), "Output test pod2usage (-message => '...', -verbose => 1)") or diag("Got:\n$text\n");
#You naughty person, what did you say?
# Usage:
#     frobnicate [ -r | --recursive ] [ -f | --force ] file ...
#
# Options:
#     -r | --recursive
#         Run recursively.
#
#     -f | --force
#         Just do it!
#
#     -n number
#         Specify number of frobs, default is 42.
#
EOT

($exit, $text) = getoutput( sub { pod2usage(
  -verbose => 2, -exit => 42 ) } );
is ($exit, 42,                "Exit status pod2usage (-verbose => 2, -exit => 42)");
ok (compare ($text, <<'EOT'), "Output test pod2usage (-verbose => 2, -exit => 42)");
#NAME
#     frobnicate - do what I mean
#
# SYNOPSIS
#     frobnicate [ -r | --recursive ] [ -f | --force ] file ...
#
# DESCRIPTION
#     frobnicate does foo and bar and what not.
#
# OPTIONS
#     -r | --recursive
#         Run recursively.
#
#     -f | --force
#         Just do it!
#
#     -n number
#         Specify number of frobs, default is 42.
#
EOT

($exit, $text) = getoutput( sub { pod2usage(0) } );
is ($exit, 0,                 "Exit status pod2usage (0)");
ok (compare ($text, <<'EOT'), "Output test pod2usage (0)");
#Usage:
#     frobnicate [ -r | --recursive ] [ -f | --force ] file ...
#
# Options:
#     -r | --recursive
#         Run recursively.
#
#     -f | --force
#         Just do it!
#
#     -n number
#         Specify number of frobs, default is 42.
#
EOT

($exit, $text) = getoutput( sub { pod2usage(42) } );
is ($exit, 42,                "Exit status pod2usage (42)");
ok (compare ($text, <<'EOT'), "Output test pod2usage (42)");
#Usage:
#     frobnicate [ -r | --recursive ] [ -f | --force ] file ...
#
EOT

($exit, $text) = getoutput( sub { pod2usage(-verbose => 0, -exit => 'NOEXIT') } );
is ($exit, 0,                 "Exit status pod2usage (-verbose => 0, -exit => 'NOEXIT')");
ok (compare ($text, <<'EOT'), "Output test pod2usage (-verbose => 0, -exit => 'NOEXIT')");
#Usage:
#     frobnicate [ -r | --recursive ] [ -f | --force ] file ...
#
# --NORMAL-RETURN--
EOT

($exit, $text) = getoutput( sub { pod2usage(-verbose => 99, -sections => 'DESCRIPTION') } );
is ($exit, 1,                 "Exit status pod2usage (-verbose => 99, -sections => 'DESCRIPTION')");
ok (compare ($text, <<'EOT'), "Output test pod2usage (-verbose => 99, -sections => 'DESCRIPTION')");
#Description:
#     frobnicate does foo and bar and what not.
#
EOT

# does the __DATA__ work ok as input
my (@blib, $test_script, $pod_file1, , $pod_file2);
if (!$ENV{PERL_CORE}) {
  @blib = '-Mblib';
}
$test_script = File::Spec->catfile(qw(t pod p2u_data.pl));
$pod_file1 = File::Spec->catfile(qw(t pod usage.pod));
$pod_file2 = File::Spec->catfile(qw(t pod usage2.pod));


($exit, $text) = getoutput( sub { system($^X, @blib, $test_script); exit($?  >> 8); } );
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 17,                 "Exit status pod2usage (-verbose => 2, -input => \*DATA)");
ok (compare ($text, <<'EOT'), "Output test pod2usage (-verbose => 2, -input => \*DATA)") or diag "Got:\n$text\n";
#NAME
#    Test
#
#SYNOPSIS
#    perl podusagetest.pl
#
#DESCRIPTION
#    This is a test.
#
EOT

# test that SYNOPSIS and USAGE are printed
($exit, $text) = getoutput( sub { pod2usage(-input => $pod_file1,
                                            -exitval => 0, -verbose => 0); });
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with USAGE");
ok (compare ($text, <<'EOT'), "Output test pod2usage with USAGE") or diag "Got:\n$text\n";
#Usage:
#    This is a test for CPAN#33020
#
#Usage:
#    And this will be also printed.
#
EOT

# test that SYNOPSIS and USAGE are printed with options
($exit, $text) = getoutput( sub { pod2usage(-input => $pod_file1,
                                            -exitval => 0, -verbose => 1); });
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with USAGE and verbose=1");
ok (compare ($text, <<'EOT'), "Output test pod2usage with USAGE and verbose=1") or diag "Got:\n$text\n";
#Usage:
#    This is a test for CPAN#33020
#
#Usage:
#    And this will be also printed.
#
#Options:
#    And this with verbose == 1
#
EOT

# test that only USAGE is printed when requested
($exit, $text) = getoutput( sub { pod2usage(-input => $pod_file1,
                                            -exitval => 0, -verbose => 99, -sections => 'USAGE'); });
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with USAGE and verbose=99");
ok (compare ($text, <<'EOT'), "Output test pod2usage with USAGE and verbose=99") or diag "Got:\n$text\n";
#Usage:
#    This is a test for CPAN#33020
#
EOT

# test with self

my $src = File::Spec->catfile(qw(lib Pod Usage.pm));
($exit, $text) = getoutput( sub { pod2usage( -input => $src,
                                             -exitval => 0, -verbose => 0) } );
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with self");
ok (compare ($text, <<'EOT'), "Output test pod2usage with self") or diag "Got:\n$text\n";
#Usage:
#      use Pod::Usage;
#
#      my $message_text  = "This text precedes the usage message.";
#      my $exit_status   = 2;          ## The exit status to use
#      my $verbose_level = 0;          ## The verbose level to use
#      my $filehandle    = \*STDERR;   ## The filehandle to write to
#
#      pod2usage($message_text);
#
#      pod2usage($exit_status);
#
#      pod2usage( { -message => $message_text ,
#                   -exitval => $exit_status  ,
#                   -verbose => $verbose_level,
#                   -output  => $filehandle } );
#
#      pod2usage(   -msg     => $message_text ,
#                   -exitval => $exit_status  ,
#                   -verbose => $verbose_level,
#                   -output  => $filehandle   );
#
#      pod2usage(   -verbose => 2,
#                   -noperldoc => 1  );
#
#      pod2usage(   -verbose => 2,
#                   -perlcmd => $path_to_perl,
#                   -perldoc => $path_to_perldoc,
#                   -perldocopt => $perldoc_options );
#
EOT

# verify that sections are correctly found after nested headings
($exit, $text) = getoutput( sub { pod2usage(-input => $pod_file2,
                                            -exitval => 0, -verbose => 99,
                                            -sections => [qw(BugHeader BugHeader/.*')]) });
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with nested headings");
ok (compare ($text, <<'EOT'), "Output test pod2usage with nested headings") or diag "Got:\n$text\n";
#BugHeader:
#    Some text
#
#  BugHeader2:
#    More
#    Still More
#
EOT

# Verify that =over =back work OK
($exit, $text) = getoutput( sub {
  pod2usage(-input => $pod_file2,
            -exitval => 0, -verbose => 99, -sections => 'BugHeader/BugHeader2') } );
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with over/back");
ok (compare ($text, <<'EOT'), "Output test pod2usage with over/back") or diag "Got:\n$text\n";
#  BugHeader2:
#    More
#    Still More
#
EOT

# new array API for -sections
($exit, $text) = getoutput( sub {
  pod2usage(-input => $pod_file2,
            -exitval => 0, -verbose => 99, -sections => [qw(Heading-1/!.+ Heading-2/.+)]) } );
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
is ($exit, 0,                 "Exit status pod2usage with -sections => []");
ok (compare ($text, <<'EOT'), "Output test pod2usage with -sections => []") or diag "Got:\n$text\n";
#Heading-1:
#    One
#    Two
#
#  Heading-2.2:
#    More text.
#
EOT

# allow subheadings in OPTIONS and ARGUMENTS
($exit, $text) = getoutput( sub {
  pod2usage(-input => $pod_file2,
            -exitval => 0, -verbose => 1) } );
$text =~ s{#Using.*/blib.*\n}{}; # older blib's emit something to STDERR
$text =~ s{[*](destination|files)[*]}{$1}g; # strip * chars
is ($exit, 0,                 "Exit status pod2usage with subheadings in OPTIONS");
ok (compare ($text, <<'EOT'), "Output test pod2usage with subheadings in OPTIONS") or diag "Got:\n$text\n";
#Options and Arguments:
#  Arguments:
#    The required arguments (which typically follow any options on the
#    command line) are:
#
#    destination
#    files
#
#  Options:
#    Options may be abbreviated. Options which take values may be separated
#    from the values by whitespace or the "=" character.
#
EOT

# test various use cases of calling pod2usage to increase coverage
($exit, $text) = getoutput( sub {
  pod2usage({ -input => $pod_file2,
            -exitval => 3, -verbose => 0 }) } );
is ($exit, 3,                 "Exit status pod2usage with hash options");
like ($text, qr/^\s*$/s, "Output test pod2usage with hash options is empty") or diag "Got:\n$text\n";

# call with single string option
($exit, $text) = getoutput( sub {
  pod2usage('Just print this') } );
is ($exit, 2,                 "Exit status pod2usage with single string option");
like ($text, qr/^#Just print this/, "Output test pod2usage with single string options has first line") or diag "Got:\n$text\n";

# call with search path and relative file name
my ($file, $dir) = fileparse($0);
($exit, $text) = getoutput( sub {
  pod2usage({ -input => $file, -pathlist => [ $dir ], -exit => 0, -verbose => 2 } ) } );
is ($exit, 0,                 "Exit status pod2usage with relative path");
like ($text, qr/frobnicate - do what I mean/, "Output test pod2usage with relative path works OK") or diag "Got:\n$text\n";

# trigger specific perldoc case
# ...and one coverage line
{ no warnings;
  *Pod::Usage::initialize = sub { 1; };
}

our $TODO;
SKIP: {
    my $perldoc = $^X . 'doc';
    skip "Missing perldoc binary", 2 unless -x $perldoc;

    my $out = qx[$perldoc 2>&1] || '';
    skip "Need perl-doc package", 2 if $out =~ qr[You need to install the perl-doc package to use this program];

    ($exit, $text) = getoutput( sub {
        require Pod::Perldoc;
      my $devnull = File::Spec->devnull();
      open(SAVE_STDOUT, '>&', \*STDOUT);
      open(STDOUT, '>', $devnull);
      pod2usage({ -verbose => 2, -input => $0, -output => \*STDOUT, -exit => 0, -message => 'Special perldoc case', -perldocopt => '-i' });
      open(STDOUT, '>&', \*SAVE_STDOUT);
      } );
    is ($exit, 0,                 "Exit status pod2usage with special perldoc case");
    # output went to devnull
    TODO: {
        local $TODO = q[Can get output from stty view #14];
        is( length($text), 0, "Output test pod2usage with special perldoc case") or diag "Got:\n$text\n";
    }
}

# bad regexp syntax
($exit, $text) = getoutput( sub { pod2usage( -verbose => 99, -sections => 'DESCRIPTION{BLAH') } );
like ($text, qr/Bad regular expression/, "Output test pod2usage with bad section regexp");

} # end SKIP

__END__

=head1 NAME

frobnicate - do what I mean

=head1 SYNOPSIS

B<frobnicate> S<[ B<-r> | B<--recursive> ]> S<[ B<-f> | B<--force> ]>
  file ...

=head1 DESCRIPTION

B<frobnicate> does foo and bar and what not.

=head1 OPTIONS

=over 4

=item B<-r> | B<--recursive>

Run recursively.

=item B<-f> | B<--force>

Just do it!

=item B<-n> number

Specify number of frobs, default is 42.

=back

=cut

