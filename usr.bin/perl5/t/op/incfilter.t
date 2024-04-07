#!./perl -w

# Tests for the source filters in coderef-in-@INC

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(. ../lib) );
    skip_all_if_miniperl(
	'no dynamic loading on miniperl, no Filter::Util::Call'
    );
}

skip_all_without_perlio();

use strict;
use Config;
use Filter::Util::Call;

plan(tests => 153);

unshift @INC, sub {
    no warnings 'uninitialized';
    ref $_[1] eq 'ARRAY' ? @{$_[1]} : $_[1];
};

my $fh;

open $fh, "<", \'pass("Can return file handles from \@INC");';
do $fh or die;

my @origlines = ("# This is a blank line\n",
		 "pass('Can return generators from \@INC');\n",
		 "pass('Which return multiple lines');\n",
		 "1",
		 );
my @lines = @origlines;
sub generator {
    $_ = shift @lines;
    # Return of 0 marks EOF
    return defined $_ ? 1 : 0;
};

do \&generator or die;

@lines = @origlines;
# Check that the array dereferencing works ready for the more complex tests:
do [\&generator] or die;

sub generator_with_state {
    my $param = $_[1];
    is (ref $param, 'ARRAY', "Got our parameter");
    $_ = shift @$param;
    return defined $_ ? 1 : 0;
}

do [\&generator_with_state,
    ["pass('Can return generators which take state');\n",
     "pass('And return multiple lines');\n",
    ]] or die;
   

open $fh, "<", \'fail("File handles and filters work from \@INC");';

do [$fh, sub {s/fail/pass/; return;}] or die;

open $fh, "<", \'fail("File handles and filters with state work from \@INC");';

do [$fh, sub {s/$_[1]/pass/; return;}, 'fail'] or die;

print "# 2 tests with pipes from subprocesses.\n";

my ($echo_command, $pass_arg, $fail_arg);

if ($^O eq 'VMS') {
    $echo_command = 'write sys$output';
    $pass_arg = '"pass"';
    $fail_arg = '"fail"';
}
else {
    if ($^O =~ /android/) {
        $echo_command = q{sh -c 'echo $@' -- };
    }
    else {
        $echo_command = 'echo';
    }
    $pass_arg = 'pass';
    $fail_arg = 'fail';
}

open $fh, "$echo_command $pass_arg|" or die $!;

do $fh or die;

open $fh, "$echo_command $fail_arg|" or die $!;

do [$fh, sub {s/$_[1]/pass/; return;}, 'fail'] or die;

sub rot13_filter {
    filter_add(sub {
		   my $status = filter_read();
		   tr/A-Za-z/N-ZA-Mn-za-m/;
		   $status;
	       })
}

open $fh, "<", \<<'EOC';
BEGIN {rot13_filter};
cnff("This will rot13'ed prepend");
EOC

do $fh or die;

open $fh, "<", \<<'EOC';
ORTVA {ebg13_svygre};
pass("This will rot13'ed twice");
EOC

do [$fh, sub {tr/A-Za-z/N-ZA-Mn-za-m/; return;}] or die;

my $count = 32;
sub prepend_rot13_filter {
    filter_add(sub {
		   my $previous = $_;
		   # Filters should append to any existing data in $_
		   # But (logically) shouldn't filter it twice.
		   my $test = "fzrt!";
		   $_ = $test;
		   my $status = filter_read();
		   my $got = substr $_, 0, length $test, '';
		   is $got, $test, "Upstream didn't alter existing data";
		   tr/A-Za-z/N-ZA-Mn-za-m/;
		   $_ = $previous . $_;
		   die "Looping infinitely" unless $count--;
		   $status;
	       })
}

open $fh, "<", \<<'EOC';
ORTVA {cercraq_ebg13_svygre};
pass("This will rot13'ed twice");
EOC

do [$fh, sub {tr/A-Za-z/N-ZA-Mn-za-m/; return;}] or die;

# This generates a heck of a lot of oks, but I think it's necessary.
my $amount = 1;
sub prepend_block_counting_filter {
    filter_add(sub {
		   my $output = $_;
		   my $count = 256;
		   while (--$count) {
		       $_ = '';
		       my $status = filter_read($amount);
		       cmp_ok (length $_, '<=', $amount, "block mode works?");
		       $output .= $_;
		       if ($status <= 0 or /\n/s) {
			   $_ = $output;
			   return $status;
		       }
		   }
		   die "Looping infinitely";
			  
	       })
}

open $fh, "<", \<<'EOC';
BEGIN {prepend_block_counting_filter};
pass("one by one");
pass("and again");
EOC

do [$fh, sub {return;}] or die;

open $fh, "<", \<<'EOC';
BEGIN {prepend_block_counting_filter};
pas("SSS make s fast SSS");
EOC

do [$fh, sub {s/s/ss/gs; s/([\nS])/$1$1$1/gs; return;}] or die;

sub prepend_line_counting_filter {
    filter_add(sub {
		   my $output = $_;
		   $_ = '';
		   my $status = filter_read();
		   my $newlines = tr/\n//;
		   cmp_ok ($newlines, '<=', 1, "1 line at most?");
		   $_ = $output . $_ if defined $output;
		   return $status;
	       })
}

open $fh, "<", \<<'EOC';
BEGIN {prepend_line_counting_filter};
pass("You should see this line thrice");
EOC

do [$fh, sub {$_ .= $_ . $_; return;}] or die;

do \"pass\n(\n'Scalar references are treated as initial file contents'\n)\n"
or die;

use constant scalarreffee =>
  "pass\n(\n'Scalar references are treated as initial file contents'\n)\n";
do \scalarreffee or die;
is scalarreffee,
  "pass\n(\n'Scalar references are treated as initial file contents'\n)\n",
  'and are not gobbled up when read-only';

{
    local $SIG{__WARN__} = sub {}; # ignore deprecation warning from ?...?
    do qr/a?, 1/;
    pass "No crash (perhaps) when regexp ref is returned from inc filter";
    # Even if that outputs "ok", it may not have passed, as the crash
    # occurs during globular destruction.  But the crash will result in
    # this script failing.
}

open $fh, "<", \"ss('The file is concatenated');";

do [\'pa', $fh] or die;

open $fh, "<", \"ff('Gur svygre vf bayl eha ba gur svyr');";

do [\'pa', $fh, sub {tr/A-Za-z/N-ZA-Mn-za-m/; return;}] or die;

open $fh, "<", \"SS('State also works');";

do [\'pa', $fh, sub {s/($_[1])/lc $1/ge; return;}, "S"] or die;

@lines = ('ss', '(', "'you can use a generator'", ')');

do [\'pa', \&generator] or die;

do [\'pa', \&generator_with_state,
    ["ss('And generators which take state');\n",
     "pass('And return multiple lines');\n",
    ]] or die;

@origlines = keys %{{ "1\n+\n2\n" => 1 }};
@lines = @origlines;
do \&generator or die;
is $origlines[0], "1\n+\n2\n", 'ink filters do not mangle cow buffers';

@lines = ('$::the_array = "', [], '"');
do \&generator or die;
like ${$::{the_array}}, qr/^ARRAY\(0x.*\)\z/,
   'setting $_ to ref in inc filter';
@lines = ('$::the_array = "', do { no warnings 'once'; *foo}, '"');
do \&generator or die;
is ${$::{the_array}}, "*main::foo", 'setting $_ to glob in inc filter';
@lines = (
    '$::the_array = "',
     do { no strict; no warnings; *{"foo\nbar"}},
    '"');
do \&generator or die;
is ${$::{the_array}}, "*main::foo\nbar",
    'setting $_ to multiline glob in inc filter';

sub TIESCALAR { bless \(my $thing = pop), shift }
sub FETCH {${$_[0]}}
my $done;
do sub {
    return 0 if $done;
    tie $_, "main", '$::the_scalar = 98732';
    return $done = 1;
} or die;
is ${$::{the_scalar}}, 98732, 'tying $_ in inc filter';
@lines = ('$::the_scalar', '= "12345"');
tie my $ret, "main", 1;
do sub :lvalue {
    return 0 unless @lines;
    $_ = shift @lines;
    return $ret;
} or die;
is ${$::{the_scalar}}, 12345, 'returning tied val from inc filter';


# d8723a6a74b2c12e wasn't perfect, as the char * returned by SvPV*() can be
# a temporary, freed at the next FREETMPS. And there is a FREETMPS in
# pp_require

for (0 .. 1) {
    # Need both alternatives on the regexp, because currently the logic in
    # pp_require for what is written to %INC is somewhat confused
    open $fh, "<",
	\'like(__FILE__, qr/(?:GLOB|CODE)\(0x[0-9a-f]+\)/, "__FILE__ is valid");';
    do $fh or die;
}

# [perl #91880] $_ having the wrong refcount inside a
{ #             filter sub
    local @INC; local $|;
    unshift @INC, sub { sub { undef *_; --$| }};
    do "dah";
    pass '$_ has the right refcount inside a filter sub';
}
